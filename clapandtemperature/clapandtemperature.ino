#include <Servo.h> // include servo library
#include <DHT.h>

#define buzzerpin D5
#define dhtpin D2

Servo servooff;
DHT dht(dhtpin, DHT22);
float temperature = 0;
bool temperatureChecked = false;

void setup() {
  servooff.attach(D3);
  servooff.write(90);
  pinMode(A0, INPUT); // set A0 as input for microphone sensor
  pinMode(buzzerpin, OUTPUT);
  dht.begin();
}

void loop() {
  temperature = dht.readTemperature();
  int micVal = analogRead(A0); // read microphoneS sensor value
  if (micVal > 1000) { // if loud sound is detected (clap)
    servooff.write(0);
    delay(1000);
    servooff.write(90);
    for(int i = 0; i < 2; i++){
      tone(buzzerpin, HIGH);
      delay(100);
      noTone(buzzerpin);
    }
    delay(500); // delay to prevent multiple claps being detected
  }

  if (temperature > 30 && !temperatureChecked) {
    servooff.write(0);
    delay(1000);
    servooff.write(90);
    temperatureChecked = true;
    for(int i = 0; i < 3; i++){
      tone(buzzerpin, HIGH);
      delay(100);
      noTone(buzzerpin);
      delay(100);
    }
  }
  // Penanda agar servo bisa kembali bergerak jika suhu turun kemudian naik lagi ke 30 derajat
  else if (temperature <= 30 && temperatureChecked) {
    temperatureChecked = false;
  }
}