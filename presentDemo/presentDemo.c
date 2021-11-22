#include <msp430.h>
#include <libTimer.h>
#include "buzzer.h"
#include "lcdutils.h"
#include "lcddraw.h"

// WARNING: LCD DISPLAY USES P1.0.  Do not touch!!! 

#define LED BIT6		/* note that bit zero req'd for display */

#define SW1 1
#define SW2 2
#define SW3 4
#define SW4 8

#define SWITCHES 15

static char 
switch_update_interrupt_sense()
{
  char p2val = P2IN;
  /* update switch interrupt to detect changes from current buttons */
  P2IES |= (p2val & SWITCHES);	/* if switch up, sense down */
  P2IES &= (p2val | ~SWITCHES);	/* if switch down, sense up */
  return p2val;
}

void 
switch_init()			/* setup switch */
{  
  P2REN |= SWITCHES;		/* enables resistors for switches */
  P2IE |= SWITCHES;		/* enable interrupts from switches */
  P2OUT |= SWITCHES;		/* pull-ups for switches */
  P2DIR &= ~SWITCHES;		/* set switches' bits for input */
  switch_update_interrupt_sense();
}

int switches = 0;

void
switch_interrupt_handler()
{
  char p2val = switch_update_interrupt_sense();
  switches = ~p2val & SWITCHES;
}


// axis zero for col, axis 1 for row
short drawPos[2] = {10,10}, controlPos[2] = {10,10};
short velocity[2] = {3,8}, limits[2] = {screenWidth-36, screenHeight-8};

short redrawScreen = 1;
u_int controlFontColor = COLOR_GREEN;
int blink_count = 0;
void wdt_c_handler()
{
  static int secCount = 0;
  if(SW1 & SWITCHES) blink_count++;
  if (blink_count >= 4000) blink_count = 0;
  
  secCount ++;
  if (secCount >= 25) {		/* 10/sec */
    secCount = 0;
    redrawScreen = 1;
  }
}
  
void update_shape();
void draw_star(unsigned char col, unsigned char row, char openPresent);
void open_top(int leftWallPos[], int rightWallPos[], char presentHeight, char rowStep);
void write_happyBday(unsigned char col, unsigned char row);

void main()
{
  P1DIR |= LED;		/**< Green led on when CPU on */
  P1OUT |= LED;
  configureClocks();
  lcd_init();
  switch_init();
  buzzer_init();
  
  enableWDTInterrupts();      /**< enable periodic interrupt */
  or_sr(0x8);	              /**< GIE (enable interrupts) */
  
  clearScreen(COLOR_BLACK);
  while (1) {			/* forever */
    if (redrawScreen) {
      redrawScreen = 0;
      update_shape();
    }
    P1OUT &= ~LED;	/* led off */
    or_sr(0x10);	/**< CPU OFF */
    P1OUT |= LED;	/* led on */
  }
}
    
void
update_shape()
{
  static unsigned char row = screenHeight / 2, col = screenWidth / 2;
  static char blue = 31, green = 0, red = 31;
  static char currStripeColor = 0; // 0 for red stripe or 1 for white
  static char presentHeight = (screenWidth * 3) / 4;
  static char openPresent = 0, openPresentStep = 0, songToggled = 0, stripeStep = 0;
  static char rowStep = 0;
  
  int leftWallPos[2] = {col/4, row - 25}; // Position for present left wall
  int rightWallPos[2] = { limits[0] + 16, row - 25}; // Position for present right wall
  
  if (switches & SW1) songToggled ^= 1; // Toggle song
  if (switches & SW2) openPresent = 1; // Toggled open present
  if (switches & SW3) draw_star(col, row, openPresent);
  if (switches & SW4) {
    openPresent = openPresentStep = rowStep = stripeStep = songToggled = blink_count = 0;
    buzzer_off();
    clearScreen(COLOR_BLACK);
    return;
  }
  
  if (songToggled) {  // Play happy bday song
    play_happyBirthday(blink_count); 
  } else { // Reset the counter and turn off buzzer
    blink_count = 0;
    buzzer_off();
  }
 
  // When open present animation is toggled
  if(openPresent) {
    // Draw left and right present wall
    fillRectangle(leftWallPos[0], leftWallPos[1], 4, presentHeight, COLOR_RED);
    fillRectangle(rightWallPos[0], rightWallPos[1], 4, presentHeight, COLOR_RED);
    
    // Use as black mask for present front face
    if(openPresentStep <= presentHeight - 4) 
      fillRectangle(leftWallPos[0] + 4, leftWallPos[1], presentHeight - 8, openPresentStep+=3, COLOR_BLACK);

    // Open box flaps
    if(rowStep < 30) {
      open_top(leftWallPos, rightWallPos, presentHeight , rowStep);
      rowStep += 3;
    } else {
      write_happyBday(col, row);
    }
    
  } else {
    // Draw present with stripes
    int startingCol = col / 4;
    char color =  (currStripeColor) ? COLOR_WHITE : COLOR_RED;

    drawString5x7(col - 25, 15, "PRESS S2", COLOR_GRAY, COLOR_BLACK); // First instructions
    
    if(stripeStep + 16 <= presentHeight){
      fillRectangle(startingCol + stripeStep, row - 25, 16, presentHeight, color);
      stripeStep += 16;
      currStripeColor ^= 1;
    } else {
      fillRectangle(startingCol + (presentHeight - stripeStep), row - 25, 16, presentHeight, color); 
    }
    // Draw top of present
    fillRectangle(leftWallPos[0] - 8, leftWallPos[1] - 16, presentHeight + 16, 16, COLOR_RED);
  }// end of else
}

void draw_star(unsigned char col, unsigned char row, char openPresent)
{
  if(!openPresent) return; // If present is not open then dont draw the star
  
}

/* Draw the box flaps and open them */
void open_top(int leftWallPos[], int rightWallPos[], char presentHeight, char rowStep)
{
  fillRectangle(leftWallPos[0] - 8, leftWallPos[1] - rowStep, presentHeight + 16, 3, COLOR_BLACK);
  fillRectangle(leftWallPos[0] - 8, leftWallPos[1] - 16 - rowStep, presentHeight + 16, 16, COLOR_RED);
}

/* Writes a happy birthday message on to the LCD */
void write_happyBday(unsigned char col, unsigned char row)
{
  drawString5x7(col - 45 , row - 50, "Happy Birthday!", COLOR_WHITE, COLOR_BLACK);
}

/* Switch on S2 */
void
__interrupt_vec(PORT2_VECTOR) Port_2(){
  if (P2IFG & SWITCHES) {	      /* did a button cause this interrupt? */
    P2IFG &= ~SWITCHES;		      /* clear pending sw interrupts */
    switch_interrupt_handler();	/* single handler for all switches */
  }
}
