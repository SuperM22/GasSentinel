// Define the analog pin connected to the MQ-2 sensor
#define MQ2_PIN 36 

// kΩ
#define RL 5.0

// Number of samples to average
#define NUM_SAMPLES 1000

void setup() {
  Serial.begin(115200);
  
  Serial.println("Warming up the sensor, please wait...");
  //delay(30 * 60 * 1000); // 30 minutes delay for warm-up
  Serial.println("Warm-up complete. Starting calibration...");
}

void loop() {
  // Take multiple readings and average them
  float Rs_total = 0;
  for (int i = 0; i < NUM_SAMPLES; i++) {
    int sensorValue = analogRead(MQ2_PIN);
    float Vout = sensorValue * (5.0 / 4095.0); // Convert ADC value to voltage (ESP32 ADC resolution is 12-bit)
    float Rs = ((5.0 - Vout) * RL) / Vout;     // Calculate sensor resistance
    Rs_total += Rs;
    delay(100); 
  }

  float R0 = Rs_total / NUM_SAMPLES; // Calculate average Rs to get R0
  Serial.print("R0 (Clean Air Resistance) = ");
  Serial.print(R0);
  Serial.println(" kΩ");
  
  // Stop the loop after calibration
  while (1);
}
