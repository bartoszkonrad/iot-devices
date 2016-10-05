#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

const char* ssid = "kolasz_iot";
const char* password = "neV3na4E";

String hue = "";
String saturation = "";
String light = "";
String room = "";

ESP8266WebServer server(80);

void handleRoot() {
  server.send(200, "text/plain", "rgb controller");
}

void handleColor() {
  if(server.hasArg("h")) {
    hue = server.arg("h");
  }
  if(server.hasArg("s")) {
    saturation = server.arg("s");
  }
  if(server.hasArg("l")) {
    light = server.arg("l");
  }    
  if(server.hasArg("r")) {
    room = server.arg("r");
  }
  String payload = hue + ";" + saturation + ";" + light + ";" + room;
  Serial.println(payload);
  server.send(200, "text/plain", payload);
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

void setup(void){
  
  Serial.begin(115200);
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
  
  server.on("/color", handleColor);

  server.on("/raw", [](){
    String message = "RAW\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET)?"GET":"POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
    server.send(200, "text/plain", message);
  });
  
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void){
  server.handleClient();
}
