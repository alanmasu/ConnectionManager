#include "MQTTManager.h"
#include <WebServer.h>

WiFiClient espClient;
PubSubClient client(espClient);

MQTTManager prova;

WebServer server(80);

void provaCallback() {
  Serial.println("Questa è una callbak");
}

void callback(char* topic, byte* message, unsigned int length) {

  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");

  char messageChar[length];
  String messageTemp;
  String toSend;
  char toSendChar[12];

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
    messageChar[i] += (char)message[i];
  }

}

void setup() {
  // put your setup code here, to run once:
  String ver = String(__FILE__) + " Time: " + String(__DATE__) + " " + String(__TIME__);
  Serial.begin(115200);
  Serial.println("Versione: " + ver);

  prova.setVersion(ver);
  prova.setServer(&server, true);
  prova.startConnection();
  //prova.startWiFi("Alan", "alanmasu2001");
  //prova.startWiFi("alan-PC", "alanmasu2001");
  prova.startWebServer();
  prova.setOTAHostname("prova");
  prova.startOTA();
  prova.setOnRebootCallback(provaCallback);
  prova.setRebootOptions(false, true, false, true);
  Serial.println("You can reach me olso at: " + prova.getOTAHostname() + ".local/");

  //Provo le features di MQTT
  Serial.println("Setto il server MQTT");
  const char* mosquittoServer = "test.mosquitto.org";
  //prova.setMQTTServer(&client, String("myEsp32Alan2001"), mosquittoServer);
  prova.setMQTTServer(&client, "myEsp32Alan2001", "test.mosquitto.org");

  String topics[1] = {"try/"};
  prova.setTopics(topics, 1);
  prova.startMQTT();
  prova.setCallback(callback);
  //prova
}

uint32_t t = 0;

void loop() {
  // put your main code here, to run repeatedly:
  prova.loop();
  if (millis() - t > 1000) {
    t = millis();
    Serial.println(client.connected());
    Serial.println(prova.getStringState());
  }
}
