#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <math.h>

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

if (abs(angleX) >= 85 && abs(angleX) <= 95) {
    Serial.println("WARNING: Cart is TILTED on X-axis (90 degrees)");
    isCartFallen = true;
} 
if (abs(angleX) >= 175 && abs(angleX) <= 185) {
    Serial.println("ALERT: Cart is UPSIDE-DOWN!");
    isCartFallen = true;
} 
if (abs(angleY) >= 85 && abs(angleY) <= 95) {
    Serial.println("WARNING: Cart is TILTED on Y-axis (90 degrees)");
    isCartFallen = true;
} 
if (abs(angleY) >= 175 && abs(angleY) <= 185) {
    Serial.println("ALERT: Cart is FLIPPED OVER!");
    isCartFallen = true;
}

// If the cart has fallen, take action
if (isCartFallen) {
    Serial.println("!!! CART UNSTABLE - TAKE ACTION !!!");
    // Add buzzer, LED, or wireless alert
}


  Serial.println("");
  delay(500);
}
