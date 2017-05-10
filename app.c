
#include <avr/io.h>
#include <avr/interrupt.h>
#include <limits.h>
#include "lcd.h"
#include "fortunet.h"

// timer0 overflow
ISR(TIMER0_OVF_vect) {
  char s[100];
  rx(s,100);
  tfp_printf("%s\n", s);
}

int main(void) {
  /* 8MHz clock, no prescaling (DS, p. 48) */
  CLKPR = (1 << CLKPCE);
  CLKPR = 0;

  init_uart();
  init_lcd();

  // enable timer overflow interrupt for Timer0
  // TIMSK0 =(1<<TOIE0);
  // // set timer0 counter initial value to 0
  // TCNT0=0x00;
  // // start timer0 with /1024 prescaler
  // TCCR0A = (1<<CS02) | (1<<CS00);

  // sei();


	display_color(GOLD, BLACK);
  tfp_printf("Attempting WiFi\n");



  int timer = 0;


  while(1){
		printf("AT+CWMODE?\r\n");



    if(timer% 1000 == 0){
      char s[100];
      rx(s,100);
      tfp_printf("%s\n", s);
    }

    if(timer == INT_MAX){
      timer = 0;
    } else {
      timer++;
    }
  }

  return 0;
}
