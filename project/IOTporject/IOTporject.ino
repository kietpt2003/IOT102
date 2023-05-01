#include <RTClib.h>
#include<Wire.h>
#include <LiquidCrystal_I2C.h>
RTC_DS1307 rtc;
LiquidCrystal_I2C lcd(0X27,16,2); //SCL A5 SDA A4
char date[7][7] = {"Sun","Mon", "Tues", "Wed", "Thurs", "Fri", "Sat"}; //Dùng m?ng 2 chi?u ?? l?u th?
int val; //value cho tempreture
int tempPin = 0; //trmpreture pin
int functionNo = 1; //?ánh th? t? hàm ch?c n?ng (hàm ??u tiên là 1)
bool isPush = false; //check coi có ?n nút function ch?a(bi?n dùng ?? làm ?i?u ki?n)
bool isChoose = false; //check coi có ch?n không(ban ??u s? là không vì ch?a ch?n)
bool chooseYes = false;
bool chooseNo = false;
bool isAlarm = false; //T?t b?t ch?c n?ng báo th?c(ban ??u t?t)


//Button for setting second
const int butSec = 5;
//Button for setting minute
const int butMin = 6;
//Button for setting hour
const int butHour = 7;
//Button for choosing function
const int butFunc = 8;
//Button for confirm yes/no
const int butConfirm = 9;


//LED RED Nh?n di?n ?ang ? mode show currentdate, temp
const int redLed = 10;
//LED YELLOW Nh?n di?n ?ang ? mode set alarm
const int yellowLed = 11;
//LED REDALARM ch?p thay cho báo th?c reng
const int redAlarm = 12;


//hh for setting alarm hour
int hh = 0;
//mm for setting alarm minute
int mm = 0;
//ss for setting alarm second
int ss = 0;
bool isDelete = false; //N?u báo th?c không delete thì s? báo


void setup() {
  Serial.begin(9600);
  Wire.begin();
  rtc.begin(); //load the time from your computer
  rtc.adjust(DateTime(__DATE__,__TIME__)); //Set current date, time for RTC


  //Set up LCD
  lcd.init();
  lcd.backlight();
  
  //Set up buttons
  pinMode(butSec, INPUT);
  pinMode(butMin, INPUT);
  pinMode(butHour, INPUT);
  pinMode(butFunc, INPUT);
  pinMode(butConfirm, INPUT);


  //Set up LEDs
  pinMode(redLed, OUTPUT);
  pinMode(yellowLed, OUTPUT);
  pinMode(redAlarm, OUTPUT);


}


void loop() {
  changeMode(); //dùng ?? ??i gi?a 2 mode


  if (functionNo == 1) {
    digitalClock();
    digitalWrite(redLed, HIGH);
    digitalWrite(yellowLed, LOW);
  } else if (functionNo == 2) { //setAlarm(Yes/No)?
    setAlarm();
    digitalWrite(redLed, LOW);
    digitalWrite(yellowLed, HIGH);
    if (isChoose && chooseYes) {
      lcd.print("Yes");
      confirmYesNo();
    } else if (isChoose && chooseNo) {
      lcd.print("No");
      confirmYesNo();
    } else if (!isChoose) { //Tr??ng h?p không ch?n mà nh?n back luôn
      confirmYesNo();
    }
  } else if (functionNo == 3) {
    lcd.setCursor(0, 0);
    lcd.print("hh:mm:ss");
    lcd.setCursor(9, 0);
    lcd.print("PushFor");
    lcd.setCursor(9, 1);
    lcd.print("Reset");
    setTimeAlarm();
  }


  if (isAlarm) {
    DateTime timeNow = rtc.now();
    if (hh == timeNow.hour() && mm == timeNow.minute() && ss == timeNow.second()) {
      for (int i = 0; i < 200; i ++) { //?èn ch?p ???c kho?ng 4-5s
        if (digitalRead(redAlarm) == HIGH) {
          digitalWrite(redAlarm, LOW);
        } else {
          digitalWrite(redAlarm, HIGH);
        }
        Serial.println("Alarm Ringging...");
      }
    }
  } else {
    Serial.println("Alarm Off");
  }


  delay(1000);
  lcd.clear();
}


void changeMode() {
  if (digitalRead(butFunc) == HIGH && isPush == false) {
    functionNo = 2;
    isPush = true;
  } else if (digitalRead(butFunc) == HIGH && isPush == true) {
    functionNo = 1;
    isPush = false;
  }
}


float calculateTemp() { //LM35
  val = analogRead(tempPin); //??c giá tr? nhi?t ?? ra, sau ?ó l?u bi?n value(val) ?? tính toán
  float mv = ( val/1023.0)*5000; 
  float cel = mv/10; //?? C
  return cel;
  // float farh = (cel*9)/5 + 32; //?? F
  //return farh;
}


