#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <Wire.h>

#define DHTPIN 2 // Digital pin connected to the DHT sensor
#define buttonPin 3
#define buzzerPin 4
#define DHTTYPE DHT11       // DHT 11
#define COV_RATIO 0.2       // ug/mmm / mv
#define NO_DUST_VOLTAGE 400 // mv
#define SYS_VOLTAGE 5000

const int redPin = 9;    // Pin connected to the red diode
const int greenPin = 10; // Pin connected to the green diode
const int bluePin = 11;  // Pin connected to the blue diode

DHT_Unified dht(DHTPIN, DHTTYPE);

uint32_t delayMS = 500;
LiquidCrystal_I2C lcd(0x3F, 16, 2);

/*Flags*/
bool isUpperPrinted = 0;
bool isLowerPrinted = false;
bool isDustOnTop = false;
bool isTempCleared = 0;
bool isDustCleared = 0;
int currentQuality = -1;
float prevTemp = 0.0;
float prevHum = 0.0;

const int iled = 7; // drive the led of sensor
const int vout = 0; // analog input

unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

float density, voltage;
int adcvalue;

//===========================================================================================

/*Function responsible for buzzer*/
void Buzzer(const float &value)
{
  if (value > 500)
  {
    digitalWrite(buzzerPin, HIGH);
  }
  else
    digitalWrite(buzzerPin, LOW);
}

/*Sets color on dide RGB*/
void RGB(const float &value)
{
  int red, blue, green;
  if (value > 500)
  {
    red = 255;
    blue = 0;
    green = 0;
  }
  else
  {
    red = map(value, 0, 500, 100, 255);
    green = map(value, 0, 500, 200, 0);
    blue = 0;
  }

  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);
}

/*The text scrolling function on the LCD display*/
void scrollText(const String &text)
{
  static byte position = 0;
  String scrolledText = text.substring(position) + text.substring(0, position);
  position++;
  if (scrolledText.length() > 16)
    scrolledText = scrolledText.substring(0, 16);

  if (position >= text.length())
  {
    position = 0;
  }

  lcd.setCursor(0, 0);
  lcd.print(scrolledText);

  lcd.display();
}

/*Dust value printing function*/
void printDust(const String &text)
{
  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();
  unsigned long interval = 750; // Time interval between scrolling

  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;
    scrollText(text);
  }
}

/*Printing air quality*/
void printInterval(const String interval)
{
  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  lcd.print(interval);
  lcd.display();
  Serial.println(interval);
}

/*Setting air quality*/
void setDustInterval(const float &val)
{
  if (val > 0 && val < 36)
  {
    if (currentQuality != 0)
    {
      printInterval("Excellent air");
      currentQuality = 0;
    }
  }
  else if (val > 35 && val < 76)
  {
    if (currentQuality != 1)
    {
      printInterval("Good air");
      currentQuality = 1;
    }
  }
  else if (val > 75 && val < 116)
  {
    if (currentQuality != 2)
    {
      printInterval("Average air");
      currentQuality = 2;
    }
  }
  else if (val > 115 && val < 151)
  {
    if (currentQuality != 3)
    {
      printInterval("Light pollution");
      currentQuality = 3;
    }
  }
  else if (val > 150 && val < 251)
  {
    if (currentQuality != 3)
    {
      printInterval("Mid pollution");
      currentQuality = 3;
    }
  }
  else if (val > 250 && val < 501)
  {
    if (currentQuality != 4)
    {
      printInterval("Heavy pollution");
      currentQuality = 4;
    }
  }
  else
  {
    if (currentQuality != 5)
    {
      printInterval("Out of range");
      currentQuality = 5;
    }
  }
}

/*Filtering dust sensor readings*/
int Filter(int m)
{
  static int flag_first = 0, _buff[10], sum;
  const int _buff_max = 10;
  int i;

  if (flag_first == 0)
  {
    flag_first = 1;
    for (i = 0, sum = 0; i < _buff_max; i++)
    {
      _buff[i] = m;
      sum += _buff[i];
    }
    return m;
  }
  else
  {
    sum -= _buff[0];
    for (i = 0; i < (_buff_max - 1); i++)
    {
      _buff[i] = _buff[i + 1];
    }
    _buff[9] = m;
    sum += _buff[9];

    i = sum / 10.0;
    return i;
  }
}

