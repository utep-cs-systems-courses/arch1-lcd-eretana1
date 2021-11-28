#include <msp430.h>
#include <libTimer.h>
#include "buzzer.h"
#include "lcdutils.h"
#include "lcddraw.h"
#include "led.h"
#include "presentDemo.h"
#include "switches.h"
#include "stateMachines.h" 

// axis zero for col, axis 1 for row
short drawPos[2] = {10,10}, controlPos[2] = {10,10};
short velocity[2] = {3,8}, limits[2] = {screenWidth-36, screenHeight-8};

short redrawScreen = 1;
u_int controlFontColor = COLOR_GREEN;

char songToggled = 0;
int blink_count = 0;

void wdt_c_handler()
{
  static int secCount = 0;

  // Check sw1 to enable state machine blinker
  if(sw1_state_down){
    state_advance(blink_count);
    secCount = 25;
    if (++blink_count >= 1000) blink_count = 0;
  }else {

    // Play happy birthday song
    if(songToggled){
      blink_count++;
      play_happyBirthday(blink_count);
      if(blink_count >= 4000) blink_count = 0; // Reset happy birthday song if over
    } else {
      blink_count = 0;
    }
  
    secCount ++;
    if (secCount >= 25) {		/* 10/sec */
      secCount = 0;
      redrawScreen = 1;
    }
  }
}
  
void main()
{
  P1DIR |= LED_RED;		/**< red led on when CPU on */
  P1OUT |= LED_RED;
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
    P1OUT &= ~LED_RED;	/* led off */
    or_sr(0x10);	/**< CPU OFF */
    P1OUT |= LED_RED;	/* led on */
  }
}
    
void
update_shape()
{
  static unsigned char row = screenHeight / 2, col = screenWidth / 2;
  static char blue = 31, green = 0, red = 31;
  static char currStripeColor = 0; // 0 for red stripe or 1 for white
  static char presentHeight = (screenWidth * 3) / 4;
  static char openPresent = 0, openPresentStep = 0, stripeStep = 0;
  static char rowStep = 0;
  
  int leftWallPos[2] = {col/4, row - 25}; // Position for present left wall
  int rightWallPos[2] = { limits[0] + 16, row - 25}; // Position for present right wall
  
  if (sw2_state_down) {
    openPresent = 1; // Toggled open present
    songToggled = 1;
  }
  if (sw3_state_down) draw_star(col, row, openPresent);
  if (sw4_state_down) {
    openPresent = openPresentStep = rowStep = stripeStep = songToggled = blink_count = 0;
    buzzer_off();
    clearScreen(COLOR_BLACK);
    return;
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
  
  static char blue = 31, green = 0, red = 31;
  static char moveStep = 0;
  static char isNeg = 0;
  // a color in this BGR encoding is BBBB BGGG GGGR RRRR
  unsigned int color = (blue << 11) | (green << 5) | red;

  green = (green + 2) % 64;
  blue = (blue + 4) % 32;
  red = (red + 6) % 32;

  // Keep track of whether to increment up or down
  if(moveStep >=  30) isNeg = 1;
  if(moveStep <= 0) isNeg = 0;

  // Clear previous shape
  for (int i = 0 ; i < 30; i++) {
    for(int j = 0; j < i; j++) {
      // Upside triangle
      drawPixel(col - j, row + i + moveStep, COLOR_BLACK);
      drawPixel(col + j, row + i + moveStep, COLOR_BLACK);
      
      // Downside triangle
      drawPixel(col - j, row - i + 40 + moveStep, COLOR_BLACK);
      drawPixel(col + j, row - i + 40 + moveStep, COLOR_BLACK);
    }
  }

  // Update star step
  if(isNeg) moveStep-=4;
  else moveStep+=4;
  
  for (int i = 0 ; i < 30; i++) {
    for(int j = 0; j < i; j++) {
      // Upside triangle
      drawPixel(col - j, row + i + moveStep, color);
      drawPixel(col + j, row + i + moveStep, color);
      
      // Downside triangle
      drawPixel(col - j, row - i + 40 + moveStep, color);
      drawPixel(col + j, row - i + 40 + moveStep, color);
    }
  }
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
