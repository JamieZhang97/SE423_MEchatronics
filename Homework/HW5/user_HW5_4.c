#include "msp430g2553.h"
#include "UART.h"

char newprint = 0;
long NumOn = 0;
long NumOff = 0;
int statevar = 1;
int timecheck = 0;
int print_timecheck = 0;

unsigned int ADC[7];  // Array to hold ADC values
int A0value = 0;
int A3value = 0;
int A6value = 0;


void print_every(int rate) {  //determine the frequence of being called in ISR
    if (rate < 15) {     //set a print rate limit
        rate = 15;
    }
    if (rate > 10000) {
        rate = 10000;
    }
    print_timecheck++;
    if (print_timecheck == rate) {  //if time is off, chjange the flag of ready to print
        print_timecheck = 0;
        newprint = 1;
    }

}


void main(void) {

	WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT

	if (CALBC1_16MHZ ==0xFF || CALDCO_16MHZ == 0xFF) while(1);

	DCOCTL = CALDCO_16MHZ;    // Set uC to run at approximately 16 Mhz
	BCSCTL1 = CALBC1_16MHZ; 

	// Initialize Port 1
	P1SEL &= ~0x01;  // See page 42 and 43 of the G2553's datasheet, It shows that when both P1SEL and P1SEL2 bits are zero   
	P1SEL2 &= ~0x01; // the corresponding pin is set as a I/O pin.  Datasheet: http://coecsl.ece.illinois.edu/ge423/datasheets/MSP430Ref_Guides/msp430g2553datasheet.pdf  
	P1REN = 0x0;  // No resistors enabled for Port 1
	P1DIR |= 0x10; // Set P1.4 to output to drive LED on LaunchPad board.  Make sure shunt jumper is in place at LaunchPad's Red LED
	P1OUT &= ~0x10;  // Initially set P1.4 to 0

	P1SEL &= ~0x49;  // See page 42 and 43 of the G2553's datasheet, It shows that when both P1SEL and P1SEL2 bits are zero
    P1SEL2 &= ~0x49; // the corresponding pin is set as a I/O pin.  Datasheet: http://coecsl.ece.illinois.edu/ge423/datasheets/MSP430Ref_Guides/msp430g2553datasheet.pdf
    P1REN = 0x0;  // No resistors enabled for Port 1
    P1DIR &= ~0x49; // Set P1.0, P1.3 and P1.6 as intput

	// Initializing ADC10
    ADC10CTL0 = ADC10ON + MSC + ADC10IE;  // Turn on ADC,  Multiple Sample and Conversion mode, Enable Interrupt
    ADC10CTL1 = INCH_6 + ADC10SSEL_3 + CONSEQ_1; //INCH_6: Enable A6 first, Use SMCLK, Sequence of Channels

    ADC10AE0 |= 0x49;                   // Enable A0, A3 and A6 

    ADC10DTC1 = 7;                 // Seven conversions.
    ADC10SA = (short)&ADC[0];           // ADC10 data transfer starting address.

	// Timer A Config
	TACCTL0 = CCIE;       		// Enable Periodic interrupt
	TACCR0 = 16000;                // period = 1ms   
	TACTL = TASSEL_2 + MC_1; // source SMCLK, up mode


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
            UART_printf("A0:%d A3:%d A6:%d\n\r",A0value,A3value,A6value);

            newprint = 0;
        }

    }
}


// Timer A0 interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
    timecheck++; // Keep track of time for main while loop.
    print_every(250);  // units determined by the rate Timer_A ISR is called, print every "rate" calls to this function
    ADC10CTL0 |= ENC + ADC10SC;         // Enable Sampling and start conversion.
    if (timecheck == 250){
        P1OUT ^= 0x10;  // P1.4 send message
        timecheck = 0;
    }
}



// ADC 10 ISR - Called when a sequence of conversions (A7-A0) have completed
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void) {
    // Notice the reverse in index
	A0value = ADC[6];  // ADC[6] has A0 value
    A3value = ADC[3];  // ADC[3] has A3 value 
    A6value = ADC[0];  // ADC[0] has A6 value

    ADC10CTL0 &= ~ADC10IFG;  // clear interrupt flag

    ADC10SA = (short)&ADC[0]; // ADC10 data transfer starting address.
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




