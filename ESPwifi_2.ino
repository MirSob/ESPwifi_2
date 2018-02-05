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
long godzina = 0;
long GODZINA = 2400; //144000;
const char* host = "10.1.1.213"; //it will tell you the IP once it starts up
																				//just write it here afterwards and upload
int ledPin = D8;
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
	pinMode(D6, OUTPUT);
	pinMode(D7, OUTPUT);
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
	digitalWrite(D6, !digitalRead(D6));
	//analogWrite(ledPin5, sl5Value);
	ESP.wdtFeed();
	if (digitalRead(D6))
		Serial.write("|");
	else
		Serial.write("*");

	//next = ESP.getCycleCount() + 999450;
	ESP.wdtFeed();
	//timer0_write(next);
	interrupts();
}

void inline blinkISR_t1(void) {
	if (godzina > GODZINA){
		digitalWrite(ledPin, LOW);
		digitalWrite(D7, LOW);
		digitalWrite(D6, LOW);
		return;
	}
	godzina++;

	ESP.wdtFeed();
	noInterrupts();
	//	digitalWrite(D1, !digitalRead(D1));
	//	digitalWrite(D2, !digitalRead(D2));
	digitalWrite(D7, !digitalRead(D7));
	digitalWrite(D6, !digitalRead(D6));
	//analogWrite(ledPin5, sl5Value);
	if (digitalRead(D6)) {
		if (!offserial) Serial.write("#");
	}
	else
		if (!offserial) Serial.write("r");
	godzina++;
	interrupts();
}

