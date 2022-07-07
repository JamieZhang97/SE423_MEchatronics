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
int adc_value = 0;
char received_val = 0;
int count = 0;

void main(void) {

    WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT

    if (CALBC1_16MHZ ==0xFF || CALDCO_16MHZ == 0xFF) while(1);

    DCOCTL = CALDCO_16MHZ;    // Set uC to run at approximately 16 Mhz
    BCSCTL1 = CALBC1_16MHZ;

    // Initializing ADC10
    ADC10CTL0 |= ENC; // ADC10 enabled
    ADC10CTL0 = ADC10SHT_2 + ADC10ON + ADC10IE; // ADC10ON, interrupt enabled
    ADC10CTL1 = INCH_1;                       // input A1
    ADC10AE0 |= 0x01;                         // P1.0 ADC option select

//    SREFx = 0x0;
//    ADC10SHTx = 0x1;
//    ADC10SR = 0;
//    REFOUT = 0;
//    REFBURST = 0;
//    MSC = 0;
//    REFON = 0;
//    ADC10IE = 1;
//    INCHx = 0x0;
//    ADC10SC = 1;
//    ADC10DF = 0;
//    ADC10DIVx = 0x0;
//    //ADC10DIVx = 0x3;
//    //ADC10DIVx = 0x7;
//    ADC10SSELx = 0x0;
//    ConSEQx = 0x0;
//    ADC10AE0 = 0x01;  // Enable A0 ADC channel
//    ADC10AE0 = 0x08;  // Enable A3 ADC channel
    ADC10CTL0 = SREF_0 + ADC10SHT_1 + ADC10ON + ADC10IE;
    ADC10CTL1 = INCH_3 + SHS_0 + ADC10DIV_0 + ADC10SSEL_0 + CONSEQ_0;

    ADC10AE0 = 0x08;  // Enable A3 ADC channel
    // Initialize Port 1
//    P1SEL &= ~0x01;  // See page 42 and 43 of the G2553's datasheet, It shows that when both P1SEL and P1SEL2 bits are zero
//    P1SEL2 &= ~0x01; // the corresponding pin is set as a I/O pin.  Datasheet: http://coecsl.ece.illinois.edu/ge423/datasheets/MSP430Ref_Guides/msp430g2553datasheet.pdf
//    P1REN = 0x0;  // No resistors enabled for Port 1
//    P1DIR |= 0x10; // Set P1.4 to output to drive LED on LaunchPad board.  Make sure shunt jumper is in place at LaunchPad's Red LED
//    P1OUT &= ~0x10;  // Initially set P1.4 to 0

    P1SEL &= ~0xFF;  // All P1 Pins GPIO
    P1SEL2 &= ~0xFF; // All P1 Pins GPIO
    P1REN &= ~0xFF;  // All P1 Pins Resistor Disabled
    P1DIR |= 0x1;    // Set P1.0 to output direction
    P1OUT |= 0x1;    // Initially set P1.0 to 1

    // Initialize Port 2 - TA1.1 PWM Output
    P2DIR |= 0x04;   // P2.2 output
    P2SEL |= 0x04;   // P2.2 TA1.1 option
    P2SEL2 &= ~0x04; // P2.2 TA1.1 option

    // Timer A Config
    TACCTL0 = CCIE;             // Enable Periodic interrupt
    TACCR0 = 16000;                // period = 1ms
    TACTL = TASSEL_2 + MC_1; // source SMCLK, up mode

    // PWM Config
    TA1CCR0 = 512;              // PWM Period
    TA1CCTL1 = OUTMOD_7;         // TA1CCR1 reset/set
    TA1CCR1 = 384;                 // TA1CCR1 PWM duty cycle
    TA1CTL = TASSEL_2 + MC_1;    // SMCLK, up mode

    Init_UART(115200,1);    // Initialize UART for 115200 baud serial communication

    _BIS_SR(GIE);       // Enable global interrupt


    while(1) {  // Low priority Slow computation items go inside this while loop.  Very few (if anyt) items in the HWs will go inside this while loop

// for use if you want to use a method of receiving a string of chars over the UART see USCI0RX_ISR below
        if(newmsg) {
          newmsg = 0;
        }

        // The newprint variable is set to 1 inside the function "print_every(rate)" at the given rate
        if ( (newprint == 1) && (senddone == 1) )  { // senddone is set to 1 after UART transmission is complete

            // only one UART_printf can be called every 15ms
            senddone = 0;
            UART_printf("P1.0: %ld\n\r",adc_value);
            senddone = 1;
            newprint = 0;
        }



        if (newprint)  {

                    UART_printf("%dmV\n\r", adc_value);

                    // adc_value < 200 for 2 seconds turn on LED on P1.4

                    if (adc_value < 200) {
                        count++;
                        if (count >= 200){
                            P1OUT |= 0x10; // turn on LED
                            count = 0;
                        }
                    }
                    else if (adc_value > 200) {
                        count++;
                        if (count >= 200){
                            P1OUT &= ~0x10; // turn off LED
                            count = 0;
                        }
                    }

                    newprint = 0;
                }

    }
}


// Timer A0 interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
    ADC10CTL0 |= ENC + ADC10SC;
}


/*
// ADC 10 ISR - Called when a sequence of conversions (A7-A0) have completed
#pragma vector=ADC10_VECTOR
*/
__interrupt void ADC10_ISR(void) {
    timecheck++; // Keep track of time for main while loop.
    adc_value = (int)ADC10MEM; // save conversion value to adc_value
    print_every(250);  // units determined by the rate Timer_A ISR is called, print every "rate" calls to this function
    if (timecheck == 250) {
            newprint = 1;
            timecheck = 0;
    }

    if (adc_value < 256){
            P1OUT = 0x00;
        }
        else if (adc_value > 512){
            P1OUT = 0xF0;
        }
        else{
            P1OUT = 0x30;
        }}



// USCI Transmit ISR - Called when TXBUF is empty (ready to accept another character)
#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR(void) {

    if(IFG2&UCB0RXIFG) {  // USCI_B0 requested RX interrupt (UCB0RXBUF is full)

        IFG2 &= ~UCB0RXIFG;   // clear IFG
    }

    if(IFG2&UCA0RXIFG) {  // USCI_A0 requested RX interrupt (UCA0RXBUF is full)

        // The received ASCII value of a character is placed in the register UCA0RXBUF
        // global char received_val

        received_val = UCA0RXBUF;


        if (received_val == '1') {
            // Turn on LED 1
            P1OUT |= 0x10;
        }
        else if (received_val == '2'){
            // Turn on LED 2
            P1OUT |= 0x20;
        }
        else if (received_val == '3'){
            // Turn on LED 3
            P1OUT |= 0x40;
        }
        else if (received_val == '4'){
            // Turn on LED 4
            P1OUT |= 0x80;
        }
        else if (received_val == '5'){
            // Turn off LED 1
            P1OUT &= ~0x10;
        }
        else if (received_val == '6'){
            // Turn off LED 2
            P1OUT &= ~0x20;
        }
        else if (received_val == '7'){
            // Turn off LED 3
            P1OUT &= ~0x40;
        }
        else if (received_val == '8'){
            // Turn off LED 4
            P1OUT &= ~0x80;
        }
        else
        {
            // Turn off all LED 1 - LED 4
            P1OUT &= ~0xF0;
        }

        sendchar(received_val);

        IFG2 &= ~UCA0RXIFG; //For USCIA

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
