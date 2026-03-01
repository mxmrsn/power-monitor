#include <Arduino.h>
#include <EEPROM.h>
#include <math.h>

constexpr int kHallPin = A2;
constexpr int kVoltagePin = A0;
constexpr bool kVoltageEnabled = true;
constexpr float kVRef = 3.3f;
constexpr float kSampleRate = 4000.0f;
constexpr unsigned long kSampleIntervalUs = 1000000UL / static_cast<unsigned long>(kSampleRate);
constexpr int kWindowSize = 512;
constexpr float kNominalVoltage = 120.0f;
constexpr int kCalibrationSamples = 512;

constexpr int kMagicAddr = 0;
constexpr int kCurrentOffsetAddr = 2;
constexpr int kCurrentScaleAddr = 6;
constexpr int kVoltageOffsetAddr = 10;
constexpr int kVoltageScaleAddr = 14;
constexpr uint16_t kMagicValue = 0xA5A5;

constexpr float kDefaultMidrail = 1.65f;

struct Calibration {
  float current_offset = kDefaultMidrail;
  float current_scale = 0.030f;
  float voltage_offset = kDefaultMidrail;
  float voltage_scale = 1.0f;
  bool valid = false;
};

struct ChannelStats {
  float sum = 0.0f;
  float sum_sq = 0.0f;
  int count = 0;
  float max = -1e6f;
  float min = 1e6f;

  void reset() {
    sum = 0.0f;
    sum_sq = 0.0f;
    count = 0;
    max = -1e6f;
    min = 1e6f;
  }

  void add(float value) {
    sum += value;
    sum_sq += value * value;
    ++count;
    if (value > max) {
      max = value;
    }
    if (value < min) {
      min = value;
    }
  }

  float rms() const {
    return count ? sqrtf(sum_sq / count) : 0.0f;
  }

  float avg() const {
    return count ? sum / count : 0.0f;
  }
};

Calibration g_calib;
ChannelStats g_current_stats;
ChannelStats g_voltage_stats;
unsigned long g_next_sample_us = 0;
float g_energy_wh = 0.0f;

void loadCalibration();
void saveCalibration();
float readHallRaw();
float readVoltageRaw();
void addSample();
void reportWindow();
void processSerial();
void calibrateZeroOffset();
void calibrateScale(float reference_rms);
void calibrateVoltage(float reference_rms);
ChannelStats captureRawSamples(int pin, int samples, bool subtract_offset, float offset);
Calibration defaultCalibration();

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  analogReadResolution(12);
  Serial.begin(115200);
  loadCalibration();
  g_next_sample_us = micros();
}

void loop() {
  processSerial();
  unsigned long now = micros();
  while (static_cast<long>(now - g_next_sample_us) >= 0) {
    addSample();
    g_next_sample_us += kSampleIntervalUs;
    now = micros();
  }

  if (g_current_stats.count >= kWindowSize) {
    reportWindow();
  }
}

void loadCalibration() {
  uint16_t magic = 0;
  EEPROM.get(kMagicAddr, magic);
  if (magic == kMagicValue) {
    EEPROM.get(kCurrentOffsetAddr, g_calib.current_offset);
    EEPROM.get(kCurrentScaleAddr, g_calib.current_scale);
    EEPROM.get(kVoltageOffsetAddr, g_calib.voltage_offset);
    EEPROM.get(kVoltageScaleAddr, g_calib.voltage_scale);
    g_calib.valid = true;
  } else {
    g_calib = defaultCalibration();
    saveCalibration();
  }
}

void saveCalibration() {
  uint16_t magic = kMagicValue;
  EEPROM.put(kMagicAddr, magic);
  EEPROM.put(kCurrentOffsetAddr, g_calib.current_offset);
  EEPROM.put(kCurrentScaleAddr, g_calib.current_scale);
  EEPROM.put(kVoltageOffsetAddr, g_calib.voltage_offset);
  EEPROM.put(kVoltageScaleAddr, g_calib.voltage_scale);
}

float readHallRaw() {
  float voltage = analogRead(kHallPin) * (kVRef / 4095.0f);
  return voltage - g_calib.current_offset;
}

float readVoltageRaw() {
  float voltage = analogRead(kVoltagePin) * (kVRef / 4095.0f);
  return voltage - g_calib.voltage_offset;
}

void addSample() {
  float raw_current = readHallRaw();
  g_current_stats.add(raw_current);
  if (kVoltageEnabled) {
    float raw_voltage = readVoltageRaw();
    g_voltage_stats.add(raw_voltage);
  }
}

