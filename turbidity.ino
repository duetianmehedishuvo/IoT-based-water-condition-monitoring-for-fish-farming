int sensorPin = 12;
float volt;
float ntu;
void setup() {
  Serial.begin(115200);
}

void loop() {
  volt = 0;
  for (int i = 0; i < 800; i++) {
    volt += ((float)analogRead(sensorPin) / 4096) * 3.3; // Conversion for 3.3V, as ESP has a 12-bit ADC
  }
  volt = volt / 800;
  volt = round_to_dp(volt, 2);

  // Transformation of voltage values
  volt = mapf(volt, 0.0, 3.3, 1.83, 3.68); // Mapping actual voltage to the required range

  ntu = 493.17 * pow(volt, 2) - 3749.1 * volt + 7152.3; // Calculating NTU using the provided quadratic equation

  // Limiting the NTU value between 0 and 2000
  if (ntu < 0) {
    ntu = 0;
  } else if (ntu > 2000) {
    ntu = 2000;
  }

  Serial.print(volt);
  Serial.print(" V, ");
  Serial.print(ntu);
  Serial.println(" NTU");
  delay(500);
}

float round_to_dp(float in_value, int decimal_place) {
  float multiplier = powf(10.0f, decimal_place);
  in_value = roundf(in_value * multiplier) / multiplier;
  return in_value;
}

float mapf(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min; // Mapping function for float values
}
