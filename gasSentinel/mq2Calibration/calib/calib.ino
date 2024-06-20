// Define the analog pin connected to the MQ-2 sensor
#define MQ2_PIN 1 // GPIO1 corresponds to ADC1_CHANNEL_0 on the ESP32

// Define the load resistance (RL) on the sensor in kΩ
#define RL 5.0

// Number of samples to average
#define NUM_SAMPLES 100

void setup() {
  // Initialize serial communication for debugging
  Serial.begin(115200);
  
  Serial.println("Warming up the sensor, please wait...");
  Serial.println("Warm-up complete. Starting calibration...");
}

void loop() {
  // Take multiple readings and average them
  float Rs_total = 0;
  bool validReading = true;

  for (int i = 0; i < NUM_SAMPLES; i++) {
    int sensorValue = analogRead(MQ2_PIN);
    Serial.println(sensorValue);
    float Vout = sensorValue * (5.0 / 4095.0); // Convert ADC value to voltage (ESP32 ADC resolution is 12-bit)
    Serial.println(Vout);
    if (Vout <= 0 || Vout >= 5.0) {
      Serial.println("Error: Vout out of range.");
      validReading = false;
      break;
    }

    float Rs = ((5.0 - Vout) * RL) / Vout;     // Calculate sensor resistance
    Rs_total += Rs;
    delay(100); 
  }

  if (validReading) {
    float R0 = Rs_total / NUM_SAMPLES; // Calculate average Rs to get R0
    Serial.print("R0 (Clean Air Resistance) = ");
    Serial.print(R0);
    Serial.println(" kΩ");
  } else {
    Serial.println("Calibration failed ");
  }
  
  // Stop the loop after calibration
  while (1);
}
