/*
    Connection Manager created by Alan Masutti on 06th Mar 2022
                                   Last modify on 28th Jun 2022
      Ver 1.0 based on ESP32

      Notes:
        - La parte ereditata funziona
        - Set del server MQTT               [DONE]
        - Connessione MQTT                  [DONE]
        - Lampeggio del LED                 [DONE]
        - Aggiunta la funzione per stampare
          lo stato in stringa               [DONE]

      To DO List
        - Sistemare i costruttori           [DONE]
        - Reboot options                    [DONE]
        - Aggiungere i setter               [DONE]
        - Dinamic MQTT server
*/

#ifndef __MQTT_MANAGER_H__
#define __MQTT_MANAGER_H__

#include "ConnectionManager.h"
#include <PubSubClient.h>
#include <WiFiClient.h>


//Costanti di stato
#define MQTT_CONNECTED 7
#define MQTT_DISCONNECTED CONNECTED
#define MQTT_SUBSCRIBE_FAILED 8

//Costanti timing
#define MQTT_RECONNECT_INTERVAL 5 * SEC  //Tempo minimo tra una connessione e l'altra
#define MQTT_DISCONNECTED_BLINK_INTERVAL 1000
#define REBOOT_ON_NOT_MQTT_RECONNECTION_FOR_RETRIES false  //Modalita di reboot dopo aver raggiunto il max tentativo di riconnesione
#define MAX_MQTT_RECONNECTION_TIMES 15                     //Numero massimo di tentativi per riconnessione
#define REBOOT_ON_NOT_MQTT_RECONNECTION_FOR_TIME false     //Modalita di reboot dopo aver raggiunto il tempo di disc. max
#define MAX_MQTT_DISCONNECTED_TIME 10 * MIN                //tempo massimo di disconnessione


class MQTTManager : public ConnectionManager {
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
    ~MQTTManager() {};

    //Setter
    void setMQTTServer(PubSubClient* server, String deviceName, const char* ipAddress, uint16_t port = 1883);
    void setMQTTServer(PubSubClient* server, String deviceName, uint8_t* ipAddress, uint16_t port = 1883);
    void setMQTTServer(PubSubClient* server, String deviceName, IPAddress ipAddress, uint16_t port = 1883);
    void setCallback(MQTT_CALLBACK_SIGNATURE);
    void setTopics(String* topics, uint16_t len);
    void setDisconnectBlink(uint16_t interval);
    void setRebootOptions(bool forTime = REBOOT_ON_NOT_RECONNECTION_FOR_TIME,
                          bool forRetries = REBOOT_ON_NOT_RECONNECTION_FOR_RETRIES,
                          uint16_t max_time = MAX_DISCONNECTED_TIME,
                          uint32_t max_retries = MAX_RECONNECTION_TIMES,
                          bool forMQTTTime = REBOOT_ON_NOT_MQTT_RECONNECTION_FOR_TIME,
                          bool forMQTTRetries = REBOOT_ON_NOT_MQTT_RECONNECTION_FOR_RETRIES,
                          uint16_t maxMQTTTtime = MAX_MQTT_DISCONNECTED_TIME,
                          uint32_t maxMQTTRetries = MAX_MQTT_RECONNECTION_TIMES);

    //Getter
    String getStringState();

    //Metodi
    byte startMQTT();

    //Override methods
    virtual void loop(bool withServer = true, bool withOTA = true) override;
    virtual void setHomepage() override;
    virtual void startConnection(bool withWPS = true) override;

  protected:
    PubSubClient *MQTTServer;

    //intervalli per le connessioni
    uint16_t MQTTIntervalTraConnetion;
    uint32_t MQTTLastConnectionIstant;

    //intervalli per il reboot
    bool RebootOnMQTTReconectionMaxTimes;
    uint16_t MQTTReconnectionTimes;
    uint16_t maxMQTTReconnetcRetries;
    bool RebootOnMQTTReconectionMaxTime;
    uint16_t maxMQTTDisconnectedTime;
    uint32_t lastMQTTConnectedIstant;

    void MQTTReconnect();
    virtual void connectionHandler() override;         //TO DO //gestisce lo stato e le riconnessioni, return: stato
    virtual void connectionLedRoutine() override;      //TO DO //CONTROLLA IL LED
    uint16_t MqttDisconnectedBlinkInterval;
    String _name;
    String* topics;
    uint16_t topicsLen;
    void subscribe(String* topics, uint16_t len);
};

#endif
