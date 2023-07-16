#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <stdlib.h>
#include <stdint.h>

#define LED_PIN 18

const char* ssid = "";
const char* password = "";
const char* bootstrapper_address = "";
static String ldt_address;
bool paired = false;

WiFiServer server(80);
String header;
unsigned long currentTime = millis();
unsigned long previousTime = 0; 
const long timeoutTime = 2000;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  if(WiFi.status()== WL_CONNECTED){
    WiFiClient client;
    HTTPClient http;

    connectToBootstrapper(&client, &http);

    Serial.println("Next: sending my IP to the LDT");
    sleep(1);

    WiFiClient client2;
    HTTPClient http2;

    connectToLDT(&client2, &http2);
    
    server.begin();
  }
}

void connectToBootstrapper(WiFiClient *client, HTTPClient *http) {
    http->begin(*client, bootstrapper_address);
    http->addHeader("Content-Type", "application/json");
    http->setTimeout(20000);

    Serial.println("Preparing JSON");
    StaticJsonDocument<1024> doc;

    doc["Name"] = "lightbulb";
    doc["MacAddress"] = WiFi.macAddress();
    doc["Version"] = "0.12.0";

    char httpRequestData[1024];
    uint32_t size = serializeJson(doc, httpRequestData);
  
    if(size == 0) {
      return;
    }

    int httpResponseCode = http->POST(httpRequestData);
    Serial.print("HTTP Response code from Bootstrapper: ");
    Serial.println(httpResponseCode);

    if (httpResponseCode > 0) {
      String payload = http->getString();
      Serial.print("Payload: ");
      Serial.println(payload);
      ldt_address = payload;
    }
    http->end();
}

void connectToLDT(WiFiClient *client, HTTPClient *http) {
    String ldt_register_address = "http://" + ldt_address + "/register";
    Serial.print("sending to: ");
    Serial.println(ldt_register_address);

    http->begin(*client, ldt_register_address);
    http->addHeader("Content-Type", "application/json");

    Serial.println("Preparing JSON");
    StaticJsonDocument<1024> doc;
    doc["Device_IPv4"] = WiFi.localIP().toString();
    doc["Device_MAC"] = WiFi.macAddress();

    char httpRequestData[1024];
    uint32_t size = serializeJson(doc, httpRequestData);

    int httpResponseCode = http->POST(httpRequestData);
    Serial.print("HTTP Response code from LDT: ");
    Serial.println(httpResponseCode);

    if (httpResponseCode > 0) {
      String payload2 = http->getString();
      Serial.print("Payload: ");
      Serial.println(payload2);
      if (payload2 == "ack") {
        paired = true;
        Serial.println("paired successfully");
      }
    }
    http->end();
}

void loop() {
  WiFiClient client = server.available();
  if(client) {
    currentTime = millis();
    previousTime = currentTime;
    String currentLine = "";

    while (client.connected() && currentTime - previousTime <= timeoutTime) {
       currentTime = millis();
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        header += c;
        if (c == '\n') {
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            if (header.indexOf("GET /off") >= 0) {
              digitalWrite(LED_PIN, LOW);
            } else if (header.indexOf("GET /on") >= 0) {
              digitalWrite(LED_PIN, HIGH);
            }
          }
        }
      }
    }
    header = "";
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}