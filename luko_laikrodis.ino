// ESP32 Simulation in Wokwi.com simulator
#include <WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C LCD = LiquidCrystal_I2C(0x27, 16, 2);

#define WIFI_SSID "Wokwi-GUEST"
#define NTP_SERVER     "pool.ntp.org" // Network Time Protocol Server
// Time offset. Vilnius is typically in UTC+2 so we have to add 2 hours here
#define UTC_OFFSET     7200 // This is in seconds, so 1 Hour offset would be 3600 seconds.
#define UTC_OFFSET_DST 3600 // Daylight Saving Time. Since Lithuania is currently (Summer 2024) in DST, set this to 3600

// Additional controls
#define SET_ALARM_PIN 15 // Pin for the "Set Alarm" menu
#define ENABLE_ALARM_PIN 16 // Pin for Enabling/Disabling Alarm
#define INCREASE_HOUR_PIN 4 // Pin for the "Increase Hour" button
#define DECREASE_HOUR_PIN 5 // Pin for the "Decrease Hour" button
#define BUZZER_PIN 18 // Pin for the buzzer

int alarmHour = 7; // Default alarm hour
bool settingAlarm = false;
bool enabledAlarm = false;

void printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) { // If error getting time from the server
    LCD.setCursor(0, 1);
    LCD.println("Connection Err");
    return;
  }
  LCD.setCursor(0, 0);
  LCD.println("Time: ");

  LCD.setCursor(8, 0);
  LCD.println(&timeinfo, "%H:%M:%S");

  LCD.setCursor(0, 1);
  LCD.println(&timeinfo, "%d/%m/%Y   %Z");
}

void printAlarmSetting() {
  LCD.clear();
  LCD.setCursor(0, 0);
  LCD.print("Alarm ");
  LCD.setCursor(7, 0);
  LCD.printf("Hour: %02d", alarmHour);
  LCD.setCursor(0, 1); // Adjusted to make sure "Enabled" fits on the screen
  LCD.printf("Enabled: %s", enabledAlarm ? "Yes" : "No");
}

void setup() {
  Serial.begin(115200);
  Wire.begin(23, 22); //Assign SDA to PIN 23, SCL to PIN 22

  pinMode(SET_ALARM_PIN, INPUT_PULLUP);
  pinMode(ENABLE_ALARM_PIN, INPUT_PULLUP);
  pinMode(INCREASE_HOUR_PIN, INPUT_PULLUP);
  pinMode(DECREASE_HOUR_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW); // Turn off buzzer

  LCD.init();
  LCD.backlight();
  LCD.setCursor(0, 0);
  LCD.print("Connecting to ");
  LCD.setCursor(0, 1);
  LCD.print("WiFi ");

  WiFi.begin(WIFI_SSID, "", 6); // Custom Wifi access point for Wokwi simulator
  while (WiFi.status() != WL_CONNECTED) { // Waiting for connection
    delay(250);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  LCD.clear();
  LCD.setCursor(0, 0);
  LCD.println("Connected");
  
  configTime(UTC_OFFSET, UTC_OFFSET_DST, NTP_SERVER);

}

void loop() {
  if (digitalRead(SET_ALARM_PIN) == LOW) {
    settingAlarm = !settingAlarm;
    if (settingAlarm) {
      printAlarmSetting();
    } else {
      LCD.clear();
      printLocalTime();
      delay(100);
    }
    delay(300); // Debounce delay
    while(digitalRead(SET_ALARM_PIN) == LOW) { // Apsauga, kad nesokinetu vaizdas kai laikai paspausta mygtuka
      delay(100);
    }
  }

  if (settingAlarm) {
    if (digitalRead(INCREASE_HOUR_PIN) == LOW) {
      alarmHour = (alarmHour + 1) % 24;
      printAlarmSetting();
      delay(300); // Debounce delay
    }
    if (digitalRead(DECREASE_HOUR_PIN) == LOW) {
      alarmHour = (alarmHour + 23) % 24; // To handle negative wrap around
      printAlarmSetting();
      delay(300); // Debounce delay
    }
    if (digitalRead(ENABLE_ALARM_PIN) == LOW) {
      enabledAlarm = !enabledAlarm;
      printAlarmSetting();
      delay(300);
    }
  } else {
    printLocalTime();
    delay(250);

    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      if (enabledAlarm && timeinfo.tm_hour == alarmHour && timeinfo.tm_min == 0 && timeinfo.tm_sec == 0) {
        digitalWrite(BUZZER_PIN, HIGH);
        delay(1000); // Buzzer duration
        digitalWrite(BUZZER_PIN, LOW);
      }
    }
  }
}
