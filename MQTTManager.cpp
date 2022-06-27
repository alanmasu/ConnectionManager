#include "MQTTManager.h"
#include "ConnectionManager.h"
//Costruttori
MQTTManager::MQTTManager():ConnectionManager(){
  _name = "";
  mqttClient = NULL;
  wifiClient = NULL;
}

MQTTManager::MQTTManager(bool autoReconnect,
                         byte connection_led_pin,
                         uint32_t wifi_max_initial_timeout,
                         uint32_t wifi_min_time_tra_connection,
                         uint32_t wps_blink_interval,
                         byte conn_button_pin,
                         byte conn_button_pin_mode,
                         byte conn_button_mode): ConnectionManager(autoReconnect,
                               connection_led_pin,
                               wifi_max_initial_timeout,
                               wifi_min_time_tra_connection,
                               wps_blink_interval,
                               conn_button_pin,
                               conn_button_pin_mode,
                               conn_button_mode)
{
  _name = "";
  mqttClient = NULL;
  wifiClient = NULL;
}



//Override methods
void MQTTManager::loop(bool withServer, bool withOTA) {
  ConnectionManager::loop(withServer, withOTA);
}

void MQTTManager::setHomepage() {
  ConnectionManager::setHomepage();
}

void MQTTManager::startConnection(bool withWPS) {
  ConnectionManager::startConnection(withWPS);
}