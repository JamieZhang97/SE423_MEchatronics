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

int ADC_value;
int rampCount = 0;
char flagDAC = 0;


void main(void) {

    WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT

    // Initializing ADC10
    ADC10CTL0 = SREF_0 + ADC10SHT_1 + ADC10ON + ADC10IE; // Vr+ = Vcc, Vr- = Vss, 8 x ADC10CLKS, ADC10 on, ADC Interrupt enable
    ADC10CTL1 = INCH_3 + SHS_0 + ADC10DIV_0 + ADC10SSEL_0 + CONSEQ_0; // P3: Input channel A3, ADC10SC, Clock divide /1, Clock source = ADC10OSC, Single-channel-single-conversion
    //ADC10CTL1 = INCH_0 + SHS_0 + ADC10DIV_3 + ADC10SSEL_0 + CONSEQ_0; // Clock divide /4
    //ADC10CTL1 = INCH_0 + SHS_0 + ADC10DIV_7 + ADC10SSEL_0 + CONSEQ_0; // Clock divide /8

    ADC10AE0 = 0x08;  // P3: Enable A3 ADC channel

    ADC10CTL0 |= ENC; // ADC10 enabled

    if (CALBC1_16MHZ ==0xFF || CALDCO_16MHZ == 0xFF) while(1);

    DCOCTL = CALDCO_16MHZ;    // Set uC to run at approximately 16 Mhz
    BCSCTL1 = CALBC1_16MHZ;

    // Initialize Port 1
    P1SEL &= ~0x40; //?
    P1SEL2 &= ~0x40;
    P1REN = 0x0;  // No resistors enabled for Port 1
    P1DIR |= 0x40;   // Set P1.6 as a Digital Output to control FS
    P1OUT &= ~0x40;  // Initially set P1.6 to 0

    // Port 1 SPI pins
    P1SEL |= 0xA0;   // Secondary Peripheral Module Function for P1.5 P1.6 P1.7
    P1SEL2 |= 0xA0;

    // Initialize SPI
    UCB0CTL0 = UCCKPH + UCCKPL + UCMSB + UCMST + UCSYNC; // 3-pin, 8-bit SPI master ?
    UCB0CTL1 = UCSSEL_2 + UCSWRST; // SMCLK, enable SW Reset
    UCB0BR0 = 80; //?
    UCB0BR1 = 0;
    UCB0CTL1 &= ~UCSWRST; // Initialize USC1 state machine
    IFG2 &= ~UCB0RXIE; // Clear RX interrupt flag in case it was set during init
    IE2 |= UCB0RXIE; // Enable USCI0 RX interrupt


    // Timer A Config
    TACCTL0 = CCIE;             // Enable Periodic interrupt
    TACCR0 = 16000;                // period = 1ms
    TACTL = TASSEL_2 + MC_1; // source SMCLK, up mode


    Init_UART(115200,1);    // Initialize UART for 115200 baud serial communication

    _BIS_SR(GIE);       // Enable global interrupt


    while(1) {  // Low priority Slow computation items go inside this while loop.  Very few (if anyt) items in the HWs will go inside this while loop

// for use if you want to use a method of receiving a string of chars over the UART see USCI0RX_ISR below
//      if(newmsg) {
//          newmsg = 0;
//      }

        // The newprint variable is set to 1 inside the function "print_every(rate)" at the given rate
        if ( (newprint == 1) && (senddone == 1) )  { // senddone is set to 1 after UART transmission is complete

            // UART_printf is called every 0.25s
            UART_printf("A3: %d \n\r", ADC_value);
            newprint = 0;
        }

    }
}
unsigned char MSB = 0;
unsigned char LSB = 0;
void write_DAC(int value){

    // TLV5606 10-bit data is from D2 - D11
    int Data = 0;
    Data = value;
    Data = Data >> 6;  // Get D8 - D11
    MSB = (unsigned char)Data;
    MSB |= 0X40;                          // Set D14 as fast mode

    Data = 0;                     // Clear Data

    Data = value;
    Data = Data & 0x003F;  // Get D2 - D7 => 6 bits
    Data = Data << 2;
    LSB = (unsigned char)Data;

    P1OUT |= 0x40;     // Set P1.6 high then low to start the transmit
    P1OUT &= ~0x40;


    UCB0TXBUF = MSB;   // Transmit the MSB 8 bit
    flagDAC = 1;       // ready to send LSB

}

// Timer A0 interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
    ADC10CTL0 |= ENC + ADC10SC; // Trigger ADC10 every 1  millisecond
}

// ADC 10 ISR - Called when a sequence of conversions (A7-A0) have completed
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void) {
    timecheck++; // Keep track of time for main while loop.
    rampCount++;
    print_every(250);
    if (rampCount < 1024){
        //write_DAC(rampCount);
    }
    else{
        rampCount = 0;
    }
    ADC_value = (int)ADC10MEM; // save conversion value to adc_value
    write_DAC(ADC_value);
}



// USCI Transmit ISR - Called when TXBUF is empty (ready to accept another character)
#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR(void) {

    if(IFG2&UCA0TXIFG) {        // USCI_A0 requested TX interrupt
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

    if(IFG2&UCB0TXIFG) {    // USCI_B0 requested TX interrupt (UCB0TXBUF is empty)

        IFG2 &= ~UCB0TXIFG;   // clear IFG
    }
}


// USCI Receive ISR - Called when shift register has been transferred to RXBUF
// Indicates completion of TX/RX operation
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void) {

    if(IFG2&UCB0RXIFG) {  // USCI_B0 requested RX interrupt (UCB0RXBUF is full)

        IFG2 &= ~UCB0RXIFG;   // clear IFG

        if(flagDAC == 1)
        {
            UCB0TXBUF = LSB;      // Send LSB data
            flagDAC = 0;
        }

    }

    if(IFG2&UCA0RXIFG) {  // USCI_A0 requested RX interrupt (UCA0RXBUF is full)

//    Uncomment this block of code if you would like to use this COM protocol that uses 253 as STARTCHAR and 255 as STOPCHAR
/*      if(!started) {  // Haven't started a message yet
            if(UCA0RXBUF == 253) {
                started = 1;
                newmsg = 0;
            }
        }
        else {  // In process of receiving a message
            if((UCA0RXBUF != 255) && (msgindex < (MAX_NUM_FLOATS*5))) {
                rxbuff[msgindex] = UCA0RXBUF;

                msgindex++;
            } else {    // Stop char received or too much data received
                if(UCA0RXBUF == 255) {  // Message completed
                    newmsg = 1;
                    rxbuff[msgindex] = 255; // "Null"-terminate the array
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

