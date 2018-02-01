#include <WiFiUdp.h>
#include <WiFiServer.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>
#include <ESP8266WiFiType.h>
#include <ESP8266WiFiSTA.h>
#include <ESP8266WiFiScan.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFiGeneric.h>
#include <ESP8266WiFiAP.h>
char ssid[] = "SIC4NET";
char password[] = "karmenw0parach";
#include <ESP8266WiFi.h>

const char* host = "10.1.1.213"; //it will tell you the IP once it starts up
																				//just write it here afterwards and upload
int ledPin = D7;
volatile bool pin_state = false;
volatile bool offserial = false;
volatile unsigned long next;
volatile unsigned long interruptCounter;

void inline offSerial(void);

void inline blinkISR_t1(void);
WiFiServer server(80); //just pick any port number you like

void setup() {
	Serial.begin(115200);
	delay(10);
	Serial.println(WiFi.localIP());
	// prepare GPIO2
	pinMode(ledPin, OUTPUT);
	digitalWrite(D3, LOW);

	// Connect to WiFi network
	Serial.println();
	Serial.println();
	Serial.print("Connecting to ");
	Serial.println(ssid);

	WiFi.begin(ssid, password);

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	Serial.println("");
	Serial.println("WiFi connected");

	// Start the server
	server.begin();
	Serial.println("Server started");

	// Print the IP address
	Serial.println(WiFi.localIP());
	pinMode(D3, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(D3), offSerial, FALLING);
	noInterrupts();
	// timer0_isr_init();
	//timer0_attachInterrupt(blinkISR);

	//next = ESP.getCycleCount() + 1000;

	//timer0_write(next);

	timer1_isr_init();
	timer1_attachInterrupt(blinkISR_t1);
	timer1_enable(TIM_DIV265, TIM_EDGE, TIM_LOOP);
	timer1_write(3906);
	//	 redkea.begin(ssid, pass, deviceID);
	//Set up ESP watchdog
	//	ESP.wdtDisable();
	//	 ESP.wdtEnable(WDTO_8S);

}

void inline offSerial(void) {
	noInterrupts();
	offserial = !offserial;
	interruptCounter++;
	interrupts();
}

void inline blinkISR_t0(void) {
	noInterrupts();

	//digitalWrite(D1, !digitalRead(D1));
	//digitalWrite(D2, !digitalRead(D2));
	digitalWrite(D7, !digitalRead(D7));
	digitalWrite(D5, !digitalRead(D5));
	//analogWrite(ledPin5, sl5Value);
	ESP.wdtFeed();
	if (digitalRead(D5))
		Serial.write("|");
	else
		Serial.write("*");

	//next = ESP.getCycleCount() + 999450;
	ESP.wdtFeed();
	//timer0_write(next);
	interrupts();
}

void inline blinkISR_t1(void) {
	ESP.wdtFeed();
	noInterrupts();
	//	digitalWrite(D1, !digitalRead(D1));
	//	digitalWrite(D2, !digitalRead(D2));
	digitalWrite(D7, !digitalRead(D7));
	digitalWrite(D5, !digitalRead(D5));
	//analogWrite(ledPin5, sl5Value);
	if (digitalRead(D5)) {
		if (!offserial) Serial.write("#");
	}
	else
		if (!offserial) Serial.write("r");

	ESP.wdtFeed();
	interrupts();
}


void loop() {
	// Check if a client has connected

	WiFiClient client = server.available();
	if (!client) {
		return;
	}

	// Wait until the client sends some data
	while (!client.available()) {
		delay(1);
	}

	// Read the first line of the request
	String req = client.readStringUntil('\r');
	client.flush();

	// Match the request
	if (req.indexOf("") != -10) {  //checks if you're on the main page

		if (req.indexOf("/OFF") != -1) { //checks if you clicked OFF
			digitalWrite(ledPin, LOW);
			Serial.println("You clicked OFF");
		}
		if (req.indexOf("/ON") != -1) { //checks if you clicked ON
			digitalWrite(ledPin, HIGH);
			Serial.println("You clicked ON");
		}
	}

	else {
		Serial.println("invalid request");
		client.stop();
		return;
	}

	// Prepare the response
	String s = "HTTP/1.1 200 OK\r\n";
	s += "Content-Type: text/html\r\n\r\n";
	s += "<!DOCTYPE HTML>\r\n<html>\r\n";
	s += "<br><input type=\"button\" name=\"bl\" value=\"Turn LED ON \" onclick=\"location.href='/ON'\">";
	s += "<br><br><br>";
	s += "<br><input type=\"button\" name=\"bl\" value=\"Turn LED OFF\" onclick=\"location.href='/OFF'\">";
	s += "</html>\n";

	client.flush();


	// Send the response to the client
	client.print(s);
	delay(1);



}
