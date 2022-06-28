#include "ConnectionManager.h"
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <esp_wps.h>
#include <ArduinoOTA.h>

ConnectionManager::ConnectionManager():
  ConnectionManager(true, CONNECTION_LED_PIN, WIFI_MAX_INITIAL_TIMEOUT, WIFI_RECONNECT_INTERVAL, WPS_BLINK_INTERVAL, CONN_BUTTON_PIN, CONN_BUTTON_PIN_MODE, CONN_BUTTON_MODE)
{}

ConnectionManager::ConnectionManager(  bool autoReconnect,
                                       byte connection_led_pin,
                                       uint32_t wifi_max_initial_timeout,
                                       uint32_t wifi_min_time_tra_connection,
                                       uint32_t wps_blink_interval,
                                       byte conn_button_pin,
                                       byte conn_button_pin_mode,
                                       byte conn_button_mode) {
  WiFiAutoReconnect = autoReconnect;
  ConnLedPin = connection_led_pin;
  WiFiMaxInitialTimeout = wifi_max_initial_timeout;
  WiFiIntervalTraConnetion = wifi_min_time_tra_connection;
  WPSBlinkInterval = wps_blink_interval;
  ConnButtonPin = conn_button_pin;
  ConnButtonPinMode = conn_button_pin_mode;
  ConnButtonMode = conn_button_mode;
  serverPtr = NULL;
  OTAHostname = "";
  OTAStarted = false;
  serverStarted = false;
  //WPS Default configuration
  WPSConfigured = false;
  WPSDisabled = false;
  WPSDefaultConfig = true;
  lastConnectionIstant = 0;
  _state = NOT_CONNECTED;

  //Reboot
  RebootOnReconectionMaxTimes = REBOOT_ON_NOT_RECONNECTION_FOR_RETRIES;
  maxReconnetcRetries = MAX_RECONNECTION_TIMES;
  reconnectionTimes = 0;
  RebootOnReconectionMaxTime = REBOOT_ON_NOT_RECONNECTION_FOR_TIME;
  maxDisconnectedTime = MAX_DISCONNECTED_TIME;
  lastWiFiConnectedIstant = 0;
  onRebootCallback = NULL;
}

uint8_t ConnectionManager::getState()const {
  return _state;
}

String ConnectionManager::getOTAHostname()const {
  return OTAHostname;
}

String ConnectionManager::toString()const {
  return String(_state) + " " +  String(WiFiAutoReconnect) + " " + String(WiFiMaxInitialTimeout) + " " + String(WiFiIntervalTraConnetion) +
         " " + String(WPSBlinkInterval) + " " + String(ConnLedPin) + " " + String(ConnButtonPin) + " " + String(ConnButtonPinMode) +
         " " + String(WPSConfigured) + " " + String(WPSDefaultConfig) + " " + String(OTAStarted) + " " + String(OTAHostname) +
         " " + String(version) + " " + String(serverStarted);
}

String ConnectionManager::getStringState() {
      switch (_state) {
          case FIRST_CONNECTION: return "FIRST_CONNECTION";
          case CONNECTED: return "CONNECTED";
          case DISCONNECTED: return "DISCONNECTED";
          case NOT_CONNECTED: return "NOT_CONNECTED";
          case WPS_CONNECTION: return "WPS_CONNECTION";
          case WPS_FAILED: return "WPS_FAILED";
          case WPS_TIMEOUT: return "WPS_TIMEOUT";
      }
      return "";
    }

void ConnectionManager::setAutoReconnect(bool en) {
  WiFiAutoReconnect = en;
}

void ConnectionManager::setMaxInitialTimeout(uint16_t time) {
  WiFiMaxInitialTimeout = time;
}

void ConnectionManager::setReconnectTimeout(uint16_t time) {
  WiFiIntervalTraConnetion = time;
}

void ConnectionManager::setWPSBlinkInterval(uint16_t time) {
  WPSBlinkInterval = time;
}

void ConnectionManager::setWPSSpech(const esp_wps_config_t *spech) {
  config = *spech;
}

void ConnectionManager::setRebootOptions(bool forTime, bool forRetries, uint16_t max_time, uint32_t max_retries) {
  maxReconnetcRetries = max_retries;
  RebootOnReconectionMaxTimes = forRetries;
  maxDisconnectedTime = max_time;
  RebootOnReconectionMaxTime = forTime;
}

void ConnectionManager::setOnRebootCallback(void (*callback)(void)) {
  onRebootCallback = callback;
}

void ConnectionManager::setVersion(String ver) {
  version = ver;
}

