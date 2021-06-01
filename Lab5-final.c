/*--------------------------------------------------------
GEORGE MASON UNIVERSITY
ECE 447 - Lab 4 - LED Matrix test for red and green LEDs skeleton
  Using timer ISR for multiplexing

  P2.6 unused
  P2.7 Row Initialization (CCR6)
  P3.6 Column Clock (CCR2)
  P3.7 Column Done  (CCR3) is now also Row Clock
  P9.0 Green Row Data (serial, one column at a time with LSB first,
                 LSB will show as column 8 in matrix)
  P9.1 Red Row Data (serial, one column at a time with LSB first,
                 LSB will show as column 8 in matrix)


Date:   Fall 2020
Author: Jens-Peter Kaps

Change Log:
20200928 Initial Version, Jens-Peter Kaps
--------------------------------------------------------*/

#include <msp430.h>


unsigned char rowcnt;           // row counter
unsigned char colcnt;           // column counter
unsigned char g_matrix[8];      // content for LED matrix
unsigned char r_matrix[8];      // content for red LED matrix
unsigned char g_row;            // current row of the green LED matrix
unsigned char r_row;            // current row of the red LED matrix
unsigned int str_len;
unsigned int str_len1;
unsigned int str_len2;
unsigned char g_next[8];
unsigned char r_next[8];
unsigned int g_1 = 0;
unsigned int r_1;
unsigned char g_str[] = "           Connect 4";
unsigned char r_str[] = "Welcome to         4";
unsigned int button;
unsigned int BounceDelay = 300;
unsigned int blink;
unsigned int winner_msg = 0;
unsigned char winner_col[4];
unsigned char winner_row[4];
unsigned int winner;
unsigned int player;
unsigned char r_curr;
unsigned char col_s;
unsigned char g_str1[] = "Green WON!!!";
unsigned char r_str1[] = "         !!!";
unsigned char g_str2[] = "       !!!";
unsigned char r_str2[] = "Red WON!!!";
/*
 * main.c
 */
