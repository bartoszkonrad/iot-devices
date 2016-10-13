#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

const char* password = "neV3na4E";
const char* ssid = "kolasz_iot";

const byte kitchenPin = D5;
const byte bathroomPin = D6;

const byte atxPowerPin = D1;
const byte atxStatePin = A0;

bool kitchenAtxLock = 0;
bool bathroomAtxLock = 0;
bool rgbAtxLock = 0;

int bathroomPWM = 0;
int kitchenPWM = 0;

int brightness = 0;
int state = 0;

ESP8266WebServer server(80);

void setup() {
  Serial.begin(9600);
  Serial.println("IoT");

  pinMode(kitchenPin, OUTPUT);
  pinMode(bathroomPin, OUTPUT);
  pinMode(atxPowerPin, OUTPUT);
  pinMode(atxStatePin, INPUT);

  digitalWrite(kitchenPin, LOW);
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
  checkAtxLock();
}

void handleRoot() {
  String payload = "";
  payload += "kitchen: ";
  payload += String(kitchenPWM);
  payload += "\nbathroom: ";
  payload += String(bathroomPWM);
  payload += "\nkitchenAtxLock: ";
  payload += String(kitchenAtxLock);
  payload += "\nbathroomAtxLock: ";
  payload += String(bathroomAtxLock);
  payload += "\nrgbAtxLock: ";
  payload += String(rgbAtxLock);
  payload += "\natxState: ";
  payload += String(getAtxState());
  payload += "\natxStatus: ";
  payload += String(getAtxStatus());

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
  String payload = "leds and atx driver";

  if(server.hasArg("kitchen")) {
    brightness = server.arg("kitchen").toInt();
    if (brightness <= 0) {
      brightness = 0;
      kitchenAtxLock = 0;
    }
    if (brightness > 1023) {
      brightness = 1023;
    }
    if (brightness > 0) {
      kitchenAtxLock = 1;
    }
    kitchenPWM = brightness;
    analogWrite(kitchenPin, brightness);
    Serial.println("kitchen leds set");
    payload = "kitchen leds set";
    } else if(server.hasArg("bathroom")) {
    brightness = server.arg("bathroom").toInt();
    if (brightness <= 0) {
      brightness = 0;
      bathroomAtxLock = 0;
    }
    if (brightness > 1023) {
      brightness = 1023;
    }
    if (brightness > 0) {
      bathroomAtxLock = 1;
    }
    bathroomPWM = brightness;
    analogWrite(bathroomPin, brightness);
    Serial.println("bathroom leds set");
    payload = "bathroom leds set";
    } else if(server.hasArg("rgb")) {
    state = server.arg("rgb").toInt();
    if (state == 0) {
      rgbAtxLock = 0;
      Serial.println("rgb power off");
      payload = "rgb power off";
    } else {
      rgbAtxLock = 1;
      Serial.println("rgb power on");
      payload = "rgb power on";
    }
  } else if(server.hasArg("atx")) {
    state = server.arg("atx").toInt();
    if (state == 0) {
      digitalWrite(atxPowerPin, HIGH);
      rgbAtxLock = 0;
      bathroomAtxLock = 0;
      kitchenAtxLock = 0;
      Serial.println("atx power off");
      payload = "atx power off";
    } else {
      digitalWrite(atxPowerPin, LOW);
      rgbAtxLock = 1;
      bathroomAtxLock = 1;
      kitchenAtxLock = 1;
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
  if (kitchenAtxLock || bathroomAtxLock || rgbAtxLock){
    digitalWrite(atxPowerPin, LOW);
  } else {
    digitalWrite(atxPowerPin, HIGH);
  }
}
