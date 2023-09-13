//////////////////////////////////////////
// ручка
// PC0  - 14/A0     - поворот ручки
// PD4  - 4   - col1  - 1-я строка
// PD1  - 1   - col2  - 2-я строка
// PD5  - 5   - row1  - 1-й столбец
// PD6  - 6   - row2  - 2-й столбец
// PB4  - 12  - col3  - 3-я строка (z_neg)
// PD7  - 7   - row3  - 3-й столбец
// PB5  - 13  - col4  - 4-я строка (z_pos)

// кнопки
// GND
// PC1  - 15/A1    - газ 
// VCC
// GND
// PB3  - 11  - but7  - общий gnd
// PB2  - 10  - but8  - общий gnd
// PB1  - 9   - but9  - общий gnd
// PB0  - 8   - but10 - общий gnd

// оси
// VCC
// PC2  - 16/A2    - ось Y
// PC3  - 17/A3    - ось X
// GND

// USB
// PD2  - 2 - D+
// PD3  - 3 - D-

// blinker
// PC5  - 19/A5  - общий gnd
//////////////////////////////////////////

#include <EasyHID.h>


const char k4x3 [4][3] = {      // символы на клавиатуре
  {'2', 'r', '-'},
  {'4', 'l', '5'},
  {'6', 'd', '1'},
  {'3', 'u', '-'}
};

#define LED_PIN 19

#define axis_z_PIN A0
#define axis_acceleration A1
#define axis_x_PIN A2
#define axis_y_PIN A3

#define but10 8
#define but9 9
#define but8 10
#define but7 11

#define D_pos 2
#define D_neg 3

const int P[] = {4, 1, 12, 13}; // пины строк
const int M[] = {5, 6, 7};   // пины столбцов

#define Ry_pos 13
#define Ry_neg 12

long axisX = 0;
long axisY = 0;
long axisZ = 0;
long axisT = 0;

static unsigned long timer = millis();

uint8_t report_buffer[8];

void setup() {
  // Serial.begin(9600);
  analogReference(DEFAULT);
  HID.begin();
  pinMode (LED_PIN, OUTPUT);
  init_joy();
}

void loop() {
  blink();
  clearReport();
  read_joy();
  // test();
  // HID.tick();
  usbReportSend(7);
}

void blink(void){
  if (millis() - timer > 1000) {
    timer = millis();
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
  }
}

void test(void){
  report_buffer [0] = 0xff;
	report_buffer [1] = 0xee;
	report_buffer [2] = 0xdd;
	report_buffer [3] = 0xcc;
	report_buffer [4] = 0xbb;
	report_buffer [5] = 0xaa;
  report_buffer [6] = 0x88;
  report_buffer [7] = 0x77;
}

int averageAnalogread(int analogInPin, int samples){
   unsigned int avg_sum=0;
   for(byte i=0;i<samples;i++){
       avg_sum+=analogRead(analogInPin);
       delay(2); // небольшая пауза между замерами
   }
    
   return avg_sum/samples;
}
	

void detectAxisXYZ(void){

  // int sensX = analogRead(A2);
  // int sensY = analogRead(A3);
  // int sensRx = analogRead(A1);

  int sensX = averageAnalogread(A2,7);
  int sensY = averageAnalogread(A3,7);
  int sensRx = averageAnalogread(A1,7);




  int reportValueX = map(sensX, 0, 1023, 0, 255)+127;
  int reportValueY = map(sensY, 0, 1023, 0, 255)+127;
  int reportValueRx = map(sensRx, 0, 1023, 255, 0)+127;

  report_buffer[0] = reportValueX;
  report_buffer[1] = reportValueY;
  report_buffer[2] = reportValueRx;
}

void detectAxisRy(void){
  pinMode(Ry_pos, OUTPUT);
  pinMode(Ry_neg, OUTPUT);
  digitalWrite(Ry_pos, HIGH);
  digitalWrite(Ry_neg, LOW);
  delay(10);

  // int sensRy = analogRead(A0);
  int sensRy = averageAnalogread(A0,10);
  int reportValueRy = map(sensRy, 0, 1023, 0, 255)-127; // преобразуем в диапазон 0-255
  report_buffer[3] = reportValueRy; // записываем значение в элемент массива report для оси Z
}

static void init_joy( void )
{
  for (int i = 0; i <= 3; i++) {  // выставляем пины строк на выход, столбцов на вход
    pinMode(P[i], OUTPUT);
    digitalWrite(P[i], HIGH);
  }
  for (int i = 0; i <= 2; i++) {
    pinMode(M[i], INPUT_PULLUP);
  }

  pinMode (but7, INPUT_PULLUP);
  pinMode (but8, INPUT_PULLUP);
  pinMode (but9, INPUT_PULLUP);
  pinMode (but10, INPUT_PULLUP);
}

void clearReport(void){
  report_buffer [0] = 0;
	report_buffer [1] = 0;
	report_buffer [2] = 0;
	report_buffer [3] = 0;
	report_buffer [4] = 0;
	report_buffer [5] = 0;
  report_buffer [6] = 0;
  report_buffer [7] = 0;
}

void read_joy( void )
{

  detectAxisXYZ();
  detectAxisRy();

	// Buttons
	if (getGripKey()=='1') report_buffer[4] |= 0x01;		        //PRESSED  1
	if (getGripKey()=='2') report_buffer[4] |= 0x02;    		    //PRESSED  2
	if (getGripKey()=='3') report_buffer[4] |= 0x04;		        //PRESSED  3
	if (getGripKey()=='4') report_buffer[4] |= 0x08;		        //PRESSED  4
	if (getGripKey()=='5') report_buffer[4] |= 0x10;		        //PRESSED  5
	if (getGripKey()=='6') report_buffer[4] |= 0x20;		        //PRESSED  6
	if (!digitalRead(but7)) report_buffer[4] |= 0x80;	     		  //PRESSED  7
	if (!digitalRead(but8)) report_buffer[4] |= 0x40;			      //PRESSED  8
	if (!digitalRead(but9)) report_buffer[5] |= 0x01;			      //PRESSED  9
	if (!digitalRead(but10)) report_buffer[5] |= 0x02;			    //PRESSED  10

  // d-pad
  if (getGripKey()=='u') report_buffer[6] |= 0x01;
  if (getGripKey()=='d') report_buffer[6] |= 0x02;
  if (getGripKey()=='l') report_buffer[6] |= 0x04;
  if (getGripKey()=='r') report_buffer[6] |= 0x08;
}

char getGripKey() {
  static unsigned long timer;
  char a = 0;
  for (byte p = 0; p <= 3; p++) {    // последовательно выставляем по одной строке в LOW
    digitalWrite(P[p], LOW);
    for (byte m = 0; m <= 2; m++) {  // и считываем столбцы вылавнивая где LOW происходит
      if (!digitalRead(M[m])) {
        a = k4x3[p][m];             // считываем соотвествующий символ для комбинации столбца и строки
      }
    }
    digitalWrite(P[p], HIGH);       // возвращем строку в HIGH и крутим дальше
  }
  timer = millis();
  return a;
}