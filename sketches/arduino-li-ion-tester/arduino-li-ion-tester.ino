#include "U8glib.h"  //Library for display 
#define MOSFET_Pin 2
#define Bat_Pin A0 //bat+
#define Res_Pin A1 //mosfet

U8GLIB_SH1106_128X64 u8g(U8G_I2C_OPT_NONE);  // I2C interface for OLED  
float Capacity = 0.0; 
float Res_Value = 10.0;  
float Vcc = 4.410; //Measure it and change  
float Current = 0.0; 
float mA=0;         
float Bat_Volt = 0.0;  
float Res_Volt = 0.0;  
unsigned long previousMillis = 0; 
unsigned long millisPassed = 0;  
float sample1 =0.000;
float sample2= 0.000;
int x = 0;
int row = 0;

void draw(void) {
   u8g.setFont(u8g_font_fub14r); 
   if ( Bat_Volt < 1){
    u8g.setPrintPos(10,40);      
    u8g.println("No Battrey"); 
   }
   else if ( Bat_Volt > 4.3){
    u8g.setPrintPos(3,40);       
    u8g.println("High Voltage"); 
   }
   else if(Bat_Volt < 2.9){
    u8g.setPrintPos(3,40);      
    u8g.println("Low Voltage"); 
   }
   else if(Bat_Volt >= 2.9 && Bat_Volt < 4.3  ){
      
   u8g.drawStr(0, 20, "V: ");  
   u8g.drawStr(0, 40, "I: ");
   u8g.drawStr(0, 60, "mAh: ");
   u8g.setPrintPos(58,20);       
   u8g.print( Bat_Volt,2);  
   u8g.println("V"); 
   u8g.setPrintPos(58,40);        
   u8g.print( mA,0);  
   u8g.println("mA"); 
   u8g.setPrintPos(58, 60);       
   u8g.print( Capacity ,1);    
  
}
}
  void buz()
  {
  digitalWrite(9, HIGH);
  delay(100);          
  digitalWrite(9, LOW);  
  delay(10);            

}   
  void setup() {
   Serial.begin(9600);
   pinMode(A4,INPUT_PULLUP); //mosi
   pinMode(A5, INPUT_PULLUP); //clk
   pinMode(MOSFET_Pin, OUTPUT);
   pinMode(9, OUTPUT); // buz
   digitalWrite(MOSFET_Pin, LOW); 
  }
  
  void loop() {
  
  for(int i=0;i< 100;i++) //100  раз меряем напряжение
  {
   sample1=sample1+analogRead(Bat_Pin);
   delay (2);
  }
  sample1=sample1/100; //усредняем
  Bat_Volt = 2* sample1 *Vcc/ 1024.0; 

   for(int i=0;i< 100;i++)
  {
   sample2=sample2+analogRead(Res_Pin); 
   delay (2);
  }
  sample2=sample2/100;
  Res_Volt = 2* sample2 * Vcc/ 1024.0;
  
  if ( Bat_Volt > 4.3){
    digitalWrite(MOSFET_Pin, LOW); 
    buz();
    Serial.println( "Warning High-V! ");
    delay(1000);
   }
   
   else if(Bat_Volt < 2.9){
      digitalWrite(MOSFET_Pin, LOW);
      buz();
      Serial.println( "Warning Low-V! ");
      delay(1000);
  }
  else if(Bat_Volt > 2.9 && Bat_Volt < 4.3  ) { 
      digitalWrite(MOSFET_Pin, HIGH);
      millisPassed = millis() - previousMillis;
      Current = (Bat_Volt - Res_Volt) / Res_Value;
      mA = Current * 1000.0 ;
      Capacity = Capacity + mA * (millisPassed / 3600000.0);
      previousMillis = millis();
      row++;
      x++;
      delay(100); 
 
     }

  u8g.firstPage();  
  do {
    draw();
  } while( u8g.nextPage() );
  
 }    
