
#include <SoftwareSerial.h>
SoftwareSerial espSerial(7, A0);  // RX = D7, TX = A0 (không dùng TX)


///Sonic sensor
const unsigned int TRIG_PIN = A3;  
const unsigned int ECHO_PIN = A2;  


//Chuyen dong hong ngoai hoặc quang trở 
const int zero = 0;
const int one = 1;

///
const int sensor1 = 6;
const int sensor2 = 5;
const int sensor3 = 4;
const int sensor4 = 3;
const int sensor5 = 2;


// ==================== CẤU HÌNH PIN ====================
const int in1 = 13;  // Motor A (Trái)
const int in2 = 12;
const int ena = 11;

const int in3 = 9;   // Motor B (Phải)
const int in4 = 8;
const int enb = 10;

// ==================== MACRO CHIỀU QUAY ====================
#define MOTOR_A_FORWARD()  { digitalWrite(in1, HIGH); digitalWrite(in2, LOW);  } //BÊN PHẢI NGƯỜI NGỒI XE
#define MOTOR_A_BACKWARD() { digitalWrite(in1, LOW);  digitalWrite(in2, HIGH); } 
#define MOTOR_A_STOP()     { digitalWrite(in1, LOW);  digitalWrite(in2, LOW);  } 

// ==================== MOTOR B = TRÁI (theo hướng người ngồi) ====================
#define MOTOR_B_FORWARD()  { digitalWrite(in3, HIGH); digitalWrite(in4, LOW);  } //BÊN TRÁI NGƯỜI NGỒI XE
#define MOTOR_B_BACKWARD() { digitalWrite(in3, LOW);  digitalWrite(in4, HIGH); } 
#define MOTOR_B_STOP()     { digitalWrite(in3, LOW);  digitalWrite(in4, LOW);  } 

bool checkOutLined = true;


bool CheckBienBao = false;
bool CheckOj = false;

int motorSpeed = 75;

long unsigned lastTime=0;
bool CheckFinish = false;
unsigned long lastDistanceTime = 0;
int distance = 0;



