
#include <StaticThreadController.h> //Подключаем все библиотеки для работы с многопоточностью
#include <ThreadController.h>
#include <Thread.h>

#include <DFPlayer_Mini_Mp3.h> //Библиотека для работы с плеером
#include <GyverTM1637.h> //Библиотека для работы с дисплеем (в оригинале для 4х рязрядного дисплея)
#include <SoftwareSerial.h> //Библиотека для работы с serial port и uart

#define in1  3 //контакт медной нити
#define in2  8 //Контакт площадки старта
#define in3  7 //Контак площадки конца 
#define stick  4 //Контакт замыкающей палочки 
#define CLK 5 // подключение clc от дисплея
#define DIO 6 //Подключение dio от дисплея

GyverTM1637 disp(CLK, DIO); //Создаем объект дисплея

uint64_t Now, clocktimer; //переменные текущего времени и времени, выводимого на часы
boolean flag; //Включение/выключение пикселей на дисплее
byte secs = 0, santisecs = 0;//Переменные секунд и сантисекунд (сотые доли секунд)
byte t = 0;
/* используем 10 и 9 пины для программного порта
 10 - RX, 9 - TX*/
SoftwareSerial mySerial(10, 9);

//Разводим программу в 2 потока: для часов и для касания, чтобы эти два действия не зависели друг от друга
Thread timerThread = Thread();
Thread touchThread = Thread();

void setup() {
  Serial.begin(9600);
  mySerial.begin (9600); // открываем софт-порт
  mp3_set_serial (mySerial); // задаем порт для плеера
  mp3_set_volume (22); //устанавливаем уровень громкости
  
  disp.clear(); //Очищаем дисплей перед каждым запуском
  disp.brightness(7);  // яркость, 0 - 7 (минимум - максимум)

  //Задаем программную подтяжку на 5v
  pinMode(in1,INPUT_PULLUP);
  pinMode(in2,INPUT_PULLUP);
  pinMode(in3,INPUT_PULLUP);
  
  //Подаем нулевой сигнал на палочку 
  pinMode(stick, OUTPUT);
  digitalWrite(stick, LOW);

  timerThread.onRun(timer);  // назначаем потоку задачу
  timerThread.setInterval(10); // задаём интервал срабатывания, мсек
    
  touchThread.onRun(touch);     // назначаем потоку задачу
  touchThread.setInterval(200); // задаём интервал срабатывания, мсек
}

void loop() {
  // put your main code here, to run repeatedly:
    if(!digitalRead(in2))// Если палочка коснулась стартовой площадки
    {
      while (digitalRead(in3))// Пока палочка не коснулась конечной площадки 
      {
        // Проверим, пришло ли время переключиться первой цифре на табло:
        if (timerThread.shouldRun()) //Если прошло время, назначенное в setInterval
            timerThread.run(); // запускаем поток
        
        // Проверим, есть ли касание:
        if(!digitalRead(in1))// Если палочка коснулась проволоки
          if (touchThread.shouldRun())
                touchThread.run(); // запускаем поток
        }
      secs = 0, santisecs = 0; //С каждым новым стартом обнуляем табло
      t = 0;
    }
}
void timer()
{
  uint64_t tmr;
  
  Now = millis();//Запоминаем время старта
  
  while (millis () - Now < 100) {   // каждые 100 милисекунд
    if (millis() - tmr > 10) {  // каждые 10 милисекунду
      tmr = millis();
      flag = !flag;
      disp.point(flag);   // выкл/выкл точки
      if (flag) {
        santisecs ++;
        if (santisecs > 59) {
          santisecs = 0;
          secs++;
          if (secs > 60) secs = 0;
        }
        // ***** Выводим время массивом на дисплей ****
        if (t <= 20) { 
          byte dispTime[] = {' ', t % 10, t / 10 , (byte)santisecs % 10, (byte)santisecs / 10, (byte)secs % 10};
          disp.scroll(dispTime, 10);  
        }
        else { 
          byte dispTime[] = {_l, _b, _t ,_h , _o, _l}; 
          disp.displayByte(dispTime); 
          }  
      }
    }
  }
  disp.point(0);   // выкл точки
}
//При косании проигрывается трек 0003.mp3 из папки mp3 на флеш-карте в модуле DFPPlayer Mini
void touch() { 
  mp3_play (3);
  t++;}
