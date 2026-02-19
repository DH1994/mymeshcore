#include "MqttInterface.h"
#include <cstdint>
#include <mqtt/ArduinoMqttClient.h>
#include <mutex>

MqttInterface *MqttInterface::instance = nullptr;

MqttInterface::MqttInterface(String brokerAddress, uint16_t port)
    : _mqttUsername(), _mqttPassword(), _brokerAddress(brokerAddress),
      _port(port), _topicVc(), _messageVc(), _outMessageVc(), _mqttClient(0) {
  instance = this;
}

void MqttInterface::setWifiClient(const WiFiClient &client) {
  _client = client;
}

void MqttInterface::begin() {
  _mqttClient.setClient(_client);
  _mqttClient.onMessage(mqttMessageWrapper);
  connect();
}

void MqttInterface::loop() {
  if (_mqttClient.connected()) {
    _mqttClient.poll();
    if (!_outMessageVc.empty()) {
      std::lock_guard<std::mutex> lock(_messageOutMtx);
      for (Message &msgRf : _outMessageVc) {
        _mqttClient.beginMessage(msgRf.topic);
        _mqttClient.print(msgRf.message);
        _mqttClient.endMessage();
      }
      _outMessageVc.clear();
    }
  } else {
    reconnect();
  }
}

void MqttInterface::pushMsg(Message msg) {
  std::lock_guard<std::mutex> lock(_messageOutMtx);
  _outMessageVc.push_back(msg);
}

void MqttInterface::connect() {
  _mqttClient.setClient(_client);
  _mqttClient.onMessage(mqttMessageWrapper);
  _mqttClient.setUsernamePassword(_mqttUsername, _mqttPassword);
  if (!_mqttClient.connect(_brokerAddress.c_str(), _port)) {
    MQTT_DEBUG_PRINTLN("Failed to connect");
    return;
  }

  MQTT_DEBUG_PRINTLN("Connected");

  for (String &topicStRf : _topicVc) {
    _mqttClient.subscribe(topicStRf.c_str());
  }
}

void MqttInterface::reconnect() {
  if (millis() - _reconnectTime >= 5000) {
    MQTT_DEBUG_PRINTLN("Reconnecting");
    _mqttClient = MqttClient(0);
    connect();
    _reconnectTime = millis();
  }
}

void MqttInterface::mqttMessageWrapper(int messageSize) {
  if (instance) {
    instance->onMqttMessage(messageSize); // Call the instance method
  }
}

void MqttInterface::onMqttMessage(int messageSize) {
  Message m;
  m.topic = _mqttClient.messageTopic();
  char buf[256] = {0};
  _mqttClient.read((uint8_t *)buf, messageSize);
  m.message = buf;
  MQTT_DEBUG_PRINT("MSG Received: ");
  Serial.print(m.topic);
  Serial.print(", ");
  Serial.println(m.message);
  _messageVc.push_back(m);
}