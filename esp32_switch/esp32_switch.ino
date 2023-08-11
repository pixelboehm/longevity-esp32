#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <stdlib.h>
#include <stdint.h>

#define BUTTON_PIN 18
#define LED_PIN 21

const char* ssid = "";
const char* password = "";
String bootstrapper_address = ""; // "IP:PORT"
static String ldt_address;
bool paired = false;

WiFiServer server(80);
String header;
unsigned long currentTime = millis();
unsigned long previousTime = 0; 
const long timeoutTime = 2000;

int lastState = HIGH; // the previous state from the input pin
int currentState;     // the current reading from the input pin
bool mode = false;
int i = 0;

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
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
    String bootstrapper_register_address = "http://" + bootstrapper_address + "/register";
    http->begin(*client, bootstrapper_register_address);
    http->addHeader("Content-Type", "application/json");
    http->setTimeout(20000);

    Serial.println("Preparing JSON");
    StaticJsonDocument<1024> doc;

    doc["Name"] = "switch";
    doc["MacAddress"] = WiFi.macAddress();
    doc["Version"] = "latest";

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
  checkManualButton();
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

            if (header.indexOf("GET /toggle") >= 0) {
              mode = !mode;
              digitalWrite(LED_PIN, mode);
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

void checkManualButton() {
  currentState = digitalRead(BUTTON_PIN);

  if(lastState == LOW && currentState == HIGH) {
    i++;
    Serial.println(i);
    mode = !mode;
  }
  // save the last state
  lastState = currentState;
  digitalWrite(LED_PIN, mode);
}