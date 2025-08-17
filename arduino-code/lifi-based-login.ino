#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

#define LDR_PIN 2   // LDR digital pin
#define SERVO1_PIN 3 // 1st Servo pin
#define SERVO2_PIN 5 // 2nd Servo pin
#define SERVO3_PIN 6 // 3rd Servo pin

Servo myServo1; // 1st Servo
Servo myServo2; // 2nd Servo
Servo myServo3; // 3rd Servo

LiquidCrystal_I2C lcd(0x27, 16, 2);  // LCD at I2C address 0x27

int receivedCode[4] = {0, 0, 0, 0}; // Stores detected digits
int codeIndex = 0; // Track detected digits

unsigned long startTime = 0;       // Tracks when light is detected
unsigned long lastDigitTime = 0;   // Tracks when the last digit was detected
bool lightDetected = false;

int digitTimes[10] = {100, 150, 200, 250, 300, 350, 400, 450, 500, 550}; // Timing for 0-9
int threshold = 30; // Acceptable error margin for timing

int wrongAttempts = 0; // Track number of wrong attempts
unsigned long lockoutStartTime = 0; // Tracks when the lockout period starts
bool isLockedOut = false; // Indicates if the system is in a lockout state

void setup() {
    Serial.begin(9600); // Set baud rate for Arduino Uno
    pinMode(LDR_PIN, INPUT);
    
    // Attach servos to their respective pins
    myServo1.attach(SERVO1_PIN);
    myServo2.attach(SERVO2_PIN);
    myServo3.attach(SERVO3_PIN);
    
    // Initialize servos to locked position (0 degrees)
    myServo1.write(0);
    myServo2.write(0);
    myServo3.write(0);

    // Initialize LCD
    lcd.init();
    lcd.backlight();
    updateLockStatus(); // Display initial lock status
    lcd.setCursor(0, 1); // Move cursor to the second line
    lcd.print("Enter Code:");
}

void loop() {
    // Check if the system is in a lockout state
    if (isLockedOut) {
        if (millis() - lockoutStartTime >= 30000) { // 30 seconds lockout
            isLockedOut = false; // End lockout
            wrongAttempts = 0; // Reset wrong attempts counter
            lcd.clear();
            updateLockStatus(); // Update lock status
            lcd.setCursor(0, 1); // Move cursor to the second line
            lcd.print("Enter Code:");
        } else {
            // Display lockout message
            lcd.setCursor(0, 1);
            lcd.print("Locked! Wait ");
            lcd.print((30000 - (millis() - lockoutStartTime)) / 1000);
            lcd.print("s");
            return; // Skip the rest of the loop during lockout
        }
    }

    int ldrState = digitalRead(LDR_PIN);

    // Check if the timeout has occurred (3 seconds since the last digit)
    if (codeIndex > 0 && millis() - lastDigitTime > 3000) {
        Serial.println("Timeout: Resetting code input.");
        lcd.clear();
        lcd.print("Timeout!");
        delay(2000); // Display "Timeout!" for 2 seconds
        updateLockStatus(); // Update lock status
        lcd.setCursor(0, 1); // Move cursor to the second line
        lcd.print("Enter Code:");
        resetCode(); // Reset the code input
    }

    if (ldrState == LOW && !lightDetected) { 
        startTime = millis(); // Start timing when light is detected
        lightDetected = true;
    } 
    else if (ldrState == HIGH && lightDetected) { 
        unsigned long duration = millis() - startTime;
        detectDigit(duration);
        lightDetected = false;
    }

    if (codeIndex == 4) {
        checkCode();
        resetCode(); // Reset the code input after checking
    }
}

void detectDigit(unsigned long duration) {
    int detectedNumber = -1;

    for (int i = 0; i < 10; i++) {
        if (abs((int)duration - digitTimes[i]) < threshold) {
            detectedNumber = i;
            break;
        }
    }

    if (detectedNumber != -1 && codeIndex < 4) {
        receivedCode[codeIndex] = detectedNumber;
        Serial.print("Detected: ");
        Serial.println(detectedNumber);
        codeIndex++;
        lastDigitTime = millis(); // Update the time of the last detected digit
    }
}

