#include <EEPROM.h>
#include <DHT.h>

#define DHTPIN 4      // Pin which is connected to the DHT sensor
#define DHTTYPE DHT22 // DHT 22 (AM2302), AM2321

DHT dht(DHTPIN, DHTTYPE);

#define EEPROM_SIZE 512
#define TEMP_DATA_START 0
#define TEMP_DATA_COUNT_ADDR EEPROM_SIZE - sizeof(int)
#define MAX_TEMP_COUNT ((EEPROM_SIZE - sizeof(int)) / sizeof(float))

void setup() {
  Serial.begin(115200);
  dht.begin();

  if (!EEPROM.begin(EEPROM_SIZE)) {
    Serial.println("Failed to initialize EEPROM");
    return;
  }

  int tempCount;
  EEPROM.get(TEMP_DATA_COUNT_ADDR, tempCount);
  if (tempCount < 0 || tempCount >= EEPROM_SIZE / sizeof(float)) {
    tempCount = 0;
    EEPROM.put(TEMP_DATA_COUNT_ADDR, tempCount);
    EEPROM.commit();
  }

  Serial.println("Temp Logger Initialized");
}

void logTemperature() {
  float temp = dht.readTemperature();
  if (isnan(temp)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  int tempCount;
  EEPROM.get(TEMP_DATA_COUNT_ADDR, tempCount);
  
  if (tempCount >= MAX_TEMP_COUNT) {
    // Overwrite the oldest data
    tempCount = 0;
  }

  EEPROM.put(TEMP_DATA_START + tempCount * sizeof(float), temp);
  tempCount++;
  EEPROM.put(TEMP_DATA_COUNT_ADDR, tempCount);
  EEPROM.commit();

  Serial.print("Logged temperature: ");
  Serial.println(temp);
}

// Print out the logged temperature to the Serial Monitor
void readEEPROMData() {
  int tempCount;
  EEPROM.get(TEMP_DATA_COUNT_ADDR, tempCount);
  Serial.print("Logged temperatures: ");
  Serial.println(tempCount);
  for (int i = 0; i < tempCount; i++) {
    float temp;
    EEPROM.get(TEMP_DATA_START + i * sizeof(float), temp);
    Serial.print("Temp ");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(temp);
  }
}
// Clear the EEPROM data
void clearEEPROMData() {
  int tempCount = 0;
  EEPROM.put(TEMP_DATA_COUNT_ADDR, tempCount);
  EEPROM.commit();
  Serial.println("EEPROM data cleared.");
}

void loop() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    if (command == "read") {
      readEEPROMData();
    } else if (command == "clear") {
      clearEEPROMData();
    }
  }

  static unsigned long lastLogTime = 0;
  if (millis() - lastLogTime > 60000) { // Log temperature every minute (60 000 milliseconds)
    lastLogTime = millis();
    logTemperature();
  }
}
