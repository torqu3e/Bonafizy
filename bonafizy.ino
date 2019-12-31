#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

//------------------------------------------

#ifndef STASSID
#define STASSID "WIFI_SSID"
#define STAPSK  "WIFI_PASSWORD"
#endif

#define POWER_BUTTON D1 //Pin attached to kettle's power button
#define HOLD_BUTTON D2 //Pin attached to kettle's hold button
#define POWER_LED D3 //Pin attached to kettle's power LED
#define HOLD_LED D4 //Pin attached to kettle's hold LED

//------------------------------------------
//WIFI
const char* ssid = STASSID;
const char* password = STAPSK;

//------------------------------------------
//HTTP
ESP8266WebServer server(80);

//------------------------------------------
const int wait_time_ms = 100;

//------------------------------------------
void setup() {
  pinMode (POWER_BUTTON, OUTPUT);
  pinMode (HOLD_BUTTON, OUTPUT);
  pinMode (POWER_LED, INPUT);
  pinMode (HOLD_LED, INPUT);
  
  Serial.begin(115200);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  uint8_t wifi_timer = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (wifi_timer >= 10) {
      Serial.println("Wifi not connected, rebooting...");
      ESP.reset();
    }
    wifi_timer++;
  }

  Serial.println("");
  Serial.print("Connected to: ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();

  server.on("/", HTTP_GET, HandleRoot);
  server.on("/state", HTTP_GET, get_state);
  server.on("/power/on", HTTP_GET, power_on);
  server.on("/power/off", HTTP_GET, power_off);
  server.on("/hold/on", HTTP_GET, hold_on);
  server.on("/hold/off", HTTP_GET, hold_off);
  server.onNotFound(HandleNotFound);
  server.begin();
  Serial.println("HTTP server started at ip " + WiFi.localIP().toString());
}


void power_on() {
    if (digitalRead(POWER_LED) == 0) {
        String message = "Powering on kettle...";
        Serial.println(message);
        server.send(200, "text/plain", message);
        digitalWrite(POWER_BUTTON, HIGH);
        delay(wait_time_ms);
        digitalWrite(POWER_BUTTON, LOW);
        delay(wait_time_ms);
    }
}


void hold_on() {
    if (digitalRead(HOLD_LED) == 0) {
        String message = "Turning on hold...";
        Serial.println(message);
        server.send(200, "text/plain", message);
        digitalWrite(HOLD_BUTTON, HIGH);
        delay(wait_time_ms);
        digitalWrite(HOLD_BUTTON, LOW);
        delay(wait_time_ms);
    }
}


void power_off() {
    if (digitalRead(POWER_LED) == 1) {
        String message = "Powering off kettle...";
        Serial.println(message);
        server.send(200, "text/plain", message);
        digitalWrite(POWER_BUTTON, HIGH);
        delay(wait_time_ms);
        digitalWrite(POWER_BUTTON, LOW);
        delay(wait_time_ms);
    }
}


void hold_off() {
    if (digitalRead(HOLD_LED) == 1) {
        String message = "Turning off hold...";
        Serial.println(message);
        server.send(200, "text/plain", message);
        digitalWrite(HOLD_BUTTON, HIGH);
        delay(wait_time_ms);
        digitalWrite(HOLD_BUTTON, LOW);
        delay(wait_time_ms);
    }
}


void HandleRoot(){
  String message = "<html>";
  message += "<h2>Bonafizy</h2>";
  message += "<body>Wifi endpoint for the Bonavita kettle</body>";
  message += "</html>";
  server.send(200, "text/html", message );
}


void get_state(){
    String message = "{\"power\": ";
    message += digitalRead(POWER_LED);
    message += ", \"hold\": ";
    message += digitalRead(HOLD_LED);
    message += "}";
    Serial.println(message);
    server.send(200, "application/json", message);
}


void HandleNotFound(){
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
  server.send(404, "text/html", message);
}


void loop() {
  server.handleClient();
  ArduinoOTA.handle();
}
