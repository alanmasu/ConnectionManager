#include <MQTTManager.h>
#include <WebServer.h>

//MQTT
WiFiClient espClient;
PubSubClient client(espClient);
const char* mosquittoServer = "test.mosquitto.org";
String topics[2] = {"test/topic1",
                    "test/topic2"};

//ConnectionManager
WebServer server(80);
MQTTManager connectionManager;
esp_wps_config_t config;


//WiFi [not necessary]
const char* ssid = "YOUR_SSID";
const char* pass = "YOUR_PASS";

void rebootCallback() {
  Serial.println("Rebooot timeee!!...");
}

void callback(char* topic, byte* message, unsigned int length) {

  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");

  char messageChar[length];
  String messageTemp;
  String toSend;
  char toSendChar[12];

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
    messageChar[i] += (char)message[i];
  }
  Serial.println();

}

void setup() {
  //Start serial
  Serial.begin(115200);

  //Creatin a variable to store version of the project
  String ver = String(__FILE__) + " Time: " + String(__DATE__) + " " + String(__TIME__);
  Serial.println("Version: " + ver);

  //Setting version on WebServer /IP_ADD/info
  connectionManager.setVersion(ver);
  //Setting server
  // true -> whitDefaultHomePage
  // set olso /IP_ADD/reboot for call rebootCallback and after reboots the core
  //      and /IP_ADD/rebootOnly thats reboot ESP32 whitout calling callBack
  connectionManager.setServer(&server, true);

  //For enable WPS must use:
  connectionManager.setWPSConfig(&config);
  //Connect to a WiFi for the first time
  //connectionManager.startWiFi(ssid, pass);
  //or
  connectionManager.startConnection(true);

  //Start WebServer and OTA
  connectionManager.startWebServer();
  connectionManager.setOTAHostname("ESP32");
  connectionManager.startOTA();

  //Setting rebootCallback and reboot options
  connectionManager.setOnRebootCallback(rebootCallback);
  connectionManager.setRebootOptions(false, true, false, true);

  //Setting MQTT server
  connectionManager.setMQTTServer(&client, "ESP32", mosquittoServer);
  connectionManager.setTopics(topics, 2);
  connectionManager.setCallback(callback);
  connectionManager.startMQTT();
}

uint32_t t = 0;

void loop() {
  //this update everything, connection LED (default on-board led), connection BUTTON (default on-board BOOT button), servers... 
  //ATTENTION: in case of MQTT connection falliture can block the prosess for several seconds
  connectionManager.loop();

  if(millis() - t > 1000){
    t = millis();
    Serial.println(connectionManager.getStringState());
  }
}
