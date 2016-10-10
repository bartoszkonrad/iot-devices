#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

const char* password = "neV3na4E";
const char* ssid = "kolasz_iot";

const byte kitchenPin = D5;
const byte rgbPin = D6;
const byte bathroomPin = D7;

const byte atxPowerPin = D1;
const byte atxStatePin = A0;

int bathroomPWM = 0;
int kitchenPWM = 0;

ESP8266WebServer server(80);

void setup() {
  Serial.begin(9600);
  Serial.println("IoT");

  pinMode(kitchenPin, OUTPUT);
  pinMode(rgbPin, OUTPUT);
  pinMode(bathroomPin, OUTPUT);
  pinMode(atxPowerPin, OUTPUT);
  pinMode(atxStatePin, INPUT);

  digitalWrite(kitchenPin, LOW);
  digitalWrite(rgbPin, LOW);
  digitalWrite(bathroomPin, LOW);
  digitalWrite(atxPowerPin, HIGH);

  WiFi.begin(ssid, password);
  Serial.println("");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);
  server.on("/leds", handleLeds);

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}

void handleRoot() {
  String payload = "";
  payload += "kitchen: ";
  payload += String(kitchenPWM);
  payload += "\nbathroom: ";
  payload += String(bathroomPWM);
  payload += "\nrgb: ";
  payload += String(digitalRead(rgbPin));
  payload += "\natx power: ";
  payload += String(digitalRead(atxPowerPin));
  payload += "\natx state: ";
  payload += String(analogRead(atxStatePin));
  payload += "\n";
  payload += "\"http://leds.lan/leds?kitchen=0\" 0-1023";
  payload += "\n";
  payload += "\"http://leds.lan/leds?bathroom=0\" 0-1023";
  payload += "\n";
  payload += "\"http://leds.lan/leds?rgb=0\" 0/1";
  payload += "\n";
  payload += "\"http://leds.lan/leds?atx=0\" 0/1";
  server.send(200, "text/plain", payload);
  Serial.println("root");
}

void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void handleLeds() {
  String payload = "leds driver";

  if(server.hasArg("kitchen")) {
    int brightness = server.arg("kitchen").toInt();
    if (brightness < 0) brightness = 0;
    if (brightness > 1023) brightness = 1023;
    kitchenPWM = brightness;
    analogWrite(kitchenPin, brightness);
    Serial.println("kitchen leds set");
    payload = "kitchen leds set";
  } else if(server.hasArg("bathroom")) {
    int brightness = server.arg("bathroom").toInt();
    if (brightness < 0) brightness = 0;
    if (brightness > 1023) brightness = 1023;
    bathroomPWM = brightness;
    analogWrite(bathroomPin, brightness);
    Serial.println("bathroom leds set");
    payload = "bathroom leds set";
  } else if(server.hasArg("rgb")) {
    int state = server.arg("rgb").toInt();
    if (state == 0) {
      digitalWrite(rgbPin, LOW);
      Serial.println("rgb power off");
      payload = "rgb power off";
    } else {
      digitalWrite(rgbPin, HIGH);
      Serial.println("rgb power on");
      payload = "rgb power on";
    }
  } else if(server.hasArg("atx")) {
    int state = server.arg("atx").toInt();
    if (state == 0) {
      digitalWrite(atxPowerPin, HIGH);
      Serial.println("atx power off");
      payload = "atx power off";
    } else {
      digitalWrite(atxPowerPin, LOW);
      Serial.println("atx power on");
      payload = "atx power on";
    }
  }
  else {
    Serial.println("parameter not found");
    payload = "parameter not found";
  }

  Serial.println(payload);
  server.send(200, "text/plain", payload);
}