int main(void) {
    enum states {
            START,
            MOVE,
            DROP,
            FLASH,
            WIN };
    enum states state = START;

    unsigned int i;             // all purpose

    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer

    // Add code to:
    // connect the ports P2.7, P3.6, and P3.7 to timer
    // B0 CCR6, CCR2, and CCR3 respectively
    P2DIR |= BIT7;              // Make Port 2.7 output
    P3DIR |= (BIT6 | BIT7);     // Make Port 3.6 and 3.7 outputs
    P2OUT &= ~(BIT7);           // Set port to 0
    P3OUT &= ~(BIT6 | BIT7);    // Set ports to 0
    P2SEL1 &= ~(BIT7);          // connect P2.7
    P2SEL0 |= (BIT7);           // to TB0.6
    P3SEL0 &= ~(BIT6 | BIT7);   // connect P3.6 and P3.7
    P3SEL1 |= (BIT6 | BIT7);    // to TB0.2 and TB.3

    P2DIR &= ~(BIT1 | BIT2 | BIT3 |BIT4); // buttons as inputs
    P2REN |= (BIT1 | BIT2 | BIT3 |BIT4);  // enable resistor
    P2OUT |= (BIT1 | BIT2 | BIT3 |BIT4);  // pull up resistor

    P2IES = (BIT1 | BIT2 | BIT3 |BIT4);
    P2IFG &= ~(BIT1 | BIT2 | BIT3 |BIT4);
    P2IE |= (BIT1 | BIT2 | BIT3 |BIT4);


    // Add code to:
    // make the LED Matrix column serial outputs P9.0 and P  9.1 outputs
    // and output a 0 on each.
    P9DIR |= (BIT0 | BIT1);     // Makes Port P9.0 and P9.1 outputs
    P9OUT &= ~(BIT0 | BIT1);    // Sets ports to 0


    PM5CTL0 &= ~LOCKLPM5;       // Unlock ports from power manager

    // CCR2 is connected to P3.6 which is column clock.
    // CCR0 active with interrupt, column clock goes low (set/RESET).
    // When CCR2 triggers (no interrupt) column clock goes high (SET/reset).
    // CCR0 and CCR2 are 4 timer clock cycles apart.
    // Add code to:
    // Initalize TB0CCR0 and TB0CCR2 such that CCR0 has an event 4 clock cyles after CCR2
    // Enable interrupts for CCR0
    // Set the outmod for CCR2
    TB0CCR0  = 8;               // CCR0 = CCR2 + 4
    TB0CCR2 = 4;                // CCR = CLK = 8
    TB0CCR4 = 4000;
    TB0CCTL0 = CCIE;            // Enable interrupts on CCR0
    TB0CCTL4 = CCIE;
    TB0CCTL2 = OUTMOD_3;        // set/reset


    // Add code to:
    // Setup and start timer B in continuous mode, ALCK, clk /1, clear timer
    TB0CTL = MC_2 | ID__1 | TBSSEL_1 | TBCLR;

    TA2CTL = MC_2 | ID_0 | TASSEL_1 | TACLR;

    i=2;
    for(rowcnt=0; rowcnt<8; rowcnt++){
        g_matrix[rowcnt]=0x00;
    }
    i=255;
    for(rowcnt=0; rowcnt<8; rowcnt++){
        r_matrix[rowcnt]=0x00;
    }

    r_1 = 0;
    getnextchar(g_next, g_str[r_1]);
    getnextchar(r_next, r_str[r_1]);
    r_1 = 1;

    rowcnt = 0;                 // starting row
    colcnt = 0;                 // starting column
    str_len = 19;
    button = 0;
    blink = 0;
    winner_msg = 0;
    player = 0;
    state = START;
    str_len1 = 11;
    str_len2 = 8;
    unsigned char *matrix;
    if(player == 0) matrix = g_matrix;
    if(player == 1) matrix = r_matrix;
    __enable_interrupt();
    while (1) {
        __low_power_mode_3();
        switch(state) {             // next state logic
           case START:
               if(button == 1 | button == 2 | button == 3 | button == 4) {
                   i = 2;
                   for(rowcnt=0; rowcnt<8; rowcnt++){
                       g_matrix[rowcnt]=0x00;
                   }
                   i = 255;
                   for(rowcnt=0; rowcnt<8; rowcnt++){
                       r_matrix[rowcnt]=0x00;
                   }
                   state = MOVE;
                   matrix[0]=0x10;
                   TB0CCTL4 &= ~CCIE;
               }
               break;
           case MOVE:
               if(button == 2) {
                   state = START;
                   TB0CCTL4 |= CCIE;
                   TB0CCR4 = TB0R + 1000;
                   player = 0;
               }
               if(button==3) {
                       state = DROP;
                       TB0CCR4 = TB0R + 1000;
                       TB0CCTL4 |= CCIE;
                       r_curr = matrix[0];
//                   }
               }
               if(button==1 | button==4) {
                   state = MOVE;
               }
               break;
           case DROP:
               if (rowcnt <= 7) {
                   if (winner_msg == 1) {
                       state = FLASH;
                       }
                   else {
                       if (player == 0) {
                          state = MOVE;
                          matrix = g_matrix;
                          matrix[0]=0x10;
                       }
                       else {
                           matrix = r_matrix;
                           state = MOVE;
                           matrix[0]=0x10;
                       }
                       TB0CCTL4 &= ~CCIE;
                   }
               }
               if (matrix[rowcnt+1] & r_curr) {
                   state = MOVE;
                   matrix[0]=0x10;
                   if (winner_msg == 1) {
                       state = FLASH;
                   }
               }
               break;
           case FLASH:
               if ((blink==4) & (winner_msg == 1)) {
                   state = WIN;
               }
               if (button == 2) {
                   state = START;
               }
               break;
           case WIN:
               if (button == 2) {
                   state = START;
               }
               break;
           }

    switch(state) {                 // state logic
    case START:
        player = 0;
        TB0CCTL4 |= CCIE;
        shiftchar(g_matrix, g_next);
        shiftchar(r_matrix, r_next);
        g_1 += 1;
        if (g_1 == 6) {
            getnextchar(g_next, g_str[r_1]);
            getnextchar(r_next, r_str[r_1]);
            g_1 = 0;
            if (r_1 < str_len) {
                r_1 += 1;
            }
            else {
                r_1 = 0;
            }
        }
        break;
    case MOVE:
        TB0CCTL4 &= ~CCIE;
        if(player == 0) matrix = g_matrix;
        if(player == 1) matrix = r_matrix;
        if(!(matrix[0] == 7)) {
            if(button == 1) {
                matrix[0] <<= 1;
                button = 0;
            }
        }
        if(!(matrix[0] == 1)) {
            if(button == 4) {
                matrix[0] >>= 1;
                button = 0;
            }
        }
        break;
    case DROP:
        winner_msg = 0;
        matrix[rowcnt] &= ~r_curr;
        rowcnt++;
        matrix[rowcnt] |= r_curr;
            if (rowcnt < 5) {
                if (matrix[rowcnt] & colcnt & matrix[rowcnt+1] & colcnt-1 & matrix[rowcnt+2] & colcnt-2 & matrix[rowcnt+3] & colcnt-3) { // diagonal going down
                    winner_row[0] = rowcnt;
                    winner_row[1] = rowcnt + 1;
                    winner_row[2] = rowcnt + 2;
                    winner_row[3] = rowcnt + 3;
                    winner_col[0] = colcnt;
                    winner_col[1] = colcnt - 1;
                    winner_col[2] = colcnt - 2;
                    winner_col[3] = colcnt - 3;
                    winner_msg = 1;
                }
                if (matrix[rowcnt] & colcnt & matrix[rowcnt-1] & colcnt+1 & matrix[rowcnt-2] & colcnt+2 & matrix[rowcnt-3] & colcnt+3) {
                    winner_row[0] = rowcnt;
                    winner_row[1] = rowcnt - 1;
                    winner_row[2] = rowcnt - 2;
                    winner_row[3] = rowcnt - 3;
                    winner_col[0] = colcnt;
                    winner_col[1] = colcnt + 1;
                    winner_col[2] = colcnt + 2;
                    winner_col[3] = colcnt + 3;
                    winner_msg = 1;
                }
                if (matrix[rowcnt] & matrix[rowcnt+1] & matrix[rowcnt+2] & matrix[rowcnt+3] & colcnt) {
                    winner_row[0] = rowcnt;
                    winner_row[1] = rowcnt + 1;
                    winner_row[2] = rowcnt + 2;
                    winner_row[3] = rowcnt + 3;
                    winner_col[0] = colcnt;
                    winner_col[1] = colcnt;
                    winner_col[2] = colcnt;
                    winner_col[3] = colcnt;
                    winner_msg = 1;
                                }
                if (matrix[rowcnt] & colcnt & matrix[rowcnt+1] & colcnt+1 & matrix[rowcnt+2] & colcnt+2 & matrix[rowcnt+3] & colcnt+3) { // diagonal going up
                    winner_row[0] = rowcnt;
                    winner_row[1] = rowcnt + 1;
                    winner_row[2] = rowcnt + 2;
                    winner_row[3] = rowcnt + 3;
                    winner_col[0] = colcnt;
                    winner_col[1] = colcnt + 1;
                    winner_col[2] = colcnt + 2;
                    winner_col[3] = colcnt + 3;
                    winner_msg = 1;
                }
                if (matrix[rowcnt] & colcnt & matrix[rowcnt-1] & colcnt-1 & matrix[rowcnt-2] & colcnt-2 & matrix[rowcnt-3] & colcnt-3) {
                    winner_row[0] = rowcnt;
                    winner_row[1] = rowcnt - 1;
                    winner_row[2] = rowcnt - 2;
                    winner_row[3] = rowcnt - 3;
                    winner_col[0] = colcnt;
                    winner_col[1] = colcnt - 1;
                    winner_col[2] = colcnt - 2;
                    winner_col[3] = colcnt - 3;
                    winner_msg = 1;
                }
            }
            if (matrix[rowcnt] & colcnt & colcnt-1 & colcnt+1 & colcnt+2) { // horizontal case
                winner_row[0] = rowcnt;
                winner_row[1] = rowcnt;
                winner_row[2] = rowcnt;
                winner_row[3] = rowcnt;
                winner_col[0] = colcnt;
                winner_col[1] = colcnt - 1;
                winner_col[2] = colcnt + 1;
                winner_col[3] = colcnt + 2;
                winner_msg = 1;
            }
            if (matrix[rowcnt] & colcnt & colcnt-1 & colcnt-2 & colcnt-3) { // horizontal case
                winner_row[0] = rowcnt;
                winner_row[1] = rowcnt;
                winner_row[2] = rowcnt;
                winner_row[3] = rowcnt;
                winner_col[0] = colcnt;
                winner_col[1] = colcnt - 1;
                winner_col[2] = colcnt - 2;
                winner_col[3] = colcnt - 3;
                winner_msg = 1;
            }
            if (matrix[rowcnt] & colcnt & colcnt-1 & colcnt-2 & colcnt+1) { // horizontal case
                winner_row[0] = rowcnt;
                winner_row[1] = rowcnt;
                winner_row[2] = rowcnt;
                winner_row[3] = rowcnt;
                winner_col[0] = colcnt;
                winner_col[1] = colcnt - 1;
                winner_col[2] = colcnt - 2;
                winner_col[3] = colcnt + 1;
                winner_msg = 1;
            }
            if (matrix[rowcnt] & colcnt & colcnt+1 & colcnt+2 & colcnt+3) { // horizontal case
                winner_row[0] = rowcnt;
                winner_row[1] = rowcnt;
                winner_row[2] = rowcnt;
                winner_row[3] = rowcnt;
                winner_col[0] = colcnt;
                winner_col[1] = colcnt + 1;
                winner_col[2] = colcnt + 2;
                winner_col[3] = colcnt + 3;
                winner_msg = 1;
            }
        if (player == 0) { // swap players for next drop
            player = 1;
        }
        else {
            player = 0;
        }
        break;
    case FLASH:
        if (player == 0) {
            r_matrix[winner_row[0]] ^= winner_col[0];
            r_matrix[winner_row[1]] ^= winner_col[1];
            r_matrix[winner_row[2]] ^= winner_col[2];
            r_matrix[winner_row[3]] ^= winner_col[3];
            blink++;
        }
        else {
            g_matrix[winner_row[0]] ^= winner_col[0];
            g_matrix[winner_row[1]] ^= winner_col[1];
            g_matrix[winner_row[2]] ^= winner_col[2];
            g_matrix[winner_row[3]] ^= winner_col[3];
            blink++;
        }
        winner = player;
        break;
    case WIN:
            if (winner == 0) {

                shiftchar(g_matrix, g_next);
                shiftchar(r_matrix, r_next);
                g_1 += 1;
                if (g_1 == 6) {
                    getnextchar(g_next, g_str2[r_1]);
                    getnextchar(r_next, r_str2[r_1]);
                    g_1 = 0;
                    if (r_1 < str_len2) {
                        r_1 += 1;
                    }
                    else {
                        r_1 = 0;
                        break;
                    }
                }
            }
            else {

                shiftchar(g_matrix, g_next);
                shiftchar(r_matrix, r_next);
                g_1 += 1;
                if (g_1 == 6) {
                    getnextchar(g_next, g_str1[r_1]);
                    getnextchar(r_next, r_str1[r_1]);
                    g_1 = 0;
                    if (r_1 < str_len1) {
                        r_1 += 1;
                    }
                    else {
                        r_1 = 0;
                        break;
                    }
                }
            }

        break;
    }
    }
    return 0;
}



