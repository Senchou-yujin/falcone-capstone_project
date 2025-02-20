#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <math.h>

//SCL = GPIO 22
//SDA = GPIO 21

Adafruit_MPU6050 mpu;
bool isCartFallen = false;

void setup(void) {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  Serial.println("Adafruit MPU6050 Angle Detection!");

  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);
  
  Serial.println("Initialization complete!");
  delay(100);
}

void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float ax = a.acceleration.x;
  float ay = a.acceleration.y;
  float az = a.acceleration.z;

  // Compute angles using accelerometer data
  float angleX = atan2(ay, az) * 180.0 / PI;
  float angleY = atan2(ax, az) * 180.0 / PI;

  Serial.print("Angle X: "); Serial.print(angleX); Serial.print(" degrees");
  Serial.print(", Angle Y: "); Serial.print(angleY); Serial.println(" degrees");

if (abs(angleX) >= 70 && abs(angleX) <= 110) {  
    Serial.println("WARNING: Cart is HIGHLY TILTED on X-axis!");
    isCartFallen = true;
} 
if (abs(angleY) >= 70 && abs(angleY) <= 110) {  
    Serial.println("WARNING: Cart is HIGHLY TILTED on Y-axis!");
    isCartFallen = true;
} 
if (abs(angleX) >= 140 || abs(angleY) >= 140) {  
    Serial.println("ALERT: Cart is FLIPPED OVER!");
    isCartFallen = true;
}

  Serial.println("");
  delay(500);
}
