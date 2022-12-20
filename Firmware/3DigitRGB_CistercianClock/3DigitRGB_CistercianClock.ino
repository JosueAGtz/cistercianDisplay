#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include <time.h>

#define SDIGITS 8
#define SCOLON  9

#define UPDATERATE  10

unsigned int segmentColor;
unsigned int nextColor;
unsigned long requestDueTime;
unsigned int prevHour;
unsigned int prevMinute;
unsigned int prevSecond;
unsigned int prevColon;
bool updateData;

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -3600 * 6;
const int daylightOffset_sec = 3600;

const char *ssid     = "YourWifi";
const char *password = "YourPassword";

Adafruit_NeoPixel digitStrip = Adafruit_NeoPixel(93,SDIGITS,NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel colonStrip = Adafruit_NeoPixel(4, SCOLON, NEO_GRB + NEO_KHZ800);

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

struct tm timeinfo;

void setup() {
  Serial.begin(115200);


  digitStrip.begin();
  digitStrip.setBrightness(60);
  digitStrip.show(); // Initialize all pixels to 'off'

  colonStrip.begin();
  colonStrip.setBrightness(60);
  colonStrip.show(); // Initialize all pixels to 'off'

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  for (int i = 0; i < 93; i++){ digitStrip.setPixelColor(i, digitStrip.Color(0, 255, 0)); }digitStrip.show();delay(500);
  for (int i = 0; i < 93; i++){ digitStrip.setPixelColor(i, digitStrip.Color(255, 0, 0)); }digitStrip.show();delay(500);
  for (int i = 0; i < 93; i++){ digitStrip.setPixelColor(i, digitStrip.Color(0, 0, 255)); }digitStrip.show();delay(500);

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

  // Display Time and initialize varibles 
  getLocalTime(&timeinfo);
  nextColor = 1;
}

void loop() {
  getLocalTime(&timeinfo);
  
  if (millis() > requestDueTime) {  // Update clock data every 2 seconds
    updateData = 1;
    requestDueTime = millis() + UPDATERATE;
  }

  
  if (updateData == 1) {

    segmentColor++;
    if(segmentColor == 255)segmentColor = 1;
    
    displayTime(timeinfo.tm_hour,timeinfo.tm_min,timeinfo.tm_sec);
    updateData = 0;
  }
  
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return digitStrip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return digitStrip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return digitStrip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void displayTime( int hours, int minutes, int seconds ){
  
  if (hours > 9999) hours = 0;
  int h_thousands = hours / 1000;
  int h_hundreds = (hours - (h_thousands * 1000)) / 100;
  int h_tens = (hours - (h_thousands * 1000) - (h_hundreds * 100)) / 10;
  int h_units = hours - (h_thousands * 1000) - (h_hundreds * 100) - (h_tens * 10);

  if (minutes > 9999) minutes = 0;
  int m_thousands = minutes / 1000;
  int m_hundreds = (minutes - (m_thousands * 1000)) / 100;
  int m_tens = (minutes - (m_thousands * 1000) - (m_hundreds * 100)) / 10;
  int m_units = minutes - (m_thousands * 1000) - (m_hundreds * 100) - (m_tens * 10);

  if (seconds > 9999) seconds = 0;
  int s_thousands = seconds / 1000;
  int s_hundreds = (seconds - (s_thousands * 1000)) / 100;
  int s_tens = (seconds - (s_thousands * 1000) - (s_hundreds * 100)) / 10;
  int s_units = seconds - (s_thousands * 1000) - (s_hundreds * 100) - (s_tens * 10);


  for (int i = 62; i < 93; i++) {  
  if(numberThousands[s_thousands][i-62] | numberHundreds[s_hundreds][i-62] | numberTens[s_tens][i-62] | numberUnits[s_units][i-62]){
    digitStrip.setPixelColor(i, Wheel(segmentColor+(nextColor*16))); 
   }else{
    digitStrip.setPixelColor(i, digitStrip.Color(0, 0, 0));
    }nextColor++;
  }
  

  colonStrip.setPixelColor(0, Wheel(segmentColor+(nextColor*16)));
  colonStrip.setPixelColor(1, Wheel(segmentColor+(nextColor*16)));
  nextColor++;

  for (int i = 31; i < 62; i++) {  
    if(numberThousands[m_thousands][i-31] | numberHundreds[m_hundreds][i-31] | numberTens[m_tens][i-31] | numberUnits[m_units][i-31]){
    digitStrip.setPixelColor(i, Wheel(segmentColor+(nextColor*16))); 
   }else{
    digitStrip.setPixelColor(i, digitStrip.Color(0, 0, 0));
    }nextColor++;
  }
  

  colonStrip.setPixelColor(2, Wheel(segmentColor+(nextColor*16)));
  colonStrip.setPixelColor(3, Wheel(segmentColor+(nextColor*16)));
  nextColor++;

  for (int i = 0; i < 31; i++) {  
    if(numberThousands[h_thousands][i] | numberHundreds[h_hundreds][i] | numberTens[h_tens][i] | numberUnits[h_units][i]){
    digitStrip.setPixelColor(i, Wheel(segmentColor+(nextColor*16))); 
   }else{
    digitStrip.setPixelColor(i, digitStrip.Color(0, 0, 0));
    }nextColor++;  
  }
  
   digitStrip.show();
   colonStrip.show();
   nextColor = 0;
}