#pragma vector = PORT2_VECTOR      // associate funct. w/ interrupt vector
__interrupt void Port_2(void)      // name of ISR
{
    P2IFG &= ~(BIT1 | BIT2 | BIT3 | BIT4); //  clear  interrupt  flag
    P2IE |= (BIT1|BIT2|BIT3|BIT4);   // disable button interrupt during debounce
   TB0CCR5 = TB0R+BounceDelay;
   TB0CCTL5 |= CCIE;
}


#pragma vector = TIMER0_B1_VECTOR
__interrupt void T0B1_ISR(void)
{
    switch(__even_in_range(TB0IV,14)){
    case 0: break ; // No interrupt
    case 2: break ; // CCR1
    case 4: break ; // CCR2
    case 6: break ; // CCR3
    case 8: P1OUT ^= BIT0;
        TB0CCR4 += 4000;
        __low_power_mode_off_on_exit();
    break ; // CCR4
    case 10:
        P2IE |= (BIT1 | BIT2 | BIT3 | BIT4);
        if (P2IES & BIT1) {
            P2IES &= ~(BIT1 | BIT2 | BIT3 | BIT4);
            if(!(P2IN & BIT1)) {
                button = 1;
            }
            if (!(P2IN & BIT2)) {
                button = 2;
            }
            if (!(P2IN & BIT3)) {
                button = 3;
            }
            if (!(P2IN & BIT4)) {
                button = 4;
            }
            __low_power_mode_off_on_exit();
        }
        else {
            P2IES |= (BIT1 | BIT2 | BIT3 | BIT4);
        }
        TB0CCTL5 &= ~CCIE;
        P2IFG &= ~(BIT1 | BIT2 | BIT3 | BIT4);
        break ; // reserved
    case 12: break ; // reserved
    case 14: break ; // TAR overflow
    default : break ;
    }
}

