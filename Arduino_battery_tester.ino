/*
https://alexgyver.ru/capacity_tester/
Провода от аккумулятора к резистору делаем толстыми (2 мм. кв.), чтобы избежать потери и повысить точность
Не забываем указать в скетче напряжение отсечки и точное сопротивление вашей нагрузки: 2.2 Ом 4.4 Ом 8.7 Ом 

*/


#define NUM_READS 100

byte load_pin=0; //пин нагрузки (аналоговый!!!!)
byte res_pin=1; //пин нагрузки (аналоговый!!!!)
byte buzz_pin=2; //динамик сигнал
byte relay_pin=4; //пин реле

float Voff = 3.0; // напряжение отсечки (для Li-ion=2.5 В, для LiPo=3.0 В)
float R = 8.7; //сопротивление нагрузки 2.2 Ом 4.4 Ом 8.7 Ом

const float typVbg = 1.095; // 1.0 -- 1.2
float I;
float cap = 0; //начальная ёмкость
float V;
float V_res;
float Vcc;
float Wh = 0;
unsigned long prevMillis;
unsigned long testStart;
String cap_string;


void setup() {  
  start:
  pinMode(relay_pin, OUTPUT);  //пин реле как выход
  digitalWrite(relay_pin, LOW); //выключить реле 
  pinMode(buzz_pin, OUTPUT); //пищалка

  Serial.begin(9600);  //открыть порт для связи с компом
  Serial.println(" ");
  Serial.println("Тест емкости аккумуляторов. Параметры: ");
  Serial.print("Напряжение выключения ");
  Serial.print(Voff);
  Serial.print("  Сопротивление Нагрузки ");
  Serial.print(R);
  Serial.println(" ");
  Serial.println(" ");
  Serial.println("Нажмите 1 для начала теста"); 
  
  while (Serial.parseInt() != 1) {   }
 
  tone(buzz_pin,2500,500);

   if ( V < 1){ Serial.println("No Battrey"); goto start;}
   else if ( V > 4.3){Serial.println("High Voltage"); goto start;}
   else if(V < Voff){Serial.println("Low Voltage"); goto start;}  
   else {Serial.println("Test_start");} 
 
Serial.println(F("s   V   A   mAh   Wh   Vcc "));


  digitalWrite(relay_pin, HIGH); //Переключить реле (замкнуть акум на нагрузку) HIGH
  testStart = millis();  //время начала теста в системе отсчёта ардуины
  prevMillis = millis();  //время первого шага
}

void loop() {
  Vcc = readVcc(); //хитрое считывание опорного напряжения (функция readVcc() находится ниже)
  V = (readAnalog(load_pin) * Vcc) / 1023.000; //считывание напряжения АКБ
  V_res = (readAnalog(res_pin) * Vcc) / 1023.000; //считывание напряжения АКБ
  I = (V - V_res)/R; //расчет тока по закону Ома, в Амперах
  cap += I*(millis()-prevMillis)/3600000*1000; //расчет емкости АКБ в мАч
  Wh += I*V *(millis() - prevMillis)/3600000; //расчет емкости АКБ в ВтЧ
  prevMillis = millis();
  sendData(); // отправка данных
  
  if (V < Voff) { //выключение нагрузки при достижении минимального напряжения
    digitalWrite(relay_pin, LOW);  //разорвать цепь (отключить акб от нагрузки)
    Serial.println("Test is done");  //тест закончен
    
    for (int i=0; i<5; i++) {  //выполнить 5 раз
      tone(buzz_pin,1500,500);   //пищать на 3 пин частотой 100 герц 500 миллисекунд
 //     disp_print(cap_string);
      delay(1500);

    }
    
    while (2 > 1) {  //бесконечный цикл, чтобы loop() не перезапустился + моргаем результатом!
//      disp_print(cap_string);
      delay(1500);

    }
  }
}

void sendData() {       //функция, которая посылает данные в порт
  Serial.print((millis() - testStart) / 1000);
  Serial.print(" ");
  Serial.print(V, 3);
  Serial.print(" ");
  Serial.print(I, 3);
  Serial.print(" ");
  Serial.print(cap, 0);
  Serial.print(" ");
  Serial.print(Wh, 2);
  Serial.print(" ");
  Serial.println(Vcc, 3);   
  cap_string=String(round(cap));
//  disp_print(cap_string);
}

//----------Функция точного определения опорного напряжения для измерения напряжения на акуме-------
float readAnalog(int pin) {  
  // read multiple values and sort them to take the mode
  int sortedValues[NUM_READS];
  for (int i = 0; i < NUM_READS; i++) {
    delay(25);    
    int value = analogRead(pin);
    int j;
    if (value < sortedValues[0] || i == 0) {
      j = 0; //insert at first position
    }
    else {
      for (j = 1; j < i; j++) {
        if (sortedValues[j - 1] <= value && sortedValues[j] >= value) {
          // j is insert position
          break;
        }
      }
    }
    for (int k = i; k > j; k--) {
      // move all values higher than current reading up one position
      sortedValues[k] = sortedValues[k - 1];
    }
    sortedValues[j] = value; //insert current reading
  }
  //return scaled mode of 10 values
  float returnval = 0;
  for (int i = NUM_READS / 2 - 5; i < (NUM_READS / 2 + 5); i++) {
    returnval += sortedValues[i];
  }
  return returnval / 10;
}
//----------Функция точного определения опорного напряжения для измерения напряжения на акуме КОНЕЦ-------


//----------фильтр данных (для уменьшения шумов и разброса данных)-------
float readVcc() {
  // read multiple values and sort them to take the mode
  float sortedValues[NUM_READS];
  for (int i = 0; i < NUM_READS; i++) {
    float tmp = 0.0;
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
    ADCSRA |= _BV(ADSC); // Start conversion
    delay(25);
    while (bit_is_set(ADCSRA, ADSC)); // measuring
    uint8_t low = ADCL; // must read ADCL first - it then locks ADCH
    uint8_t high = ADCH; // unlocks both
    tmp = (high << 8) | low;
    float value = (typVbg * 1023.0) / tmp;
    int j;
    if (value < sortedValues[0] || i == 0) {
      j = 0; //insert at first position
    }
    else {
      for (j = 1; j < i; j++) {
        if (sortedValues[j - 1] <= value && sortedValues[j] >= value) {
          // j is insert position
          break;
        }
      }
    }
    for (int k = i; k > j; k--) {
      // move all values higher than current reading up one position
      sortedValues[k] = sortedValues[k - 1];
    }
    sortedValues[j] = value; //insert current reading
  }
  //return scaled mode of 10 values
  float returnval = 0;
  for (int i = NUM_READS / 2 - 5; i < (NUM_READS / 2 + 5); i++) {
    returnval += sortedValues[i];
  }
  return returnval / 10;
}
//----------фильтр данных (для уменьшения шумов и разброса данных) КОНЕЦ-------
/*
void disp_print(String x) {
  disp.point(POINT_OFF);
  switch (x.length()) {         //кароч тут измеряется длина строки и соотвествено выводится всё на дисплей
  case 1:
    disp.display(0,18);
    disp.display(1,18);
    disp.display(2,18);
    disp.display(3,x[0]- '0');
    break;
  case 2:
    disp.display(0,18);
    disp.display(1,18);
    disp.display(2,x[0]- '0');
    disp.display(3,x[1]- '0');
    break;
  case 3:
    disp.display(0,18);
    disp.display(1,x[0]- '0');
    disp.display(2,x[1]- '0');
    disp.display(3,x[2]- '0');
    break;
  }
}

*/
