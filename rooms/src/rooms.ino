#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ClickButton.h>

#define ONE_WIRE_BUS D3

const byte bdrBtnPin = D1;
const byte lvrBtnPin = D2;

const char* ssid = "kolasz_iot";
const char* password = "neV3na4E";

const byte lvrPin0 = D6;
const byte lvrPin1 = D7;

const byte bdrPin0 = D5;
const byte bdrPin1 = D0;

char lvrTempString[5];
char bdrTempString[5];

int httpStatusCode;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

ClickButton bdrBtn(bdrBtnPin, LOW, CLICKBTN_PULLUP);
ClickButton lvrBtn(lvrBtnPin, LOW, CLICKBTN_PULLUP);

WiFiClient client;
const int httpPort = 80;
const char* host = "iot-atx.lan";

ESP8266WebServer server(80);


void setup() {

  Serial.begin(115200);
  Serial.println("IoT");

  pinMode(lvrPin0, OUTPUT);
  pinMode(lvrPin1, OUTPUT);

  pinMode(bdrPin0, OUTPUT);
  pinMode(bdrPin1, OUTPUT);

  digitalWrite(lvrPin0, HIGH);
  digitalWrite(lvrPin1, HIGH);

  digitalWrite(bdrPin0, HIGH);
  digitalWrite(bdrPin1, HIGH);

  bdrBtn.debounceTime   = 4;
  bdrBtn.multiclickTime = 300;
  bdrBtn.longClickTime  = 600;

  lvrBtn.debounceTime   = 4;
  lvrBtn.multiclickTime = 300;
  lvrBtn.longClickTime  = 600;

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
  server.on("/temp", handleTemp);
  server.on("/info", handleInfo);

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();

  bdrBtn.Update();
  lvrBtn.Update();

  if (bdrBtn.clicks == 1) {
    if (!digitalRead(bdrPin0) || !digitalRead(bdrPin1)) {// reversed logic, false mean on
      digitalWrite(bdrPin0, HIGH);
      digitalWrite(bdrPin1, HIGH);
    } else {
      digitalWrite(bdrPin0, LOW);
    }
    Serial.println("bdr single");
  }

  if (bdrBtn.clicks == 2) {
    if (!digitalRead(bdrPin1)) {
      digitalWrite(bdrPin1, HIGH);
    } else if (digitalRead(bdrPin0) || digitalRead(bdrPin1)) {
      digitalWrite(bdrPin0, LOW);
      digitalWrite(bdrPin1, LOW);
    }
    Serial.println("bdr double");
  }

  // blink even faster if triple clicked
  if (bdrBtn.clicks == -1) {
    if (!client.connect(host, httpPort)) {
      Serial.println("connection failed");
      return;
    }

    String url = "/leds";
    url += "?bdrrgb=2";

    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
    "Host: " + host + "\r\n" +
    "Connection: close\r\n\r\n");

    Serial.println("bdr hold");
  }

  if (lvrBtn.clicks == 1) {
    if (!digitalRead(lvrPin0) || !digitalRead(lvrPin1)) {// reversed logic, false mean on
      digitalWrite(lvrPin0, HIGH);
      digitalWrite(lvrPin1, HIGH);
    } else {
      digitalWrite(lvrPin0, LOW);
    }
    Serial.println("lvr single");
  }

  if (lvrBtn.clicks == 2) {
    if (!digitalRead(lvrPin1)) {
      digitalWrite(lvrPin1, HIGH);
    } else if (digitalRead(lvrPin0) || digitalRead(lvrPin1)) {
      digitalWrite(lvrPin0, LOW);
      digitalWrite(lvrPin1, LOW);
    }
    Serial.println("lvr double");
  }

  // blink even faster if triple clicked
  if (lvrBtn.clicks == -1) {
    if (!client.connect(host, httpPort)) {
      Serial.println("connection failed");
      return;
    }

    String url = "/leds";
    url += "?lvrrgb=2";

    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
    "Host: " + host + "\r\n" +
    "Connection: close\r\n\r\n");

    Serial.println("lvr hold");
  }
}