void reportWindow() {
  float current_rms = g_current_stats.rms() * g_calib.current_scale;
  float voltage_rms = kVoltageEnabled ? g_voltage_stats.rms() * g_calib.voltage_scale : kNominalVoltage;
  float power = current_rms * voltage_rms;
  float window_seconds = g_current_stats.count / kSampleRate;
  g_energy_wh += (power * window_seconds) / 3600.0f;

  Serial.print("I_rms=");
  Serial.print(current_rms, 3);
  Serial.print("A ");
  if (kVoltageEnabled) {
    Serial.print("V_rms=");
    Serial.print(voltage_rms, 1);
    Serial.print("V ");
  } else {
    Serial.print("(V assumed ");
    Serial.print(kNominalVoltage, 0);
    Serial.print("V) ");
  }
  Serial.print("P=");
  Serial.print(power, 2);
  Serial.print("W E=");
  Serial.print(g_energy_wh, 3);
  Serial.println("Wh");

  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  g_current_stats.reset();
  if (kVoltageEnabled) {
    g_voltage_stats.reset();
  }
}

void processSerial() {
  if (!Serial.available()) {
    return;
  }

  String line = Serial.readStringUntil('\n');
  line.trim();
  if (line.length() == 0) {
    return;
  }

  String upper = line;
  upper.toUpperCase();

  if (upper == "CALIB DUMP") {
    Serial.print("Curr offset=");
    Serial.print(g_calib.current_offset, 4);
    Serial.print(" scale=");
    Serial.print(g_calib.current_scale, 6);
    if (kVoltageEnabled) {
      Serial.print(" Volt offset=");
      Serial.print(g_calib.voltage_offset, 4);
      Serial.print(" scale=");
      Serial.print(g_calib.voltage_scale, 6);
    }
    Serial.println();
  } else if (upper == "CALIB ZERO") {
    calibrateZeroOffset();
  } else if (upper.startsWith("CALIB SCALE")) {
    String payload = line.substring(11);
    payload.trim();
    float reference = payload.toFloat();
    if (reference > 0.0f) {
      calibrateScale(reference);
    } else {
      Serial.println("CALIB SCALE <A_rms> requires numeric value");
    }
  } else if (upper.startsWith("CALIB VOLT")) {
    String payload = line.substring(10);
    payload.trim();
    float reference = payload.toFloat();
    if (kVoltageEnabled && reference > 0.0f) {
      calibrateVoltage(reference);
    } else {
      Serial.println("CALIB VOLT <V_rms> requires voltage sampling enabled and a numeric value");
    }
  } else if (upper == "CALIB RESET") {
    g_calib = defaultCalibration();
    saveCalibration();
    Serial.println("Calibration reset to defaults");
  } else if (upper == "HELP") {
    Serial.println("CALIB DUMP | ZERO | SCALE <A> | VOLT <V> | RESET");
  } else {
    Serial.println("Unknown command; send HELP");
  }
}

void calibrateZeroOffset() {
  Serial.println("CALIB ZERO: sampling midpoint...");
  ChannelStats stats = captureRawSamples(kHallPin, kCalibrationSamples, false, 0.0f);
  if (stats.count == 0) {
    Serial.println("Zero calibration failed");
    return;
  }
  g_calib.current_offset = stats.avg();
  saveCalibration();
  Serial.print("New offset=");
  Serial.println(g_calib.current_offset, 6);
}

void calibrateScale(float reference_rms) {
  Serial.println("CALIB SCALE: measuring waveform...");
  ChannelStats stats = captureRawSamples(kHallPin, kCalibrationSamples, true, g_calib.current_offset);
  float measured = stats.rms();
  if (measured <= 0.0f) {
    Serial.println("Scale calibration failed (no signal)");
    return;
  }
  g_calib.current_scale = reference_rms / measured;
  saveCalibration();
  Serial.print("Current scale=");
  Serial.println(g_calib.current_scale, 6);
}

void calibrateVoltage(float reference_rms) {
  Serial.println("CALIB VOLT: measuring waveform...");
  ChannelStats stats = captureRawSamples(kVoltagePin, kCalibrationSamples, true, g_calib.voltage_offset);
  float measured = stats.rms();
  if (measured <= 0.0f) {
    Serial.println("Voltage calibration failed (no signal)");
    return;
  }
  g_calib.voltage_scale = reference_rms / measured;
  saveCalibration();
  Serial.print("Voltage scale=");
  Serial.println(g_calib.voltage_scale, 6);
}

Calibration defaultCalibration() {
  Calibration cal;
  cal.current_offset = kDefaultMidrail;
  cal.current_scale = 0.030f;
  cal.voltage_offset = kDefaultMidrail;
  cal.voltage_scale = 1.0f;
  cal.valid = false;
  return cal;
}

ChannelStats captureRawSamples(int pin, int samples, bool subtract_offset, float offset) {
  ChannelStats stats;
  for (int i = 0; i < samples; ++i) {
    float raw = analogRead(pin) * (kVRef / 4095.0f);
    if (subtract_offset) {
      raw -= offset;
    }
    stats.add(raw);
    delayMicroseconds(kSampleIntervalUs);
  }
  return stats;
}