void ConnectionManager::setOTAHostname(String h) {
  OTAHostname = h;
}

void ConnectionManager::setHomepage() {
  if (serverPtr != NULL) {
    serverPtr->on("/", [&]() {
      serverPtr->send(200, "text/html", "<a href=\"/infoAbout\"><button>Info</button></a><p><a href=\"/reboot\"><button>Reboot</button></a><p><a href=\"/rebootOnly\"><button>Only reboot</button></a>");
    });
  }
}

void ConnectionManager::setServer(WebServer *s, bool withHomepage) {
  serverPtr = s;
  if (withHomepage) {
    setHomepage();
  }
  if (version != "") {
    serverPtr->on("/infoAbout", [&]() {
      serverPtr->send(200, "text/html", "<a href=\"/\"><button>ROOT PAGE</button></a><p> Versione firmfare: " + version);
    });
  }
  serverPtr->on("/reboot", [&]() {
    serverPtr->sendHeader("Location", String("/"), true); //how to do a redirect, next two lines
    serverPtr->send(302, "text/plain", " ");
    if (onRebootCallback != NULL) {
      onRebootCallback();
    }
    ESP.restart();
  });
  serverPtr->on("/rebootOnly", [&]() {
    serverPtr->sendHeader("Location", String("/"), true); //how to do a redirect, next two lines
    serverPtr->send(302, "text/plain", " ");
    ESP.restart();
  });
}

void ConnectionManager::setStaticIPAddress(IPAddress ip, IPAddress gateway, IPAddress subnet, IPAddress DNS, IPAddress DNS2) {
  WiFi.config(ip, gateway, subnet, DNS, DNS2);
}

void ConnectionManager::startConnection(bool withWPS) {
  _state = FIRST_CONNECTION;
  setupWiFi();
  WiFiConnect();
  if (withWPS) {
    setupWPS();
  }
}

void ConnectionManager::startWiFi(const char ssid[], const char pass[], byte retries, bool bloc) {
  bool timeout = false;
  _state = FIRST_CONNECTION;
  setupWiFi();
  uint32_t startConnectionIstant = millis();
  uint32_t t1 = millis();
  if (!bloc) {
    if ((retries + 1) * 1000 > WiFiMaxInitialTimeout) {
      debugPort->println("Attenzione, sono stati impostati troppi tentativi, la connessione corrompera' il tempo massimo di connessione!");
    }
    for (int i = -1; i < retries; i++) {
      lastConnectionIstant = millis();
      debugPort->printf("Tentativo di connessione n %d di %d...\n", i + 2, retries + 1);
      WiFi.begin(ssid, pass);
      while (millis() < lastConnectionIstant + 1000 && WiFi.status() != WL_CONNECTED) {
        if (millis() > startConnectionIstant + WiFiMaxInitialTimeout) {
          timeout = true;
          break;
        }
        if (millis() < t1 + 200) {
          digitalWrite(ConnLedPin, HIGH);
        } else {
          if (millis() > t1 + 700) {
            t1 = millis();
          }
          digitalWrite(ConnLedPin, LOW);
        }
      }
      digitalWrite(ConnLedPin, LOW);
      if (timeout) {
        debugPort->println("La connessione ha impiegato più tempo del dovuto, ARRESTATA!");
        _state = NOT_CONNECTED;
        break;
      }
    }
  } else {
    uint32_t i = 0;
    while (WiFi.status() != WL_CONNECTED) {
      i++;
      WiFi.begin(ssid, pass);
      lastConnectionIstant = millis();
      debugPort->printf("Tentativo di connessione n %d...\n", i);
      while (millis() < lastConnectionIstant + WiFiIntervalTraConnetion && WiFi.status() != WL_CONNECTED) {
        if (millis() < t1 + 200) {
          digitalWrite(ConnLedPin, HIGH);
        } else {
          if (millis() > t1 + 700) {
            t1 = millis();
          }
          digitalWrite(ConnLedPin, LOW);
        }
      }
    }
  }
}

void ConnectionManager::startOTA() {
  if (OTAHostname != "") {
    ArduinoOTA.setHostname(OTAHostname.c_str());
  }
  ArduinoOTA
  .onStart([&]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    debugPort->println("Start updating " + type);
  })
  .onEnd([&]() {
    debugPort->println("\nEnd");
  })
  .onProgress([&](unsigned int progress, unsigned int total) {
    debugPort->printf("Progress: %u%%\r", (progress / (total / 100)));
  })
  .onError([&](ota_error_t error) {
    debugPort->printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) debugPort->println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) debugPort->println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) debugPort->println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) debugPort->println("Receive Failed");
    else if (error == OTA_END_ERROR) debugPort->println("End Failed");
  });
  ArduinoOTA.begin();
  OTAHostname = ArduinoOTA.getHostname();
  OTAStarted = true;
}

