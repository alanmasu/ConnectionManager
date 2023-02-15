#include <Arduino.h>
#include "MQTTManager.h"
#include "ConnectionManager.h"

#include <WiFi.h>
#include <PubSubClient.h>

//Costruttori
MQTTManager::MQTTManager():
  MQTTManager(true,
              CONNECTION_LED_PIN,
              WIFI_MAX_INITIAL_TIMEOUT,
              WIFI_RECONNECT_INTERVAL,
              WPS_BLINK_INTERVAL,
              CONN_BUTTON_PIN,
              CONN_BUTTON_PIN_MODE,
              CONN_BUTTON_MODE) {}


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
  MQTTIntervalTraConnetion = MQTT_RECONNECT_INTERVAL;
  MQTTLastConnectionIstant = 0;

  //intervalli per il reboot
  RebootOnMQTTReconectionMaxTimes = REBOOT_ON_NOT_MQTT_RECONNECTION_FOR_RETRIES;
  maxMQTTReconnetcRetries = MAX_MQTT_RECONNECTION_TIMES;
  RebootOnMQTTReconectionMaxTime = REBOOT_ON_NOT_MQTT_RECONNECTION_FOR_TIME;
  maxMQTTDisconnectedTime = MAX_MQTT_DISCONNECTED_TIME;
  MQTTReconnectionTimes = 0;
  lastMQTTConnectedIstant = 0;
  _name = "";
  MQTTServer = NULL;
  MqttDisconnectedBlinkInterval = MQTT_DISCONNECTED_BLINK_INTERVAL;
  topicsLen = 0;
}

String MQTTManager::getStringState() {
  String tmp = ConnectionManager::getStringState();
  if (tmp == "" || tmp == "CONNECTED") {
    switch (_state) {
      case MQTT_CONNECTED: return "MQTT_CONNECTED";
      case MQTT_DISCONNECTED: return "MQTT_DISCONNECTED";
      case MQTT_SUBSCRIBE_FAILED: return "MQTT_SUBSCRIBE_FAILED";
      default: return String(_state);
    }
  } else {
    return tmp;
  }
}


//METODI
void MQTTManager::setMQTTServer(PubSubClient* server, String deviceName, const char* ipAddress, uint16_t port) {
  if (server != NULL) {
    _name = deviceName;
    MQTTServer = server;
    MQTTServer->setServer(ipAddress, port);
  }
}

void MQTTManager::setMQTTServer(PubSubClient* server, String deviceName, uint8_t* ipAddress, uint16_t port) {
  if (server != NULL) {
    _name = deviceName;
    MQTTServer = server;
    MQTTServer->setServer(ipAddress, port);
  }
}
void MQTTManager::setMQTTServer(PubSubClient* server, String deviceName, IPAddress ipAddress, uint16_t port) {
  if (server != NULL) {
    _name = deviceName;
    MQTTServer = server;
    MQTTServer->setServer(ipAddress, port);
  }
}

void MQTTManager::setCallback(MQTT_CALLBACK_SIGNATURE) {
  if (MQTTServer != NULL) {
    MQTTServer->setCallback(callback);
  }
}

void MQTTManager::setTopics(String* topics, uint16_t len) {
  this->topics = topics;
  topicsLen = len;
}

void MQTTManager::setDisconnectBlink(uint16_t interval) {
  MqttDisconnectedBlinkInterval = interval;
}

void MQTTManager::setRebootOptions(bool forTime,      bool forRetries,      bool forMQTTTime,      bool forMQTTRetries,
                                   uint16_t max_time, uint32_t max_retries, uint16_t maxMQTTTtime, uint32_t maxMQTTRetries) {
  ConnectionManager::setRebootOptions(forTime, forRetries, max_time, max_retries);
  
  RebootOnMQTTReconectionMaxTimes = forMQTTRetries;
  maxMQTTReconnetcRetries = maxMQTTRetries;
  RebootOnMQTTReconectionMaxTime = forMQTTTime;
  maxMQTTDisconnectedTime = maxMQTTTtime;
}

void MQTTManager::setAsyncFunction(void (*fn)(void)){
  asyncFunction = fn;
}

byte MQTTManager::startMQTT() {
  if (MQTTServer != NULL) {
    MQTTLastConnectionIstant = millis();
    if (MQTTServer->connect(_name.c_str())) {
      debugPort->println("[LOG]: Connected to MQTT server");
      if (topicsLen != 0) {
        subscribe(topics, topicsLen);
      }
    } else {
      debugPort->println("[ERR]: Error connecting to MQTT server");
    }
  }
  return _state;
}

