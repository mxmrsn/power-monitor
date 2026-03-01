const int kHallPin = A2; // ADC pin connected to hall sensor output
const int kLedPin = LED_BUILTIN;

float g_offset = 1.65; // placeholder mid-rail voltage (calibration overwrites)
float g_scale = 0.030; // placeholder scale (A per volt)

void setup() {
  pinMode(kLedPin, OUTPUT);
  Serial.begin(115200);
  analogReadResolution(12);
  // TODO: load calibration from EEPROM
}

void loop() {
  float sample = analogRead(kHallPin) * (3.3 / 4095.0);
  float corrected = sample - g_offset;
  float current = corrected * g_scale;
  Serial.print("I=");
  Serial.println(current, 3);
  digitalWrite(kLedPin, !digitalRead(kLedPin));
  delay(250);
}