void ConnectionManager::startWebServer() {
  if (serverPtr != NULL) {
    serverPtr->begin();
    serverStarted = true;
  } else {
    debugPort->println("Errore nell'inizializzare il WebServer, server non impostato!");
  }
}


void ConnectionManager::loop(bool withServer, bool withOTA) {
  _loop(withServer, withOTA);
  connectionHandler();
  connectionLedRoutine();
}



//PRVATE METODS
void ConnectionManager::_loop(bool withServer, bool withOTA) {
  //Components loops //OK
  if (_state == CONNECTED) {
    if (withServer && serverStarted) {
      serverPtr->handleClient();
    }
    if (withOTA && OTAStarted) {
      ArduinoOTA.handle();
    }
  }
  if (ConnButtonMode == PULLUP) {
    ConnButtonState = !digitalRead(ConnButtonPin);
  } else if (ConnButtonMode == PULLDOWN) {
    ConnButtonState = digitalRead(ConnButtonPin);
  }

  if (ConnButtonState == 1) {
    //Fronte di salita
    if (ConnButtonState && !ConnButtonStateP ) {
      ConnButtonIstantUP = millis();
      if (_state == WPS_TIMEOUT || _state == WPS_FAILED) {
        debugPort->println("Ripristino la connessione");
        WiFiAutoReconnect = true;
        _state = DISCONNECTED;
      }
      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Disconnecting from WiFi!");
        WiFi.disconnect();
        WiFiAutoReconnect = false;
      } else {
        WiFiAutoReconnect = true;
      }

    }
    //Fronte di discesa
    if (!ConnButtonState && ConnButtonStateP) {
      ConnButtonIstantDWN = millis();
    }

    //WPS CONNECTION ON BUTTON PRESSED //OK
    uint32_t dt = ConnButtonIstantUP + CONN_BUTTON_FIRST_INTERVAL;
    if (millis() > dt && millis() < dt + 250 && WPSConfigured && !WPSDisabled) {
      if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Starting WPS connection!");
        WPSConnect();
      } else if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Disconnecting from WiFi!");
        WiFi.disconnect();
        Serial.println("Starting WPS connection!");
        WPSConnect();
      }
    }
  }
  ConnButtonStateP = ConnButtonState;
}

void ConnectionManager::connectionHandler() {        //TO DO //gestisce lo stato e le riconnessioni, return: stato
  //Reconnect //OK
  if(WiFi.status() == WL_DISCONNECTED && _state != WPS_CONNECTION){
    _state = DISCONNECTED;
  }
  switch (_state) {
    case DISCONNECTED:
      if(WiFiAutoReconnect){
        WiFiReconnect();
      }
      break;
    case CONNECTED:
      lastWiFiConnectedIstant = millis();
      if (WPSDisabled) {
        debugPort->println("Enabling WPS after reconnetion!");
        esp_wifi_wps_enable(&config);
        WPSDisabled = false;
      }
      break;
  }
}

void ConnectionManager::connectionLedRoutine() {     //TO DO //CONTROLLA IL LED
  //Led state: //OK
  static uint32_t t1;
  switch (_state) {
    case CONNECTED:
      digitalWrite(ConnLedPin, HIGH);
      break;
    case NOT_CONNECTED: //Da controllare
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
  }
}

void ConnectionManager::setupWiFi() {
  WiFi.onEvent([&](WiFiEvent_t event, system_event_info_t info) {
    WiFiEvent(event, info);
  });
  WiFi.mode(WIFI_STA);
  pinMode(ConnLedPin, OUTPUT);
  pinMode(ConnButtonPin, ConnButtonMode);
  if (ConnButtonMode == PULLUP || ConnButtonPinMode == INPUT_PULLUP) {
    ConnButtonState = HIGH;
    ConnButtonStateP = HIGH;
  }
  digitalWrite(ConnLedPin, LOW);
}

