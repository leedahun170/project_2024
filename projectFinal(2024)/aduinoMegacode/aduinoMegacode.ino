#include <Servo.h>
int potentiometer = A0;
int linearmotor1_dir1 = 4;
int linearmotor1_en1 = 5;
int linearmotor2_dir1 = 6;
int linearmotor2_en1 = 7;
int dcmotor_dir2 = 8;
int dcmotor_en2 = 9;
int switch1 = 24;
int servo = 2;

Servo myServo;

int angle = 0;


String receivedMessage = "";
int potvalue = analogRead(potentiometer); // 가변저항 값

void setup() {
  Serial.begin(9600);       // 기본 시리얼 통신 (USB 연결용)
  Serial1.begin(9600);
  // 포텐셔미터 핀 설정
  pinMode(potentiometer, INPUT);

  // 선형 모터 핀 설정
  pinMode(linearmotor1_dir1, OUTPUT);
  pinMode(linearmotor1_en1, OUTPUT);
  pinMode(linearmotor2_dir1, OUTPUT);
  pinMode(linearmotor2_en1, OUTPUT);

  // DC 모터 핀 설정
  pinMode(dcmotor_dir2, OUTPUT);
  pinMode(dcmotor_en2, OUTPUT);

  // 스위치 핀 설정
  pinMode(switch1, INPUT_PULLUP);  // 스위치가 풀업 저항을 사용할 경우

  // 서보 모터 핀 설정
  pinMode(servo, OUTPUT);
      // Serial1: RX1 -> 19, TX1 -> 18 (ESP8266과의 통신) / rx - D3, tx - D4

  myServo.attach(servo);
}

void loop() {
  // Serial1로부터 메시지를 수신
  if (Serial1.available()) {
    String receivedMessage = Serial1.readString(); // 수신된 메시지를 문자열로 저장
    Serial.println("Received from ESP8266: " + receivedMessage);
    Serial.println(receivedMessage[0]); // 메시지를 시리얼 모니터에 출력
  
  //모터 구동 알고리즘
  //좌우 모터제어
    if(receivedMessage[0] == 'a'){
      digitalWrite(dcmotor_dir2, HIGH);
      analogWrite(dcmotor_en2, 100); 
    }
    else if(receivedMessage[0] == 'c'){
      digitalWrite(dcmotor_dir2, LOW);
      analogWrite(dcmotor_en2, 100); 
    }
    else if(receivedMessage[0] == 'b'){
      digitalWrite(dcmotor_dir2, LOW);
      analogWrite(dcmotor_en2, 0); 
    }
    // 상하 움직임 제어
    if(receivedMessage[1] == 'd'){
      digitalWrite(linearmotor2_dir1, HIGH);
      analogWrite(linearmotor2_en1, 100); 
    }
    else if(receivedMessage[1] == 'f'){
      digitalWrite(linearmotor2_dir1, LOW);
      analogWrite(linearmotor2_en1, 100); 
    }
    else if(receivedMessage[1] == 'e'){
      digitalWrite(linearmotor2_dir1, LOW);
      analogWrite(linearmotor2_en1, 0); 
    }
    // 덤프, 피칭
    // 피칭 구조
    if(digitalRead(switch1) == 0){
      if(receivedMessage[2] == 'g'){
        myServo.write(0);
      }
      else if(receivedMessage[2] == 'i'){
        myServo.write(180);
      }
      else if(receivedMessage[2] == 'h'){
        myServo.write(90);

      }

      if(potvalue < 500){
        digitalWrite(linearmotor2_dir1, HIGH);
        analogWrite(linearmotor2_en1, 100); 
        // 덤프 구조 살짝 올려서 스위치 off / 시간으로 조정을 할지 코드 한번으로 가능한지 테스트 필요
      }
    }
    // 덤프 구조 on
    else{
      if(receivedMessage[2] == 'g'){
        digitalWrite(linearmotor1_dir1, HIGH);
        analogWrite(linearmotor1_en1, 100); 
        myServo.write(0);
      }
      else if(receivedMessage[2] == 'i'){
        digitalWrite(linearmotor1_dir1, LOW);
        analogWrite(linearmotor1_en1, 100);
        myServo.write(180);         
      }
      else if(receivedMessage[2] == 'h'){
        digitalWrite(linearmotor1_dir1, LOW);
        analogWrite(linearmotor1_en1, 0);
        myServo.write(90);         
 // 서보 모터와 linearmotor동시 구동 각도에 따라 서보 모터 또는 리니어 모터 속도 제어로 각도 정하기
      }
    }

    
    Serial.println(potvalue);
    int switch1_value = digitalRead(switch1);
    Serial.println(switch1_value);       
    Serial.println(receivedMessage[2]);
    Serial.println(receivedMessage);
 // 메시지를 시리얼 모니터에 출력
  }
}
