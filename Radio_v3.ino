//Andrew Simon 
//RADIO

//Libraries
#include <Arduino.h>
//#include <math.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
//Radio depencies. 
#include <radio.h>
#include <TEA5767N.h>
#include <DS3232RTC.h>
#include <Wire.h>

#if defined(ESP8266)
#include <pgmspace.h>
#else
#include <avr/pgmspace.h>
#endif
#include <Wire.h>  // must be incuded here so that Arduino library object file references work
#include <RtcDS3231.h>


//Sets up the LCD 
LiquidCrystal_I2C LCD(0x27, 16, 2);

//Defines up the RTC
RtcDS3231 Rtc;

//Defines the radio instance.
TEA5767N radioTEA = TEA5767N();
byte isBandLimitReached = 0;
float freq = 0.00;

//New Charaters
//Up Arrow
byte upArrow[8] = {
        B00100,
        B01110,
        B10101,
        B00100,
        B00100,
        B00100,
        B00100,
        B00100
};

//Down Arrow
byte downArrow[8] = {
        B00100,
        B00100,
        B00100,
        B00100,
        B00100,
        B10101,
        B01110,
        B00100
};

//Still Symbol
byte stillSym[8] = {
        B00100,
        B00100,
        B00100,
        B11111,
        B11111,
        B00100,
        B00100,
        B00100
};

//Lvl1
byte lvl1[8] = {
        B00000,
        B00000,
        B00000,
        B00000,
        B00000,
        B00000,
        B00000,
        B11111
};

//Lvl2
byte lvl2[8] = {
        B00000,
        B00000,
        B00000,
        B00000,
        B00000,
        B00000,
        B11111,
        B11111
};

//Lvl3
byte lvl3[8] = {
        B00000,
        B00000,
        B00000,
        B00000,
        B11111,
        B11111,
        B11111,
        B11111
};

//Lvl4
byte lvl4[8] = {
        B00000,
        B00000,
        B11111,
        B11111,
        B11111,
        B11111,
        B11111,
        B11111
};

//Lvl5
byte lvl5[8] = {
        B11111,
        B11111,
        B11111,
        B11111,
        B11111,
        B11111,
        B11111,
        B11111
};

//sigSym
byte sigSym[8] = {
        B11111,
        B10001,
        B01010,
        B00100,
        B00100,
        B00100,
        B00100,
        B00100
};

//Rotery Encoder on yellow(SW:7), blue(DT:8) green(CLK:9)
const int rtryBtn = 7;
const int rtryClk = 9;
const int rtryDt = 8;
int encoderPosCounter = 0;
int pinClkLast;
int clkVal;
bool CW;
bool clickBtn;

//Vars
String oldTop;
String oldBtm;
bool encoder = false;
bool sound = true;
int backlightCounter = 0;
int blLimit = 150;

void setup() {

  //Serial output
  Serial.begin(9600);

  //Initializes the LCD
  LCD.init();
  LCD.backlight();

  //Set the Frequency.
  //freq = 102.30;
  EEPROM.get(0, freq);

  //Initializes the Radio
  radioTEA.selectFrequency(freq);
  radioTEA.turnTheSoundBackOn();
  radioTEA.setStereoReception();
  radioTEA.setSearchHighStopLevel();
  radioTEA.setStereoNoiseCancellingOn();
  radioTEA.setHighCutControlOn();

  //Rotery Encoder
  pinMode(rtryBtn, INPUT);
  pinMode(rtryClk, INPUT);
  pinMode(rtryDt, INPUT);
  pinClkLast = digitalRead(rtryClk);

  //RTC
  Rtc.Begin();
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  RtcDateTime now = Rtc.GetDateTime();
  if (!Rtc.IsDateTimeValid()){
    Serial.println("RTC lost confidence in the DateTime!");
    Rtc.SetDateTime(compiled);
  }
  if (!Rtc.GetIsRunning()){
    Serial.println("RTC was not actively running, starting now");
    Rtc.SetIsRunning(true);
  }

  if (now < compiled){
    Serial.println("RTC is older than compile time!  (Updating DateTime)");
    Rtc.SetDateTime(compiled);
  }else if (now > compiled){
    Serial.println("RTC is newer than compile time. (this is expected)");
  }else if (now == compiled){
    Serial.println("RTC is the same as compile time! (not expected but all is fine)");
  }

  Rtc.Enable32kHzPin(false);
  Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone); 

  LCDOutput();
  
}

void loop() {

  btnEvent(btnCheck());
  //btnEvent(checkRtryEnc());
  //btnEvent(btnCheck() && checkRtryEnc());
  LCDOutput();
  

}

