/*
    Connection Manager created by Alan Masutti on 06th Mar 2022
                                   Last modify on 06th Mar 2022
      Ver 1.0 based on ESP32

      Notes:
        - Prima prova, la classe eredita da ConnectionManager

      To DO List
        - Dinamic MQTT server
*/

#ifndef __MQTT_HELPER_H__
#define __MQTT_HELPER_H__

#include "ConnectionManager.h"
#include <PubSubClient.h>
#include <WiFiClient.h>

#define MQTT_BLINK_INTERVAL 1000

//Costanti di stato
#define MQTT_CONNECTED 7
#define MQTT_DISCONNECTED 8
#define MQTT_SUBSCRIBE_FAILED 9
#define MQTT_STARTED 10

//MQTT_CALLBACK_SIGNATURE

class MQTTManager : public ConnectionManager{
public:
  MQTTManager();
  MQTTManager(bool autoReconnect,
            byte connection_led_pin, 
            uint32_t wifi_max_initial_timeout,
            uint32_t wifi_min_time_tra_connection, 
            uint32_t wps_blink_interval, 
            byte conn_button_pin, 
            byte conn_button_pin_mode,
            byte conn_button_mode);

  ~MQTTManager(){};

  //Setter
//  void setMQTTServer(const char* ipAddress, uint16_t port = 1883);
//  void setMQTTServer(uint8_t* ipAddress, uint16_t port = 1883);
//  void setMQTTServer(IPAddress ipAddress, uint16_t port = 1883);
//  void setCallback(MQTT_CALLBACK_SIGNATURE);
//  void setMQTTClient(PubSubClient &client);
//  void setWiFiClient(WiFiClient &WIFIClient);
//  void setDisconnectBlink(uint16_t interval);
//  void setName(String &name);


  //Getter

//  void subscribe(String topics[], uint16_t len);
//  bool start();

  //Override methods
  virtual void setHomepage() override;
  virtual void startConnection(bool withWPS = true) override;
  virtual void loop(bool withServer = true, bool withOTA = true) override;
  
protected:
  WiFiClient *wifiClient;
  PubSubClient *mqttClient;
  uint16_t MqttDisconnectedBlinkInterval;
  String _name;
};

#endif
