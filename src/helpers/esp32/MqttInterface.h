#include <WiFi.h>
#include <cstdint>
#include <deque>
#include <mqtt/ArduinoMqttClient.h>
#include <string>
#include <vector>
#include <mutex>

struct Message {
  String message;
  String topic;
};

class MqttInterface {

public:
  MqttInterface(String brokerAddress, uint16_t port);
  void setWifiClient(const WiFiClient &client);
  void begin();
  void loop();
  void onMqttMessage(int messageSize);
  static void mqttMessageWrapper(int messageSize);
  void reconnect();
  void connect();
  void pushMsg(Message msgRf);
  bool isConnected() const;
  void setTopic(String &topic) const;
  unsigned int getMsgCount() const;
  Message getLastMsg() const;
  void setCredential(String user, String pass) const;

  static MqttInterface *instance;

private:
  String _brokerAddress;
  String _mqttUsername, _mqttPassword;
  uint16_t _port;
  unsigned long _reconnectTime = 0;
  std::vector<String> _topicVc;
  std::deque<Message> _messageVc;
  std::vector<Message> _outMessageVc;
  MqttClient _mqttClient;
  WiFiClient _client;
  std::mutex _messageOutMtx;

public:
  inline bool isConnected() { return _mqttClient.connected(); }

  inline unsigned int getMsgCount() { return _messageVc.size(); }

  inline Message getLastMsg() {
    Message msg = _messageVc.front();
    _messageVc.pop_front();
    return msg;
  }

  inline void setTopic(String &topic) { _topicVc.push_back(topic); }
  inline void setCredential(String user, String pass) {
    _mqttUsername = user;
    _mqttPassword = pass;
  };
};

#include <Arduino.h>
#define MQTT_DEBUG_PRINT(F, ...) Serial.printf("MQTT: " F, ##__VA_ARGS__)
#define MQTT_DEBUG_PRINTLN(F, ...) Serial.printf("MQTT: " F "\n", ##__VA_ARGS__)
