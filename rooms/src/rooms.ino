#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Switch.h>
#include "ClickButton.h"

#define ONE_WIRE_BUS D3

const byte lvrBtnPin = D1;
const byte bdrBtnPin = D2;

const char* ssid = "kolasz_iot";
const char* password = "neV3na4E";

const byte lvrPin0 = D0;
const byte lvrPin1 = D5;

const byte bdrPin0 = D6;
const byte bdrPin1 = D7;

Switch lvrBtn = Switch(lvrBtnPin);
Switch bdrBtn = Switch(bdrBtnPin);

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

ESP8266WebServer server(80);

void setup() {
  Serial.begin(9600);
  Serial.println("IoT");

  pinMode(lvrPin0, OUTPUT);
  pinMode(lvrPin1, OUTPUT);

  pinMode(bdrPin0, OUTPUT);
  pinMode(bdrPin1, OUTPUT);

  digitalWrite(lvrPin0, HIGH);
  digitalWrite(lvrPin1, HIGH);

  digitalWrite(bdrPin0, HIGH);
  digitalWrite(bdrPin1, HIGH);

  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
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

  server.on("/loff", [](){
    server.send(200, "text/plain", "lON");
    Serial.println("ON");
    digitalWrite(lvrPin0, HIGH);
    digitalWrite(lvrPin1, HIGH);
  });

  server.on("/lon", [](){
    server.send(200, "text/plain", "lOFF");
    Serial.println("OFF");
    digitalWrite(lvrPin0, LOW);
    digitalWrite(lvrPin1, LOW);
  });

  server.on("/boff", [](){
    server.send(200, "text/plain", "bON");
    Serial.println("ON");
    digitalWrite(bdrPin0, HIGH);
    digitalWrite(bdrPin1, HIGH);
  });

  server.on("/bon", [](){
    server.send(200, "text/plain", "bOFF");
    Serial.println("OFF");
    digitalWrite(bdrPin0, LOW);
    digitalWrite(bdrPin1, LOW);
  });

  server.on("/temp", [](){
    DS18B20.requestTemperatures();
    delay(1000);
    String s = String(DS18B20.getTempCByIndex(0));
    s += "\n";
    s += String(DS18B20.getTempCByIndex(1));
    server.send(200, "text/plain", s);
    Serial.println(DS18B20.getTempCByIndex(0));
    Serial.println(DS18B20.getTempCByIndex(1));
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");

}

void loop() {
  server.handleClient();
  lvrBtn.poll();
  bdrBtn.poll();

  // other available options
  if (lvrBtn.switched()) {
    Serial.println("switched lvr");
  }
  // if (lvrBtn.released()) {
  //   Serial.println("released lvt");
  // }

  // if (lvrBtn.pushed()) {
    // Serial.println("pushed lvr");
  // }

  if (lvrBtn.longPress()) {
    Serial.println("longPress lvr");
    digitalWrite(lvrPin0, HIGH);
    digitalWrite(lvrPin1, HIGH);
  }

  if (lvrBtn.doubleClick()) {
    Serial.println("doubleClick lvr");
    digitalWrite(lvrPin0, LOW);
    digitalWrite(lvrPin1, LOW);
  }


  if (bdrBtn.pushed()) {
    Serial.println("pushed bdr");
  }

  if (bdrBtn.longPress()) {
    Serial.println("longPress bdr");
    digitalWrite(bdrPin0, HIGH);
    digitalWrite(bdrPin1, HIGH);
  }

  if (bdrBtn.doubleClick()) {
    Serial.println("doubleClick bdr");
    digitalWrite(bdrPin0, LOW);
    digitalWrite(bdrPin1, LOW);
  }
}

void handleRoot() {
  server.send(200, "text/plain", "IoT rooms driver");
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