#include <WiFi.h>
#include <WiFiUdp.h>

#pragma pack(push, 1)
struct OutGaugePacket {
  uint32_t time;
  char car[4];
  uint16_t flags;
  int8_t gear;
  int8_t plid;

  float speed;        // m/s
  float rpm;          // RPM
  float turbo;        // bar
  float engTemp;      // C
  float fuel;         // 0.0 - 1.0
  float oilPressure;  // bar
  float oilTemp;      // C

  uint32_t dashLights;
  uint32_t showLights;

  float throttle;
  float brake;
  float clutch;

  char display1[16];
  char display2[16];

  int32_t id;         // optional depending on BeamNG config
};
#pragma pack(pop)

const uint32_t DL_SHIFT      = 1 << 0;
const uint32_t DL_FULLBEAM   = 1 << 1;
const uint32_t DL_HANDBRAKE  = 1 << 2;
const uint32_t DL_TC         = 1 << 4;
const uint32_t DL_SIGNAL_L   = 1 << 5;
const uint32_t DL_SIGNAL_R   = 1 << 6;
const uint32_t DL_OILWARN    = 1 << 8;
const uint32_t DL_BATTERY    = 1 << 9;
const uint32_t DL_ABS        = 1 << 10;

const int LED_PIN = 1;

const int BACKLIGHT_PIN = 8;

const int LEFT_SIGNAL_PIN = 12;
const int RIGHT_SIGNAL_PIN = 11;

const int HIGH_BEAM_PIN = 18;
const int CHECK_ENGINE_PIN = 17;
const int AIRBAG_PIN = 16;
const int ABS_PIN = 15;

const int RPM_PIN = 10;
const int SPEED_PIN = 7;

const int TRC_OFF_PIN = 6;
const int SLIP_PIN = 5;
const int BRAKE_PIN = 4;

const int REAR_LIGHTS_PIN = 9;
const int HEADLIGHT_BEAM_LEVEL_PIN = 46;
const int SECURITY_PIN = 13;

struct VehicleState {
  float rpm;
  float speedMph;
  float fuel;
  float coolantTemp;

  int gear;

  bool hazards;
  bool leftSignal;
  bool rightSignal;
  bool highBeam;
  bool abs;
  bool traction;
  bool parkingBrake;
};

VehicleState vehicle;

WiFiUDP udp;
const char* WIFI_SSID = "JIOR";
const char* WIFI_PASSWORD = "0123314655";
const int UDP_PORT = 4444;

bool udpStarted = false;

unsigned long lastPacketTime = 0;
const unsigned long PACKET_TIMEOUT = 1000; // 1 second

unsigned long lastWiFiReconnect = 0;
const unsigned long WIFI_RECONNECT_INTERVAL = 5000;

void allLightsOn() {
  digitalWrite(LEFT_SIGNAL_PIN, HIGH);
  digitalWrite(RIGHT_SIGNAL_PIN, HIGH);
  digitalWrite(HIGH_BEAM_PIN, HIGH);
  digitalWrite(CHECK_ENGINE_PIN, HIGH);
  digitalWrite(AIRBAG_PIN, HIGH);
  digitalWrite(TRC_OFF_PIN, HIGH);
  digitalWrite(SLIP_PIN, HIGH);
  
  digitalWrite(ABS_PIN, LOW);
  digitalWrite(BRAKE_PIN, LOW);
  digitalWrite(REAR_LIGHTS_PIN, LOW);
}

void allLightsOff() {
  digitalWrite(LEFT_SIGNAL_PIN, LOW);
  digitalWrite(RIGHT_SIGNAL_PIN, LOW);
  digitalWrite(HIGH_BEAM_PIN, LOW);
  digitalWrite(CHECK_ENGINE_PIN, LOW);
  digitalWrite(AIRBAG_PIN, LOW);
  digitalWrite(TRC_OFF_PIN, LOW);
  digitalWrite(SLIP_PIN, LOW);

  digitalWrite(ABS_PIN, HIGH);
  digitalWrite(BRAKE_PIN, HIGH);
  digitalWrite(REAR_LIGHTS_PIN, HIGH);
}

