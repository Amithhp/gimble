#include <Wire.h>
#include <Servo.h>

// --- Servo Configuration ---
Servo pitchServo;
Servo rollServo;
Servo yawServo;

const int PITCH_PIN = 9;
const int ROLL_PIN  = 10;
const int YAW_PIN   = 11;

// --- Joystick Configuration ---
const int JOY_X_PIN = A0; // Yaw (Pan)
const int JOY_Y_PIN = A1; // Pitch (Tilt)
const int JOY_SW_PIN = 2; // Push-button for instant re-centering

// --- MPU6050 Configuration ---
const int MPU = 0x68;

float AccX, AccY, AccZ;
float GyroX, GyroY, GyroZ;
float accAngleX, accAngleY;
float roll = 0, pitch = 0;

// Dynamic accumulators for manual control
float pitchTarget = 90.0;
float yawTarget   = 90.0;

// Calibration variables
float AccErrorX = 0, AccErrorY = 0;
float GyroErrorX = 0, GyroErrorY = 0, GyroErrorZ = 0;

float elapsedTime, currentTime, previousTime;
int c = 0;

void setup() {
  Serial.begin(115200);

  // Configure switch pin with internal pull-up (reads LOW when clicked)
  pinMode(JOY_SW_PIN, INPUT_PULLUP);

  // Attach and home servos
  pitchServo.attach(PITCH_PIN);
  rollServo.attach(ROLL_PIN);
  yawServo.attach(YAW_PIN);

  pitchServo.write(90);
  rollServo.write(90);
  yawServo.write(90);

  // Initialize I2C communication
  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B); // Power management register
  Wire.write(0x00); // Wake up MPU6050
  Wire.endTransmission(true);

  // Calibrate IMU
  Serial.println("Calibrating IMU... Keep flat and still.");
  calculate_IMU_error();
  Serial.println("Calibration complete.");

  delay(100);
  previousTime = millis();
}

void loop() {
  // === 0. Check Joystick Center Button === //
  // With INPUT_PULLUP, unpressed = HIGH, pressed = LOW
  if (digitalRead(JOY_SW_PIN) == LOW) {
    pitchTarget = 90.0;
    yawTarget   = 90.0;
    roll        = 0.0;
    pitch       = 0.0;
  }

  // === 1. Read Accelerometer Data === //
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 6, true);

  AccX = (Wire.read() << 8 | Wire.read()) / 16384.0;
  AccY = (Wire.read() << 8 | Wire.read()) / 16384.0;
  AccZ = (Wire.read() << 8 | Wire.read()) / 16384.0;

  accAngleX = (atan(AccY / sqrt(pow(AccX, 2) + pow(AccZ, 2))) * 180 / PI) - AccErrorX;
  accAngleY = (atan(-1 * AccX / sqrt(pow(AccY, 2) + pow(AccZ, 2))) * 180 / PI) - AccErrorY;

  // === 2. Read Gyroscope Data === //
  currentTime = millis();
  elapsedTime = (currentTime - previousTime) / 1000.0;
  previousTime = currentTime;

  Wire.beginTransmission(MPU);
  Wire.write(0x43);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 6, true);

  GyroX = ((Wire.read() << 8 | Wire.read()) / 131.0) - GyroErrorX;
  GyroY = ((Wire.read() << 8 | Wire.read()) / 131.0) - GyroErrorY;
  GyroZ = ((Wire.read() << 8 | Wire.read()) / 131.0) - GyroErrorZ;

  // === 3. Complementary Filter === //
  roll  = 0.96 * (roll  + GyroX * elapsedTime) + 0.04 * accAngleX;
  pitch = 0.96 * (pitch + GyroY * elapsedTime) + 0.04 * accAngleY;

  // === 4. Process Joystick Stick Inputs === //
  int rawJoyX = analogRead(JOY_X_PIN);
  int rawJoyY = analogRead(JOY_Y_PIN);

  // Deadband around center (512 +/- 30)
  int joyDeltaX = (abs(rawJoyX - 512) > 30) ? (rawJoyX - 512) : 0;
  int joyDeltaY = (abs(rawJoyY - 512) > 30) ? (rawJoyY - 512) : 0;

  yawTarget   += (joyDeltaX / 512.0) * 0.7;
  pitchTarget -= (joyDeltaY / 512.0) * 0.7;

  yawTarget   = constrain(yawTarget, 10, 170);
  pitchTarget = constrain(pitchTarget, 20, 160);

  // === 5. Map to Servo Positions === //
  int servoRollPos  = constrain(map((int)roll, -90, 90, 180, 0), 0, 180);
  int servoPitchPos = constrain((int)pitchTarget - (int)pitch, 0, 180);
  int servoYawPos   = constrain((int)yawTarget, 0, 180);

  pitchServo.write(servoPitchPos);
  rollServo.write(servoRollPos);
  yawServo.write(servoYawPos);

  delay(10);
}

void calculate_IMU_error() {
  c = 0;
  while (c < 200) {
    Wire.beginTransmission(MPU);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 6, true);

    AccX = (Wire.read() << 8 | Wire.read()) / 16384.0;
    AccY = (Wire.read() << 8 | Wire.read()) / 16384.0;
    AccZ = (Wire.read() << 8 | Wire.read()) / 16384.0;

    AccErrorX += (atan((AccY) / sqrt(pow((AccX), 2) + pow((AccZ), 2))) * 180 / PI);
    AccErrorY += (atan(-1 * (AccX) / sqrt(pow((AccY), 2) + pow((AccZ), 2))) * 180 / PI);
    c++;
    delay(5);
  }
  AccErrorX /= 200.0;
  AccErrorY /= 200.0;

  c = 0;
  while (c < 200) {
    Wire.beginTransmission(MPU);
    Wire.write(0x43);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 6, true);

    GyroX = Wire.read() << 8 | Wire.read();
    GyroY = Wire.read() << 8 | Wire.read();
    GyroZ = Wire.read() << 8 | Wire.read();

    GyroErrorX += (GyroX / 131.0);
    GyroErrorY += (GyroY / 131.0);
    GyroErrorZ += (GyroZ / 131.0);
    c++;
    delay(5);
  }
  GyroErrorX /= 200.0;
  GyroErrorY /= 200.0;
  GyroErrorZ /= 200.0;
}