// Interrupt Service Routine for Timer B channel CCR0,
// active on falling edge on column clock
#pragma vector = TIMER0_B0_VECTOR   // associate funct. w/ interrupt vector
__interrupt void T0B0_ISR(void)     // name of ISR (can be anything)
{
    // Add code to:
    // output one bit (column) of the green and of the red row
    // and then shift them to move to the next column
    // Add code to:
    // create timer events for CCR0 and CCR2.
    // both 8 clock cycles from the last one
    if (BIT0 & g_row) {             // output one bit (column) of the green row
        P9OUT |= BIT0;
    }
    else {
        P9OUT &= ~(BIT0);
    }

    if (BIT0 & r_row) {             // output one bit (column) of the red row
        P9OUT |= BIT1;
    }
    else {
        P9OUT &= ~(BIT1);
    }
    g_row = g_row >>1;              // shift to next column in the green row
    r_row = r_row >>1;              // shift to next column in the red row
    TB0CCR0  += 8;                  // getting the next rising edge, TB0CCR0 has to be incremented by 8
    TB0CCR2  += 8;                  // getting the next falling edge, TB0CCR2 has to be incremented by 8

    if(colcnt == 7) {               // When on last column of matrix

        // Add code to:
        // create events for column_done and row_init based upon the
        // specifications in the lab manual
        // Add code to:
        // increment the row counter and set the column counter back to 0
        // Add code to:
        // update the current row for red and green
        TB0CCTL3 = OUTMOD_1;        // set
        TB0CCR3 = TB0CCR0;          // next event on CCR3 will occur at same time as next event on CCR0
        if (rowcnt == 7) {          // When on last row of matrix
            TB0CCR6 = TB0CCR2;      // next event on CCR6 will occur at same time as next event on CCR2
            TB0CCTL6 = OUTMOD_1;    // set
        }
        rowcnt++;                   // incrementing row counter
        if(rowcnt == 8) {           // checking if out of matrix scope
            rowcnt = 0;             // resetting row counter
        }
        colcnt = 0;                 // go to first column, i.e. next falling edge after colcnt=7
        g_row = g_matrix[rowcnt];   // updating matrix here
        r_row = r_matrix[rowcnt];   // updating matrix here
        }
    else {
        // Add code to:
        // create events for column_done and row_init based upon the
        // specifications in the lab manual
        // Add code to:
        // increment the column counter
        TB0CCR3 = TB0CCR2;          // next event on CCR3 will occur at same time as next event on CCR2
        TB0CCTL3 = OUTMOD_5;        // reset
        TB0CCTL6 = OUTMOD_5;        // reset
        TB0CCR6 = TB0CCR2;          // next event on CCR6 will occur at same time as next event on CCR2
        colcnt++;                   // incrementing column counter
    }

}