void setup() {

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(sensor1, INPUT);
  pinMode(sensor2, INPUT);
  pinMode(sensor3, INPUT);
  pinMode(sensor4, INPUT);
  pinMode(sensor5, INPUT);


  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(ena, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  pinMode(enb, OUTPUT);

  Serial.begin(9600);
  espSerial.begin(9600);       // Khớp với ESP32 baud rate

}

void loop() { 

unsigned long currentTime = millis();
  if (currentTime - lastDistanceTime > 100) { 
    lastDistanceTime = currentTime;

    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    unsigned long duration = pulseIn(ECHO_PIN, HIGH, 20000); 
    distance = duration / 29 / 2;
    Serial.println(distance);
  }


  int s1 = digitalRead(sensor1);
  int s2 = digitalRead(sensor2);
  int s3 = digitalRead(sensor3);
  int s4 = digitalRead(sensor4);
  int s5 = digitalRead(sensor5);


  if (s1 == zero && s2 == zero && s3 == zero && s4 == zero && s5 == zero && CheckBienBao ) {
    if (CheckFinish) {
      while(true){
        stopCar();
      }
    }
    CheckFinish = true;    
  }
  

  if (distance > 0 && distance <= 40) {
    stopCar();
   while (!CheckBienBao) {
        CheckFinish = false;
        stopCar();

        if (espSerial.available()) {
        String msg = espSerial.readStringUntil('\n');
        while (espSerial.available()) {
          espSerial.read();
        }
        
      if (msg.indexOf("Left") != -1 || msg.indexOf("Right") != -1) {
        int start = msg.indexOf('(');
        int end = msg.indexOf('%');
        Serial.println(msg);

        if (start != -1 && end != -1) {
          String percentStr = msg.substring(start + 1, end);
          float confidence = percentStr.toFloat(); 
          if (msg.indexOf("Left") != -1 && confidence > 85.0) {
              CheckOj = false;
              CheckBienBao = true;
              searchForLine();
          }
          else if (msg.indexOf("Right") != -1 && confidence > 85.0) {
              CheckOj = true;
              CheckBienBao = true;
              searchForLine();
             }
           }
        }
      }
    }
  }

  else {
           if (CheckOj == false){
            if (s1 == zero && s2 == zero && s3 == zero && s4 == zero && s5 == zero && CheckBienBao) {
              FourWayHandle();
              searchForLineIn();        
            }
              else  {
                  PIDControl();
            }
           } else {
            if (s1 == zero && s2 == zero && s3 == zero && s4 == zero && s5 == zero  && CheckBienBao) {
              FourWayHandle();
              searchForLine();        
            }
              else  {
                  PIDControl();
            }
           }
  }
}




void PIDControl() {

              int s1 = digitalRead(sensor1);
              int s2 = digitalRead(sensor2);
              int s3 = digitalRead(sensor3);
              int s4 = digitalRead(sensor4);
              int s5 = digitalRead(sensor5);
            
              if (s1 == 1 && (s2 == 0 || s3 == 0 || s4 == 0) && s5 == 1) {
                forward(); 
              } else if (s1 == 1 && s2 == 1 && s3 == 1 && s4 == 1 && s5 == 0) {
                turnLeft();
              } else if (s1 == 0 && s2 == 1 && s3 == 1 && s4 == 1 && s5 == 1) {
                turnRight();
              } else if (s1 == 1 && s2 == 1 && s3 == 1 && s4 == 0 && s5 == 0) {
                turnLeft();
              } else if (s1 == 0 && s2 == 0 && s3 == 1 && s4 == 1 && s5 == 1) {
                turnRight();
              }
}



void FourWayHandle() {
  forward();
  unsigned long startTime = millis();
  while (millis() - startTime < 600) {
    int s1 = digitalRead(sensor1);
    int s2 = digitalRead(sensor2);
    int s3 = digitalRead(sensor3);
    int s4 = digitalRead(sensor4);
    int s5 = digitalRead(sensor5);
    if (s1 == 0 || s2 == 0 || s3 == 0 || s4 == 0 || s5 == 0) {
      break;
    }
  }
  
  stopCar();
  delay(100);
}


void searchForLineIn() {
  bool foundLine = false;
  spinLeft();
  delay(400);
  while (!foundLine) {

    int s1 = digitalRead(sensor1);
    int s2 = digitalRead(sensor2);
    int s3 = digitalRead(sensor3);
    int s4 = digitalRead(sensor4);
    int s5 = digitalRead(sensor5);
    if (s1 == 0 || s2 == 0 || s3 == 0 || s4 == 0 || s5 == 0) {
      stopCar();
      foundLine = true;  
    }
  }
}

void searchForLine() {
  bool foundLine = false;
  
  spinRight();  
  delay(400);

  while (!foundLine) {

    int s1 = digitalRead(sensor1);
    int s2 = digitalRead(sensor2);
    int s3 = digitalRead(sensor3);
    int s4 = digitalRead(sensor4);
    int s5 = digitalRead(sensor5);
    if (s1 == 0 || s2 == 0 || s3 == 0 || s4 == 0 || s5 == 0) {
      stopCar();
      foundLine = true;  
    }
  }
}


// Quay tại chỗ trái (hai motor ngược chiều)
void spinLeft() {
  MOTOR_A_BACKWARD();
  MOTOR_B_FORWARD();
  analogWrite(ena, 130);
  analogWrite(enb, 130);
}

// Quay tại chỗ phải (hai motor ngược chiều)
void spinRight() {
  MOTOR_A_FORWARD();
  MOTOR_B_BACKWARD();
  analogWrite(ena, 130);
  analogWrite(enb, 130);
}
// Hàm in ra trạng thái tiến
void forward() {
  MOTOR_A_FORWARD();
  MOTOR_B_FORWARD();
  analogWrite(ena, motorSpeed);
  analogWrite(enb, motorSpeed);
  Serial.println("Chay thang");
}



// Hàm điều khiển xe rẽ trái
void turnLeft() {
  MOTOR_A_BACKWARD();
  MOTOR_B_FORWARD();
  analogWrite(ena, 120);
  analogWrite(enb, 120);
  Serial.println("Chay trai");

}

// Hàm điều khiển xe rẽ phải
void turnRight() {
  MOTOR_A_FORWARD();
  MOTOR_B_BACKWARD();
  analogWrite(ena, 120);
  analogWrite(enb, 120);
  Serial.println("Chay Phai");

}

void stopCar() {
  analogWrite(ena, 0);
  analogWrite(enb, 0);
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);  
}