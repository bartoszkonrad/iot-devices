#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ClickButton.h>

const char* host = "iot-btns1";
const char* ssid = "kolasz_iot";
const char* password = "neV3na4E";

#define ONE_WIRE_BUS D3

const byte kitchenBtnPin = D0;
const byte hallwayBtnPin = D1;
const byte bathroomBtnPin = D2;

const byte kitchenPin = D5;
const byte hallwayPin = D6;
const byte bathroomPin = D7;
const byte worktopPin = D8;

char kitchenTempString[5];
char hallwayTempString[5];

ClickButton kitchenBtn(kitchenBtnPin, LOW, CLICKBTN_PULLUP);
ClickButton hallwayBtn(hallwayBtnPin, LOW, CLICKBTN_PULLUP);
ClickButton bathroomBtn(bathroomBtnPin, LOW, CLICKBTN_PULLUP);

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

WiFiClient client;
const int httpPort = 80;

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

void setup(void){

  Serial.begin(115200);
  Serial.println();
  Serial.println("Booting...");

  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);

  while(WiFi.waitForConnectResult() != WL_CONNECTED){
    WiFi.begin(ssid, password);
    Serial.println("WiFi failed, retrying.");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  MDNS.begin(host);

  httpUpdater.setup(&httpServer);
  httpServer.begin();

  MDNS.addService("http", "tcp", 80);

  pinMode(kitchenPin, OUTPUT);
  pinMode(hallwayPin, OUTPUT);
  pinMode(bathroomPin, OUTPUT);
  pinMode(worktopPin, OUTPUT);

  digitalWrite(kitchenPin, LOW);
  digitalWrite(hallwayPin, LOW);
  digitalWrite(bathroomPin, LOW);
  digitalWrite(worktopPin, LOW);

  kitchenBtn.debounceTime   = 4;
  kitchenBtn.multiclickTime = 300;
  kitchenBtn.longClickTime  = 600;

  hallwayBtn.debounceTime   = 4;
  hallwayBtn.multiclickTime = 300;
  hallwayBtn.longClickTime  = 600;

  bathroomBtn.debounceTime   = 4;
  bathroomBtn.multiclickTime = 300;
  bathroomBtn.longClickTime  = 600;

  // pinMode(BUILTIN_LED, OUTPUT);
}

void loop(){
  httpServer.handleClient();

  kitchenBtn.Update();
  hallwayBtn.Update();
  bathroomBtn.Update();

  if (kitchenBtn.clicks == 1) {
    DS18B20.requestTemperatures();
    delay(1000);
    Serial.println("");
    Serial.println(DS18B20.getTempCByIndex(0));
    Serial.println(DS18B20.getTempCByIndex(1));

    if (digitalRead(kitchenPin)) {
      digitalWrite(kitchenPin, LOW);
    } else {
      digitalWrite(kitchenPin, HIGH);
    }
  }

  if (kitchenBtn.clicks == 2) {
    char* host = "iot-atx.lan";
    if (!client.connect(host, httpPort)) {
      Serial.println("connection failed");
      return;
    }

    String url = "/leds";
    url += "?kth=700";

    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
    "Host: " + host + "\r\n" +
    "Connection: close\r\n\r\n");

  }

  if (hallwayBtn.clicks == 1) {
    if (digitalRead(hallwayPin)) {
      digitalWrite(hallwayPin, LOW);
    } else {
      digitalWrite(hallwayPin, HIGH);
    }
  }

  if (bathroomBtn.clicks == 1) {
    if (digitalRead(bathroomPin)) {
      digitalWrite(bathroomPin, LOW);
    } else {
      digitalWrite(bathroomPin, HIGH);
    }
  }

  // if (bathroomBtn.clicks == 2) {
  //   if (!digitalRead(BUILTIN_LED)) {
  //     digitalWrite(BUILTIN_LED, HIGH);
  //   } else {
  //     digitalWrite(BUILTIN_LED, LOW);
  //   }
  // }
}
