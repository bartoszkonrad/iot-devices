#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

const byte bdrBtnPin = D1;
const byte lvrBtnPin = D2;

const char* ssid = "kolasz_iot";
const char* password = "neV3na4E";

const byte lvrPin0 = D6;
const byte lvrPin1 = D7;

const byte bdrPin0 = D5;
const byte bdrPin1 = D0;

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
  server.on("/light", handleLight);

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

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}

void handleRoot() {
  String payload = "";
  payload += "\nState logic inverted (HIGH means OFF and so on)";
  payload += "\nbdr0: ";
  payload += String(digitalRead(bdrPin0));
  payload += "\nbdr1: ";
  payload += String(digitalRead(bdrPin1));
  payload += "\nlvr0: ";
  payload += String(digitalRead(lvrPin0));
  payload += "\nlvr1: ";
  payload += String(digitalRead(lvrPin1));
  payload += "\n";
  payload += "<a href=\"http://rooms.lan/light?bdr=1\">Turn ON bedroom</a>";
  payload += "\n";
  payload += "<a href=\"http://rooms.lan/light?bdr=0\">Turn OFF bedroom</a>";
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

void handleLight() {
  String payload = "lamps driver";
  if(server.hasArg("bdr0")) {
    int state = server.arg("bdr0").toInt();
    if (state == 0) {
      digitalWrite(bdrPin0, HIGH);
    } else {
      digitalWrite(bdrPin0, LOW);
    }
    Serial.println("Bedroom lamp0");
  }
  if(server.hasArg("bdr1")) {
    int state = server.arg("bdr1").toInt();
    if (state == 0) {
      digitalWrite(bdrPin1, HIGH);
    } else {
      digitalWrite(bdrPin1, LOW);
    }    Serial.println("Bedroom lamp1");
  }
  if(server.hasArg("bdr")) {
    int state = server.arg("bdr").toInt();
    if (state == 0) {
      digitalWrite(bdrPin0, HIGH);
      digitalWrite(bdrPin1, HIGH);
    } else {
      digitalWrite(bdrPin0, LOW);
      digitalWrite(bdrPin1, LOW);
    }
    Serial.println("Both bedroom lamps");
  }
  if(server.hasArg("lvr0")) {
    int state = server.arg("lvr0").toInt();
    if (state == 0) {
      digitalWrite(lvrPin0, HIGH);
    } else {
      digitalWrite(lvrPin0, LOW);
    }
    Serial.println("Living room lamp0");
  }
  if(server.hasArg("lvr1")) {
    int state = server.arg("lvr1").toInt();
    if (state == 0) {
      digitalWrite(lvrPin1, HIGH);
    } else {
      digitalWrite(lvrPin1, LOW);
    }
    Serial.println("Living room lamp1");
  }
  if(server.hasArg("lvr")) {
    int state = server.arg("lvr").toInt();
    if (state == 0) {
      digitalWrite(lvrPin0, HIGH);
      digitalWrite(lvrPin1, HIGH);
    } else {
      digitalWrite(lvrPin0, LOW);
      digitalWrite(lvrPin1, LOW);
    }
    Serial.println("Both living room lamps");
  }
  Serial.println(payload);
  server.send(200, "text/plain", payload);
}
