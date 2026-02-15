#include <Wire.h>

#define MPU_ADDR 0x68
#define LED_PIN 2
#define BUTTON_PIN 4

int16_t AcX, AcY, AcZ;

float referencePitch = 0;
bool calibrated = false;

unsigned long slouchStartTime = 0;
bool slouchDetected = false;

void setup() {
  Serial.begin(115200);
  Wire.begin();

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);   // external pull-down

  // Wake up MPU6050
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

  Serial.println("MPU6050 Initialized");
}

void loop() {

  // ---- Read Accelerometer ----
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 6, true);

  AcX = Wire.read() << 8 | Wire.read();
  AcY = Wire.read() << 8 | Wire.read();
  AcZ = Wire.read() << 8 | Wire.read();

  float Ax = AcX / 16384.0;
  float Ay = AcY / 16384.0;
  float Az = AcZ / 16384.0;

  float pitch = atan2(Ax, sqrt(Ay * Ay + Az * Az)) * 180 / PI;

  Serial.print("Pitch: ");
  Serial.println(pitch);

  // ---- Calibration ----
  if (digitalRead(BUTTON_PIN) == HIGH) {
    referencePitch = pitch;
    calibrated = true;
    Serial.println("Calibrated!");
    delay(300);  // simple debounce
  }

  // ---- Posture Logic ----
  if (calibrated) {

    float deviation = pitch - referencePitch;

    if (deviation > 20) {   // forward slouch threshold

      if (!slouchDetected) {
        slouchStartTime = millis();
        slouchDetected = true;
      }

      if (millis() - slouchStartTime >= 5000) {
        Serial.println("Sustained Forward Slouch!");
        digitalWrite(LED_PIN, HIGH);
      }

    } else {
      slouchDetected = false;
      digitalWrite(LED_PIN, LOW);
      Serial.println("Good Posture");
    }
  }

  Serial.println("-------------------");
  delay(200);
}
