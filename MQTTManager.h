/*
    Connection Manager created by Alan Masutti on 06th Mar 2022
                                   Last modify on 06th Mar 2022
      Ver 1.0 based on ESP32

      Notes:
        - Prima prova, classe derivata da conn Manager

      To DO List
        - Dinamic MQTT server
*/
#ifndef __MQTT_HELPER_H__
#define __MQTT_HELPER_H__

#include <ConnectionManager.h>
#include <PubSubClient.h>

#define MQTT_BLINK_INTERVAL 1000

//Costanti di stato
#define MQTT_CONNECTED 7
#define MQTT_DISCONNECTED 8
#define MQTT_SUBSCRIBE_FAILED 9
#define MQTT_STARTED 10

//MQTT_CALLBACK_SIGNATURE

class MQTTManager : public ConnectionManager{
public:
  MQTTManager():ConnectionManager(){
    
  }
  MQTTManager(bool autoReconnect,
            byte connection_led_pin, 
            uint32_t wifi_max_initial_timeout,
            uint32_t wifi_min_time_tra_connection, 
            uint32_t wps_blink_interval, 
            byte conn_button_pin, 
            byte conn_button_pin_mode,
            byte conn_button_mode)
  {
    _name = "";
    mqttClient = NULL;
    wifiClient = NULL;
  }
  MQTTManager(String name);
  MQTTManager(String name, PubSubClient &client, IPAddress ip, uint16_t port);
  MQTTManager(String name, PubSubClient &client, uint8_t *ip, uint16_t port);
  MQTTManager(String name, PubSubClient &client, const char *ip, uint16_t port);
  MQTTManager(bool autoReconnect,
            byte connection_led_pin, 
            uint32_t wifi_max_initial_timeout,
            uint32_t wifi_min_time_tra_connection, 
            uint32_t wps_blink_interval, 
            byte conn_button_pin, 
            byte conn_button_pin_mode,
            byte conn_button_mode,
            String name);
  MQTTManager(bool autoReconnect,
            byte connection_led_pin, 
            uint32_t wifi_max_initial_timeout,
            uint32_t wifi_min_time_tra_connection, 
            uint32_t wps_blink_interval, 
            byte conn_button_pin, 
            byte conn_button_pin_mode,
            byte conn_button_mode,
            String name,
            IPAddress ip,
            uint16_t port);
  MQTTManager(bool autoReconnect,
            byte connection_led_pin, 
            uint32_t wifi_max_initial_timeout,
            uint32_t wifi_min_time_tra_connection, 
            uint32_t wps_blink_interval, 
            byte conn_button_pin, 
            byte conn_button_pin_mode,
            byte conn_button_mode,
            String name,
            uint8_t* ip,
            uint16_t port);
  MQTTManager(bool autoReconnect,
            byte connection_led_pin, 
            uint32_t wifi_max_initial_timeout,
            uint32_t wifi_min_time_tra_connection, 
            uint32_t wps_blink_interval, 
            byte conn_button_pin, 
            byte conn_button_pin_mode,
            byte conn_button_mode,
            String name,
            const char* ip,
            uint16_t port);

  ~MQTTManager(){};

  //Setter
  void setMQTTServer(const char* ipAddress, uint16_t port = 1883);
  void setMQTTServer(uint8_t* ipAddress, uint16_t port = 1883);
  void setMQTTServer(IPAddress ipAddress, uint16_t port = 1883);
  void setCallback(MQTT_CALLBACK_SIGNATURE);
  void setMQTTClient(PubSubClient &client);
  void setWiFiClient(WiFiClient &WIFIClient);
  void setDisconnectBlink(uint16_t interval);
  void setName(String &name);


  //Getter

  void subscribe(String topics[], uint16_t len);
  bool start();
  virtual void loop(bool withServer = true, bool withOTA = true);
protected:
  WiFiClient *wifiClient;
  PubSubClient *mqttClient;
  uint16_t MqttDisconnectedBlinkInterval;
  String _name;
};

#endif