void setRPM(float rpm) {
  static uint32_t lastHz = 0;

  if (rpm < 100.0f) {
    if (lastHz != 0) {
      ledcWrite(RPM_PIN, 0);
      lastHz = 0;
    }
    return;
  }

  uint32_t hz = roundf(rpm / 19.9f);

  // Nothing changed, leave LEDC alone
  if (hz == lastHz) {
    return;
  }

  lastHz = hz;

  uint32_t actual =
      ledcChangeFrequency(RPM_PIN, hz, 12);

  ledcWrite(RPM_PIN, 2048);

  Serial.print("RPM Hz=");
  Serial.print(hz);
  Serial.print("/");
  Serial.println(actual);
}

void setSpeed(float mph) {
  static uint32_t lastHz = 0;

  if (mph < 2.0f) {
    if (lastHz != 0) {
      ledcWrite(SPEED_PIN, 0);
      lastHz = 0;
    }
    return;
  }

  uint32_t hz = roundf(mph / 0.875f);

  if (hz == lastHz) {
    return;
  }

  lastHz = hz;

  uint32_t actual =
      ledcChangeFrequency(SPEED_PIN, hz, 12);

  ledcWrite(SPEED_PIN, 2048);

  Serial.print("Speed Hz=");
  Serial.print(hz);
  Serial.print("/");
  Serial.println(actual);
}

void update(const VehicleState& vehicle) {
  // Gauges
  setRPM(vehicle.rpm);
  setSpeed(vehicle.speedMph);

  // High beam
  digitalWrite(
    HIGH_BEAM_PIN,
    vehicle.highBeam ? HIGH : LOW
  );

  // ABS is inverted
  digitalWrite(
    ABS_PIN,
    vehicle.abs ? LOW : HIGH
  );

  // Brake is inverted
  digitalWrite(
    BRAKE_PIN,
    vehicle.parkingBrake ? LOW : HIGH
  );

  // left signal
  digitalWrite(
    LEFT_SIGNAL_PIN,
    vehicle.leftSignal ? HIGH : LOW
  );

  // right signal
  digitalWrite(
    RIGHT_SIGNAL_PIN,
    vehicle.rightSignal ? HIGH : LOW
  );

}

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("Connecting to WiFi...");
}

void maintainWiFi() {
  setWiFiLED();

  if (WiFi.status() == WL_CONNECTED) {

    if (!udpStarted) {
      udp.begin(UDP_PORT);
      udpStarted = true;

      Serial.print("WiFi connected! IP: ");
      Serial.println(WiFi.localIP());

      Serial.print("Listening on UDP port ");
      Serial.println(UDP_PORT);
    }

    return;
  }

  udpStarted = false;

  unsigned long now = millis();

  if (now - lastWiFiReconnect >= WIFI_RECONNECT_INTERVAL) {
    lastWiFiReconnect = now;

    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  }
}

void setWiFiLED() {
  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(LED_PIN, LOW);   // WiFi connected
  } else {
    digitalWrite(LED_PIN, HIGH);  // WiFi disconnected
  }
}

void telemetryFailsafe() {
  vehicle.rpm = 0;
  vehicle.speedMph = 0;

  vehicle.hazards = false;
  vehicle.leftSignal = false;
  vehicle.rightSignal = false;
  vehicle.highBeam = false;
  vehicle.abs = false;
  vehicle.traction = false;
  vehicle.parkingBrake = false;

  update(vehicle);

  digitalWrite(SECURITY_PIN, HIGH);
}

