#include "C:/Users/alanm/Documents/GitHub/ESP32/ConnectionManager/ConnectionManager/MQTTManager.h"
#include <WebServer.h>

MQTTManager prova;

WebServer server(80);

void provaCallback(){
  Serial.println("Questa è una callbak");
}

void setup() {
  // put your setup code here, to run once:
  String ver = String(__FILE__) + " Time: " + String(__DATE__) + " " + String(__TIME__);
  Serial.begin(115200);
  Serial.println("Versione: " + ver);
//  esp_log_level_set("wifi", ESP_LOG_VERBOSE); 
//  Serial.setDebugOutput(true);

//  prova.setVersion(ver);
//  prova.setServer(&server, true);
//  //prova.startConnection();
//  prova.startWiFi("Alan", "alanmasu2001");
//  prova.startWebServer();
//  prova.setOTAHostname("prova");
//  prova.startOTA();
//  prova.setOnRebootCallback(provaCallback);
//  prova.setRebootOptions(false, true);
//  Serial.println("You can reach me olso at: " + prova.getOTAHostname() + ".local/");
  
}

void loop() {
  // put your main code here, to run repeatedly:
  //prova.loop();
}
