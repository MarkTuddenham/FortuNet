
#include <avr/io.h>
#include <avr/interrupt.h>
#include <limits.h>
#include "lcd.h"
#include "fortunet.h"


#define lcd_print tfp_printf


// timer0 overflow
ISR(TIMER0_COMPA_vect ) {
  static int divisor = 1, counter = 0;

  if(divisor%10000 == 0){
    counter++;
        prepareSend(2);
        printf("%d\r\n",counter++);
        lcd_print("%d\n",counter);
  }
  if(divisor == INT_MAX) divisor = 0;
  divisor++;
}

int main(void) {
  /* 8MHz clock, no prescaling (DS, p. 48) */
  CLKPR = (1 << CLKPCE);
  CLKPR = 0;

  //
  TCCR0A = _BV(WGM01);
	TCCR0B = _BV(CS01) | _BV(CS00);   /* F_CPU / 64 */

  /* SET OCR0A FOR A 1 MS PERIOD */
	OCR0A = (uint8_t)(F_CPU / (64.0 * 1000) - 0.5);

    /* ENABLE TIMER INTERRUPT */
  TIMSK0 |= _BV(OCIE0A);

  init_uart();
  init_lcd();


	display_color(CYAN, BLACK);
  lcd_print("Testing WiFi\n");

  connectToAP("Fuck me its wifi", "leakyCeiling2");
  _delay_ms(100);
  connectToServer("TCP", "192.168.0.37", "55555");
  _delay_ms(100);

  sei();
  char s[100];

  while(1){
    rx(s,100);
    tfp_printf("%s\n", s);
  }

  return 0;
}
