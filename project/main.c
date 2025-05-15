#include <msp430.h>
#include "buzzer.h"
#include "led.h"
#include "libTimer.h"

#define BUTTONS (BIT0 | BIT1 | BIT2 | BIT3)

void led_init();
void led_update();
void state_advance();
void startNote();
void button_init();
void switch_interrupt_handler();

int melody[] = {262, 294, 330, 349, 392, 440, 494, 523};
int currentNote = 0;
unsigned char red_on = 1, green_on = 0;
unsigned char led_changed = 0;
volatile char speed = 100;
volatile int isPlaying = 0;

static char redVal[] = {0, LED_RED}, greenVal[] = {0, LED_GREEN};

void led_init() {
  P1DIR |= LEDS;
  led_changed = 1;
  led_update();
}

void led_update() {
  if (led_changed) {
    char ledFlags = redVal[red_on] | greenVal[green_on];
    P1OUT &= (0xFF ^ LEDS) | ledFlags;
    P1OUT |= ledFlags;
    led_changed = 0;
  }
}
char toggle_red() {
  red_on ^= 1;
  return 1;
}

char toggle_green() {
  char changed = 0;
  if (red_on) {
    green_on ^= 1;
    changed = 1;
  }
  return changed;
}

void state_advance() {
  char changed = 0;
  static enum {R=0, G=1} color = G;
  if (isPlaying && currentNote < sizeof(melody) / sizeof(melody[0])) {
    switch (color) {
    case R: changed = toggle_red(); color = G; break;
    case G: changed = toggle_green(); color = R; break;
    }
  }
  led_changed = changed;
  led_update();
}

void startNote() {
  if (isPlaying && currentNote < sizeof(melody) / sizeof(melody[0])) {
    buzzer_set_period(melody[currentNote] == 0 ? 0 : 2000000 / melody[currentNote]);
    currentNote++;
  } else {
    currentNote = 0;
    buzzer_set_period(0);
  }
}

void __interrupt_vec(WDT_VECTOR) WDT() {
  static char delay = 0;
  static char currentBlink = 0;
  if (isPlaying) {
    if (++delay == speed) {
      startNote();
      delay = 0;
    }
    if (++currentBlink >= (speed / 2)) {
      state_advance();
      currentBlink = 0;
    }
  }
}

void button_init() {
  P2REN |= BUTTONS;
  P2IE |= BUTTONS;
  P2OUT |= BUTTONS;
  P2DIR &= ~BUTTONS;
}

void __interrupt_vec(PORT2_VECTOR) Port2_ISR() {

  if (P2IFG & BIT0) {  // Start/Stop Music

    __delay_cycles(2000);  // Debounce

    isPlaying = !isPlaying;

    if (!isPlaying) {

      buzzer_set_period(0);

      P1OUT &= ~LEDS;

    }

  }

  if (P2IFG & BIT1) {  // Turn off LEDs

    __delay_cycles(2000);  // Debounce

    red_on = 0;

    green_on = 0;

    led_changed = 1;

    led_update();

  }

  if (P2IFG & BIT2) {  // Decrease speed

    __delay_cycles(2000);  // Debounce

    if (speed > 50) {

      speed -= 10;

    }

  }

  if (P2IFG & BIT3) {  // Increase speed

    __delay_cycles(2000);  // Debounce

    if (speed < 300) {

      speed += 10;

    }

  }

  P2IFG &= ~BUTTONS; // Clear interrupt flags for all buttons

}

int main() {
  WDTCTL = WDTPW | WDTTMSEL | WDTCNTCL | 1;
  IE1 |= WDTIE;
  led_init();
  buzzer_init();
  button_init();
  _BIS_SR(GIE + LPM0_bits);
}
