#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

const char* password = "neV3na4E";
const char* ssid = "kolasz_iot";

const byte kthPin = D5;
const byte bthPin = D6;

const byte atxPowerPin = D1;
const byte atxStatePin = A0;

bool kthAtxLock = 0;
bool bthAtxLock = 0;
bool bdrRgbAtxLock = 0;
bool lvrRgbAtxLock = 0;

int bthPWM = 0;
int kthPWM = 0;

int brightness = 0;
int state = 0;

ESP8266WebServer server(80);

void setup() {
  Serial.begin(9600);
  Serial.println("IoT");

  pinMode(kthPin, OUTPUT);
  pinMode(bthPin, OUTPUT);
  pinMode(atxPowerPin, OUTPUT);

  pinMode(atxStatePin, INPUT);

  // make sure that led strips are off on startup
  digitalWrite(kthPin, LOW);
  digitalWrite(bthPin, LOW);

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
  server.on("/info", handleInfo);

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
  checkAtxLock();
}

void handleRoot() {
  String payload = "";
  payload += "kthPwm: ";
  payload += String(kthPWM);
  payload += "\nbthPwm: ";
  payload += String(bthPWM);
  payload += "\nkthAtxLock: ";
  payload += String(kthAtxLock);
  payload += "\nbthAtxLock: ";
  payload += String(bthAtxLock);
  payload += "\nbdrRgbAtxLock: ";
  payload += String(bdrRgbAtxLock);
  payload += "\nlvrRgbAtxLock: ";
  payload += String(lvrRgbAtxLock);
  payload += "\natxState: ";
  payload += String(getAtxState());
  payload += "\natxStatus: ";
  payload += String(getAtxStatus());

  server.send(200, "text/plain", payload);
  Serial.println("root");
}

void handleInfo() {
  String payload = "";
  payload += "available parameters (/leds?): \n";
  payload += "'kth' (0-1023)\n";
  payload += "'bth' (0-1023)\n";
  payload += "'bdrrgb' (0|1)\n";
  payload += "'lvrrgb' (0|1)\n";
  payload += "'atx' (0|1)\n";

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

  if(server.hasArg("kth")) {
    brightness = server.arg("kth").toInt();
    if (brightness <= 0) {
      brightness = 0;
      kthAtxLock = 0;
    }
    if (brightness > 1023) {
      brightness = 1023;
    }
    if (brightness > 0) {
      kthAtxLock = 1;
    }
    kthPWM = brightness;
    analogWrite(kthPin, brightness);
    payload = String(kthPWM);
  }
  if(server.hasArg("bth")) {
    brightness = server.arg("bth").toInt();
    if (brightness <= 0) {
      brightness = 0;
      bthAtxLock = 0;
    }
    if (brightness > 1023) {
      brightness = 1023;
    }
    if (brightness > 0) {
      bthAtxLock = 1;
    }
    bthPWM = brightness;
    analogWrite(bthPin, brightness);
    payload = String(bthPWM);
  }
  if(server.hasArg("bdrrgb")) {
    state = server.arg("bdrrgb").toInt();
    if (state == 0) {
      bdrRgbAtxLock = 0;
      payload = "bdrrgblockoff";
    } else if (state == 1) {
      bdrRgbAtxLock = 1;
      payload = "bdrrgblockon";
    } else if (state == 2) {
      bdrRgbAtxLock = !bdrRgbAtxLock
      payload = "toggled";
    }
  }
  if(server.hasArg("lvrrgb")) {
    state = server.arg("lvrrgb").toInt();
    if (state == 0) {
      lvrRgbAtxLock = 0;
      payload = "lvrrgblockoff";
    } else if (state == 1) {
      lvrRgbAtxLock = 1;
      payload = "lvrrgblockon";
    } else if (state == 2) {
      lvrRgbAtxLock = !lvrRgbAtxLock
      payload = "toggled";
    }
  }
  if(server.hasArg("atx")) {
    state = server.arg("atx").toInt();
    if (state == 0) {
      digitalWrite(atxPowerPin, HIGH);
      bdrRgbAtxLock = 0;
      lvrRgbAtxLock = 0;
      bthAtxLock = 0;
      kthAtxLock = 0;
      payload = "atxpoweroff";
    } else {
      digitalWrite(atxPowerPin, LOW);
      bdrRgbAtxLock = 1;
      lvrRgbAtxLock = 1;
      bthAtxLock = 1;
      kthAtxLock = 1;
      payload = "atxpoweron";
    }
  }

  Serial.println(payload);
  server.send(200, "text/plain", payload);
}

bool getAtxStatus(){
  if (analogRead(atxStatePin) > 100) {
    return 1;
  }
  return 0;
}

bool getAtxState(){
  if (digitalRead(atxPowerPin)) {
    return 0;
  }
  return 1;
}

void checkAtxLock(){
  if (kthAtxLock || bthAtxLock || bdrRgbAtxLock || lvrRgbAtxLock){
    digitalWrite(atxPowerPin, LOW);
  } else {
    digitalWrite(atxPowerPin, HIGH);
  }
}