/*Reading dust value from sensor*/
float dustSensor()
{
  digitalWrite(iled, HIGH);
  delayMicroseconds(280);
  adcvalue = analogRead(vout);
  digitalWrite(iled, LOW);

  adcvalue = Filter(adcvalue);

  voltage = (SYS_VOLTAGE / 1024.0) * adcvalue * 11;

  if (voltage >= NO_DUST_VOLTAGE)
  {
    voltage -= NO_DUST_VOLTAGE;

    density = voltage * COV_RATIO;
  }
  else
    density = 0;

  return density;
}

/*Function called when dust value is printed on display*/
void DustOnTop()
{
  float dustValue = dustSensor();

  String textToPrint = "The current dust level is: " + String(dustValue, 2) + " ug/m3 ";

  setDustInterval(dustValue);
  RGB(dustValue);
  Buzzer(dustValue);
  printDust(textToPrint);
}

/*Function called when dust value is not printed on display*/
void DustOnBottom()
{
  float dustValue = dustSensor();
  RGB(dustValue);
  Buzzer(dustValue);
}

/*Clearing display and setting flags*/
void clearDust()
{
  lcd.clear();
  currentQuality = -1;
  isTempCleared = 0;
  isDustCleared = 1;
}

//===========================================================================================

/*Function called when temperature value is printed on display*/
void TempOnTop()
{
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature) && !isUpperPrinted)
  {
    lcd.setCursor(0, 0);
    lcd.print(F("                "));
    lcd.setCursor(0, 0);
    lcd.print(F("Error temp"));
    lcd.display();
    isUpperPrinted = 1;
  }
  else if (prevTemp != event.temperature)
  {
    prevTemp = event.temperature;
    lcd.setCursor(0, 0);
    lcd.print(F("                "));
    lcd.setCursor(0, 0);
    lcd.print(F("Temp: "));
    lcd.print(prevTemp);
    lcd.print(F("\xDF"));
    lcd.print(F("C"));
    lcd.display();
  }

  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity) && !isLowerPrinted)
  {
    lcd.setCursor(0, 1);
    lcd.print(F("                "));
    lcd.setCursor(0, 1);
    lcd.print(F("Error humidity"));
    lcd.display();
    isLowerPrinted = 1;
  }
  else if (prevHum != event.relative_humidity)
  {
    prevHum = event.relative_humidity;
    lcd.setCursor(0, 1);
    lcd.print(F("                "));
    lcd.setCursor(0, 1);
    lcd.print(F("Humidity: "));
    lcd.print(prevHum);
    lcd.print(F("%"));
    lcd.display();
  }
}

/*Clearing display and setting flags*/
void clearTemp()
{
  lcd.clear();
  prevHum = 0.0f;
  prevTemp = 0.0f;
  isTempCleared = 1;
  isDustCleared = 0;
}

//===========================================================================================

/*Interruption function*/
void buttonInterrupt()
{
  unsigned long currentMillis = millis();
  if (currentMillis - lastDebounceTime > debounceDelay)
  {
    lastDebounceTime = currentMillis;
    isDustOnTop = !isDustOnTop;
  }
}

void setup()
{
  Serial.begin(9600);
  dht.begin();
  lcd.init();
  lcd.backlight();
  Wire.begin();
  lcd.clear();
  attachInterrupt(digitalPinToInterrupt(buttonPin), buttonInterrupt, FALLING);

  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(iled, OUTPUT);
  digitalWrite(iled, LOW);

  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
}

void loop()
{
  delay(delayMS);

  if (!isDustOnTop)
  {
    if (!isTempCleared)
      clearTemp();
    TempOnTop();
    DustOnBottom();
  }
  else
  {
    if (!isDustCleared)
      clearDust();
    DustOnTop();
  }
}
