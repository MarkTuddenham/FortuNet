#include "fortunet.h"

//initial baud is 115200 -> set to 9600 with AT+UART_DEF=9600,8,1,0,0
#define BAUD  9600


int uputchar(char c, FILE *stream){
	while (!(UCSR1A & _BV(UDRE1)));
	UDR1 = c;
	return c;
}

int ugetchar(FILE *stream){
	while(!(UCSR1A & _BV(RXC1)));
	return UDR1;
}

void init_uart(){
	/* Configure UART1 baud rate, one start bit, 8-bit, no parity and one stop bit */
	UBRR1H = (F_CPU/(BAUD*16L)-1) >> 8;
	UBRR1L = (F_CPU/(BAUD*16L)-1);
	UCSR1B = _BV(RXEN1) | _BV(TXEN1);
	UCSR1C = _BV(UCSZ10) | _BV(UCSZ11);

	/* Setup new streams for input and output */
	static FILE uout = FDEV_SETUP_STREAM(uputchar, NULL, _FDEV_SETUP_WRITE);
	static FILE uin = FDEV_SETUP_STREAM(NULL, ugetchar, _FDEV_SETUP_READ);

	/* Redirect all standard streams to UART1 */
	stdout = &uout;
	stderr = &uout;
	stdin  = &uin;
}

void rx(char str[], int nchars) {
  int c, i = 0;
  while((c = getchar()) != '\n' && c != EOF ) {
    if (i > nchars - 1) break;
    str[i++] = c;
  }
  str[i] = '\0';
}

void setAsAP(char* ssid, char* pwd, int chl, int ecn){
	printf("AT+CWMODE=2\r\n");
	_delay_ms(200);
	printf("AT+CWSAP=\"%s\",\"%s\",%d,%d\r\n", ssid, pwd, chl, ecn);
}

void connectToAP(char* ssid, char* pwd){
	printf("AT+CWMODE=1\r\n");
	_delay_ms(200);
	printf("AT+CWJAP=\"%s\",\"%s\"\r\n", ssid, pwd);

}

void connectToServer(char* type, char* ip, char* port){
	printf("AT+CIPSTART=\"%s\",\"%s\",%s\r\n", type, ip, port);
}
void dissconect(){
	printf("AT+CIPCLOSE\r\n");
}

void sendString(char* data, int length){
	prepareSend(length);
	_delay_ms(100);
	printf("%s\r\n",data);
}

void prepareSend(int length){
	printf("AT+CIPSEND=%d\r\n",length);
}

void connectionStatus(){
	printf("AT+CIPSTATUS\r\n");
}

void sleepWiFi(int time){
	printf("AT+GSLP=%i\r\n", time);
}

void resetWiFi(){
	printf("AT+RST\r\n");
}

void enableEcho(){
	printf("ATE1\r\n");
}

void dissableEcho(){
	printf("ATE0\r\n");
}
