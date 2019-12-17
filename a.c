/*
  This sketch is intended to toggle a button in reponse to incoming MQTT messages.

  Many devices have a power button that can be momentarily pressed to toggle power
  on and off.  Further, the device will often have something like a power LED that
  can be monitored to determine the device's power state.

  This sketch will monitor the configured MQTT topic for "on" and "off" events.  It
  will also monitor a selected pin to determine if the local device is "on" or "off".
  If the local state is "off" and an "on" message is received, it will momentarily
  toggle the output pin to turn the device on.  Likewise, if the monitored device
  is "on" and an incoming message for "off" is received, the power button pin will be
  toggled to turn the device off.

  The sketch will also monitor the power LED and publish an outgoing MQTT message when
  the power state changes.  This will capture local user interaction and update MQTT
  with the new power state.

  Finally, the sketch will periodically announce its local state every minute.

  HARDWARE NOTES:
    * The power button pin can be connected to a relay or an optoisolator or similar device
    to actuate the device's power button.  A pull-down resistor will probably be needed.
    * The power monitor pin can be connected to one leg of an LED or maybe VCC of the
    controlled device to detect the power state.  Keep in mind input voltage limits.

  REQUIREMENTS: PubSubClient
  Large portions of the code here are based on the "Basic ESP8266 MQTT example" from
  PubSubClient by Nick O'Leary available at http://pubsubclient.knolleary.net/
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#define DEBUG true // flag to turn on/off debug messages over serial
#define Serial if(DEBUG)Serial 

// Update these with values suitable for your network.

const char* ssid = "linksys";  // Your WiFi network name
const char* password = "default";  // Your WiFi network password
const char* mqtt_server = "192.168.1.199";  // Your MQTT server IP address

// This is the topic to be subscribed and published to
const char* mqtt_topic = "smartthings/Tea Kettle/switch";
const char* mqtt_onMessage = "on";
const char* mqtt_offMessage = "off";

const int powerButton = D1;
const int holdButton = D2;
const int powerMonitor = D0;
//const int ledPin = D4; // built-in LED on D1 mini

const int powerButtonHoldTime = 100; // How long should the button be held for toggle
const int powerButtonWaitTime = 100; // How long to wait for the system to power up after toggling the power button

WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;
int powerSetState = 0;

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to WiFi network: ");
  Serial.print(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("WiFi connected succesfully and assigned IP: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  String inMessage;
  for (int i = 0; i < length; i++) {
    inMessage += (char)payload[i];
  }
  Serial.print("Incoming message ");
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(inMessage);

  if (inMessage == mqtt_onMessage) {
    Serial.println("Received MQTT power on message");
    int powerMonitorState = digitalRead(powerMonitor);
    powerSetState = 1;
    if (powerMonitorState == 0) {
      Serial.println("Toggling power on");
      digitalWrite(powerButton, HIGH);
      delay(powerButtonHoldTime);
      digitalWrite(powerButton, LOW);
      delay(powerButtonWaitTime);
      digitalWrite(holdButton, HIGH);
      delay(powerButtonHoldTime);
      digitalWrite(holdButton, LOW);
      delay(powerButtonWaitTime);
    }
  }
  else if (inMessage == mqtt_offMessage) {
    Serial.println("Received MQTT power off message");
    powerSetState = 0;
    int powerMonitorState = digitalRead(powerMonitor);
    if (powerMonitorState == 1) {
      Serial.println("Toggling power off");
      digitalWrite(powerButton, HIGH);
      delay(powerButtonHoldTime);
      digitalWrite(powerButton, LOW);
      delay(powerButtonWaitTime);
    }
  }
  else {
    Serial.print("No match for message: ");
    Serial.println(inMessage);
  }
  
}

void reconnect() {
  // Loop until we're reconnected to MQTT
  while (!client.connected()) {
//    digitalWrite(ledPin, LOW);
    Serial.print("Attempting MQTT connection to broker: ");
    Serial.println(mqtt_server);
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
//      digitalWrite(ledPin, HIGH);
      Serial.println("MQTT connected");
      client.publish(mqtt_topic, "Connected");
      // Subscribe to our selected topic
      client.subscribe(mqtt_topic);
    }
    else {
//      digitalWrite(ledPin, LOW);
      Serial.print("MQTT connection failed, rc=");
      Serial.print(client.state());
      Serial.println(".  Trying again in 5 seconds.");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup_OTA() {
  // Start up OTA
  
  // ArduinoOTA.setPort(8266); // Port defaults to 8266
  // ArduinoOTA.setHostname("myesp8266"); // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setPassword((const char *)"123"); // No authentication by default

  ArduinoOTA.onStart([]() {
    Serial.println("OTA update start");
    client.publish(mqtt_topic, "OTA update start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nOTA update complete");
    client.publish(mqtt_topic, "OTA update complete");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("OTA Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("OTA firmware update ready");  
}

void setup() {
  // Deal with hardware first
  pinMode(powerButton, OUTPUT);
  pinMode(holdButton, OUTPUT);
  pinMode(powerMonitor, INPUT_PULLUP);
//  pinMode(ledPin, OUTPUT);

  // Record the current state as the desired set state, possibly to be
  // overwritten when we start MQTT
  powerSetState = digitalRead(powerMonitor);

  Serial.begin(115200);
  Serial.println("");
  Serial.println("Hardware initialized, starting program load");

  // Start up networking
  setup_wifi();

  // Start up MQTT
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  // Start up OTA
  setup_OTA();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }

  // MQTT client loop
  client.loop();

  // OTA loop
  ArduinoOTA.handle();

  // If our local state has changed against the set state, publish a message to MQTT
  int powerMonitorState = digitalRead(powerMonitor);
  //Serial.print("powerMonitorState: ");
  //Serial.println(powerMonitorState);
  //delay(1000);
  
  if ((powerMonitorState == 0) && (powerSetState == 1)) {
    Serial.println("Caught local power off, publishing to MQTT");
    client.publish(mqtt_topic, mqtt_offMessage);
    powerSetState = 0;
  }
  else if ((powerMonitorState == 1) && (powerSetState == 0)) {
    Serial.println("Caught local power on, publishing to MQTT");
    client.publish(mqtt_topic, mqtt_onMessage);
    powerSetState = 1;

    // In this case, somebody pressed power locally.  We need to press "hold"
    // after the power button has been released by the user, but I have no way
    // to monitor that action.  Instead, wait 1 second then send the hold toggle.
    // This way, the user can override the auto-hold by simply holding down the
    // power button for longer than a second.
    delay (1000);
    digitalWrite(holdButton, HIGH);
    delay(powerButtonHoldTime);
    //delay(5000);
    digitalWrite(holdButton, LOW);
  }

  // Publish current state to topic every five minutes
  long now = millis();
  if (now - lastMsg > 300000) {
    lastMsg = now;
    if (powerMonitorState) {
      Serial.print("Periodic publishing 'on' to topic: ");
      Serial.println(mqtt_topic);
      client.publish(mqtt_topic, "on");
    }
    else {
      Serial.print("Periodic publishing 'off' to topic: ");
      Serial.println(mqtt_topic);
      client.publish(mqtt_topic, "off");
    }
  }
}