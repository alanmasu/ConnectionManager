/*
    Connection Manager created by Alan Masutti on 16th Feb 2022
                                   Last modify on 28th Jun 2022
      Ver 2.2 based on ESP32

      Notes:
        - Default homepage                                      [DONE]
        - Modularizzato il loop, è tornato virtual!             [DONE]
        - Modificato il _loop, esegue solo quello che non 
          dipende da future implementazioni                     [DONE]
        - Aggiunti i metodi virtuali connectionLedRoutine
           e connectionHandler:                                 [DONE]
          - connectionLedRoutine gestisce il lampeggio del
             LED di connessione;                                [DONE]
          - connectionHandler gestisce lo stato, pensa alla
             riconnessione se serve;                            [DONE]
        - Aggiunta la funzione per stampare lo stato in stringa [DONE]

      To DO List
      	- Controllo della visibilità
        - Debug options
        - Dinamic set of IP address on WebServer
        - Back up of settings
        - List of known Networks
        - AP configurator
*/

#ifndef __CONNECTION_MANAGER_H__
#define __CONNECTION_MANAGER_H__

#ifdef ESP32
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <esp_wps.h>
#include <ArduinoOTA.h>
#else
#error Prevista incompatibilità
#endif

//TIME COSTANTS
#define SEC 1000UL
#define MIN 60 * SEC

//WPS and WiFi
#define ESP_WPS_MODE      WPS_TYPE_PBC
#define ESP_MANUFACTURER  "ALAN MASUTTI"
#define ESP_MODEL_NUMBER  "ESP32"
#define ESP_MODEL_NAME    "ESP-WROOM-32"
#define ESP_DEVICE_NAME   "ESP32"

#define PULLUP 1
#define PULLDOWN 0

//Costants
#define CONNECTION_LED_PIN 2				      //Led di connessione
#define WIFI_MAX_INITIAL_TIMEOUT 30 * SEC	//Timeout in ms massimo per la connessione
#define WIFI_RECONNECT_INTERVAL 10 * SEC	//Tempo in ms tra una connessione e l'altra
#define WPS_BLINK_INTERVAL 250				    //Lampeggio in ms per connessione WPS.				
#define CONN_BUTTON_PIN 0					        //Pin pulsante di controllo per le connessioni
#define CONN_BUTTON_PIN_MODE INPUT			  //Modalita pin di controllo
#define CONN_BUTTON_MODE PULLUP				    //Modalita di lettura pin di controllo
#define CONN_BUTTON_FIRST_INTERVAL 3000		//Primo scatto del pulsante di connessione

#define REBOOT_ON_NOT_RECONNECTION_FOR_RETRIES false	//Modalita di reboot dopo aver raggiunto il max tentativo di riconnesione
#define MAX_RECONNECTION_TIMES 15					//Numero di tentativi
#define REBOOT_ON_NOT_RECONNECTION_FOR_TIME false		  //Modalita di reboot dopo aver raggiunto il tempo di disc. max
#define MAX_DISCONNECTED_TIME 10 * MIN 		//Numero di tentativi

//Costanti di stato
#define FIRST_CONNECTION 0
#define CONNECTED 1
#define DISCONNECTED 2
#define NOT_CONNECTED 3
#define WPS_CONNECTION 4
#define WPS_FAILED 5
#define WPS_TIMEOUT 6

class ConnectionManager {
  public:
    ConnectionManager();										//done
    ConnectionManager(	bool autoReconnect,
                        byte connection_led_pin,
                        uint32_t wifi_max_initial_timeout,
                        uint32_t wifi_min_time_tra_connection,
                        uint32_t wps_blink_interval,
                        byte conn_button_pin,
                        byte conn_button_pin_mode,
                        byte conn_button_mode);	//done
    ~ConnectionManager() {};										//done

    //Getter
    uint8_t getState() const;									//Return connection state
    String getOTAHostname()const;							//ok
    String toString() const;								  //ok
    virtual String getStringState();          //ok

    //Setter
    void setAutoReconnect(bool en);
    void setMaxInitialTimeout(uint16_t time);
    void setReconnectTimeout(uint16_t time);
    void setWPSBlinkInterval(uint16_t time);
    void setWPSSpech(const esp_wps_config_t *spech);
    void setRebootOptions(bool forTime = REBOOT_ON_NOT_RECONNECTION_FOR_TIME,
                          bool forRetries = REBOOT_ON_NOT_RECONNECTION_FOR_RETRIES,
                          uint16_t max_time = MAX_DISCONNECTED_TIME,
                          uint32_t max_retries = MAX_RECONNECTION_TIMES);
    void setOnRebootCallback(void (*callback)(void));
    void setVersion(String ver);								//done
    void setOTAHostname(String h);							//done
    virtual void setHomepage();									//done
    void setServer(WebServer *s, bool withHomepage = false);	//done
    void setDefaultWPSConfig();
    void setStaticIPAddress(IPAddress ip, IPAddress gateway, IPAddress subnet, IPAddress DNS, IPAddress DNS2);
    void configButton(byte pin, byte connPinMode, byte mode);
    void configLedPin(byte pin);

    virtual void startConnection(bool withWPS = true);							//Connect automaticly to last net and in failture case start WPS connection
    void startWiFi(const char ssid[], const char pass[], byte retries = 0, bool bloc = false);	//Start only WiFi
    void startOTA();													//ok
    void startWebServer();												//ok
    virtual void loop(bool withServer = true, bool withOTA = true);		//va modifcato per aggiungere modularità! //Check Connection, reconnect to WiFi in case of losing connection,
    // and hanlde the WPS BUTTON and CONNECTION LED
  protected:
    Stream *debugPort = &Serial;
    byte _state;
    bool WiFiAutoReconnect;
    uint32_t lastConnectionIstant;
    uint32_t WiFiMaxInitialTimeout;
    uint32_t WiFiIntervalTraConnetion;
    uint16_t WPSBlinkInterval;
    byte ConnLedPin;
    byte ConnButtonPin;
    byte ConnButtonPinMode;
    byte ConnButtonMode;
    esp_wps_config_t config;
    bool ConnButtonState;
    bool ConnButtonStateP;
    uint32_t ConnButtonIstantUP;
    uint32_t ConnButtonIstantDWN;
    bool WPSConfigured;
    bool WPSDisabled;
    bool WPSDefaultConfig;
    String version;
    bool OTAStarted;
    String OTAHostname;
    bool serverStarted;
    WebServer *serverPtr;

    bool RebootOnReconectionMaxTimes;
    uint16_t reconnectionTimes;
    uint16_t maxReconnetcRetries;
    bool RebootOnReconectionMaxTime;
    uint32_t lastWiFiConnectedIstant;
    uint32_t maxDisconnectedTime;
    void (*onRebootCallback)(void);

    void _loop(bool withServer = true, bool withOTA = true);		//va modificato per avere la modularità //CONTROLLA WEB SERVER, WPS E BUTTON
    virtual void connectionHandler();         //TO DO //gestisce lo stato e le riconnessioni, return: stato
    virtual void connectionLedRoutine();      //TO DO //CONTROLLA IL LED
    void setupWiFi();												  //ok		//Set WiFi parameters and setup those pins
    void setupWPS();												  //ok		//Set WPS pin and enable this mode
    void WiFiConnect();									 			//ok		//Start the WiFi connection
    void WiFiReconnect();											//ok		//Reconnect the last network, is not a blocking function
    void WPSConnect();												//ok		//Starts the WPS connection
    void WiFiEvent(WiFiEvent_t event, system_event_info_t info);	//ok compatibile con MQTT
};

#endif
