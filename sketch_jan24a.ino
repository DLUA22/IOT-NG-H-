#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal.h>

#define SECOND_PERIOD 1000
#define MINUTE_PERIOD 60
#define HOUR_PERIOD   60
#define DAY_PERIOD    24

#define NOTE_C4  262

#define MODE_KEY    1
#define POS_KEY     2
#define EDT_KEY     3
#define OK_KEY      4

#define KEY_READ_INTERVAL 200

#define NORMAL_MODE   0
#define EDIT_MODE     1

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
RTC_DS1307 rtc;

unsigned long CurrentMillis = 0;
unsigned long StartMillis = 0;
unsigned long KeyMillis = 0;

unsigned char Hour = 0;
unsigned char Minute = 0;
unsigned char Second = 0;

unsigned char tmpHour = 0;
unsigned char tmpMinute = 0;
unsigned char tmpSecond = 0;

bool ToggleVal = LOW;

int Mode;
int Xpos;
int KeyVal = -1;

void setup()
{
  Wire.begin();
  rtc.begin();
  
  if (!rtc.isrunning()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  InitializeLCD();
  InitializeKeys();
  InitializeGlobalVars();
  
  StartMillis = millis();
}

void loop()
{
  CurrentMillis = millis();
  if ((CurrentMillis - StartMillis) >= SECOND_PERIOD)
  {
    StartMillis = CurrentMillis;
    
    UpdateTime();

    ToggleVal ^= HIGH;
    digitalWrite(13, ToggleVal);

    if (Mode == NORMAL_MODE)
    {
      ShowTime();
    }
  }
  
  if ((CurrentMillis - KeyMillis) >= KEY_READ_INTERVAL)
  {
    KeyMillis = CurrentMillis;
    ReadKey();
  }
}

void InitializeGlobalVars(void)
{
  CurrentMillis = 0;
  StartMillis = 0;
  KeyMillis = 0;

  DateTime now = rtc.now();
  Hour = now.hour();
  Minute = now.minute();
  Second = now.second();

  tmpHour = 0;
  tmpMinute = 0;
  tmpSecond = 0;

  Mode = NORMAL_MODE;
  Xpos = 0;
  KeyVal = -1;
}

void InitializeLCD(void)
{
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("EMBEDDED SYSTEM");
  lcd.setCursor(0, 1);
  lcd.print("REAL TIME CLOCK");
  delay(5000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Current Time Is:");
}

void ShowTime(void)
{
  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  lcd.print(Hour);
  lcd.setCursor(3, 1);
  lcd.print(':');
  lcd.setCursor(5, 1);
  lcd.print(Minute);
  lcd.setCursor(8, 1);
  lcd.print(':');
  lcd.setCursor(10, 1);
  lcd.print(Second);
}

void ShowDummyTime()
{
  lcd.noBlink();
  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  lcd.print(tmpHour);
  lcd.setCursor(3, 1);
  lcd.print(':');
  lcd.setCursor(5, 1);
  lcd.print(tmpMinute);
  lcd.setCursor(8, 1);
  lcd.print(':');
  lcd.setCursor(10, 1);
  lcd.print(tmpSecond);
  lcd.blink();
  switch (Xpos)
  {
    case 0: lcd.setCursor(0, 1); break;
    case 1: lcd.setCursor(5, 1); break;
    case 2: lcd.setCursor(10, 1); break;
  }
}

void UpdateTime(void)
{
  DateTime now = rtc.now();
  Hour = now.hour();
  Minute = now.minute();
  Second = now.second();
}

void SoundBuzzer(void)
{
  tone(8, NOTE_C4, 200);  
  delay(200);             
}

void InitializeKeys(void)
{
  pinMode(6, INPUT);
  pinMode(7, INPUT);
  pinMode(9, INPUT);
  pinMode(10, INPUT);

  pinMode(13, OUTPUT);
}

void ReadKey(void)
{
  if (digitalRead(6) == HIGH)
  {
    KeyVal = MODE_KEY;
  }
  else if (digitalRead(7) == HIGH)
  {
    KeyVal = POS_KEY;
  }
  else if (digitalRead(9) == HIGH)
  {
    KeyVal = EDT_KEY;
  }
  else if (digitalRead(10) == HIGH)
  {
    KeyVal = OK_KEY;
  }
  else
  {
    KeyVal = -1;
  }
  
  if (KeyVal != -1)
  {
    SoundBuzzer();
  }
  ProcessKey();
}

void ProcessKey(void)
{
  switch (KeyVal)
  {
    case MODE_KEY: ModeKeyTask(); break;
    case POS_KEY: PosKeyTask(); break;
    case EDT_KEY: EditKeyTask(); break;
    case OK_KEY: OkKeyTask(); break;
  }
  KeyVal = -1;
}

void ModeKeyTask(void)
{
  Xpos = 0;
  lcd.setCursor(0, 1);
  
  if (Mode == NORMAL_MODE)
  {
    Mode = EDIT_MODE;
    lcd.blink();
    tmpHour = Hour;
    tmpMinute = Minute;
    tmpSecond = Second;
  }
  else
  {
    Mode = NORMAL_MODE;
    lcd.noBlink();
  }
}

void PosKeyTask(void)
{
  if (Mode == EDIT_MODE)
  {
    Xpos++;
    if (Xpos > 2)
    {
      Xpos = 0;
    }
    switch (Xpos)
    {
      case 0: lcd.setCursor(0, 1); break;
      case 1: lcd.setCursor(5, 1); break;
      case 2: lcd.setCursor(10, 1); break;
    }
  }
}

void EditKeyTask(void)
{
  if (Mode == EDIT_MODE)
  {
    switch (Xpos)
    {
      case 0:
        lcd.print("  ");
        lcd.setCursor(0, 1);
        tmpHour++;
        if (tmpHour > 23)
        {
          tmpHour = 0;
        }
        break;
      case 1:
        lcd.print("  ");
        lcd.setCursor(5, 1);
        tmpMinute++;
        if (tmpMinute > 59)
        {
          tmpMinute = 0;
        }
        break;
      case 2:
        lcd.print("  ");
        lcd.setCursor(10, 1);
        tmpSecond++;
        if (tmpSecond > 59)
        {
          tmpSecond = 0;
        }
        break;
    }
    ShowDummyTime();
  }
}

void OkKeyTask(void)
{
  if (Mode == EDIT_MODE)
  {
    rtc.adjust(DateTime(2023, 1, 1, tmpHour, tmpMinute, tmpSecond));
    Xpos = 0;
    lcd.setCursor(0, 1);
    Mode = NORMAL_MODE;
    lcd.noBlink();
  }
}