void handleRoot() {
  String payload = "";
  httpStatusCode = 200;

  DS18B20.requestTemperatures();
  delay(1000);
  dtostrf(DS18B20.getTempCByIndex(0), 2, 1, bdrTempString);
  dtostrf(DS18B20.getTempCByIndex(1), 2, 1, lvrTempString);
  // payload = "bdrTemp: " + String(DS18B20.getTempCByIndex(0));
  payload = "bdrTemp: " + String(bdrTempString);
  payload += "\n";
  // payload += "lvrTemp: " + String(DS18B20.getTempCByIndex(1));
  payload += "lvrTemp: " + String(lvrTempString);
  payload += "\nbdr0: ";
  payload += digitalRead(bdrPin0)?"0":"1";
  payload += "\nbdr1: ";
  payload += digitalRead(bdrPin1)?"0":"1";
  payload += "\nlvr0: ";
  payload += digitalRead(lvrPin0)?"0":"1";
  payload += "\nlvr1: ";
  payload += digitalRead(lvrPin1)?"0":"1";

  server.send(httpStatusCode, "text/plain", payload);
}

void handleInfo() {
  String payload = "";
  httpStatusCode = 200;

  payload += "available parameters (/light?): \n";
  payload += "'bdr' (0|1)\n";
  payload += "'bdr0' (0|1)\n";
  payload += "'bdr1' (0|1)\n";
  payload += "'lvr' (0|1)\n";
  payload += "'lvr0' (0|1)\n";
  payload += "'lvr1' (0|1)\n";

  server.send(httpStatusCode, "text/plain", payload);
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
  httpStatusCode = 200;

  if(server.hasArg("bdr0")) {
    int state = server.arg("bdr0").toInt();
    if (state == 0) {
      digitalWrite(bdrPin0, HIGH);
    } else {
      digitalWrite(bdrPin0, LOW);
    }
    payload = digitalRead(bdrPin0)?"0":"1";
  }
  if(server.hasArg("bdr1")) {
    int state = server.arg("bdr1").toInt();
    if (state == 0) {
      digitalWrite(bdrPin1, HIGH);
    } else {
      digitalWrite(bdrPin1, LOW);
  }
  payload = digitalRead(bdrPin1)?"0":"1";
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
    payload = (digitalRead(bdrPin0) && digitalRead(bdrPin1))?"0":"1";
  }
  if(server.hasArg("lvr0")) {
    int state = server.arg("lvr0").toInt();
    if (state == 0) {
      digitalWrite(lvrPin0, HIGH);
    } else {
      digitalWrite(lvrPin0, LOW);
    }
    payload = digitalRead(lvrPin0)?"0":"1";
  }
  if(server.hasArg("lvr1")) {
    int state = server.arg("lvr1").toInt();
    if (state == 0) {
      digitalWrite(lvrPin1, HIGH);
    } else {
      digitalWrite(lvrPin1, LOW);
    }
    payload = digitalRead(lvrPin1)?"0":"1";
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
    payload = (digitalRead(lvrPin0) && digitalRead(lvrPin1))?"0":"1";
  }

  server.send(httpStatusCode, "text/plain", payload);
}

void handleTemp() {
  String payload = "temperature, select sensor ?sensor=bdr|lvr";
  httpStatusCode = 200;

  if(server.hasArg("sensor")) {
    String room = server.arg("sensor");
    DS18B20.requestTemperatures();
    delay(1000);
    if (room == "bdr"){
      dtostrf(DS18B20.getTempCByIndex(0), 2, 1, bdrTempString);
      // payload = String(DS18B20.getTempCByIndex(0));
      payload = bdrTempString;
    } else if (room == "lvr") {
      dtostrf(DS18B20.getTempCByIndex(1), 2, 1, lvrTempString);
      // payload = String(DS18B20.getTempCByIndex(1));
      payload = lvrTempString;
    } else {
      payload = "not found requested sensor";
      httpStatusCode = 400;
    }
  }

  server.send(httpStatusCode, "text/plain", payload);
}
