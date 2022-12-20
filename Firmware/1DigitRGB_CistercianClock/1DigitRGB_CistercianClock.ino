#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include <time.h>

#define SERIAL_PIN  35
#define UPDATERATE  20

Adafruit_NeoPixel cDisplay = Adafruit_NeoPixel(31, SERIAL_PIN, NEO_GRB + NEO_KHZ800);

int segmentColor = 0;
int nextColor = 1;
unsigned long requestDueTime;
unsigned int displayData = 1;
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
uint32_t Wheel(byte WheelPos);
void setDigit(int digit);
struct tm timeinfo;

void setup() {
  Serial.begin(115200);

  pinMode(38,OUTPUT);
  digitalWrite(38,HIGH);

  cDisplay.begin();
  cDisplay.setBrightness(60);
  cDisplay.show(); // Initialize all pixels to 'off'

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
  for (int i = 0; i < 32; i++){ cDisplay.setPixelColor(i, cDisplay.Color(0, 255, 0)); }cDisplay.show();delay(500);
  for (int i = 0; i < 32; i++){ cDisplay.setPixelColor(i, cDisplay.Color(255, 0, 0)); }cDisplay.show();delay(500);
  for (int i = 0; i < 32; i++){ cDisplay.setPixelColor(i, cDisplay.Color(0, 0, 255)); }cDisplay.show();delay(500);

  setDigit(9999);
  delay(1000);
  segmentColor = 1;
}

void loop() {

  getLocalTime(&timeinfo);
  if (millis() > requestDueTime) {  // Update clock data every second
    updateData = 1;
    requestDueTime = millis() + UPDATERATE;
  }

  if (updateData == 1) {
    segmentColor++;
    if(segmentColor == 255)segmentColor = 1;
  
    setMinute(timeinfo.tm_min);nextColor++;
    cDisplay.setPixelColor(1,Wheel(segmentColor +(nextColor * 32)));nextColor++;
    setHour(timeinfo.tm_hour);
    nextColor = 0;
    updateData = 0;
  } 
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return cDisplay.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return cDisplay.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return cDisplay.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void setDigit(int digit) {
  if (digit > 9999) digit = 0;
  int thousands = digit / 1000;
  int hundreds = (digit - (thousands * 1000)) / 100;
  int tens = (digit - (thousands * 1000) - (hundreds * 100)) / 10;
  int units = digit - (thousands * 1000) - (hundreds * 100) - (tens * 10);
  
  for (int i = 0; i < 32; i++) {
   if(numberThousands[thousands][i] | numberHundreds[hundreds][i] | numberTens[tens][i] | numberUnits[units][i]){
      nextColor++;
      cDisplay.setPixelColor(i,Wheel(segmentColor+(nextColor*32)));
   }else{
      cDisplay.setPixelColor(i, cDisplay.Color(0,0,0));
   }
  }
  cDisplay.show();
}

void setHour(int digit) {
  if (digit > 9999) digit = 0;
  int thousands = digit / 1000;
  int hundreds = (digit - (thousands * 1000)) / 100;
  int tens = (digit - (thousands * 1000) - (hundreds * 100)) / 10;
  int units = digit - (thousands * 1000) - (hundreds * 100) - (tens * 10);
  
  for (int i = 2; i < 17; i++) {
   if(numberThousands[thousands][i] | numberHundreds[hundreds][i] | numberTens[tens][i] | numberUnits[units][i]){
      cDisplay.setPixelColor(i,Wheel(segmentColor+(nextColor*32)));
   }else{
      cDisplay.setPixelColor(i, cDisplay.Color(0,0,0));
   }
  }
  cDisplay.show();
}

void setMinute(int digit) {
  digit = digit*100;
  if (digit > 9999) digit = 0;
  int thousands = digit / 1000;
  int hundreds = (digit - (thousands * 1000)) / 100;
  int tens = (digit - (thousands * 1000) - (hundreds * 100)) / 10;
  int units = digit - (thousands * 1000) - (hundreds * 100) - (tens * 10);
  
  for (int i = 17; i < 31; i++) {
   if(numberThousands[thousands][i] | numberHundreds[hundreds][i] | numberTens[tens][i] | numberUnits[units][i]){
      cDisplay.setPixelColor(i,Wheel(segmentColor+(nextColor*32)));
   }else{
      cDisplay.setPixelColor(i, cDisplay.Color(0,0,0));
   }
  }
  cDisplay.setPixelColor(0,Wheel(segmentColor+(nextColor*32)));
  cDisplay.show();
}
