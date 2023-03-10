#include "Adafruit_GFX.h"  
#include "MCUFRIEND_kbv.h"   

//#include "TouchScreen.h"

#include <SimpleDHT.h>
#include "DHT.h"
#define DHTPIN 31 
int smokeA0 = 35;
int smokeDO = 33;
#define DHTTYPE DHT11   
DHT dht(DHTPIN, DHTTYPE);

#include <Wire.h>
#include "RTClib.h"

DateTime now;
char daysOfTheWeek[7][12] = {" Sunday", " Monday", " Tueday", " Wednesday", " Thursday", " Friday", " Saturday"};

RTC_DS3231 rtc;

void showDate(void);
void showTime(void);
void showDay(void); 

MCUFRIEND_kbv tft;

#include "Fonts/FreeSans9pt7b.h"
#include "Fonts/FreeSans12pt7b.h"
#include "Fonts/FreeSerif12pt7b.h"
#include "Fonts/FreeMonoBold12pt7b.h"
#include "Fonts/FreeMonoBold18pt7b.h"
#include "Fonts/FreeMonoBold24pt7b.h"
#include "FreeDefaultFonts.h"

#include "EmonLib.h"
#define VOLT_CAL 57
float pert;

EnergyMonitor emon1;

int reedSwitch = 39;  //Declaring pin for reed switch to be used. Pin 2 or 3 must be used for the reed switch on the Uno
int radius = 258;   //hard coding in the radius of the wheel for future conversion into circumference 
int circumference;   //declaring the circumference variable
int totaltime;       //declaring the total time until the next trip on the reed switch variable
int Speed;           //declaring the Speed variable
int timer;           //declaring the timer to calculate the total time variable

#define BLACK 0x0000
#define NAVY 0x000F
#define DARKGREEN 0x03E0
#define DARKCYAN 0x03EF
#define MAROON 0x7800
#define PURPLE 0x780F
#define OLIVE 0x7BE0
#define LIGHTGREY 0xC618
#define DARKGREY 0x7BEF
#define BLUE 0x001F
#define GREEN 0x07E0
#define CYAN 0x07FF
#define RED 0xF800
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF
#define ORANGE 0xFD20
#define GREENYELLOW 0xAFE5
#define PINK 0xF81F

#define sensorPower 7
#define sensorPin 37

void showmsgXY(int x, int y, int sz, const GFXfont *f, const char *msg)
{
  int16_t x1, y1;
  uint16_t wid, ht;
  tft.setFont(f);
  tft.setCursor(x, y);
  tft.setTextSize(sz);
  tft.println(msg);
}

uint8_t r = 255, g = 255, b = 255;
uint16_t color;

//speed
void isr()           
 {
      timer++;                     
 }
 
