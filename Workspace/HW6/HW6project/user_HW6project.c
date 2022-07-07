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
unsigned int previous = 0;
unsigned int current = 5;
unsigned int time1 = 0;
unsigned int time2 = 0;
int input = 0;

void main(void) {

    WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT

    if (CALBC1_16MHZ ==0xFF || CALDCO_16MHZ == 0xFF) while(1);

    DCOCTL = CALDCO_16MHZ;    // Set uC to run at approximately 16 Mhz
    BCSCTL1 = CALBC1_16MHZ;

    // Initialize Port 1
    P1SEL &= ~0x01;  //  Make sure P1.0 GPIO
    P1SEL |= 0x40;  // Set P1.6 as TA0.1
    P1SEL2 &= ~0x41; // P1.0 GPIO P1.6 TA0.1
    P1REN = 0x0;  // No resistors enabled for Port 1
    P1DIR |= 0x41; // Set P1.0 output P1.6 TA0.1
    P1OUT &= ~0x01;  // Initially set P1.0 to 0

    // TODO: Initialize Port 2.1 as TA1.CCI1A capture input pin
    P2SEL |= 0x02;  // Set P2.1 TA1.CCI1A
    P2SEL2 &= ~0x02; // Set P2.1 TA1.CCI1A
    P2DIR &= ~0x02; // Set P2.1 TA1.CCI1A

    // Timer0 A Config  So this sets Timer0 to interrupt every 1ms
    //                 and generate a PWM signal on P1.6
    TA0CCTL0 = CCIE;       // Enable Periodic interrupt
    TA0CCR0 = 16000;       // period = 1ms
    TA0CCR1 = 8000;        // Start with 50% duty cycle
    TA0CCTL1 = OUTMOD_7;   // Set/Reset
    TA0CTL = ID_1 + TASSEL_2 + MC_1; // divide by 1, source SMCLK, up mode

    // TODO : Timer A1 Config  Set Timer A1 in capture mode
    //              and use capture pin TA1.CCI1A at P2.1
    TA1CCTL1 = CM_3 + CCIS_0 + CAP + CCIE;        // TA1CCR1 Capture mode; CCI1A; Both
                           // Rising and Falling Edge; interrupt enable

    TA1CTL = TASSEL_2 + MC_2 + TACLR;        // SMCLK, Continous Mode; Clear timer

    Init_UART(115200,1);    // Initialize UART for 115200 baud serial communication

    _BIS_SR(GIE);       // Enable global interrupt


    while(1) {  // Low priority Slow computation items go inside this while loop.  Very few (if anyt) items in the HWs will go inside this while loop

// for use if you want to use a method of receiving a string of chars over the UART see USCI0RX_ISR below
//      if(newmsg) {
//          newmsg = 0;
//      }

        // The newprint variable is set to 1 inside the function "print_every(rate)" at the given rate
        if ( (newprint == 1) && (senddone == 1) )
            { // senddone is set to 1 after UART transmission is complete

            // only one UART_printf can be called every 15ms
            UART_printf("TA0Perid %uus,TA0OnTime %uus\n\r",(int)(((time1+time2)*1000L)/16000),(int)((time1*1000L)/16000));

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
    if (timecheck == 500) {
        P1OUT ^= 0x1;

        if (TACCR1<(TACCR0-1000)){
            TACCR1 = TACCR1+1000;
        }
        else{
            TACCR1 = 4000;
        }

        timecheck = 0;
    }

}

// TA1_A1 Interrupt vector
#pragma vector = TIMER1_A1_VECTOR
__interrupt void TIMER1_A1_ISR (void)
{
  switch(TA1IV)
  {
      case  TA1IV_NONE:
          // Should not get here
          break;
      case  TA1IV_TACCR1:  // TACCR1 CCIFG  Capture interrupt
          //TODO  Read TACCR1 to know when the Capture interrupt occurred.
          // and then additional code to perform given tasks.
          current = TA1CCR1;
          if (previous != 0){
              input = TA1CCTL1;
              if ((input &= 0x08) ==  0){
                  time1 = current - previous;
              }
              else{
                  time2 = current - previous;
              }
          }
          previous = current;
          break;
      case TA1IV_TACCR2: break; // TACCR2 CCIFG Not used in HW6
      case TA1IV_6: break;      // Reserved CCIFG Not used
      case TA1IV_8: break;      // Reserved CCIFG Not used
      case TA1IV_TAIFG:         // TAIFG  Overflow interrupt. Used in Challenge part

          break;
      default:  break;
  }
}

/*
// ADC 10 ISR - Called when a sequence of conversions (A7-A0) have completed
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void) {

}
*/


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
