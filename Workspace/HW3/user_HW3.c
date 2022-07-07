/******************************************************************************
MSP430G2553 Project Creator

SE 423  - Dan Block
        Spring(2019)

        Written(by) : Steve(Keres)
College of Engineering Control Systems Lab
University of Illinois at Urbana-Champaign
*******************************************************************************/

#include "msp430g2553.h"
#include "UART.h"

void print_every(int rate);

char newprint = 0;
long NumOn = 0;
long NumOff = 0;
int statevar = 1;
int timecheck = 0;
int timecnt = 0;
int sendData = 0;
int adc_value = 0;
int rampCount = 0;
char MSB = 0;
char LSB = 0;
char flag = 0;
char flagDac = 0;

void main(void) {

	WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT

	if (CALBC1_16MHZ ==0xFF || CALDCO_16MHZ == 0xFF) while(1);

	DCOCTL = CALDCO_16MHZ;    // Set uC to run at approximately 16 Mhz
	BCSCTL1 = CALBC1_16MHZ; 

	// Initialize Port 1
	P1SEL &= ~0x40;                  // P1.6 GPIO
	P1SEL2 &= ~0x40;                 // P1.6 GPIO
	P1REN &= 0;                      // all pin Resistor disabled
	P1DIR |= 0x40;                   // Set P1.6 to output direction
	P1OUT &= ~0x40;                  // Initially set P1.6 to 1

	// Port 1 SPI pins
	P1SEL |= 0xA0;        //Set up P1.7 & P1.5 as second use
	P1SEL2 |= 0xA0;

	UCB0CTL0 = UCCKPH + UCCKPL + UCMSB + UCMST + UCSYNC;   //clock inilization select
    UCB0CTL1 = UCSSEL_2 + UCSWRST;
    UCB0BR0 = 8;
    UCB0BR1 = 0;
    UCB0CTL1 &= ~UCSWRST;
    IFG2 &= ~UCB0RXIE;
    IE2 |= UCB0RXIE;

	// Timer A Config
	TACCTL0 = CCIE;       		// Enable Periodic interrupt
	TACCR0 = 16000;                // period = 1ms   
	TACTL = TASSEL_2 + MC_1; // source SMCLK, up mode

    ADC10CTL0 = ADC10SHT_2 + ADC10ON + ADC10IE; // ADC10ON, interrupt enabled
    ADC10CTL1 = INCH_3;                       // input A3
    ADC10AE0 |= 0x01;                         // P1.0 ADC option select
    ADC10AE0 = 0x08;  // Enable A3 ADC channel

	Init_UART(115200,1);	// Initialize UART for 115200 baud serial communication

	_BIS_SR(GIE); 		// Enable global interrupt


    while(1) {  // Low priority Slow computation items go inside this while loop.  Very few (if anyt) items in the HWs will go inside this while loop

// for use if you want to use a method of receiving a string of chars over the UART see USCI0RX_ISR below
//      if(newmsg) {
//          newmsg = 0;
//      }

        // The newprint variable is set to 1 inside the function "print_every(rate)" at the given rate
        if ( (newprint == 1) && (senddone == 1) )  { // senddone is set to 1 after UART transmission is complete

            // only one UART_printf can be called every 15ms
            UART_printf("%d\n\r", adc_value);						//print adc_value

            newprint = 0;
        }

    }
}

void write_DAC(int digitalValue)
{

    int transmitData = 0;

    transmitData = digitalValue;
    transmitData = transmitData >> 6;     // Get D8 - D11
    MSB = transmitData;    // Format the MSB 8 bit
    MSB |= 0X40;                          // Set D14 as fast mode

    transmitData = 0;                     // Clear transmitData

    transmitData = digitalValue;
    transmitData = transmitData & 0x003F;  // Get D2 - D7 => 6 bits
    transmitData = transmitData << 2;
    LSB = transmitData;     // Format the LSB 8 bit

    P1OUT |= 0x40;     // Set P1.6 high then low to control FS of TLV5606
    P1OUT &= ~0x40;


    UCB0TXBUF = MSB;   // Transmit the MSB 8 bit, TX interrupt will be called here
    flagDac = 1;       // ready to send LSB



}

