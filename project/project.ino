#include <RTClib.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <LM35.h>

LM35 temp(A0);
RTC_DS1307 rtc;
LiquidCrystal_I2C lcd(0X27,16,2); //SCL A5 SDA A4

char date[7][7] = {"Sun","Mon", "Tues", "Wed", "Thurs", "Fri", "Sat"}; //Dùng mảng 2 chiều để lưu thứ
int val; //value cho temperature
int tempPin = 0; //temperature pin
int functionNo = 1; //đánh thứ tự hàm chức năng (hàm đầu tiên là 1)
bool isPush = false; //check coi có ấn nút function chưa(biến dùng để làm điều kiện)
bool isChoose = false; //check coi có chọn không(ban đầu sẽ là không vì chưa chọn)
bool chooseYes = false;
bool chooseNo = false;
bool isAlarm = false; //Tắt bật chức năng báo thức(ban đầu tắt)

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

//LED RED Nhận diện đang ở mode show currentdate, temp
const int redLed = 10;
//LED YELLOW Nhận diện đang ở mode set alarm
const int yellowLed = 11;
//LED REDALARM chớp thay cho báo thức reng
const int redAlarm = 12;

//hh for setting alarm hour
int hh = 0;
//mm for setting alarm minute
int mm = 0;
//ss for setting alarm second
int ss = 0;
bool isDelete = false; //Nếu báo thức không delete thì sẽ báo

void setup() {
  //Set the pin mode for LM35
  pinMode(A0, INPUT);

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
  changeMode(); //dùng để đổi giữa 2 mode

  if (functionNo == 1) { //show date, temp
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
    } else if (!isChoose) { //Trường hợp không chọn mà nhấn back luôn
      confirmYesNo();
    }

    //Xuất thgian ra serial để ktra alarm
    DateTime timeNow = rtc.now();
    //Print time for testing setAlarn() function
    Serial.print(timeNow.hour(), DEC);
    Serial.print(':');
    Serial.print(timeNow.minute(), DEC);
    Serial.print(':');
    Serial.print(timeNow.second(), DEC);
    Serial.println();
    Serial.print("-------------------------------------");
    Serial.println();
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
    Serial.println("Alarm On");
    DateTime timeNow = rtc.now();
    if (hh == timeNow.hour() && mm == timeNow.minute() && ss == timeNow.second()) {
      for (int i = 0; i < 200; i ++) { //Đèn chớp được khoảng 4-5s
        if (digitalRead(redAlarm) == HIGH) {
          digitalWrite(redAlarm, LOW);
        } else {
          digitalWrite(redAlarm, HIGH);
        }
        Serial.println("Alarm Ringing...");
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

void digitalClock() {
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
  lcd.print(temp.cel()); //Độ C
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

void setAlarm() { //Hàm hỏi có muốn cài báo thức không, nếu có mới setDateAlarm()
  lcd.setCursor(0, 0);
  lcd.print("OnOffAlarm(Y/N)");
  lcd.setCursor(0, 1);
  if (digitalRead(butHour) == HIGH) { //Mượn đỡ nút butHour làm choosing yes
    isChoose = true;
    chooseYes = true;
    chooseNo = false;
  } else if (digitalRead(butMin) == HIGH) { ////Mượn đỡ nút butMin làm choosing no
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
    } else if (digitalRead(butConfirm) == HIGH && chooseNo) { //Nếu chọn No thì trả về Digital Clock và set lại isChoose thành false
      functionNo = 1;
      isChoose = false;
      isAlarm = false;
    } else if (digitalRead(butConfirm) == HIGH && !chooseYes && !chooseNo) { //Trường hợp không chọn mà nhấn back luôn
      functionNo = 1;
      isAlarm = false;
    }
  }
}

void setTimeAlarm() { //Hàm hẹn giờ báo thức
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

void showSettingTime() { //Dùng để xuất ra LCD thgian đang nhập
  lcd.setCursor(0, 1);
  if (hh >= 0 && hh <= 24) { //ifelse này để chặn LCD print ra >= 25
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

  if (mm >= 0 && mm <= 60) { //ifelse này để chặn LCD print ra >= 60
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

  if (0 <= ss && ss <= 60) { //ifelse này để chặn LCD print ra >= 60
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

  //Xuất thgian ra serial để ktra alarm
  DateTime timeNow = rtc.now();
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