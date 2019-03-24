#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "DHT.h"

/* Put your SSID & Password */
const char* ssid = "WebServer";  // Enter SSID here
const char* password = "12345678";  //Enter Password here
#define DHTTYPE DHT11   

/* Put IP Address details */
IPAddress local_ip(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

ESP8266WebServer server(80);

const uint8_t LED1pin = 13; //d7
bool LED1status = LOW;

const uint8_t LED2pin = 12; //d6
bool LED2status = LOW;

const int DHTPin = 4; //d2
DHT dht(DHTPin, DHTTYPE);

const uint8_t AlarmLEDpin = 5; //d1
bool temepture_alarm = false;


static char celsiusTemp[7];
static char humidityTemp[7];

int delay_counter = 0;
int temperature_treshold = 30;
int temperature = 0;
int lux = 0;

void readLDR()
{
  int val = analogRead(A0);
  lux = map(val, 330, 1024, 0, 100);
  //lux = val;
}

void readDHT11()
{
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  temperature = (int)t;
  if (isnan(h) || isnan(t) ) {
    Serial.println("Failed to read from DHT sensor!");
    //strcpy(celsiusTemp, "Failed");
    //strcpy(humidityTemp, "Failed");
  }            else {
    //float hic = dht.computeHeatIndex(t, h, false);
    dtostrf(t, 6, 2, celsiusTemp);
    dtostrf(h, 6, 2, humidityTemp);
  }

}

void temperatureAlarmControl()
{
  if (temperature >= temperature_treshold)
  {
    digitalWrite(AlarmLEDpin, !digitalRead(AlarmLEDpin));
    temepture_alarm = true;
  } else if (temperature <= temperature_treshold - 2) {
    digitalWrite(AlarmLEDpin, LOW);
    temepture_alarm = false;
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED1pin, OUTPUT);
  pinMode(LED2pin, OUTPUT);
  pinMode(AlarmLEDpin, OUTPUT);

  pinMode(LED_BUILTIN, OUTPUT);

  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(100);

  server.on("/", handle_OnConnect);
  server.on("/led1on", handle_led1on);
  server.on("/led1off", handle_led1off);
  server.on("/led2on", handle_led2on);
  server.on("/led2off", handle_led2off);

  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("HTTP server started");
}
void loop() {
  server.handleClient();
  if (LED1status)
  {
    digitalWrite(LED1pin, HIGH);
  }
  else
  {
    digitalWrite(LED1pin, LOW);
  }

  if (LED2status)
  {
    digitalWrite(LED2pin, HIGH);
  }
  else
  {
    digitalWrite(LED2pin, LOW);
  }
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  delay(100);
  delay_counter++;
  temperatureAlarmControl();
  if (delay_counter == 4)
  {
    delay_counter = 0;
    readDHT11();
    readLDR();
  }
}

void handle_OnConnect() {
  LED1status = LOW;
  LED2status = LOW;
  Serial.println("GPIO7 Status: OFF | GPIO6 Status: OFF");
  server.send(200, "text/html", SendHTML(LED1status, LED2status));
}

void handle_led1on() {
  LED1status = HIGH;
  Serial.println("GPIO7 Status: ON");
  server.send(200, "text/html", SendHTML(true, LED2status));
}

void handle_led1off() {
  LED1status = LOW;
  Serial.println("GPIO7 Status: OFF");
  server.send(200, "text/html", SendHTML(false, LED2status));
}

void handle_led2on() {
  LED2status = HIGH;
  Serial.println("GPIO6 Status: ON");
  server.send(200, "text/html", SendHTML(LED1status, true));
}

void handle_led2off() {
  LED2status = LOW;
  Serial.println("GPIO6 Status: OFF");
  server.send(200, "text/html", SendHTML(LED1status, false));
}

void handle_NotFound() {
  server.send(404, "text/plain", "Not found");
}

String SendHTML(uint8_t led1stat, uint8_t led2stat) {
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr += "<head><meta http-equiv=\"refresh\" content=\"2\", name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr += "<title>LED Control</title>\n";
  ptr += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr += "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr += ".button {display: block;width: 80px;background-color: #1abc9c;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr += ".button-on {background-color: #1abc9c;}\n";
  ptr += ".button-on:active {background-color: #16a085;}\n";
  ptr += ".button-off {background-color: #34495e;}\n";
  ptr += ".button-off:active {background-color: #2c3e50;}\n";
  ptr += "p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr += "</style>\n";
  ptr += "</head>\n";
  ptr += "<body>\n";
  ptr += "<h1>Eltemtek ESP8266 Web Server</h1>\n";
  ptr += "<h3>Using Access Point(AP) Mode</h3>\n";
  ptr += "<h3>Temperature: " + String(celsiusTemp) + "C Humidity : %" + String(humidityTemp) + "</h3>\n";
  ptr += "<h3>Light intensity: " + String(lux)  + "</h3>\n";





  if (led1stat)
  {
    ptr += "<p>LED1 Status: ON</p><a class=\"button button-off\" href=\"/led1off\">OFF</a>\n";
  }
  else
  {
    ptr += "<p>LED1 Status: OFF</p><a class=\"button button-on\" href=\"/led1on\">ON</a>\n";
  }

  if (led2stat)
  {
    ptr += "<p>LED2 Status: ON</p><a class=\"button button-off\" href=\"/led2off\">OFF</a>\n";
  }
  else
  {
    ptr += "<p>LED2 Status: OFF</p><a class=\"button button-on\" href=\"/led2on\">ON</a>\n";
  }

  if (temepture_alarm)
  {
    ptr += "<h2><font color=\"#b2140c\">Alarm: Temperature is higher then 30C </h2>\n";
  }

  ptr += "</body>\n";
  ptr += "</html>\n";
  return ptr;
}