void digitalClock() {
  float cel = calculateTemp(); //?? C


  DateTime timeNow = rtc.now();
  lcd.setCursor(0, 0);
  lcd.print(timeNow.year());
  lcd.setCursor(4, 0);


  lcd.print('/');
  lcd.setCursor(5, 0);


  if (timeNow.month() <= 9) {
    lcd.print(0);
    lcd.setCursor(6, 0);
    lcd.print(timeNow.month());
  } else {
    lcd.print(timeNow.month());
    lcd.setCursor(7, 0);
  }


  lcd.print('/');
  lcd.setCursor(8, 0);


  if (timeNow.day() <= 9) {
    lcd.print(0);
    lcd.setCursor(9, 0);
    lcd.print(timeNow.day());
  } else {
    lcd.print(timeNow.day());
    lcd.setCursor(11, 0);
  }


  lcd.print(date[timeNow.dayOfTheWeek()]);


  lcd.setCursor(0, 1);
  lcd.print("Temp: ");
  lcd.setCursor(6, 1);
  lcd.print(cel);
  lcd.setCursor(10, 1);
  lcd.print("*C");


  //Print time for testing setAlarn() function
  Serial.print(timeNow.hour(), DEC);
  Serial.print(':');
  Serial.print(timeNow.minute(), DEC);
  Serial.print(':');
  Serial.print(timeNow.second(), DEC);
  Serial.println();
  Serial.print("-------------------------------------");
  Serial.println();
}


void setAlarm() { //Hàm h?i có mu?n cài báo th?c không, n?u có m?i setDateAlram()
  lcd.setCursor(0, 0);
  lcd.print("OnOffAlarm(Y/N)");
  lcd.setCursor(0, 1);
  if (digitalRead(butHour) == HIGH) { //M??n ?? nút butHour làm choosing yes
    isChoose = true;
    chooseYes = true;
    chooseNo = false;
  } else if (digitalRead(butMin) == HIGH) { ////M??n ?? nút butMin làm choosing no
    isChoose = true;
    chooseYes = false;
    chooseNo = true;
  }
}


void confirmYesNo() {
  if (functionNo == 3) {
    if (digitalRead(butConfirm) == HIGH) { //Bush for reset
      hh = 0;
      mm = 0;
      ss = 0;
    }
  } else {
    if (digitalRead(butConfirm) == HIGH && chooseYes) {
      functionNo = 3;
      isAlarm = true;
    } else if (digitalRead(butConfirm) == HIGH && chooseNo) { //N?u ch?n No thì tr? v? Digital Clock và set l?i isChoose thành false
      functionNo = 1;
      isChoose = false;
      isAlarm = false;
    } else if (digitalRead(butConfirm) == HIGH && !chooseYes && !chooseNo) { //Tr??ng h?p không ch?n mà nh?n back luôn
      functionNo = 1;
      isAlarm = false;
    }
  }
  
}


void setTimeAlarm() { //Hàm h?n gi? báo th?c
  showSettingTime();
  if (digitalRead(butHour) == HIGH) {
    if (hh >= 0 && hh <= 24) {
      hh++;
    } else {
      hh = 0;
    }
  }
  if (digitalRead(butMin) == HIGH) {
    if (mm >= 0 && mm <= 60) {
      mm++;
    } else {
      mm = 0;
    }
  }
  if (digitalRead(butSec) == HIGH) {
    if (ss >= 0 && ss <= 60) {
      ss++;
    } else {
      ss = 0;
    }
  }
  confirmYesNo(); //For reset Time to format(Not turn off Alarm mode)
}


void showSettingTime() { //Dùng ?? xu?t ra LCD thgian ?ang nh?p
  lcd.setCursor(0, 1);
  if (hh >= 0 && hh <= 24) { //ifelse này ?? ch?n LCD print ra >= 25
    if (hh <= 9) {
      lcd.print(0);
      lcd.setCursor(1, 1);
      lcd.print(hh);
    } else {
      lcd.print(hh);
    }
  } else {
    lcd.print("00");
  }
  


  lcd.setCursor(2, 1);
  lcd.print(":");
  lcd.setCursor(3, 1);


  if (mm >= 0 && mm <= 60) { //ifelse này ?? ch?n LCD print ra >= 60
    if (mm <= 9) {
      lcd.print(0);
      lcd.setCursor(4, 1);
      lcd.print(mm);
    } else {
      lcd.print(mm);
    }
  } else {
    lcd.print("00");
  }


  lcd.setCursor(5, 1);
  lcd.print(":");
  lcd.setCursor(6, 1);


  if (0 <= ss && ss <= 60) { //ifelse này ?? ch?n LCD print ra >= 60
    if (ss <= 9) {
      lcd.print(0);
      lcd.setCursor(7, 1);
      lcd.print(ss);
    } else {
      lcd.print(ss);
    }
  } else {
    lcd.print("00");
  }
}