void LCDOutput(){
  
  String topLn = "  ";
  
  topLn += String(radioTEA.readFrequencyInMHz());
  topLn += " MHz";

  String btmLn = "test ";
  btmLn += signalLevel(false);
  //Serial.println(btmLn);
  //To fix wierd printing error.
  //btmLn += "  ";
  //btmLn += getDateTime("Time");
  //LCD.setCursor(8, 1);
  //LCD.print(getDateTime("Time"));

  //PrintLCD(topLn, btmLn);

  if(((topLn != oldTop)) || (backlightCounter < 2)){
    PrintLCD("                ", "");
    PrintLCD(topLn, "");
    //Serial.println("Top diff");
  }

  if(((btmLn != oldBtm) || (backlightCounter < 2))){
    PrintLCD("", "             ");
    PrintLCD("", btmLn);
    //Serial.println("Bottom diff");
  }

  oldTop = topLn;
  oldBtm = btmLn;

}

void seekFreq(String dir){
  
  if (dir == "Up"){

    //Scan Radio stations going up.
    if(!isBandLimitReached){
      PrintLCD("SEARCHING UP", "");
      radioTEA.setSearchUp();
      radioTEA.searchNextMuting();
    }else{
      seekFreq("Down");
    }
    
  }else if (dir == "Down"){

    //Scan Radio stations going down.
    if(!isBandLimitReached){
      PrintLCD("SEARCHING DOWN", "");
      radioTEA.setSearchDown();
      radioTEA.searchNextMuting();
    }else{
      seekFreq("Up");
    }
    
  }
  
}

#define countof(a) (sizeof(a) / sizeof(a[0]))

String getDateTime(String opt){

  String ans = "";

  RtcDateTime now = Rtc.GetDateTime();
  const RtcDateTime& dt = now;
  char timestring[20];
  char datestring[20];

  if(opt == "Date"){
    
  snprintf_P(datestring, 
    countof(datestring),
    PSTR("%02u/%02u/%04u"),
    dt.Day(),
    dt.Month(),
    dt.Year() );
    ans += String(datestring);
    
  }else if(opt == "Time"){

  snprintf_P(timestring, 
    countof(timestring),
    PSTR("%02u:%02u"),
    //PSTR("%02u:%02u:%02u"),
    dt.Hour(),
    dt.Minute() );
    //dt.Second() );
    ans += String(timestring);
    
  }
  
  return ans;
  
}

String signalLevel(bool seeking){

  //int precent = (((int)radioTEA.getSignalLevel())/15.0)*100.0;
  int signalLvl = (int)radioTEA.getSignalLevel();
  String ans = "";

  int lim = 0;

  LCD.createChar(0, lvl1);
  LCD.createChar(1, lvl2);
  LCD.createChar(2, lvl3);
  LCD.createChar(3, lvl4);
  LCD.createChar(4, lvl5);
  
  if(signalLvl <= 3){
    lim = 1;
  }else if(signalLvl <= 6){
    lim = 2;
  }else if(signalLvl <= 9){
    lim = 3;
  }else if(signalLvl <= 12){
    lim = 4;
  }else if(signalLvl <= 15){
    lim = 5;
  }

  LCD.createChar(5, sigSym);
  ans += (char)5;
  
  if(seeking){
    for(int i = 0; i < 6; i++){
      ans += (char)0;
    }
    
  }else{

    for(int i = 0; i < lim; i++){
      ans += (char)i;
    }
  
    for(int i = (5 - (lim)); i > 0; i--){
      ans += (char)0;
    }
    
  }
  
  //ans += String(precent);
  //ans += "%"; 
  return ans;
  
}

