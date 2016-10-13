#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <OneWire.h>
#include <DallasTemperature.h>

const int sleepSeconds = 300;

#define ONE_WIRE_BUS 14

const byte dbVCC = 4;

// const int adc = A0;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);
char temperatureString[5];

const char* ssid = "kolasz_iot";
const char* password = "neV3na4E";

WiFiClient client;
const int httpPort = 80;
const char* host = "rpi.lan";

void setup() {
  Serial.begin(9600);
  Serial.println("\n\nWake up");
  blink();

  // pinMode(adc, INPUT);

  pinMode(dbVCC, OUTPUT);
  digitalWrite(dbVCC, HIGH);

  pinMode(BUILTIN_LED, OUTPUT);

  pinMode(16, WAKEUP_PULLUP);

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

  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  String url = "/outdoor";
  url += "?temp=";
  url += getTemperature();
  // url += "&v=";
  // url += String(analogRead(adc));

  Serial.print("Requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");

  blink();
  Serial.println("Going to sleep");

  ESP.deepSleep(sleepSeconds * 1000000);
}

void loop() {
}

String getTemperature() {
  Serial.println("Requesting DS18B20 temperature");
  float temp;
  DS18B20.requestTemperatures();
  delay(1000);
  temp = DS18B20.getTempCByIndex(0);
  dtostrf(temp, 2, 1, temperatureString);
  return temperatureString;
}

void blink() {
  // LED: LOW = on, HIGH = off
  for (int i = 0; i < 2; i++)
  {
    digitalWrite(BUILTIN_LED, LOW);
    delay(100);
    digitalWrite(BUILTIN_LED, HIGH);
    delay(100);
  }
}
