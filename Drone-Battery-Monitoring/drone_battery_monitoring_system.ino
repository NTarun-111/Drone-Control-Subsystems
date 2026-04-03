// Drone Battery Monitoring System
// Monitors Li-Po battery voltage and provides status indication
// Green LED: Battery OK (>7.4V)
// Red LED + Buzzer: Low Battery (<7.4V)

// Pin definitions
const int batteryPin = A0;    // Analog pin for battery voltage reading
const int greenLED = 8;       // Battery OK indicator
const int redLED = 9;         // Low battery indicator  
const int buzzer = 7;         // Low battery alarm

// Voltage divider constants
const float R1 = 10000.0;     // 10kΩ resistor (high side)
const float R2 = 4700.0;      // 4.7kΩ resistor (low side)
const float voltageDividerRatio = (R1 + R2) / R2;  // Scaling factor

// Battery thresholds
const float batteryLowThreshold = 7.4;   // Low battery threshold (V)
const float batteryFullVoltage = 8.4;    // Typical Li-Po full charge (V)
const float adcReference = 5.0;          // Arduino ADC reference voltage

// Timing variables
unsigned long previousMillis = 0;
const unsigned long readInterval = 500;  // Read battery every 500ms
unsigned long buzzerMillis = 0;
const unsigned long buzzerInterval = 200; // Buzzer beep interval
bool buzzerState = false;

// Battery status variables
float batteryVoltage = 0.0;
float batteryPercentage = 0.0;
bool batteryLow = false;
bool previousBatteryLow = false;

void setup() {
  // Initialize serial communication
  Serial.begin(9600);
  
  // Initialize digital pins
  pinMode(greenLED, OUTPUT);
  pinMode(redLED, OUTPUT);
  pinMode(buzzer, OUTPUT);
  
  // Turn off all outputs initially
  digitalWrite(greenLED, LOW);
  digitalWrite(redLED, LOW);
  digitalWrite(buzzer, LOW);
  
  // Display startup information
  Serial.println("=====================================");
  Serial.println("   DRONE BATTERY MONITORING SYSTEM   ");
  Serial.println("=====================================");
  Serial.println("Battery Type: Li-Po (Simulated)");
  Serial.print("Low Battery Threshold: ");
  Serial.print(batteryLowThreshold);
  Serial.println("V");
  Serial.print("Voltage Divider Ratio: ");
  Serial.println(voltageDividerRatio);
  Serial.println("=====================================");
  Serial.println();
  Serial.println("Time(s) | Voltage | Percentage | Status | Action");
  Serial.println("--------|---------|------------|--------|--------");
  
  delay(2000); // Startup delay
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Read battery voltage at specified interval
  if (currentMillis - previousMillis >= readInterval) {
    previousMillis = currentMillis;
    
    // Read and process battery voltage
    readBatteryVoltage();
    
    // Check battery status and update outputs
    updateBatteryStatus();
    
    // Print status to serial monitor
    printBatteryStatus(currentMillis);
  }
  
  // Handle buzzer beeping pattern for low battery
  if (batteryLow) {
    handleLowBatteryAlarm(currentMillis);
  }
  
  delay(10); // Small delay for system stability
}

void readBatteryVoltage() {
  // Read ADC value (0-1023)
  int adcValue = analogRead(batteryPin);
  
  // Convert ADC value to voltage at the analog pin
  float adcVoltage = (adcValue * adcReference) / 1023.0;
  
  // Scale up to actual battery voltage using voltage divider ratio
  batteryVoltage = adcVoltage * voltageDividerRatio;
  
  // For potentiometer testing: map 0-5V input to realistic battery range
  // Comment out these lines if using real 9V battery
  batteryVoltage = map(batteryVoltage * 100, 0, 500, 600, 900) / 100.0; // Maps to 6.0V - 9.0V range
  
  // Calculate battery percentage (simplified linear model)
  // Note: Real Li-Po batteries have non-linear discharge curves
  batteryPercentage = ((batteryVoltage - 6.0) / (batteryFullVoltage - 6.0)) * 100.0;
  
  // Constrain percentage between 0-100%
  batteryPercentage = constrain(batteryPercentage, 0.0, 100.0);
}

void updateBatteryStatus() {
  // Store previous state for change detection
  previousBatteryLow = batteryLow;
  
  // Determine battery status
  batteryLow = (batteryVoltage < batteryLowThreshold);
  
  if (batteryLow) {
    // Low battery condition
    digitalWrite(greenLED, LOW);   // Turn off green LED
    digitalWrite(redLED, HIGH);    // Turn on red LED
    
    // Alert if status just changed
    if (!previousBatteryLow) {
      Serial.println("*** LOW BATTERY ALERT ***");
    }
  } else {
    // Battery OK condition
    digitalWrite(greenLED, HIGH);  // Turn on green LED
    digitalWrite(redLED, LOW);     // Turn off red LED
    digitalWrite(buzzer, LOW);     // Turn off buzzer
    buzzerState = false;
    
    // Alert if status just changed
    if (previousBatteryLow) {
      Serial.println("*** Battery Voltage Restored ***");
    }
  }
}

void handleLowBatteryAlarm(unsigned long currentMillis) {
  // Create beeping pattern for low battery alarm
  if (currentMillis - buzzerMillis >= buzzerInterval) {
    buzzerMillis = currentMillis;
    buzzerState = !buzzerState;
    digitalWrite(buzzer, buzzerState ? HIGH : LOW);
  }
}

void printBatteryStatus(unsigned long currentTime) {
  // Calculate elapsed time in seconds
  float elapsedTime = currentTime / 1000.0;
  
  // Print formatted status line
  Serial.print(elapsedTime, 1);
  Serial.print("     | ");
  Serial.print(batteryVoltage, 2);
  Serial.print("V   | ");
  Serial.print(batteryPercentage, 1);
  Serial.print("%       | ");
  
  if (batteryLow) {
    Serial.print("LOW    | RED+BUZZ");
  } else {
    Serial.print("OK     | GREEN   ");
  }
  
  Serial.println();
  
  // Print detailed info every 10 readings
  static int readingCount = 0;
  readingCount++;
  
  if (readingCount >= 10) {
    readingCount = 0;
    Serial.println();
    Serial.print("ADC Reading: ");
    Serial.print(analogRead(batteryPin));
    Serial.print("/1023 | Estimated Flight Time: ");
    
    if (batteryPercentage > 20) {
      Serial.print(batteryPercentage * 0.3, 1); // Rough estimate
      Serial.println(" minutes");
    } else {
      Serial.println("LAND IMMEDIATELY");
    }
    Serial.println();
  }
}

// Function to calibrate the voltage reading (call once with known voltage)
void calibrateVoltage(float actualVoltage) {
  int adcValue = analogRead(batteryPin);
  float adcVoltage = (adcValue * adcReference) / 1023.0;
  float calculatedRatio = actualVoltage / adcVoltage;
  
  Serial.print("Calibration: Use ratio = ");
  Serial.println(calculatedRatio);
}