bool processBeamNGPacket(uint8_t* buffer, int length) {
  if (length < sizeof(OutGaugePacket) - 4) {
    return false;
  }

  OutGaugePacket packet;

  memset(&packet, 0, sizeof(packet));
  memcpy(&packet, buffer, min(length, (int)sizeof(packet)));

  vehicle.rpm = packet.rpm;
  vehicle.speedMph = packet.speed * 2.236936f;
  vehicle.fuel = packet.fuel;
  vehicle.coolantTemp = packet.engTemp;
  vehicle.gear = packet.gear;

  vehicle.leftSignal = packet.showLights & DL_SIGNAL_L;
  vehicle.rightSignal = packet.showLights & DL_SIGNAL_R;
  vehicle.highBeam = packet.showLights & DL_FULLBEAM;
  vehicle.abs = packet.showLights & DL_ABS;
  vehicle.traction = packet.showLights & DL_TC;
  vehicle.parkingBrake = packet.showLights & DL_HANDBRAKE;

  Serial.print("RPM: ");
  Serial.print(vehicle.rpm);

  Serial.print("  Speed: ");
  Serial.print(vehicle.speedMph);

  Serial.print(" MPH  Gear: ");
  Serial.println(vehicle.gear);

  return true;
}

void updateBeamNG() {
  int packetSize = udp.parsePacket();

  if (packetSize <= 0) {
    return;
  }

  uint8_t buffer[128];

  int len = udp.read(buffer, sizeof(buffer));

  if (len > 0 && processBeamNGPacket(buffer, len)) {
    lastPacketTime = millis();

    // Telemetry alive
    digitalWrite(SECURITY_PIN, LOW);
  }
}

void setup() {
  Serial.begin(115200);

  // // Speed & RPM
  ledcAttachChannel(RPM_PIN, 1000, 12, 0);
  ledcAttachChannel(SPEED_PIN, 800, 12, 2);
  ledcWrite(RPM_PIN, 0);
  ledcWrite(SPEED_PIN, 0);

  //Backlight
  pinMode(BACKLIGHT_PIN, OUTPUT);

  // Cluster Light Pins
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(BACKLIGHT_PIN, LOW);

  pinMode(LEFT_SIGNAL_PIN, OUTPUT);
  pinMode(RIGHT_SIGNAL_PIN, OUTPUT);
  digitalWrite(LEFT_SIGNAL_PIN, LOW);
  digitalWrite(RIGHT_SIGNAL_PIN, LOW);

  pinMode(HIGH_BEAM_PIN, OUTPUT);
  pinMode(CHECK_ENGINE_PIN, OUTPUT);
  pinMode(AIRBAG_PIN, OUTPUT);
  pinMode(ABS_PIN, OUTPUT);
  digitalWrite(HIGH_BEAM_PIN, LOW);
  digitalWrite(CHECK_ENGINE_PIN, LOW);
  digitalWrite(AIRBAG_PIN, LOW);
  digitalWrite(ABS_PIN, LOW);

  pinMode(TRC_OFF_PIN, OUTPUT);
  pinMode(SLIP_PIN, OUTPUT);
  pinMode(BRAKE_PIN, OUTPUT);
  digitalWrite(TRC_OFF_PIN, LOW);
  digitalWrite(SLIP_PIN, LOW);
  digitalWrite(BRAKE_PIN, LOW);

  pinMode(REAR_LIGHTS_PIN, OUTPUT);
  pinMode(SECURITY_PIN, OUTPUT);
  digitalWrite(REAR_LIGHTS_PIN, LOW);
  digitalWrite(SECURITY_PIN, LOW);

  // Flash all the lights once, and sweep through the gauges
  allLightsOn();
  delay(100);
  setRPM(8000);
  setSpeed(160);
  delay(1500);
  setRPM(0);
  setSpeed(0);
  delay(100);
  allLightsOff();

  // Open Wifi and wait for a connection
  connectWiFi();
}

void loop() {
  //telementary data
  maintainWiFi();
  updateBeamNG();

  // update gauges and signal lights
  if (millis() - lastPacketTime > PACKET_TIMEOUT) {
    telemetryFailsafe();
  } else {
    update(vehicle);
  }
}


