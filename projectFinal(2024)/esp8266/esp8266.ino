#include <ESP8266WiFi.h>     // ESP8266 보드용
#include <PubSubClient.h>
#include <SoftwareSerial.h>

// ESP에서 메가로 메시지 보내는 시간 설정
const long interval = 3000; // 0.5초
unsigned long lastMsgTime = 0;

// Wi-Fi 설정
const char* ssid = "Dahun";   // Wi-Fi SSID (이름)
const char* password = "dlekgns1@";   // Wi-Fi 비밀번호

// MQTT 브로커 설정
const char* mqtt_server = "mqtt.eclipseprojects.io";  // MQTT 브로커 주소
const int mqtt_port = 1883;   // MQTT 포트 (기본적으로 1883)

// WiFi 및 MQTT 클라이언트 객체 생성
WiFiClient espClient;
PubSubClient client(espClient);


SoftwareSerial espSerial(D2, D3); // D2 -> RX, D3 -> TX

// 메시지 저장 변수
String lastReceivedMessage = "Hello";

// 수신한 MQTT 메시지를 처리하는 콜백 함수
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);

  Serial.print("Message: ");
  lastReceivedMessage = "";  // 이전 메시지 초기화
  for (int i = 0; i < length; i++) {
    lastReceivedMessage += (char)payload[i];
  }
  Serial.println(lastReceivedMessage);
  Serial.println("-----------------------");
}

// Wi-Fi 연결 함수
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// MQTT 서버에 연결하는 함수
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ArduinoClient")) {
      Serial.println("connected");
      // 주제 구독
      client.subscribe("dahun_test");  // 구독할 주제(topic)
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  espSerial.begin(9600);  // SoftwareSerial 시작

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  // MQTT 클라이언트가 연결되어 있지 않으면 재연결 시도
  if (!client.connected()) {
    reconnect();
  }

  // 클라이언트의 네트워크 이벤트를 처리
  client.loop();

  unsigned long now = millis();
  if (now - lastMsgTime >= interval) {
    lastMsgTime = now;
    if (lastReceivedMessage != "") {  // 메시지가 존재할 때만 전송
      espSerial.println(lastReceivedMessage);
      Serial.println("Sent to Arduino Mega: " + lastReceivedMessage);
    }
  }
}
