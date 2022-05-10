#include <Ds1302.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <SD.h>


//<-- this is the RTC setup -->//
Ds1302 RTC(A0, 2, 3);
//<-- screen setup END -->//



//<-- this is the screen setup -->//
LiquidCrystal_I2C lcd(0x27,16,2);
//<-- screen setup END -->//



//<-- this is the SD setup -->//
File myFile;
//<-- SD setup END -->//



//<-- this is the keypad setup -->//
const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
//define the cymbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  {'D', 'C', 'B', 'A'},
  {'#', '9', '6', '3'},
  {'0', '8', '5', '2'},
  {'*', '7', '4', '1'} // the behavior of this matrix is awfully random, but this works somehow so.
};
//char hexaKeys[ROWS][COLS] = {
//  {'C', '0', 'E', 'X'},
//  {'3', '9', '6', 'X'},
//  {'2', '8', '5', 'X'},
//  {'1', '7', '4', 'X'} // the behavior of this matrix is awfully random, but this works somehow so.
//};
byte rowPins[ROWS] = {5, 6, 7, 8}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {9, A3, A2, A1}; //connect to the column pinouts of the keypad
Keypad theKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, COLS, ROWS);
char enteredKey = 'A';
//<-- keypad setup END -->//




uint8_t alarmHours = 0, alarmMinutes = 0;  // Holds the current alarm time
uint8_t menu = 0;
uint8_t setAll = 0;



void setup() {
  lcd.init();                      // initialize the lcd 
  lcd.backlight();
  Serial.begin(9600);
  Serial.println("Initializing SD card ..");
  while(!Serial) {}
  pinMode(10, OUTPUT);
  if(!SD.begin(4)) {
    Serial.println("failed lol");
    return;
  }
  Serial.println("Success!");
  RTC.init();
  theKeypad.addEventListener(keypadEvent); // Add an event listener for this keypad
  menu = 0;
  uint8_t char_idx = 0;
  char alarm[4];
  myFile = SD.open("alarm.txt");
  if (myFile) {
    Serial.print("alarm.txt:");
    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      alarm[char_idx++] = myFile.read();
    }
    if(char_idx == 4) {
      alarmHours = parseDigits(alarm, 2);
      alarmMinutes = parseDigits(alarm + 2, 2);
    }
    Serial.println(alarm);
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening alarm.txt in reading");
  }
}



//  Ds1302::DateTime now;
//  RTC.getDateTime(&now);
  //----------------------------//

void keypadEvent(KeypadEvent key){
    switch (theKeypad.getState()){
      case PRESSED:
        if (key == 'A') {
          if(menu == 1) {
            menu = 0;
          } else {
            menu++;
          }
          lcd.clear();
          lcd.setCursor(5, 0);
          lcd.print("Mode ");
          lcd.print(menu + 1);
          delay(500);
          lcd.clear();       
        } else if (key == 'D') {
          setAll++;
        }
        break;

      case RELEASED:
        break;

      case HOLD:
        break;
    }
}


void loop()
{
  getKeypadInput();
// in which subroutine should we go?
  switch(menu) {
    case 0:
      DisplayDateTime(); // void DisplayDateTime
      Alarm(); // Alarm control
      break;
    case 1:
      SetAlarm();
      break;
  }
  delay(100);
}

bool getKeypadInput() {
  char tempKey = theKeypad.getKey();
  if (tempKey) {
    enteredKey = tempKey;
    return 1;
  }
  return 0;
}


void DisplayDateTime ()
{
// We show the current date and time
  Ds1302::DateTime now;
  RTC.getDateTime(&now);
  lcd.setCursor(0, 0);
  
  if (now.hour<=9){
    lcd.print("0");
  }
  lcd.print(now.hour, DEC);
  lcd.print(":");
  if (now.minute<=9){
    lcd.print("0");
  }
  lcd.print(now.minute, DEC);
  lcd.print(":");
  if (now.second<=9){
    lcd.print("0");
  }
  lcd.print(now.second, DEC);

  
  lcd.setCursor(10, 0);
  if (now.day<=9){
    lcd.print("0");
  }
  lcd.print(now.day, DEC);
  lcd.print("/");
  if (now.month<=9){
    lcd.print("0");
  }
  lcd.print(now.month, DEC);
}




void printAllOn(){
  lcd.setCursor(0,1);
  lcd.print("Alarm: ");
  
  if (alarmHours <= 9)
  {
    lcd.print("0");
  }
  lcd.print(alarmHours, DEC);
  
  lcd.print(":");
  if (alarmMinutes <= 9)
  {
    lcd.print("0");
  }
  lcd.print(alarmMinutes, DEC); 

}



void printAllOff() {
  lcd.setCursor(0, 1);
  lcd.print("Alarm: Off  ");  
}



void Alarm(){
  if (setAll==0){
    printAllOff();
  }

  if (setAll==1){
    printAllOn();
  
    Ds1302::DateTime now;
    RTC.getDateTime(&now);
    if ( now.hour == alarmHours && now.minute == alarmMinutes ){
      lcd.noBacklight();
      delay (300);
      lcd.backlight();
    }
  } 
  if (setAll==2){
    setAll=0;
  }
  delay(50);
}






uint8_t parseDigits(char* str, uint8_t count){
  uint8_t val = 0;
  while(count-- > 0) val = (val * 10) + (*str++ - '0');
  return val;
}
void SetAlarm() {
  uint8_t char_idx = 0;
  char buffer[4];
  while(true) {
    if (char_idx == 4){
      alarmHours = parseDigits(buffer, 2);
      alarmMinutes = parseDigits(buffer + 2, 2);
      if(SD.remove("alarm.txt")) {
        Serial.println("removed the file");
      } else {
        Serial.println("failed to remove the file");
      }
      myFile = SD.open("alarm.txt", FILE_WRITE);
      if (myFile) {
        Serial.print("Writing to alarm.txt...");
        myFile.print(alarmHours);
        myFile.print(alarmMinutes);
        // close the file:
        myFile.close();
        Serial.println("done.");
      } else {
        // if the file didn't open, print an error:
        Serial.println("error opening alarm.txt in writing");
      }
      
      lcd.clear();
      lcd.print(alarmHours);
      lcd.print(':');
      lcd.print(alarmMinutes);
      delay(600);
      lcd.clear();
      enteredKey = ' ';
      menu = 0;
      Serial.println("done setting the alarm");
      return;
    }
    if (getKeypadInput()){
      if(enteredKey == 'C' || enteredKey == 'B' ||
      enteredKey == '#' || enteredKey == '*') {
        lcd.clear();
        lcd.print("Enter a");
        lcd.setCursor(0, 1);
        lcd.print("Number");
        delay(600);
        lcd.clear();
      } else if( enteredKey == 'A' || enteredKey == 'D') {
        return;
      } else {
        lcd.clear();
        buffer[char_idx++] = enteredKey;
        if(char_idx < 1) {
          lcd.clear();
          lcd.print(buffer);
        } else if (char_idx < 2) {
          lcd.clear();
          lcd.print(buffer[0]);
          lcd.print(buffer[1]);
        } else if (char_idx < 3) {
          lcd.clear();
          lcd.print(buffer[0]);
          lcd.print(buffer[1]);
          lcd.print(':');
          lcd.print(buffer[2]);
        } else if (char_idx < 4) {
          lcd.clear();
          lcd.print(buffer[0]);
          lcd.print(buffer[1]);
          lcd.print(':');
          lcd.print(buffer[2]);
          lcd.print(buffer[3]);
        }
      }
    }
  }
}