void MQTTManager::subscribe(String topics[], uint16_t len) {
  if (len > 0 && MQTTServer != NULL) {
    for (int i = 0; i < len; i++) {
      if (!MQTTServer->subscribe(topics[i].c_str())) {
        _state = MQTT_SUBSCRIBE_FAILED;
        return;
      }
    }
  }
}

//Override methods
void MQTTManager::setHomepage() {
  ConnectionManager::setHomepage();
}

void MQTTManager::startConnection(bool withWPS, bool tryReconnection) {
  ConnectionManager::startConnection(withWPS, tryReconnection);
  startMQTT();
}

void MQTTManager::startAsyncLoop(void (*fn)(void)){
  asyncFunction = fn;
  asyncMode = true;
  if(fn != nullptr){
    asyncFunction = fn;
  }
  asyncMode = true;
  mutex = xSemaphoreCreateMutex();
  auto funct = [](void* vPar){
    MQTTManager* cmptr;
    if(vPar != nullptr){
      cmptr = (MQTTManager*)vPar;
    }
    while(1){
      if(vPar != nullptr){
        cmptr->connectionHandler();
        if(cmptr->asyncFunction != nullptr){       
          cmptr->asyncFunction(); 
        }
      }
      vTaskDelay(10);
    }
    vTaskDelete(NULL);
  };
  xTaskCreate(
    funct,
    "CONECTION_MANAGER_ASYNC",
    2000,
    (void*)this,
    1,
    &asyncTask
  );
}

void MQTTManager::loop(bool withServer, bool withOTA ) {
  ConnectionManager::_loop(withServer, withOTA);
  connectionLedRoutine();
  if(!asyncMode){
    connectionHandler();
  }
  if (MQTTServer != NULL && MQTTServer->connected()) {
    MQTTServer->loop();
  }
}

bool MQTTManager::publish(const char* topic, const char* payload){
  if(mutex != NULL && asyncMode){
    if(xSemaphoreTake(mutex, 100/portTICK_PERIOD_MS) == pdFALSE){
      debugPort->println("[ERR]: Error publishing to MQTT server because mutex is taken");
      return false;
    }
  }
  bool status;
  if(MQTTServer != nullptr){
    status = MQTTServer->publish(topic, payload);
  }
  if(mutex != NULL && asyncMode){
    xSemaphoreGive(mutex);
  }
  return status;
}
bool MQTTManager::publish(const char* topic, const String& payload){
   if(mutex != NULL && asyncMode){
    if(xSemaphoreTake(mutex, 100/portTICK_PERIOD_MS) == pdFALSE){
      debugPort->println("[ERR]: Error publishing to MQTT server because mutex is taken");
      return false;
    }
  }
  bool status;
  if(MQTTServer != nullptr){
    status = MQTTServer->publish(topic, payload.c_str());
  }
  if(mutex != NULL && asyncMode){
    xSemaphoreGive(mutex);
  }
  return status;
}

bool MQTTManager::publish(const char* topic, const char* payload, bool retained){
   if(mutex != NULL && asyncMode){
    if(xSemaphoreTake(mutex, 100/portTICK_PERIOD_MS) == pdFALSE){
      debugPort->println("[ERR]: Error publishing to MQTT server because mutex is taken");
      return false;
    }
  }
  bool status;
  if(MQTTServer != nullptr){
    status = MQTTServer->publish(topic, payload, retained);
  }
  if(mutex != NULL && asyncMode){
    xSemaphoreGive(mutex);
  }
  return status;
}
bool MQTTManager::publish(const char* topic, const uint8_t * payload, unsigned int plength){  
   if(mutex != NULL && asyncMode){
    if(xSemaphoreTake(mutex, 100/portTICK_PERIOD_MS) == pdFALSE){
      debugPort->println("[ERR]: Error publishing to MQTT server because mutex is taken");
      return false;
    }
  }
  bool status;
  if(MQTTServer != nullptr){
    status = MQTTServer->publish(topic, payload, plength);
  }
  if(mutex != NULL && asyncMode){
    xSemaphoreGive(mutex);
  }
  return status;
}
bool MQTTManager::publish(const char* topic, const uint8_t * payload, unsigned int plength, bool retained){
   if(mutex != NULL && asyncMode){
    if(xSemaphoreTake(mutex, 100/portTICK_PERIOD_MS) == pdFALSE){
      debugPort->println("[ERR]: Error publishing to MQTT server because mutex is taken");
      return false;
    }
  }
  bool status;
  if(MQTTServer != nullptr){
    status = MQTTServer->publish(topic, payload, plength, retained);
  }
  if(mutex != NULL && asyncMode){
    xSemaphoreGive(mutex);
  }
  return status;
}

