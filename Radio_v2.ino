
//Andrew Simon 
//RADIO

//Libraries
#include <Arduino.h>
#include <math.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
//Radio depencies. 
//#include <Si4703_Breakout.h>
//#include <newchip.h>
#include <radio.h>
//#include <RDA5807M.h>
//#include <RDSParser.h>
//#include <SI4703.h>
//#include <SI4705.h>
//#include <TEA5767.h>
#include <TEA5767N.h>



//Sets up the LCD 
LiquidCrystal_I2C LCD(0x27, 16, 2);

//Defines the radio instance.
//TEA5767 radioTEA; - OLD
TEA5767N radioTEA = TEA5767N();
byte isBandLimitReached = 0;
float freq = 102.30;

//Radio Definitions (Si4703). 
//const int resetPin = 2;
//const int SDIO = A4;
//const int SCLK = A5;
//Si4703_Breakout RadioSi4703(resetPin, SDIO, SCLK);
//int channel;
//int volume;
//char rdsBuffer[10];

//Vars
String oldTop;
String oldBtm;
bool sound = true;
int backlightCounter = 0;
int blLimit = 150;

void setup() {

  //Serial output
  Serial.begin(9600);

  //Initializes the LCD
  LCD.init();
  LCD.backlight();

  //Initializes the Radio
  radioTEA.selectFrequency(freq);
  radioTEA.turnTheSoundBackOn();
  radioTEA.setStereoReception();
  radioTEA.setSearchHighStopLevel();
  radioTEA.setStereoNoiseCancellingOn();
  radioTEA.setHighCutControlOn();

  //SI4703 LIB
  //RadioSi4703.powerOn();
  //RadioSi4703.setVolume(0);

  LCDOutput();
  
}

void loop() {

  btnEvent(btnCheck());
  LCDOutput();
  
  //Serial.print("Nominal Level: ");
  //Serial.println(radioTEA.getSignalLevel());

}

void LCDOutput(){
  
  String topLn = "";
  topLn += String(radioTEA.readFrequencyInMHz());
  topLn += " MHz";

  String btmLn = "";
  btmLn += signalLevel();

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

String signalLevel(){
  
  int precent = (radioTEA.getSignalLevel()/15)*100;
  String ans = String(precent);
  ans += "%"; 
  return ans;
  
}

void btnEvent(String btn){

  bool event = false;
  
  if(btn == "Right"){

    backlightCounter = 0;
    seekFreq("Up");
    event = true;
    
  }else if(btn == "Left"){

    backlightCounter = 0;
    seekFreq("Down");
    event = true;
    
  }else if(btn == "Up"){

    backlightCounter = 0;
    freq = radioTEA.readFrequencyInMHz();
    freq += 0.10;
    event = true;
    radioTEA.selectFrequency(freq);
    
  }else if(btn == "Down"){

    backlightCounter = 0;
    freq = radioTEA.readFrequencyInMHz();
    freq -= 0.10;
    event = true;
    radioTEA.selectFrequency(freq);
    
  }else if(btn == "Select"){

    backlightCounter = 0;
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
    }
    
  }

  if(backlightCounter > blLimit){
      LCD.noBacklight();
  }else{
      LCD.backlight();
  }

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

    if(radioTEA.readFrequencyInMHz() != freq){
      //radioTEA.selectFrequency((radioTEA.readFrequencyInMHz())+0.01);
    }
    
    
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



