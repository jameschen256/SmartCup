#include <Adafruit_MLX90614.h>
#include <Wire.h>
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <SPI.h>
#include <stdio.h>

#include <millisDelay.h>
// screen ports
#define TFT_DC 9
#define TFT_CS 10
#define TFT_RST 8
#define TFT_MISO 12
#define TFT_MOSI 11
#define TFT_CLK 13


// infared sensor
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
// screen
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

const int heatButton = 3; //find pin
const int coolButton = 2; //find pin
const int tempAdjustButton = 4; //findpin

const int heatRelay = 5; // find pin
const int coolRelay = 6; //find pin
const int fanRelay = A9;
float temperature, target, prevTarget, current;
const int buzzer = A0;
int tempAdjustCounter = 0;
bool screenOn = true;
bool justPressed = false;
// 5 minute delay for screen to turn off
int64_t screenDelayTime = 300000;
// fan runs for 30 seconds after peltier turns off
int64_t fanDelayTime = 0;
// time before current temperature on screen is updated
int64_t tempUpdateDelayTime = 750;
millisDelay ledDelay;
millisDelay fanDelay;
millisDelay tempUpdateDelay;
// black theme
#define COLOR1 ILI9341_WHITE
#define COLOR2 ILI9341_BLACK

void checkButtons();
void target_to_lcd(float target, unsigned char text_position);
void temperature_to_lcd(float temperature, unsigned char text_position);
void checkTouchscreen();
void status_to_lcd(String status, unsigned char text_position);
void fillScreenInfo();
void timeOutScreen();
void turnOffCooling();

void setup() {
  // put your setup code here, to run once:

  pinMode(heatButton, INPUT);
  pinMode(coolButton, INPUT);
  pinMode(tempAdjustButton, INPUT);

  digitalWrite(heatButton, LOW);
  digitalWrite(coolButton, LOW);
  digitalWrite(tempAdjustButton, LOW);

  pinMode(heatRelay, OUTPUT);
  pinMode(coolRelay, OUTPUT);
  pinMode(fanRelay, OUTPUT);
  pinMode(buzzer, OUTPUT);
  digitalWrite(heatRelay, HIGH);
  digitalWrite(coolRelay, HIGH);
  digitalWrite(fanRelay, HIGH);

  Serial.begin(9600);
  // buzzer
  tone(buzzer, 440);
  delay(100);
  noTone(buzzer);
  // get temps
  //start infared sensor
  mlx.begin();
  temperature = (mlx.readObjectTempC() * 9.0) / 5.0 + 32.0; // read object temp
  target = (int)temperature; // have target be object temp
  prevTarget = target;
  // fill screen
  tft.begin();
  tft.setRotation(0); 
  fillScreenInfo();
  
  // start screen timeout timer
  ledDelay.start(screenDelayTime);

  tempUpdateDelay.start(tempUpdateDelayTime);
}
void loop() {
  // reads current temperature from infared sensor
  temperature = (mlx.readObjectTempC() * 9.0) / 5.0 + 32.0; // read temp
  Serial.print("temperature: ");
  Serial.print(temperature);
  Serial.print("\n");
  // checks if buttons have been pressed
  checkButtons();
  // checks if 5 minutes have passed to time out screen
  timeOutScreen();
  // updates current temperature to LCD once every designated amount of time
  // running these functions each refresh would slow down arduino considerably
  
  if (screenOn && tempUpdateDelay.justFinished()) {
    temperature_to_lcd(temperature, 4);
    tempUpdateDelay.restart();
  }
  // updates target temperature only if target temperature was changed
  if (target != prevTarget) {
    target_to_lcd(target, 111);
    prevTarget = target;
  }
  // checks if sufficent time has passed after peltier turns off to turn off fan
  if (fanDelay.justFinished() && digitalRead(coolRelay)==HIGH){
    digitalWrite(fanRelay, HIGH);
    tft.begin();
    fillScreenInfo();
  }

}

