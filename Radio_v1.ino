
//Andrew Simon 
//RADIO

//Libraries
#include <Arduino.h>
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
#include <TEA5767.h>
#include <TEA5767N.h>


//Sets up the LCD 
LiquidCrystal_I2C LCD(0x27, 16, 2);

//Defines the radio instance.
//TEA5767 radioTEA; - OLD
TEA5767N radioTEA = TEA5767N();


//Radio Definitions (Si4703). 
//const int resetPin = 2;
//const int SDIO = A4;
//const int SCLK = A5;
//Si4703_Breakout RadioSi4703(resetPin, SDIO, SCLK);
//int channel;
//int volume;
//char rdsBuffer[10];


void setup() {

  //Serial output
  Serial.begin(9600);

  //Initializes the LCD
  LCD.init();
  LCD.backlight();

  //Initializes the Radio
  //radioTEA.init();
  //radioTEA.debugEnable();
  //radioTEA.setVolume(15);
  //radioTEA.setMono(false);
  //radioTEA.setBandFrequency(RADIO_BAND_FM, 10231);

  radioTEA.selectFrequency(102.3);
  radioTEA.turnTheSoundBackOn();


  //SI4703 LIB
  //RadioSi4703.powerOn();
  //RadioSi4703.setVolume(0);
  
}

void loop() {

  //char s[12];
  //radioTEA.formatFrequency(s, sizeof(s));
  //PrintLCD(s, String(radioTEA.getFrequency()));

  //String topLn = "  ";
  //topLn += s;
  //PrintLCD(topLn, "");

  PrintLCD(String(radioTEA.readFrequencyInMHz()), String(radioTEA.getSignalLevel()));
  
  //String get_band = radioTEA.getBand();
  btnEvent(btnCheck());

  String info = "";
  //info = radioTEA.getRadioInfo();

}

void btnEvent(String btn){
  
  if(btn == "Right"){

    //radioTEA.seekUp(true);
    //radioTEA.setFrequency();
    radioTEA.setSearchUp();
    radioTEA.searchNext();
    
    
  }else if(btn == "Left"){
    
    //radioTEA.seekDown(true);
    radioTEA.setSearchDown();
    radioTEA.searchNext();
    
  }else if(btn == "Up"){
    //Moves up with an incriment of .1
    //int get_freq = radioTEA.getFrequency();
    //get_freq += 11;
    //radioTEA.setFrequency(get_freq);
    
  }else if(btn == "Down"){
    //Moves down with an incriment of .1
    //int get_freq = radioTEA.getFrequency();
    //get_freq -= 9;
    //radioTEA.setFrequency(get_freq);
    
  }

  //Range check - GOOD!
  //int freq = radioTEA.getFrequency();
  //108.5 - 10850 -> 10.85
  //100.0 - 10000 -> 10
  //99.9  - 99900 -> 99.9
  //87.5  - 87500 -> 87.5
/*

  if(freq == 8740){
    //radioTEA.setFrequency(10851);
  }else if(freq == 10860){
    //radioTEA.setFrequency(8751);
  }
*/
  
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