// Timer A0 interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
    //timecheck++; // Keep track of time for main while loop.
    print_every(100);  // units determined by the rate Timer_A ISR is called, print every "rate" calls to this function

    timecnt++;
    rampCount++;


    ADC10CTL0 |= ENC + ADC10SC;

    if (timecnt == 1000){							//print each secons
        timecnt = 0;
        newprint = 1;
    }


    // Increase sendData by 10 every 0.1 seconds
//    if (rampCount == 1)							//generate input signal
//    {                                             //a ramp per 1ms, add 1mV
//                                                  //if reach 1023, set to 0
//        if (flag == 0)
//        {
//            sendData = sendData + 1;
//            if (sendData < 1023)
//            {
//                write_DAC(sendData);
//            }
//            else
//            {
//                flag = 1;
//            }
//        }
//        else
//        {
//            sendData = 0;
//            write_DAC(sendData);
//            flag = 0;
//        }
//
//
//        rampCount = 0;
//
//    }


}



// ADC 10 ISR - Called when a sequence of conversions (A7-A0) have completed
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void) {
    adc_value = (int)ADC10MEM;   // save conversion value to adc_value
    write_DAC(adc_value);		 //send adc_value to DAC
    newprint = 1;
}



// USCI Transmit ISR - Called when TXBUF is empty (ready to accept another character)
#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR(void) {

    if(IFG2&UCA0TXIFG) {		// USCI_A0 requested TX interrupt
        if(printf_flag) {
            if (currentindex == txcount) {
                senddone = 1;
                printf_flag = 0;
                IFG2 &= ~UCA0TXIFG;
            } else {
                UCA0TXBUF = printbuff[currentindex];
                currentindex++;
            }
        } else if(UART_flag) {
            if(!donesending) {
                UCA0TXBUF = txbuff[txindex];
                if(txbuff[txindex] == 255) {
                    donesending = 1;
                    txindex = 0;
                }
                else txindex++;
            }
        } else {  // interrupt after sendchar call so just set senddone flag since only one char is sent
            senddone = 1;
        }

        IFG2 &= ~UCA0TXIFG;
    }

	if(IFG2&UCB0TXIFG) {	// USCI_B0 requested TX interrupt (UCB0TXBUF is empty)

		IFG2 &= ~UCB0TXIFG;   // clear IFG
	}
}


// USCI Receive ISR - Called when shift register has been transferred to RXBUF
// Indicates completion of TX/RX operation
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void) {

	if(IFG2&UCB0RXIFG) {  // USCI_B0 requested RX interrupt (UCB0RXBUF is full)

		IFG2 &= ~UCB0RXIFG;   // clear IFG

		if(flagDac == 1)
		{
		    UCB0TXBUF = LSB;      // Send LSB data
		    flagDac = 0;
		}
	}

	if(IFG2&UCA0RXIFG) {  // USCI_A0 requested RX interrupt (UCA0RXBUF is full)

//    Uncomment this block of code if you would like to use this COM protocol that uses 253 as STARTCHAR and 255 as STOPCHAR
/*		if(!started) {	// Haven't started a message yet
			if(UCA0RXBUF == 253) {
				started = 1;
				newmsg = 0;
			}
		}
		else {	// In process of receiving a message		
			if((UCA0RXBUF != 255) && (msgindex < (MAX_NUM_FLOATS*5))) {
				rxbuff[msgindex] = UCA0RXBUF;

				msgindex++;
			} else {	// Stop char received or too much data received
				if(UCA0RXBUF == 255) {	// Message completed
					newmsg = 1;
					rxbuff[msgindex] = 255;	// "Null"-terminate the array
				}
				started = 0;
				msgindex = 0;
			}
		}
*/


		IFG2 &= ~UCA0RXIFG;
	}

}

// This function takes care of all the timing for printing to UART
// Rate determined by how often the function is called in Timer ISR
int print_timecheck = 0;
void print_every(int rate) {
    if (rate < 15) {
        rate = 15;
    }
    if (rate > 10000) {
        rate = 10000;
    }
    print_timecheck++;
    if (print_timecheck == rate) {
        print_timecheck = 0;
        newprint = 1;
    }

}

