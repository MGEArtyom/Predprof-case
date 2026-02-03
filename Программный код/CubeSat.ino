// Настройка библиотек
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Servo.h>

// Пины сервомоторов, радиомодулей, лазера
#define PIN_RF_CE 9
#define PIN_RF_CSN 10
#define PIN_LASER 2
#define PIN_SERVO_H 7
#define PIN_SERVO_V 8

#define del 500

RF24 radio(PIN_RF_CE, PIN_RF_CSN);

// Адреса передачи и чтения для радиомодуля
const byte READ_ADDRESS[6] = "CMD01";
const byte TRANSMIT_ADDRESS[6] = "TEL01";

Servo servo_h, servo_v;

// Структура для передачи данных о состоянии и движении сервомоторов
struct Data { 
  uint8_t state; 
  int8_t h; 
  int8_t v; 
}data;


bool started = false;

// Инициализация радиомодулей, начальное положение сервомоторов
void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_1MBPS);
  radio.openReadingPipe(1, READ_ADDRESS);
  radio.openWritingPipe(TRANSMIT_ADDRESS);
  radio.startListening();

  pinMode(PIN_LASER, OUTPUT);
  servo_h.attach(PIN_SERVO_H);
  servo_v.attach(PIN_SERVO_V);
  servo_h.write(90);
  servo_v.write(90);
}

// Функция, передающая данные из структуры в станцию через радиомодуль
void send_data(uint8_t state, int8_t h, int8_t v){
  data.state = state; data.h = h; data.v = v;
  digitalWrite(PIN_LASER, HIGH);
  radio.write(&data, sizeof(data));
  delay(del);
  digitalWrite(PIN_LASER, LOW);
}

// Функция, выводящая данные из структуры в монитор порта
void print_data(const Data &x) {
  Serial.print("Data: { state=");
  Serial.print(x.state);
  Serial.print(", h=");
  Serial.print(x.h);
  Serial.print(" deg, v=");
  Serial.print(x.v);
  Serial.println(" deg }");
}


// Функция, отвечающая за движения сервомоторов
void move_servos() {
  // 1) Горизонталь
  for (int i=-40;i<=40;i+=10){ 
    servo_h.write(90+i); 
    servo_v.write(90); 
    send_data(1,i,0); 
    print_data(data);
  }
  servo_h.write(90);

  // 2) Вертикаль
  for (int j=-40;j<=40;j+=10){ 
    servo_v.write(90+j); 
    servo_h.write(90);
    send_data(2,0,j); 
    print_data(data);
  }
  servo_v.write(90);

  // 3) Диагональ 1
  for (int a=-40;a<=40;a+=10){ 
    servo_h.write(90+a); 
    servo_v.write(90+a); 
    send_data(3,a,a);
    print_data(data);
    }

  // 4) Диагональ 2
  for (int a=-40;a<=40;a+=10){ 
    servo_h.write(90+a); 
    servo_v.write(90-a); 
    send_data(4,a,-a);
    print_data(data);
  }

  servo_h.write(90); 
  servo_v.write(90);
  started = false;
  radio.startListening();
}

// Основная функция: принимает сообщение станции "start" и начинает движение сервомоторов
void loop() {
  if (!started) {
    if (radio.available()) {
      char msg[6] = {0};
      radio.read(msg, sizeof(msg));
      if (!strcmp(msg, "start")) {
        started = true;
        radio.stopListening();
      }
    }
  }
  else {
    move_servos();
  }

}
