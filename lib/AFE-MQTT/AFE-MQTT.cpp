#include "AFE-MQTT.h"
#include "AFE-MQTT-callback.cpp"

AFEMQTT::AFEMQTT() {}

void AFEMQTT::begin(
  const char * domain,
  uint16_t port,
  const char* user,
  const char* password,
  const char* subscribe_to,
  const char* device_name) {

  Serial << endl << "INFO: Initializaing MQTT: ";
  mqttUser = user;
  mqttPassword = password;
  sprintf(mqttTopicForSubscription, "%s#", (char*)subscribe_to);
  deviceName = device_name;
  Broker.setClient(esp);
  Broker.setServer(domain, port);
  Broker.setCallback(callbackMQTT);
}


void AFEMQTT::connect() {

 if (sleepMode) {
   if (millis() - sleepStartTime >= durationBetweenNextConnectionAttemptsSeries*1000) {
      sleepMode = false;
   }
 } else {

  uint8_t connections = 0;

  Serial << endl << "INFO: Connecting to MQTT";

  while (!Broker.connected()) {
    if (Broker.connect(deviceName, mqttUser, mqttPassword)) {
      Serial << endl << "INFO: Connected";
      Serial << endl << "INFO: Subscribing to : " << mqttTopicForSubscription;
      Broker.subscribe((char*)mqttTopicForSubscription);
      Serial << endl << "INFO: Subsribed";
    } else {
      connections++;
      Serial << endl << "INFO: MQTT Connection attempt: " << connections+1 << " from " << noConnectionAttempts;
      if (connections >= noConnectionAttempts) {
        sleepMode = true;
        sleepStartTime = millis();
        Serial << endl << "WARN: Not able to connect to MQTT.Going to sleep mode for " << durationBetweenNextConnectionAttemptsSeries << "sec.";
        break;
      }
        delay(durationBetweenConnectionAttempts*1000);
        Serial << ".";
    }
  }
  Serial << endl << "INFO: MQTT connection status: " << Broker.state();
  }
}


void AFEMQTT::setReconnectionParams(
  uint8_t no_connection_attempts,
  uint8_t duration_between_connection_attempts,
  uint8_t duration_between_next_connection_attempts_series) {
    noConnectionAttempts = no_connection_attempts;
    durationBetweenConnectionAttempts = duration_between_connection_attempts;
    durationBetweenNextConnectionAttemptsSeries = duration_between_next_connection_attempts_series;
}

boolean AFEMQTT::connected() {
  return Broker.connected();
}

void  AFEMQTT::loop() {
  Broker.loop();
}

void AFEMQTT::publish(const char* type, const char* message) {
    char _mqttTopic[50];
    sprintf(_mqttTopic, "%s%s", mqttTopic, type);
    publishToMQTTBroker(_mqttTopic, message);
}

void AFEMQTT::publish(const char* topic, const char* type, const char* message) {
  char _mqttTopic[50];
  sprintf(_mqttTopic, "%s%s", topic, type);
  publishToMQTTBroker(_mqttTopic, message);
}


void AFEMQTT::publishToMQTTBroker(const char* topic, const char* message) {
  if (Broker.state() == MQTT_CONNECTED) {
    Serial << endl << "INFO: MQTT publising:  " << topic << "  \\ " << message;
    Broker.publish(topic, message);
  } else {
    Serial << endl << "WARN: MQTT not connected. State:  " << Broker.state();
  }
}