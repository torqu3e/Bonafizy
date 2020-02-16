/* Sketch - Bonafizy
 * Version - 0.1.1
 * Author - Tejinder Singh
 *
 * TODO:
    - Help Page
    - Circuit diagram, 
    - Chain toggle functions under an API handler
 */


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
//------------------------------------------

#define FW_VER "0.1.1"
#define POWER_BUTTON D3 //Pin attached to kettle's power button
#define HOLD_BUTTON D2 //Pin attached to kettle's hold button
#define POWER_LED D6 //Pin attached to kettle's power LED
#define HOLD_LED D5 //Pin attached to kettle's hold LED
#define BUILTIN_LED D4

const char* ssid = STASSID;
const char* password = STAPSK;
const int wait_time_ms = 100;

ESP8266WebServer server(80);

void setup() {
  pinMode (POWER_BUTTON, OUTPUT);
  digitalWrite(POWER_BUTTON, HIGH);
  pinMode (HOLD_BUTTON, OUTPUT);
  digitalWrite(HOLD_BUTTON, HIGH);
  pinMode (POWER_LED, INPUT);
  pinMode (HOLD_LED, INPUT);

  Serial.begin(115200);
  Serial.print("Firmware version");
  Serial.println(FW_VER);

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
  server.on("/brew", HTTP_GET, brew);
  server.on("/power/on", HTTP_GET, power_on);
  server.on("/power/off", HTTP_GET, power_off);
  server.on("/hold/on", HTTP_GET, hold_on);
  server.on("/hold/off", HTTP_GET, hold_off);
  server.onNotFound(HandleNotFound);
  server.begin();
  Serial.println("HTTP server started at ip " + WiFi.localIP().toString());
}

void brew() {
  String message = "{\"state\":";
  if (digitalRead(POWER_LED) == 1) {
    digitalWrite(POWER_BUTTON, LOW);
    delay(wait_time_ms);
    digitalWrite(POWER_BUTTON, HIGH);   
    delay(wait_time_ms); 
    if (digitalRead(HOLD_LED) == 1) {
      digitalWrite(HOLD_BUTTON, LOW);
      delay(wait_time_ms);
      digitalWrite(HOLD_BUTTON, HIGH);
    }
    message += "{\"power\": ";
    message += 1 - digitalRead(POWER_LED);
    message += ", \"hold\": ";
    message += 1 - digitalRead(HOLD_LED);
    message += "}, \"message\": ";
    message += "\"Brewing now!!!\"}\n";
  }
  else {
    message += "{\"power\": ";
    message += 1 - digitalRead(POWER_LED);
    message += ", \"hold\": ";
    message += 1 - digitalRead(HOLD_LED);
    message += "}, \"message\": ";
    message += "\"Kettle already on\"}\n";    
    }
  Serial.println(message);
  server.send(200, "text/plain", message);
}

void power_on() {
  String message = "{\"state\":";
  if (digitalRead(POWER_LED) == 1) {
    digitalWrite(POWER_BUTTON, LOW);
    delay(wait_time_ms);
    digitalWrite(POWER_BUTTON, HIGH);
    message += "{\"power\": ";
    message += 1 - digitalRead(POWER_LED);
    message += ", \"hold\": ";
    message += 1 - digitalRead(HOLD_LED);
    message += "}, \"message\": ";
    message += "\"Kettle powered on\"}\n";
    }
  else {
    message += "{\"power\": ";
    message += 1 - digitalRead(POWER_LED);
    message += ", \"hold\": ";
    message += 1 - digitalRead(HOLD_LED);
    message += "}, \"message\": ";
    message += "\"Kettle already on\"}\n";
    }
  Serial.println(message);
  server.send(200, "text/plain", message);
}


void hold_on() {
  String message = "{\"state\":";
  if (digitalRead(POWER_LED) == 0) {
    if (digitalRead(HOLD_LED) == 1) {
      digitalWrite(HOLD_BUTTON, LOW);
      delay(wait_time_ms);
      digitalWrite(HOLD_BUTTON, HIGH);
      message += "{\"power\": ";
      message += 1 - digitalRead(POWER_LED);
      message += ", \"hold\": ";
      message += 1 - digitalRead(HOLD_LED);
      message += "}, \"message\": ";
      message += "\"Hold temperature on\"}\n";
      }
    else {
      message += "{\"power\": ";
      message += 1 - digitalRead(POWER_LED);
      message += ", \"hold\": ";
      message += 1 - digitalRead(HOLD_LED);
      message += "}, \"message\": ";
      message += "\"Hold temperature already on\"}\n";
      }
  }
  else {
      message += "{\"power\": ";
      message += 1 - digitalRead(POWER_LED);
      message += ", \"hold\": ";
      message += 1 - digitalRead(HOLD_LED);
      message += "}, \"message\": ";
      message = "\"Kettle is off, cannot turn hold on\"}\n";
      }
  Serial.println(message);
  server.send(200, "text/plain", message);
}


void power_off() {
  String message = "{\"state\":";
  if (digitalRead(POWER_LED) == 0) {
    digitalWrite(POWER_BUTTON, LOW);
    delay(wait_time_ms);
    digitalWrite(POWER_BUTTON, HIGH);
    message += "{\"power\": ";
    message += 1 - digitalRead(POWER_LED);
    message += ", \"hold\": ";
    message += 1 - digitalRead(HOLD_LED);
    message += "}, \"message\": ";
    message += "\"Kettle powered off\"}\n";
    }
  else {
    message += "{\"power\": ";
    message += 1 - digitalRead(POWER_LED);
    message += ", \"hold\": ";
    message += 1 - digitalRead(HOLD_LED);
    message += "}, \"message\": ";    
    message += "\"Kettle already off\"}\n";}
  Serial.println(message);
  server.send(200, "text/plain", message);
}


void hold_off() {
  String message = "{\"state\":";
  if (digitalRead(HOLD_LED) == 0) {
    digitalWrite(HOLD_BUTTON, LOW);
    delay(wait_time_ms);
    digitalWrite(HOLD_BUTTON, HIGH);
    message += "{\"power\": ";
    message += 1 - digitalRead(POWER_LED);
    message += ", \"hold\": ";
    message += 1 - digitalRead(HOLD_LED);
    message += "}, \"message\": ";
    message += "\"Hold temperature off\"}\n";
    }
  else {
    message += "{\"power\": ";
    message += 1 - digitalRead(POWER_LED);
    message += ", \"hold\": ";
    message += 1 - digitalRead(HOLD_LED);
    message += "}, \"message\": ";    
    message += "\"Hold already off\"}\n";
    }
  Serial.println(message);
  server.send(200, "text/plain", message);
}


void HandleRoot() {
  String message = "<html>";
  message += "<h2>Bonafizy</h2>";
  message += "<body>Wifi endpoint for the Bonavita kettle</body>";
  message += "</html>";
  server.send(200, "text/html", message );
}


void get_state() {
  String message = "{\"state\":";
  message += "{\"power\": ";
  message += 1 - digitalRead(POWER_LED); // Kettle's power is inverted
  message += ", \"hold\": ";
  message += 1 - digitalRead(HOLD_LED); // Kettle's power is inverted
  message += "}, \"version\": \"";
  message += FW_VER;
  message += "\"}\n";
  Serial.println(message);
  server.send(200, "application/json", message);
}


void HandleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/html", message);
}


void loop() {
  server.handleClient();
  ArduinoOTA.handle();
}