void checkCode() {
    lcd.clear();
    
    bool codeValid = false; // Flag to check if the code is valid

    // Check for 1st Servo codes
    if (receivedCode[0] == 1 && receivedCode[1] == 9 && receivedCode[2] == 6 && receivedCode[3] == 0) {
        Serial.println("Correct Code 1960: Unlocking 1st Servo!");
        myServo1.write(180);
        lcd.print("1st Safe Unlocked");
        codeValid = true;
    }
    else if (receivedCode[0] == 2 && receivedCode[1] == 0 && receivedCode[2] == 2 && receivedCode[3] == 0) {
        Serial.println("Reset Code 2020: Locking 1st Servo!");
        myServo1.write(0);
        lcd.print("1st Safe Locked");
        codeValid = true;
    }

    // Check for 2nd Servo codes
    if (receivedCode[0] == 1 && receivedCode[1] == 9 && receivedCode[2] == 4 && receivedCode[3] == 7) {
        Serial.println("Correct Code 1947: Unlocking 2nd Servo!");
        myServo2.write(180);
        lcd.print("2nd Safe Unlocked");
        codeValid = true;
    }
    else if (receivedCode[0] == 2 && receivedCode[1] == 0 && receivedCode[2] == 1 && receivedCode[3] == 6) {
        Serial.println("Reset Code 2016: Locking 2nd Servo!");
        myServo2.write(0);
        lcd.print("2nd Safe Locked");
        codeValid = true;
    }

    // Check for 3rd Servo codes
    if (receivedCode[0] == 1 && receivedCode[1] == 9 && receivedCode[2] == 4 && receivedCode[3] == 0) {
        Serial.println("Correct Code 1940: Unlocking 3rd Servo!");
        myServo3.write(180);
        lcd.print("3rd Safe Unlocked");
        codeValid = true;
    }
    else if (receivedCode[0] == 2 && receivedCode[1] == 0 && receivedCode[2] == 2 && receivedCode[3] == 2) {
        Serial.println("Reset Code 2022: Locking 3rd Servo!");
        myServo3.write(0);
        lcd.print("3rd Safe Locked");
        codeValid = true;
    }

    // If no valid code was found
    if (!codeValid) {
        Serial.println("Wrong code");
        lcd.print("Wrong code");
        wrongAttempts++; // Increment wrong attempts counter

        // Check if the system should be locked out
        if (wrongAttempts >= 5) {
            isLockedOut = true;
            lockoutStartTime = millis(); // Start the lockout period
            lcd.clear();
            lcd.print("Locked! Wait 30s");
            return; // Skip the rest of the function
        }
    } else {
        wrongAttempts = 0; // Reset wrong attempts counter on correct code
    }

    // Display "Please Wait" during the delay
    lcd.setCursor(0, 1); // Move cursor to the second line
    lcd.print("Please Wait");
    delay(5000);  // Delay for 5 seconds

    // Clear the LCD and reset
    updateLockStatus(); // Update lock status
    lcd.setCursor(0, 1); // Move cursor to the second line
    lcd.print("Enter Code:");
}

void resetCode() {
    // Reset the code input
    codeIndex = 0;
    for (int i = 0; i < 4; i++) {
        receivedCode[i] = 0;
    }
    lastDigitTime = 0; // Reset the last digit time
}

void updateLockStatus() {
    lcd.setCursor(0, 0); // Move cursor to the first line

    // Display Lock 1 status
    lcd.print("L1=");
    lcd.print(myServo1.read() == 180 ? "O" : "X");

    // Display Lock 2 status
    lcd.print(" L2=");
    lcd.print(myServo2.read() == 180 ? "O" : "X");

    // Display Lock 3 status
    lcd.print(" L3=");
    lcd.print(myServo3.read() == 180 ? "O" : "X");

    // Clear any extra characters (if necessary)
    lcd.print("   "); // Add spaces to clear any leftover characters
}