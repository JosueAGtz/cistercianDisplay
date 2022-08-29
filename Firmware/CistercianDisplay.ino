#include <WiFi.h>
#include <time.h>

#define SK6812LED 18

#define SERIAL_PIN 35
#define CLOCK_PIN 36
#define LATCH_PIN 33
#define ENABLE_PIN 34

#define HRS_LED 11
#define MIN_LED 10
#define MONTH_LED 9
#define DAY_LED 8
#define YEAR_LED 7

#define CLOCKCYCLES 3
#define UPDATERATE  2000

unsigned long requestDueTime;
unsigned int displayData = 1;
unsigned int turnOnClock = 0;
unsigned int prevMinutes = 0;
bool updateData;

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -3600 * 6;
const int daylightOffset_sec = 3600;

const char *ssid     = "YourWifi";
const char *password = "YourPassword";

// 1 to 9 digit cistercian representation
// 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32
bool numberUnits[10][32] = { { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },    // 0
                             { 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },    // 1
                             { 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },    // 2
                             { 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },    // 3
                             { 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },    // 4
                             { 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },    // 5
                             { 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },    // 6
                             { 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },    // 7
                             { 1, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },    // 8
                             { 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } };  // 9

// 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32
bool numberTens[10][32] = { { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },    // 0
                            { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },    // 1
                            { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },    // 2
                            { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },    // 3
                            { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },    // 4
                            { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },    // 5
                            { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },    // 6
                            { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },    // 7
                            { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },    // 8
                            { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } };  // 9

// 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32
bool numberHundreds[10][32] = { { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },    // 0
                                { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },    // 1
                                { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },    // 2
                                { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },    // 3
                                { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },    // 4
                                { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },    // 5
                                { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },    // 6
                                { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },    // 7
                                { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },    // 8
                                { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } };  // 9

// 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32
bool numberThousands[10][32] = { { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },    // 0
                                 { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 },    // 1
                                 { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0 },    // 2
                                 { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0 },    // 3
                                 { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0 },    // 4
                                 { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0 },    // 5
                                 { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0 },    // 6
                                 { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0 },    // 7
                                 { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 },    // 8
                                 { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0 } };  // 9

void setDigit(int digit);
struct tm timeinfo;

void setup() {
  Serial.begin(115200);

  pinMode(ENABLE_PIN, OUTPUT);
  pinMode(SERIAL_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(LATCH_PIN, OUTPUT);

  pinMode(MONTH_LED, OUTPUT);
  pinMode(DAY_LED, OUTPUT);
  pinMode(YEAR_LED, OUTPUT);
  pinMode(HRS_LED, OUTPUT);
  pinMode(MIN_LED, OUTPUT);

  digitalWrite(HRS_LED, LOW);
  digitalWrite(MIN_LED, LOW);
  digitalWrite(MONTH_LED, LOW);
  digitalWrite(DAY_LED, LOW);
  digitalWrite(YEAR_LED, LOW);
  digitalWrite(ENABLE_PIN, HIGH);

  uint8_t wifiConectionAttmps = 0;

  WiFi.begin(ssid, password);
  Serial.print("Connecting to ");
  Serial.print(ssid);
  while (WiFi.status() != WL_CONNECTED & wifiConectionAttmps < 20) {
    delay(500);
    Serial.print(".");
    wifiConectionAttmps++;
  }

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  digitalWrite(HRS_LED, HIGH);
  digitalWrite(MIN_LED, HIGH);
  digitalWrite(MONTH_LED, HIGH);
  digitalWrite(DAY_LED, HIGH);
  digitalWrite(YEAR_LED, HIGH);

  digitalWrite(ENABLE_PIN, LOW);

  setDigit(1111);
  delay(250);
  setDigit(2222);
  delay(250);
  setDigit(3333);
  delay(250);
  setDigit(4444);
  delay(250);
  setDigit(5555);
  delay(250);
  setDigit(6666);
  delay(250);
  setDigit(7777);
  delay(250);
  setDigit(8888);
  delay(250);
  setDigit(9999);
  delay(250);
  setDigit(0000);
  delay(250);
  digitalWrite(HRS_LED, LOW);
  digitalWrite(MIN_LED, LOW);
  digitalWrite(MONTH_LED, LOW);
  digitalWrite(DAY_LED, LOW);
  digitalWrite(YEAR_LED, LOW);
}

void loop() {
  getLocalTime(&timeinfo);

  if (prevMinutes != timeinfo.tm_min) {  // Display Clocl for a couple of cycles every minute
    turnOnClock = 0;
    prevMinutes = timeinfo.tm_min;
  }

  if (millis() > requestDueTime) {  // Update clock data every 2 seconds
    updateData = 1;
    requestDueTime = millis() + UPDATERATE;
  }

  if (updateData == 1) {
    switch (displayData++) {
      case 1:
        if (turnOnClock <= (CLOCKCYCLES)) {
          turnOnClock++;
          digitalWrite(ENABLE_PIN, LOW);
          Serial.print("Clock cycle: ");
          Serial.println(turnOnClock);
        }

        if (turnOnClock > (CLOCKCYCLES) ) {
          digitalWrite(ENABLE_PIN, HIGH);
          Serial.println("Display Power off!");
        }

        digitalWrite(YEAR_LED, LOW);
        digitalWrite(HRS_LED, HIGH);
        setDigit(timeinfo.tm_hour);
        Serial.print(" Hour: ");
        Serial.print(timeinfo.tm_hour);
        break;
      case 2:
        digitalWrite(HRS_LED, LOW);
        digitalWrite(MIN_LED, HIGH);
        setDigit(timeinfo.tm_min);
        Serial.print("  Minute: ");
        Serial.print(timeinfo.tm_min);
        break;
      case 3:
        digitalWrite(MIN_LED, LOW);
        digitalWrite(MONTH_LED, HIGH);
        setDigit(timeinfo.tm_mon + 1);
        Serial.print("  Month: ");
        Serial.print(timeinfo.tm_mon + 1);
        break;
      case 4:
        digitalWrite(MONTH_LED, LOW);
        digitalWrite(DAY_LED, HIGH);
        setDigit(timeinfo.tm_mday);
        Serial.print("  Day: ");
        Serial.print(timeinfo.tm_mday);
        break;
      case 5:
        digitalWrite(DAY_LED, LOW);
        digitalWrite(YEAR_LED, HIGH);
        setDigit(timeinfo.tm_year + 1900);
        Serial.print("  Year: ");
        Serial.println(timeinfo.tm_year + 1900);
        break;
      default:
        setDigit(9999);
        break;
    }
    updateData = 0;
    if (displayData > 5)
      displayData = 1;
  }
}

void setDigit(int digit) {
  if (digit > 9999) digit = 0;
  int thousands = digit / 1000;
  int hundreds = (digit - (thousands * 1000)) / 100;
  int tens = (digit - (thousands * 1000) - (hundreds * 100)) / 10;
  int units = digit - (thousands * 1000) - (hundreds * 100) - (tens * 10);

  digitalWrite(LATCH_PIN, HIGH);
  for (int i = 0; i < 32; i++) {
    digitalWrite(CLOCK_PIN, LOW);
    digitalWrite(SERIAL_PIN, numberThousands[thousands][i] | numberHundreds[hundreds][i] | numberTens[tens][i] | numberUnits[units][i]);
    digitalWrite(CLOCK_PIN, HIGH);
  }
  digitalWrite(LATCH_PIN, LOW);
}
