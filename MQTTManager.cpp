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
}

void MQTTManager::loop(bool withServer, bool withOTA ) {
  ConnectionManager::loop(withServer, withOTA);
  if (MQTTServer != NULL && MQTTServer->connected()) {
    MQTTServer->loop();
  }
}

//TO DO //gestisce lo stato e le riconnessioni, return: stato
void MQTTManager::connectionHandler() { 
  //Controllo di stato
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
}
