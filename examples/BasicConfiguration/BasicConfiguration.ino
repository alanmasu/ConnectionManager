#include <ConnectionManager.h>

WebServer server(80);

ConnectionManager connectionManager;

const char* ssid = "YOUR_SSID";
const char* pass = "YOUR_PASS";

void rebootCallback() {
  Serial.println("Rebooot timeee!!...");
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
  //Connect to a WiFi for the first time
  //connectionManager.startWiFi(ssid, pass);
  //For enable WPS may use:
  connectionManager.startConnection(true);

  //Start WebServer and OTA
  connectionManager.startWebServer();
  connectionManager.setOTAHostname("ESP32");
  connectionManager.startOTA();

  //Setting rebootCallback and reboot options
  connectionManager.setOnRebootCallback(rebootCallback);
  connectionManager.setRebootOptions(false, true);

  //Print the hostname
  Serial.println("You can reach me also at: " + connectionManager.getOTAHostname() + ".local/");
}

void loop() {
  //this update everything, connection LED (default on-board led), connection BUTTON (default on-board BOOT button), servers... 
  connectionManager.loop();
}
