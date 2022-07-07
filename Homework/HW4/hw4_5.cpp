#include "msp430g2553.h"
#include "UART.h"

void print_every(int rate);

char newprint = 0;
long NumOn = 0;
long NumOff = 0;
int statevar = 1;
int timecheck1 = 0;
int timecheck2 = 0;
int state = 0;

char count2_6 = 0;
char count2_7 = 0;

char flag2_6 = 0;
char flag2_7 = 0;

void main(void) {

	WDTCTL = WDTPW + WDTHOLD;                 // Stop watchdog timer

	if (CALBC1_16MHZ ==0xFF || CALDCO_16MHZ == 0xFF) while(1);

	DCOCTL = CALDCO_16MHZ;    // Set uC to run at approximately 16 Mhz
	BCSCTL1 = CALBC1_16MHZ; 

	// Initialize Port 1
    P1SEL &= ~0x01;  //Set P1.0 GPIO 
    P1SEL2 &= ~0x01; 
    P2SEL &= ~0xc0;  //Set P2.6 P2.7 GPIO  
    P2SEL2 &= ~0xc0;
    P1REN = 0x0;  // No resistors enabled for Port 1
    P1DIR |= 0x10; // Set P1.4 to output
    P2DIR |= 0x04; // Set P2.2 to output 
    P2DIR &= ~0xc0; //Set P2.6 and P2.7 to input
    P2REN |= 0xc0;  // P2.6 and P2.7 Resistor enabled
    P2OUT |= 0xc0;  // P2.6 and P2.7 Pullup Resistor selected

    // Port 2 Interrupts
    P2IE |= 0xc0; // P2.6 and P2.7 interrupt enabled
    P2IES |= 0xc0; // P2.6 and P2.7 H/L edge
    P2IFG &= ~0xc0; // P2.6 and P2.7 IFG cleared

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
            UART_printf("count2_6: %d count2_7: %d\n\r",count2_6,count2_7);

            newprint = 0;
        }

    }
}

#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void){

    if((P2IFG & 0x40) == 0x40) {
        P1OUT ^= 0x10;  // P1.4 toggled
        P2IFG &= ~0x40; // Clear P2.6 interrupt bit
        P2IE &= ~0x40;  // Disable P2.6 interrupt
        flag2_6 = 1;   // P2.6 interrupt has been disabled
        count2_6++;
    }


    if((P2IFG & 0x80) == 0x80) {
        P2OUT ^= 0x04;  // P2.2 toggled
        P2IFG &= ~0x80; // Clear P2.7 interrupt bit
        P2IE &= ~0x80;  // Disable P2.7 interrupt
        flag2_7 = 1;   // P2.7 interrupt has been disabled
        count2_7++;
    }

}


// Timer A0 interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
    print_every(500); //print in TeraTerm

    if (flag2_6 == 1){
        timecheck1++;
        if (timecheck1 == 300){
            timecheck1 = 0;
            P2IFG &= ~0x40; // Clear P2.6 interrupt bit
            P2IE |= 0x40;  // Enable P2.6 interrupt
            flag2_6 = 0;
        }
    }

    if (flag2_7 == 1){
        timecheck2++;
        if (timecheck2 == 300){
            timecheck2 = 0;
            P2IFG &= ~0x80; // Clear P2.7 interrupt bit
            P2IE |= 0x80;  // Enable P2.7 interrupt
            flag2_7 = 0;
        }
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

