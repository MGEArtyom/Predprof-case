#include <nRF24L01.h>
#include <printf.h>
#include <RF24.h>
#include <RF24_config.h>

// Подключение библиотек
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// Настройка пинов радиомодулей
#define PIN_RF_CE 9
#define PIN_RF_CSN 10

RF24 radio(PIN_RF_CE, PIN_RF_CSN);

// Адреса радиомодулей (адреса платы Кубсата и станции одинаковые!)
const byte TRANSMIT_ADDRESS[6] = "CMD01";   // сюда шлём "start" (спутник слушает CMD01)
const byte READ_ADDRESS[6] = "TEL01";   // отсюда читаем данные (спутник шлёт в TEL01)

// Структура для передачи данных о состоянии и движении сервомоторов
struct Data {
  uint8_t state;
  int8_t  h;
  int8_t  v;
} data;


// Инициализация радиомодулей
void setup() {
  Serial.begin(9600);

  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_1MBPS);
  radio.openWritingPipe(TRANSMIT_ADDRESS);
  radio.openReadingPipe(1, READ_ADDRESS);
  radio.startListening();

  Serial.println("Type: start");
}

// Основная функция: Если в мониторе порта отправить +, то станция передаст сообщение "start" Кубсату, начнётся движение сервомоторов
void loop() {
  // отправка start из Serial
  if (Serial.available()) {
    char s = Serial.read();
    if (s == '+') {
      char msg[6] = "start";        // ровно 6 байт: s t a r t \0
      radio.stopListening();
      radio.write(msg, sizeof(msg));
      radio.startListening();
      Serial.println("sent start");
    }
  }

  // Приём данных со станции
  if (radio.available()) {
    radio.read(&data, sizeof(data));
    if (data.state == 0) {
      Serial.print("CubeSat cycle over. Type: start");
    }
    else {
      Serial.print("state="); Serial.print(data.state);
      Serial.print(" h=");    Serial.print(data.h);
      Serial.print(" v=");    Serial.println(data.v);
    }
  }
}
