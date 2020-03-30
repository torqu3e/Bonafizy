/* Sketch - Bonafizy
 * Version - 0.1.3
 * Author - Tejinder Singh
 *
 * TODO:
    - Wifi initial setup
 */


#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

//------------------------------------------
#define HOSTNAME "kettle"
#ifndef STASSID
#define STASSID "WIFI_SSID"
#define STAPSK  "WIFI_PASSWORD"
#endif
//------------------------------------------

#define FW_VER "0.1.3"
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
  Serial.print("");
  Serial.println("Bonafizy kettle automator");
  Serial.print("Firmware version: ");
  Serial.println(FW_VER);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");

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
  Serial.println("");

  Serial.println("Setting up mDNS responder...");
  uint8_t mdns_counter = 0;
  while (!MDNS.begin(HOSTNAME)) {
    delay(1000);
    Serial.print(".");
    if (mdns_counter >= 5) {
      Serial.println("mDNS responder setup failed, rebooting...");
      ESP.reset();
    }
    mdns_counter++;
  }
  MDNS.addService("http", "tcp", 80);
  Serial.print("mDNS responder up, responding at ");
  Serial.printf("%s.local", HOSTNAME);
  Serial.println("");

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
  server.on("/state", HTTP_GET, api_handler);
  server.on("/brew", HTTP_GET, api_handler);
  server.on("/power/on", HTTP_GET, api_handler);
  server.on("/power/off", HTTP_GET, api_handler);
  server.on("/hold/on", HTTP_GET, api_handler);
  server.on("/hold/off", HTTP_GET, api_handler);
  server.onNotFound(HandleNotFound);
  server.begin();
  Serial.print("HTTP server started on ");
  Serial.print(HOSTNAME);
  Serial.println(" at ip " + WiFi.localIP().toString());
}

void api_handler() {
  String message = "{\"message\": ";
  if (server.uri() == "/power/off") {
    message += power_off();
  }
  else if (server.uri() == "/power/on") {
     message += power_on();
  }
  else if (server.uri() == "/hold/on") {
     message += hold_on();
  }
  else if (server.uri() == "/hold/off") {
     message += hold_off();
  }
  else if (server.uri() == "/brew") {
     message += brew();
  }
  else if (server.uri() == "/state") {
    message = "{\"version\": \"";
    message += FW_VER;
    message += "\"";
  }
  message += ", \"state\": ";
  message += "{\"power\": ";
  message += 1 - digitalRead(POWER_LED);
  message += ", \"hold\": ";
  message += 1 - digitalRead(HOLD_LED);
  message += "}}\n";
  Serial.println(message);
  server.send(200, "text/plain", message);
}

void blip(String function) {
  if (function == "power") {
    digitalWrite(POWER_BUTTON, LOW);
    delay(wait_time_ms);
    digitalWrite(POWER_BUTTON, HIGH);
  }
  else if (function == "hold") {
    digitalWrite(HOLD_BUTTON, LOW);
    delay(wait_time_ms);
    digitalWrite(HOLD_BUTTON, HIGH);
  }
} 


String power_on() {
  if (digitalRead(POWER_LED) == 1) {
    blip("power");
    delay(wait_time_ms);
    return ("\"Kettle powered on\"");
    }
  else { return ("\"Kettle already on\"");}
}

String power_off() {
  if (digitalRead(POWER_LED) == 0) {
    blip("power");
    return ("\"Kettle powered off\"");
    }
  else { return ("\"Kettle already off\""); }
}


String brew() {
  if (digitalRead(POWER_LED) == 1) {
    blip("power");
    delay(wait_time_ms);
    if (digitalRead(HOLD_LED) == 1) {
      blip("hold");
    }
    return ("\"Brewing now!!!\"");
  }
  else { return("\"Kettle already on\"");}
}


String hold_on() {
  if (digitalRead(POWER_LED) == 0) {
    if (digitalRead(HOLD_LED) == 1) {
      blip("hold");
      return("\"Hold temperature on\"");
      }
    else { return("\"Hold temperature already on\""); }
  }
  else { return("\"Kettle is off, cannot turn hold on\"");}
}


String hold_off() {
  if (digitalRead(POWER_LED) == 0) {
    if (digitalRead(HOLD_LED) == 0) {
      blip("hold");
      return("\"Hold temperature off\"");
      }
    else { return("\"Hold already off\""); }
  }
  else { return("\"Kettle is off, cannot turn hold off\""); }
}


void HandleRoot() {
  String message = "<html>";
  message += "<h2>Bonafizy</h2>";
  message += "<body>Wifi endpoint for the Bonavita kettle</body>";
  message += "</html>";
  server.send(200, "text/html", message );
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
  MDNS.update();
  server.handleClient();
  ArduinoOTA.handle();
}