void btnEvent(String btn){

  bool event = false;

  //if(((blLimit/backlightCounter)%2) == 1){
  //  LCDOutput();
  //}

  //Rtry encoder
  if(btn == "Btn" || encoder == true){
    backlightCounter = 0;
    backlit();
    //Draw a Right arrow.
      //LCD.createChar(0, downArrow);
      //LCD.setCursor(0,0);
      LCD.print(">");
    scroll();
    event = true;    
  }
  
  if(btn == "Right"){

    backlightCounter = 0;
    backlit();
    seekFreq("Up");
    event = true;
    freq = radioTEA.readFrequencyInMHz();
    
  }else if(btn == "Left"){

    backlightCounter = 0;
    backlit();
    seekFreq("Down");
    event = true;
    freq = radioTEA.readFrequencyInMHz();
    
  }else if(btn == "Up"){

    backlightCounter = 0;
    backlit();
    //Draw a up arrow.
      LCD.createChar(6, upArrow);
      LCD.setCursor(0,0);
      LCD.print((char)6);
    freq = radioTEA.readFrequencyInMHz();
    freq += 0.10;
    event = true;
    radioTEA.selectFrequency(freq);
    
  }else if(btn == "Down"){

    backlightCounter = 0;
    backlit();
      //Draw a down arrow.
      LCD.createChar(6, downArrow);
      LCD.setCursor(0,0);
      LCD.print((char)6);
    freq = radioTEA.readFrequencyInMHz();
    freq -= 0.10;
    event = true;
    radioTEA.selectFrequency(freq);
    
  }else if(btn == "Select"){

    backlightCounter = 0;
    backlit();
    if(sound == true){
      radioTEA.mute();
      PrintLCD("MUTING SOUND", "");
      sound = false;
      delay(500);
    }else if (sound == false){
      radioTEA.turnTheSoundBackOn();
      PrintLCD("UN-MUTING SOUND", "");
      sound = true;
      delay(500);
    }

  }else if(btn == "N/A"){
    if(backlightCounter <= blLimit){
      backlightCounter++;
      //Draw a still symbol.
      LCD.createChar(6, stillSym);
      LCD.setCursor(0,0);
      LCD.print((char)6);
    }
    
  }

  //backlight
  backlit();

  if (event == true){
    //Rounding the radio station.
    freq = radioTEA.readFrequencyInMHz();
    double rounded = freq;
    rounded *= 10;
    rounded = round(rounded);
    rounded /= 10;
    freq = float(rounded);
  
    //Range check - GOOD!
    //int freq = radioTEA.getFrequency();
    //108.5 - 10850 -> 10.85
    //100.0 - 10000 -> 10
    //99.9  - 99900 -> 99.9
    //87.5  - 87500 -> 87.5
    
    if(freq < 87.40){
      radioTEA.selectFrequency(108.50);
      freq = 108.50;
    }else if(freq > 108.60){
      radioTEA.selectFrequency(87.50);
      freq = 87.50;
    }

    radioTEA.selectFrequency(freq);

    //Save the current frequency.
    EEPROM.put(0, freq);
    
    if(radioTEA.readFrequencyInMHz() != freq){
      //radioTEA.selectFrequency((radioTEA.readFrequencyInMHz())+0.01);
    }
    
    
  }

}

void backlit(){

  if(backlightCounter > blLimit){
      LCD.noBacklight();
  }else{
      LCD.backlight();
  }
  
}

void PrintLCD (String msgLine1, String msgLine2){

  if(msgLine1 != ""){
    LCD.setCursor(0,0);
    LCD.print(msgLine1);
  }
  
  if(msgLine2 != ""){
    LCD.setCursor(0,1);
    LCD.print(msgLine2);
  }
  
}

void scroll(){

  encoder = true;
  
  while(encoder == true){

    String topLn = "";
    String temp = "";
    if(checkRtryEnc() == "CW"){
      //LCD.setCursor(0,0);
      //LCD.print(msgLine1);
      //topLn += upChar;
      temp += "+ 0.1";
    }else if(checkRtryEnc() == "CCW"){
      //topLn += downChar;
      temp += "- 0.1";
    }else{
      topLn += ">";
    }

  




    
  }

}

String checkRtryEnc(){

  String ans = "";
  clickBtn = !(digitalRead(rtryBtn));
  clkVal = digitalRead(rtryClk);
  if(clkVal != pinClkLast){ //Means the knob is rotationg
    //If the knob is rotating, we need to determine direction
    //We do that by reading pin Dt.
    if(digitalRead(rtryDt) != clkVal){ //Meas pinClk changed first - We're rotating counter clockwise.
      encoderPosCounter--;
      CW = false;
      ans = "CCW";
    } else { // Otherwise Dt changed first and we're moving CW.
      encoderPosCounter++;
      CW = true;
      ans = "CW";
    }

    //
    Serial.println("Rotery Encoder");
    Serial.print("Rotated: ");

    if(CW){
      Serial.println("Clockwise");
    }else{
      Serial.println("Counter Clockwise");
    }
    Serial.print("Encoder Position:");
    Serial.println(encoderPosCounter);
    //

  }

  if(clickBtn){

    ans = "Btn";
    
    //
    Serial.print("Button State: ");
    if(clickBtn){
      Serial.println("Pressed");
    }else{
      Serial.println("NOT Pressed");
    }
    Serial.println("");
    Serial.println("");
    //
  }else{
    clickBtn = false;
  }

  return ans;

  
}

String btnCheck (){

  //2. Up - (0)
  //3. Dn - (500)
  //4. Lf - (741)
  //1. Rt - (324)
  //5. Sl - (142)

  String btn = "";
  int val = analogRead(A0);
  
  if((val > 319) && (val < 329)){
    btn = "Right";
  }else if((val > -5) && (val < 5)){
    btn = "Up";
  }else if((val > 495) && (val < 505)){
    btn = "Down";
  }else if((val > 736) && (val < 746)){
    btn = "Left";
  }else if((val > 137) && (val < 147)){
    btn = "Select";
  }else{
    btn = "N/A";
  }

  return btn;
  
}



