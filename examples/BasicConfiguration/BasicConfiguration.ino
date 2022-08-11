#include <ConnectionManager.h>

WebServer server(80);

ConnectionManager connectionManager;

esp_wps_config_t config;

const char* ssid = "YOUR_SSID";
const char* pass = "YOUR_PASS";

void rebootCallback() {
  Serial.println("Rebooot timeee!!...");
}

void setup() {
  //Start serial
  Serial.begin(115200);

  //Creating a variable to store version of the project
  String ver = String(__FILE__) + " Time: " + String(__DATE__) + " " + String(__TIME__);
  Serial.println("Version: " + ver);

  //Setting version on WebServer /IP_ADD/infoAbout
  connectionManager.setVersion(ver);
  //Setting server
  // true -> whitDefaultHomePage
  // set olso /IP_ADD/reboot for call rebootCallback and after reboots the core
  //      and /IP_ADD/rebootOnly thats reboot ESP32 whitout calling callBack
  connectionManager.setServer(&server, true);
  //Connect to a WiFi for the first time
  //connectionManager.startWiFi(ssid, pass);
  //For enable WPS you may use:  
  connectionManager.setWPSConfig(&config);
  connectionManager.startConnection(true, false);

  //Start WebServer and OTA
  connectionManager.startWebServer();
  connectionManager.setOTAHostname("ESP32");
  connectionManager.startOTA();

  //Setting rebootCallback and reboot options
  connectionManager.setOnRebootCallback(rebootCallback);
  connectionManager.setRebootOptions(false, true);

}

uint32_t t = 0;

void loop() {
  //this update everything, connection LED (default on-board led), connection BUTTON (default on-board BOOT button), servers... 
  connectionManager.loop();
  if(millis() - t > 1000){
    t = millis();
    Serial.println(connectionManager.getStringState());
  }
}