void ConnectionManager::WiFiEvent(WiFiEvent_t event, system_event_info_t info) {
  switch (event) {
    case SYSTEM_EVENT_STA_START:
      debugPort->println("Station Mode Started");
      _state = NOT_CONNECTED;
      break;
    case SYSTEM_EVENT_STA_GOT_IP:
      debugPort->println("Connected to: " + String(WiFi.SSID()));
      debugPort->print("Got IP: ");
      debugPort->println(WiFi.localIP());
      _state = CONNECTED;
      reconnectionTimes = 0;
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      if (_state != DISCONNECTED) {
        debugPort->println("Disconnected from station");
        _state = DISCONNECTED;
      }
      break;
    case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
      debugPort->println("WPS Successfull, stopping WPS and connecting to: " + String(WiFi.SSID()));
      esp_wifi_wps_disable();
      delay(10);
      WiFi.begin();
      break;
    case SYSTEM_EVENT_STA_WPS_ER_FAILED:
      debugPort->println("WPS Failed");
      _state = WPS_FAILED;
      esp_wifi_wps_disable();
      esp_wifi_wps_enable(&config);
      break;
    case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
      debugPort->println("WPS Timeout");
      _state = WPS_TIMEOUT;
      esp_wifi_wps_disable();
      esp_wifi_wps_enable(&config);
      break;
    default:
      break;
  }
}

void ConnectionManager::setupWPS() {
  debugPort->println("Starting WPS service");
  if (WPSDefaultConfig) {
    debugPort->println("Setting default WPS config!");
    setDefaultWPSConfig();
  }
  esp_wifi_wps_enable(&config);
  WPSConfigured = true;
}

void ConnectionManager::setDefaultWPSConfig() {
  config.crypto_funcs = &g_wifi_default_wps_crypto_funcs;
  config.wps_type = ESP_WPS_MODE;
  strcpy(config.factory_info.manufacturer, ESP_MANUFACTURER);
  strcpy(config.factory_info.model_number, ESP_MODEL_NUMBER);
  strcpy(config.factory_info.model_name, ESP_MODEL_NAME);
  strcpy(config.factory_info.device_name, ESP_DEVICE_NAME);
}

void ConnectionManager::WiFiConnect() {
  uint32_t t1;
  uint32_t t2;
  if (millis() > lastConnectionIstant + 5000 || _state == FIRST_CONNECTION) {
    debugPort->println("Trying to connect the last WiFi connection... ");
    for (int i = 0; i < 8; i++) {
      if ( WiFi.status() != WL_CONNECTED) {
        WiFi.begin();
        lastConnectionIstant = millis();
        t1 = millis();
        t2 = millis();
        debugPort->println("Wait for connection...");
        while (millis() < t1 + 1500) {
          //LAMPEGGIO ASINCRONO NON BLOCCANTE
          if (millis() < t2 + 200) {
            digitalWrite(ConnLedPin, HIGH);
          } else {
            if (millis() > t2 + 700) {
              t2 = millis();
            }
            digitalWrite(ConnLedPin, LOW);
          }
        }
      } else {
        return;
      }
    }
    _state = NOT_CONNECTED;
    digitalWrite(ConnLedPin, LOW);
  }
}

void ConnectionManager::WiFiReconnect() {
  if (millis() - lastConnectionIstant > 5000) {
    //Reboot fro retries
    if (RebootOnReconectionMaxTimes && reconnectionTimes == maxReconnetcRetries) {
      debugPort->println("REBOOTING: for max reconnection times!!");
      if (onRebootCallback != NULL) {
        onRebootCallback();
      }
      ESP.restart();
    }
    //Reboot for time
    if (RebootOnReconectionMaxTime && WiFiAutoReconnect && millis() - lastWiFiConnectedIstant > maxDisconnectedTime) {
      debugPort->println("REBOOTING: too much time is passed from the last connected time! ");
      if (onRebootCallback != NULL) {
        onRebootCallback();
        delay(250);
      }
      ESP.restart();
    }
    debugPort->println("Reconnecting... ");
    if (WiFi.status() != WL_CONNECTED && _state != WPS_CONNECTION) {
      lastConnectionIstant = millis();
      if (WPSConfigured && !WPSDisabled) {
        debugPort->println("Disabling WPS for try reconnection!");
        esp_wifi_wps_disable();
        WPSDisabled = true;
      }
      if (RebootOnReconectionMaxTimes) {
        debugPort->printf("Tentativo di riconnessione n %d di %d...\n", reconnectionTimes + 1, maxReconnetcRetries);
      } else {
        debugPort->printf("Tentativo di riconnessione n %d...\n", reconnectionTimes + 1);
      }
      WiFi.begin();
      reconnectionTimes++;
    }
  }
}

void ConnectionManager::WPSConnect() {
  esp_wifi_wps_start(120000);
  _state = WPS_CONNECTION;
}
