#include <Wire.h> // библиотека для управления устройствами по I2C 
#include <LiquidCrystal_I2C.h> // библиотека для работы с ЖК-дисплеем через I2C
#include <IRremote.h> // библиотека для ИК датчика
#include <DHT.h>  // библиотека для температуры
#define PIN_A1 18
#define PIN_A2 19
#define PIN_B1 16
#define PIN_B2 17
#define echoPin 25 // Echo Pin
#define trigPin 26 // Trigger Pin
IRrecv irrecv(12); // ИК датчик
DHT dht(27, DHT11);  // сообщаем на каком порту будет датчик


LiquidCrystal_I2C lcd(0x27, 16, 2); // объект для управления ЖК-экраном с 16 колонками и 2 строками
decode_results results; // ИК датчик 
const int MaxDistanceMeter = 10; // максимально допустимое число для дальномера в СМ (сколько до предмета)
int MinimumRange = 0; // Минимальное допустимое значение в СМ
long duration, distance; // Длительность, используемая для вычисления расстояния, растояние 
const int TimeToTurn = 1000; // время для поворота на 90
unsigned long last_time; // для millis() для дальномера
const long interval = 100; // С каким интервалом дальномер будет чекать расстояние миллисекундах
byte speed = 128; // значение скорости движения
float h; // переменная влажности
float t; // переменная температуры
byte direction = 0; // робот едет/не едет, направление

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(PIN_A1, OUTPUT); 
  pinMode(PIN_A2, OUTPUT);
  pinMode(PIN_B1, OUTPUT);
  pinMode(PIN_B2, OUTPUT);
  irrecv.enableIRIn(); // вкл ИК приемник 
  Serial.begin(115200);
  dht.begin();         // запускаем датчик DHT11
  lcd.init();   // начинаем работу с ЖК-дисплеем
  lcd.backlight();  // включаем подсветку ЖК-дисплея
  lcd.setCursor(0, 0); // устанавливаем на ЖК-дисплее курсор в колонку 0, строку 0
}


void loop() 
{
  if(millis() - last_time > interval) {     //Прерывание для дальномера
    last_time = millis();
    calculate_distance();
  }

  if (irrecv.decode(&results)) {  // возвращает код кнопки с пульта 
    Serial.println(results.value, HEX);


    if (results.value==0x490 && speed < 255 && speed+30 <= 255) { // увеличение скорости при нажатии кнопки
        speed += 30;
        proceed();
      Serial.print(speed);
    }
    if (results.value==0xC90 && speed > 0 && speed-30 >= 50) { // уменьшение скорости при нажатии кнопки
        speed -= 30;
        proceed();
      Serial.print(speed);
      }


    if (results.value==0x9CB47){ // вперед на пульте
      forward();
      direction = 1;
    }
    if (results.value==0x5CB47){ // назад
      backward();
      direction = 2;
    }   
    if (results.value==0x3CB47){ // вправо
      right();
      direction = 3;
    }
    if (results.value==0xDCB47){ // влево
      left();
      direction = 4;
    }
    if (results.value==0xBCB47){ // стоп
      stop();
      direction = 0;
    }
    
    irrecv.resume();  
  }
  // считываем температуру (t) и влажность (h)
   h = dht.readHumidity();
   t = dht.readTemperature();

   // выводим температуру (t) и влажность (h) на монитор порта
   Serial.print("Humidity: ");
   Serial.println(h);
   Serial.print("Temperature: ");
   Serial.println(t);

  displayView(); // выводим температуру (t), влажность (h), скорость на ЖК-дисплей
}

void proceed() { // продолжение движения в заданном направлении после увеличения/уменьшения скорости
  if (direction == 1) {
    forward();
  }
  else if (direction == 2) {
    backward();
  }
  else if (direction == 3) {
    right();
  }
  else if (direction == 4) {
    left();
  }
}

void forward() // Движение вперед
{
  analogWrite(PIN_A1, speed);
  analogWrite(PIN_A2, 0);
  analogWrite(PIN_B1, speed);
  analogWrite(PIN_B2, 0);
}

void backward() // Движение назад
{
  analogWrite(PIN_A1, 0);
  analogWrite(PIN_A2, speed);
  analogWrite(PIN_B1, 0);
  analogWrite(PIN_B2, speed);
}

void right() // Движение вправо
{
  analogWrite(PIN_A1, speed);
  analogWrite(PIN_A2, 0);
  analogWrite(PIN_B1, 0);
  analogWrite(PIN_B2, speed);
}

void left() // Движение влево
{
  analogWrite(PIN_A1, 0);
  analogWrite(PIN_A2, speed);
  analogWrite(PIN_B1, speed);
  analogWrite(PIN_B2, 0);
}

void stop() // Остановка движения
{
  analogWrite(PIN_A1, 0);
  analogWrite(PIN_A2, 0);
  analogWrite(PIN_B1, 0);
  analogWrite(PIN_B2, 0);
}

int calculate_distance()      //Следующая функция возвращает расстояние и останавливает если есть препятствие
{ 
// trigPin/echoPin используется для определения 
//расстояние до ближайшего объекта, отражаясь от него звуковыми волнами 
 digitalWrite(trigPin, LOW); 
 delayMicroseconds(2); 
 digitalWrite(trigPin, HIGH);
 delayMicroseconds(10); 
 digitalWrite(trigPin, LOW);
 duration = pulseIn(echoPin, HIGH);
 distance = duration/58.2;  //расстояние (в см) исходя из скорости звука.
if(distance >= MaxDistanceMeter || distance <= MinimumRange){
  //Serial.println(distance); // далеко 
}
else{
  //Serial.println(distance); // близко
  stop();
}
return distance;
} 

void displayView() { // функция вывода температуры (t), влажности (h), скорости (speed) на ЖК-дисплей
  lcd.setCursor(0, 0);
  lcd.print("h=");
  lcd.print(h); // вывод влажности
  lcd.print(char(37)); // знак процента
  lcd.setCursor(8, 0);
  lcd.print("t=");
  lcd.print(t); // вывод температуры
  lcd.print(char(223)); // знак градуса
  lcd.print("C");
  lcd.setCursor(0, 1);
  lcd.print("v=");
  lcd.print(speed); // вывод скорости
}