void checkButtons() {
  // checks if button to increase target temp has been pressed
  if (digitalRead(heatButton) == HIGH)
  {
  // updates target temp on screen, increases taret temperature, and resets 5 minutes before turning off screen
    ledDelay.restart();
    justPressed = true;
    delay(100);
    Serial.print("pressed heat");
    target += 1;
  }
  // checks if button to decrease target temp has been pressed
  if (digitalRead(coolButton) == HIGH)
  {
    // updates target temp on screen, decreases taret temperature, and resets 5 minutes before turning off screen
    ledDelay.restart();
    justPressed = true;
    delay(100);
    Serial.print("pressed cool");
    target -= 1;
  }
  // checks if button to update temperature has been pressed
  if (digitalRead(tempAdjustButton) == HIGH) {
    // resets 5 minutes before turning off screen
    justPressed = true;
    ledDelay.restart();
    Serial.print("adjusted temp");
    tempAdjustCounter++;
    delay(300);
    // if not already heating/cooling starts heating if target temperature is above current and starts cooling if target
    // temperature is below current
    if (tempAdjustCounter == 1) {
      if (temperature > target) {
        status_to_lcd("Cool", 218);
        digitalWrite(fanRelay, LOW);
        if (digitalRead(fanRelay) == LOW){
          digitalWrite(coolRelay, LOW);
        }
      }
      else if (temperature < target) {
        status_to_lcd("Heat", 218);
        digitalWrite(heatRelay, LOW);
      }
    }
    // if already heating/cooling and button is pressed turns off heating/cooling
    else {
      tempAdjustCounter = 0;
      status_to_lcd("Idle", 218);
      turnOffCooling();
      digitalWrite(heatRelay, HIGH);
    }
  }
  
  if ((digitalRead(coolRelay) == LOW && (temperature < target))
        || (digitalRead(heatRelay) == LOW && (temperature > target))){
        status_to_lcd("Idle", 218);
        turnOffCooling();
        if(digitalRead(heatRelay) == LOW){
          digitalWrite(heatRelay, HIGH);
          tft.begin();
          fillScreenInfo();
        }
  }
  
      
}
// updates target temp on screen
void target_to_lcd(float target, unsigned char text_position)
{
  int color;
  if (target < temperature){
    color = 0x001F;
  }
  else {
    color = 0xF800;
  }
  tft.setTextColor(color, COLOR2);
  tft.setTextSize(5);
  tft.setCursor(50, text_position + 40);
  tft.println(target);
  tft.setCursor(175, text_position + 40);
  tft.setTextColor(color, COLOR2);
  tft.print("F");
  tft.drawChar(165, text_position + 40, 247, COLOR1, COLOR2, 2);
  tft.setTextColor(COLOR1, COLOR2);
}
// updates current temperature on screen
void temperature_to_lcd(float temperature, unsigned char text_position)
{
  tft.setTextSize(5);
  tft.setCursor(50, text_position + 40);
  tft.setTextColor(COLOR1, COLOR2);
  tft.print(temperature, 1);
  tft.setCursor(175, text_position + 40);
  tft.print("F");
  tft.drawChar(165, text_position + 40, 247, COLOR1, COLOR2, 2); //degree symbol
}
// updates status on screen
void status_to_lcd(String status, unsigned char text_position)
{
  int color;
  if (status == "Cool"){
    color = 0x001F;
  }
  else if (status == "Heat"){
    color = 0xF800;
  }
  else{
    color = 0xFFFF;
  }
  tft.setTextSize(5);
  tft.setCursor(50, text_position + 40);
  tft.setTextColor(color, COLOR2);
  tft.print(status);
  tft.setTextColor(COLOR1, COLOR2);

}
// fills screen with current temp, target temp, status, and correct labels
void fillScreenInfo()
{
  tft.fillScreen(COLOR2);
  // uncomment when touchscreen works
  /*
    tft.fillRoundRect(5, 210, 100, 50, 20, ILI9341_BLUE);
    tft.fillRoundRect(125, 210, 100, 50, 20, ILI9341_ORANGE);
    tft.fillRoundRect(5, 265, 220, 50, 20, ILI9341_GREEN);
  */
  //display temp
  tft.setTextColor(COLOR1, COLOR2);
  tft.setCursor(4, 4);
  tft.setTextSize(2);
  tft.print("Temperature:");
  tft.setCursor(30, 4 + 60);
  // display target
  tft.setCursor(4, 111);
  tft.setTextSize(2);
  tft.println("Target Temp:");
  temperature_to_lcd(temperature, 4);
  target_to_lcd(target, 111);
  tft.setCursor(4, 218);
  tft.setTextSize(2);
  tft.print("Status:");
  status_to_lcd("Idle", 218);
}
// turns off screen after five minutes
// if button has been pressed when screen is off, turns back on screen
void timeOutScreen(){
  if(ledDelay.justFinished()){
    screenOn = false;
    tft.fillScreen(COLOR2);
  }  
  if(!screenOn && justPressed){
    screenOn = true;
    fillScreenInfo();
    ledDelay.restart();
  }
  justPressed = false;
}
// turns off cooling and begins timer til the fan turns off
void turnOffCooling(){
  if (digitalRead(coolRelay) == LOW){
    digitalWrite(coolRelay,HIGH);
    fanDelay.start(fanDelayTime);
  }
}