uint8 srvst_last = 0;
void loop() {
	// Check if a client has connected
	String answer;
	uint8 srvst = server.status();
	if (srvst_last != srvst) {
		Serial.printf("Connection status: %d\n", srvst);
		srvst_last = srvst;
	}

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
	Serial.println(req);
	// Match the request
	if (req.indexOf("") != -10) {  //checks if you're on the main page

		if (req.indexOf("/OFF") != -1) { //checks if you clicked OFF
			digitalWrite(ledPin, LOW);
			digitalWrite(D7, LOW);
			digitalWrite(D6, LOW);
			Serial.println("You clicked OFF");
			godzina = 1 + GODZINA;
		}
		if (req.indexOf("/ON") != -1) { //checks if you clicked ON
			digitalWrite(ledPin, HIGH);
			Serial.println("You clicked ON");
			godzina = 0;
		}
		if (godzina > GODZINA) {
			answer = "<div style=\"font-size:32px\" class=\"IO_box bOff\" > <br>You clicked OFF</div>";
		}
		else {
			answer = "<div style=\"font-size:32px\" class=\"IO_box bOn\" > <br>You clicked ON</div>";
		}
	}

	else {
		Serial.println("invalid request");
		answer = "<div style=\"font-size:32px\" class=\"IO_box\" > <br> Invalid request</div>";
//		client.stop();
//		return;
	}

	// Prepare the response
	String head = "HTTP/1.1 200 OK\r\n";
	head += "Content-Type: text/html\r\n\r\n";
	head += "<!DOCTYPE HTML>"
"<html>"
"<style>"
".IO_box {"
"float: left;"
"padding: 1px 1px 1px 1px;"
"text-align: center;"
"width: 95%;}"
""
".button {"
    "background-color: #844444; "
    "border: none;"
    "color: white;"
    "padding: 52px 20px;"
    "font-size: 48px;"
    "margin: 4px 2px;"
    "cursor: pointer;"
    "border-radius: 12px;"
		"width: 100%;"
"}"
".bOn { background-color: green; }"
".bOff { background-color: red; }"
"</style>"
"<body>"
"<div class=\"IO_box\" >"
"<input class=\"button bOn\" type=\"button\" name=\"bl\" value=\"Turn LED ON \" onclick=\"location.href='/ON'\">"
"</div>"
"<div class=\"IO_box\" >"
"<input class=\"button bOff\" type=\"button\" name=\"bl\" value=\"Turn LED OFF\" onclick=\"location.href='/OFF'\">"
"</div>";

String footer = "</body></html>";
	
	String page = "<!DOCTYPE html>"
"<html>"
    "<head>"
        "<title>Arduino Ajax LED Button Control</title>"
        "<script>"
        "strLED1 = \"\";"
        "strLED2 = \"\";"
        "var LED2_state = 0;"
        "function GetArduinoIO()"
        "{"
            "nocache = \"&nocache=\" + Math.random() * 1000000;"
            "var request = new XMLHttpRequest();"
            "request.onreadystatechange = function()"
            "{"
                "if (this.readyState == 4) {"
                    "if (this.status == 200) {"
                        "if (this.responseXML != null) {"
                            "// XML file received - contains analog values, switch values and LED states"
                            "var count;"
                            "// LED 1"
                            "if (this.responseXML.getElementsByTagName('LED')[0].childNodes[0].nodeValue === \"checked\") {"
                                "document.LED_form.LED1.checked = true;"
                            "}"
                            "else {"
                                "document.LED_form.LED1.checked = false;"
                            "}"
                            "// LED 2"
                            "if (this.responseXML.getElementsByTagName('LED')[1].childNodes[0].nodeValue === \"on\") {"
                                "document.getElementById(\"LED2\").innerHTML = \"LED 2 is ON (D7)\";"
                                "LED2_state = 1;"
                            "}"
                            "else {"
                                "document.getElementById(\"LED2\").innerHTML = \"LED 2 is OFF (D7)\";"
                                "LED2_state = 0;"
                            "}"
                        "}"
                    "}"
                "}"
            "}"
            "// send HTTP GET request with LEDs to switch on/off if any"
            "request.open(\"GET\", \"ajax_inputs\" + strLED1 + strLED2 + nocache, true);"
            "request.send(null);"
            "setTimeout('GetArduinoIO()', 1000);"
            "strLED1 = \"\";"
            "strLED2 = \"\";"
        "}"
        "// service LEDs when checkbox checked/unchecked"
        "function GetCheck()"
        "{"
            "if (LED_form.LED1.checked) {"
                "strLED1 = \"&LED1=1\";"
            "}"
            "else {"
                "strLED1 = \"&LED1=0\";"
            "}"
        "}"
        "function GetButton1()"
        "{"
            "if (LED2_state === 1) {"
                "LED2_state = 0;"
                "strLED2 = \"&LED2=0\";"
            "}"
            "else {"
                "LED2_state = 1;"
                "strLED2 = \"&LED2=1\";"
            "}"
        "}"
    "</script>"
    "<style>"
        ".IO_box {"
            "float: left;"
            "margin: 0 20px 20px 0;"
            "border: 1px solid blue;"
            "padding: 0 5px 0 5px;"
            "width: 120px;"
        "}"
        "h1 {"
            "font-size: 120%;"
            "color: blue;"
            "margin: 0 0 10px 0;"
        "}"
        "h2 {"
            "font-size: 85%;"
            "color: #5734E6;"
            "margin: 5px 0 5px 0;"
        "}"
        "p, form, button {"
            "font-size: 80%;"
            "color: #252525;"
        "}"
        ".small_text {"
            "font-size: 70%;"
            "color: #737373;"
        "}"
    "</style>"
    "</head>"
    "<body onload=\"GetArduinoIO()\">"
        "<h1>Arduino Ajax LED Button Control</h1>"
        "<div class=\"IO_box\">"
            "<h2>LED Using Checkbox</h2>"
            "<form id=\"check_LEDs\" name=\"LED_form\">"
                "<input type=\"checkbox\" name=\"LED1\" value=\"0\" onclick=\"GetCheck()\" />LED 1 (D6)<br /><br />"
            "</form>"
        "</div>"
        "<div class=\"IO_box\">"
            "<h2>LED Using Button</h2>"
            "<button type=\"button\" id=\"LED2\" onclick=\"GetButton1()\">LED 2 is OFF (D7)</button><br /><br />"
        "</div>"
    "</body>"
"</html>";


	client.flush();


	// Send the response to the client
	client.print(head + answer + footer);
	delay(1);



}
