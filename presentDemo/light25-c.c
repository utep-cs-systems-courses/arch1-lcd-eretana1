#include <msp430.h>
#include "stateMachines.h"
#include "led.h"

// Function to set light intesity to 25%
void light_25(int state)
{
  switch(state){
  case 0:
  case 1:
  case 2: 
    P1OUT &= ~LED_GREEN;
    P1OUT |= LED_RED;
    break;
  case 3:
    P1OUT |= LED_GREEN;
    P1OUT &= ~LED_RED;
  default:
    break;
  }
}
