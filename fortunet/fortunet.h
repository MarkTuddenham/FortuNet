#include <stdio.h>
#include <stdbool.h>
#include <avr/io.h>
#include <util/delay.h>

/*
<str> char buffer
<nchars> length of char buffer
*/
void rx(char str[], int nchars);

/*
<ssid>: string parameter, SSID of AP.
<pwd>: string parameter, length of password: 8 ~ 64
bytes ASCII.
<chl>: channel ID.
<ecn>: encryption method; WEP is not supported.
‣ 0: OPEN
‣ 2: WPA_PSK
‣ 3: WPA2_PSK
‣ 4: WPA_WPA2_PSK
*/
void setAsAP(char* ssid, char* pwd, int chl, int ecn);

/*
<ssid>: the SSID of the target AP.
<pwd>: password, MAX: 64-byte ASCII.
<error code>: (for reference only, returned by ESP8266)
‣ 1: connection timeout.
‣ 2: wrong password.
‣ 3: cannot find the targ
*/
void connectToAP(char* ssid, char* pwd);

/*
<time>: the duration of ESP8266’s sleep. Unit: ms.
*/
void sleepWiFi(int time);

/*
<type>: string parameter indicating the connection type: "TCP", "UDP"	or "SSL".
<ip>: string parameter indicating the remote IP address.
<port>: the remote port number
*/
void connectToServer(char* type, char* ip, char* port);
void dissconect();

/*
<data> data to send over connection
<length> size of data
*/
void sendString(char* data, int length);

/*
<length> size of data
*/
void prepareSend(int length);

void init_uart();
void resetWiFi();
void enableEcho();
void dissableEcho();


int uputchar(char c, FILE *stream);
int ugetchar(FILE *stream);
