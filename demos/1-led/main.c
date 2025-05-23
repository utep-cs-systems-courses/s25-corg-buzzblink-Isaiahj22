//Alternate LEDs from Off, Green, and Red
#include <msp430.h>
#include "libTimer.h"
#include "led.h"

int main(void) {
  P1DIR |= LEDS;
  P1OUT &= ~LED_GREEN;
  P1OUT |= 0xff;

  P1OUT &= 0x00;
  P1OUT |= LED_GREEN;

  or_sr(0x18);		/* CPU off, GIE on */
}