//TO DO //gestisce lo stato e le riconnessioni
void MQTTManager::connectionHandler() { 
  //Controllo di stato
  if(mutex != NULL && asyncMode){
    xSemaphoreTake(mutex, portMAX_DELAY);
  }
  if (MQTTServer != NULL) {
    if (WiFi.status() == WL_CONNECTED && _state != MQTT_SUBSCRIBE_FAILED) {
      if (millis() - MQTTLastConnectionIstant > 2000) { //Per evitare false connessioni
        if (MQTTServer->connected()) {
          _state = MQTT_CONNECTED;
        } else if (!MQTTServer->connected()) {
          _state = MQTT_DISCONNECTED;
        }
      }
    }
    if(mutex != NULL && asyncMode){
      xSemaphoreGive(mutex);
    }
    if(WiFi.status() != WL_CONNECTED && _state == MQTT_CONNECTED){
      _state = DISCONNECTED;
    }
    ConnectionManager::connectionHandler();
    
    switch (_state) {
      case MQTT_CONNECTED:
        lastMQTTConnectedIstant = millis();
        lastWiFiConnectedIstant = millis();
        MQTTReconnectionTimes = 0;
        if (!WPSConfigurated) {
          debugPort->println("[LOG MQTT]: Enabling WPS after reconnetion!");
          setupWPS();
        }
        break;
      case MQTT_DISCONNECTED:
      case MQTT_SUBSCRIBE_FAILED:
        MQTTReconnect();
        break;
    }
  }else{
    if(mutex != NULL && asyncMode){
      xSemaphoreGive(mutex);
    }
  }
}

void MQTTManager::connectionLedRoutine() {     //TO DO //CONTROLLA IL LED
  static uint32_t t1;
  switch (_state) {
    case NOT_CONNECTED:
    case DISCONNECTED:
      digitalWrite(ConnLedPin, LOW);
      break;
    case WPS_CONNECTION:
      if (millis() > t1 + WPSBlinkInterval) {
        t1 = millis();
        digitalWrite(ConnLedPin, !digitalRead(ConnLedPin));
      }
      break;
    case WPS_TIMEOUT:
    case WPS_FAILED:
      if (millis() < t1 + 2000) {
        int dt = millis() - t1;
        if (dt < 200) {
          digitalWrite(ConnLedPin, HIGH);
        } else if (200 <= dt && dt < 2 * 200) {
          digitalWrite(ConnLedPin, LOW);
        } else if (2 * 200 <= dt && dt < 3 * 200) {
          digitalWrite(ConnLedPin, HIGH);
        } else {
          digitalWrite(ConnLedPin, LOW);
        }
      } else {
        t1 = millis();
      }
      break;
    case MQTT_CONNECTED:
      digitalWrite(ConnLedPin, HIGH);
      break;
    case MQTT_DISCONNECTED:
    case MQTT_SUBSCRIBE_FAILED:
      if (millis() > t1 + MqttDisconnectedBlinkInterval) {
        t1 = millis();
        digitalWrite(ConnLedPin, !digitalRead(ConnLedPin));
      }
      break;
  }
}

void MQTTManager::MQTTReconnect() {
  if(mutex != NULL && asyncMode){
    xSemaphoreTake(mutex, portMAX_DELAY);
  }
  if (MQTTServer != NULL && !MQTTServer->connected()) {
    //Reboot for reconnection times
    if (RebootOnMQTTReconectionMaxTimes && MQTTReconnectionTimes == maxMQTTReconnetcRetries) {
      debugPort->println("[REBOOT]: for max MQTT reconnection times!!");
      if (onRebootCallback != NULL) {
        onRebootCallback();
        delay(250);
      }
      ESP.restart();
    }
    //Reboot for max disconnected time
    if (RebootOnMQTTReconectionMaxTime && millis() - lastMQTTConnectedIstant > maxMQTTDisconnectedTime) {
      debugPort->println("[REBOOT]: for max MQTT disconnected time!!");
      if (onRebootCallback != NULL) {
        onRebootCallback();
        delay(250);
      }
      ESP.restart();
    }
    if (millis() - MQTTLastConnectionIstant > MQTTIntervalTraConnetion) {
      if (RebootOnMQTTReconectionMaxTimes) {
        debugPort->printf("[LOG]: Tentativo di riconnessione al server MQTT n %d di %d...\n", MQTTReconnectionTimes + 1, maxMQTTReconnetcRetries);
      } else {
        debugPort->printf("[LOG]: Tentativo di riconnessione al server MQTT n  %d...\n", MQTTReconnectionTimes + 1);
      }
      startMQTT();
      MQTTReconnectionTimes++;
    }
  }
  if(mutex != NULL && asyncMode){
    xSemaphoreGive(mutex);
  }
}