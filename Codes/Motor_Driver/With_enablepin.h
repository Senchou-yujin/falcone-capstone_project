// L298N Module 1 (Motors A & B)
#define IN1_1 12  // Motor A direction
#define IN2_1 14  // Motor A direction
#define IN3_1 27  // Motor B direction
#define IN4_1 26  // Motor B direction

// L298N Module 2 (Motors C & D)
#define IN1_2 32  // Motor C direction
#define IN2_2 33  // Motor C direction
#define IN3_2 25  // Motor D direction
#define IN4_2 13  // Motor D direction

// Single Enable Pin (Connected to ENA1, ENB1, ENA2, ENB2)
#define ENABLE_PIN 5   // PWM control for all motors

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  Serial.println("Motor Test with Single Enable Pin...");

  // Set all motor control pins as OUTPUT
  pinMode(IN1_1, OUTPUT); pinMode(IN2_1, OUTPUT);
  pinMode(IN3_1, OUTPUT); pinMode(IN4_1, OUTPUT);
  pinMode(IN1_2, OUTPUT); pinMode(IN2_2, OUTPUT);
  pinMode(IN3_2, OUTPUT); pinMode(IN4_2, OUTPUT);

  // Set Enable Pin as OUTPUT
  pinMode(ENABLE_PIN, OUTPUT);

  // Start with motors off
  stopAllMotors();
}

void loop() {
  // Set full speed
  analogWrite(ENABLE_PIN, 255);  // Full speed (0-255 range)
  Serial.println("Setting Speed to Full (255)");

  // Move all motors forward
  Serial.println("Moving Forward...");
  moveForward();
  delay(3000);

  // Stop motors
  Serial.println("Stopping Motors...");
  stopAllMotors();
  delay(1000);

  // Reduce speed to 128 (half speed)
  analogWrite(ENABLE_PIN, 128);
  Serial.println("Setting Speed to Half (128)");

  // Move all motors backward
  Serial.println("Moving Backward...");
  moveBackward();
  delay(3000);

  // Stop motors
  Serial.println("Stopping Motors...");
  stopAllMotors();
  delay(1000);
}

// Function to move all 4 motors forward
void moveForward() {
  Serial.println("Motors Running: Forward");
  digitalWrite(IN1_1, HIGH); digitalWrite(IN2_1, LOW);
  digitalWrite(IN3_1, HIGH); digitalWrite(IN4_1, LOW);
  digitalWrite(IN1_2, HIGH); digitalWrite(IN2_2, LOW);
  digitalWrite(IN3_2, HIGH); digitalWrite(IN4_2, LOW);
}

// Function to move all 4 motors backward
void moveBackward() {
  Serial.println("Motors Running: Backward");
  digitalWrite(IN1_1, LOW); digitalWrite(IN2_1, HIGH);
  digitalWrite(IN3_1, LOW); digitalWrite(IN4_1, HIGH);
  digitalWrite(IN1_2, LOW); digitalWrite(IN2_2, HIGH);
  digitalWrite(IN3_2, LOW); digitalWrite(IN4_2, HIGH);
}

// Function to stop all motors
void stopAllMotors() {
  Serial.println("Motors Stopped");
  digitalWrite(IN1_1, LOW); digitalWrite(IN2_1, LOW);
  digitalWrite(IN3_1, LOW); digitalWrite(IN4_1, LOW);
  digitalWrite(IN1_2, LOW); digitalWrite(IN2_2, LOW);
  digitalWrite(IN3_2, LOW); digitalWrite(IN4_2, LOW);

  analogWrite(ENABLE_PIN, 0);  // Cut motor power
}