void setup()
{
  tft.reset();
  Serial.begin(9600);
  uint16_t ID = tft.readID();
  tft.begin(ID);
  tft.invertDisplay(true);
  tft.setRotation(1);
  tft.println(F("TEAM FALCON CRUISERS"));
  
  pinMode(smokeA0, INPUT);
  dht.begin();

  if (! rtc.begin()) 
  {
    Serial.println("Couldn't find RTC Module");
    while (1);
  }

  if (rtc.lostPower()) 
  {
    Serial.println("RTC lost power, lets set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  emon1.voltage(49,VOLT_CAL,1.7);

   attachInterrupt(digitalPinToInterrupt(39), isr, RISING);    

   pinMode(reedSwitch, INPUT_PULLUP);                         
   timer = 0;                                                 
   Speed = 0;                                                 
   totaltime = 0;                                             
   circumference = 6.28 * radius;                             


  
  pinMode(sensorPower, OUTPUT);
  digitalWrite(sensorPower, LOW);
}

void loop()
{
  int analogSensor = analogRead(smokeA0);
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float f = dht.readTemperature(true);
  if (isnan(h) || isnan(t) || isnan(f) || isnan(analogSensor)){
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  {
    now = rtc.now();
  showDate();
  showDay();
  showTime();
  }
  float hif = dht.computeHeatIndex(f, h);
  float hic = dht.computeHeatIndex(t, h, false);
  tft.invertDisplay(true);
  tft.fillScreen(0xFFFF);
  color = tft.color565(0, 0, 0);
  tft.setTextColor(color);
  showmsgXY(20, 30, 1, &FreeMonoBold18pt7b,"    FALCON CRUISERS"); 
  showmsgXY(0, 40, 1.8, &FreeSerif12pt7b,"________________________________________"); 
  tft.print(" VP:");
  tft.print(analogSensor); //delay(100);
  tft.println(F("Rh"));
  tft.println(" ");
  tft.print((" H:"));
  tft.print(h);//delay(100);
  tft.println(F("%"));
  tft.println(" ");
  tft.print(F(" T:"));
  tft.print(t);//delay(100);
  tft.println(F("˚C"));
  tft.println(" ");
  tft.print(F("H.I:")); //delay(100);
  tft.print(hic);
  tft.print(F("C"));//delay(100);
  

  showmsgXY(0, 280, 1, &FreeSerif12pt7b ,"________________________________________"); 
   tft.print(now.day());//delay(100);
  tft.print('/');
  tft.print(now.month());//delay(100);
  tft.print('/');
  tft.print(now.year());//delay(100);
  tft.print(daysOfTheWeek[now.dayOfTheWeek()]);//delay(100);
  tft.print("                     Time:");
  tft.print(now.hour());//delay(100);
  tft.print(':');
 tft.print(now.minute());//delay(100);
  tft.print(':');
 tft.print(now.second());//delay(100);
  tft.print("    ");
  tft.drawFastVLine(120,45,240,RED);
  tft.drawFastHLine(122,80,370,RED);
  tft.drawFastVLine(375,80,200,RED);

   if (millis() - totaltime >= 1000) {                                  
                                                                            
        detachInterrupt(digitalPinToInterrupt(39));                         

        Speed = ((8.181818182*circumference)/(millis() - totaltime)*timer); 

        timer = 0;                                                          
        totaltime = millis();                                               
        attachInterrupt(digitalPinToInterrupt(39), isr, RISING);            
        //showmsgXY(180, 100, 1, &FreeSerif12pt7b,"");
        tft.print("         Speed=                              ");         
        showmsgXY(170, 150, 2, &FreeSerif12pt7b, "Speed");
        tft.println(Speed);                                               
        tft.println("                km/hr");
       
   }

 showmsgXY(300, 100, 2, &FreeMonoBold24pt7b,"");

  if(Speed<=45)
  {
    tft.print("       1");
  }
  else if(Speed>=45 || Speed<=60)
  {
    tft.print("       2");
    }
  else
  {
    tft.print("       3");
  }
  
   int val = readSensor();
  tft.drawFastHLine(380,250,110,OLIVE);
  showmsgXY(380, 250, 1, &FreeSerif12pt7b,"");
  if (val) {
    tft.println("                                                               Shift to-0");
  } else {
    tft.println("                                                               Shift to-1");
  }
  Serial.println();

  emon1.calcVI(25,1000);
  float supplyVoltage=emon1.Vrms;
  if(supplyVoltage<=9)
  {
  //tft.print("       Voltage:");
  //tft.print(      supplyVoltage);
  }
  else
  {
  //tft.println("       P-off");
  }
  pert=(supplyVoltage/9)*100;
 
  showmsgXY(125, 70, 1, &FreeSerif12pt7b,"pert");
  tft.print(         pert);
}


void showDate()
 {
  //tft.setCursor(0, 290);
  //lcd.setCursor(0,0);
  Serial.print(now.day());
  Serial.print('/');
  Serial.print(now.month());
  Serial.print('/');
  Serial.print(now.year());
 }
 void showDay()
 {
  Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
 }
 void showTime()
 {
  //lcd.setCursor(0,1);
  //tft.setCursor(100, 280);
  Serial.print("Time:");
  Serial.print(now.hour());
  Serial.print(':');
 Serial.print(now.minute());
  Serial.print(':');
 Serial.print(now.second());
  Serial.print("    ");
  delay(1000);
 } 

 int readSensor() {
  digitalWrite(sensorPower, HIGH); 
  delay(10);           
  int val = digitalRead(sensorPin); 
  digitalWrite(sensorPower, LOW);   
  return val;            
}