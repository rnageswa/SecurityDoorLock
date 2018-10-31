
//*******************************************************************************************************//
//*************************************** NO RFID, NO USER ID + FIXES ***********************************//
//*******************************************************************************************************//

//****IMPORTING LIBRARIES****//
#include "SoftwareSerial.h"
#include "FPS_GT511C3.h"
#include <EEPROM.h>
#include <LiquidCrystal.h>
#include<Keypad.h>

//****KEYPAD MAPPING****//
char keymap[4][4] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[4] = {53, 51, 49, 47};
byte colpins[4] = {45, 43, 41, 39};

//****INITIALIZING DEVICES****//
FPS_GT511C3 fps(11, 10); //(TX,RX)
Keypad myKeypad = Keypad(makeKeymap(keymap), rowPins, colpins, 4, 4);
LiquidCrystal lcd(25, 27, 29, 31, 33, 35); //LCD Pins: (4,6,11,12,13,14)

//****VARIABLES USED****//
boolean lookingForFinger = true;
boolean locked = true; //The door remains locked at first
byte fingerprint = 21;
char a = ' '; //This variable takes a char from keypad and check it it is '*' then asks for RFID
char userPinSaved [] = {'1', '1', '1', '1', '1'}; //sample user's saved PIN to test
int fpId = -1; //The main fingerprint id which recognizez each user individually and checks the rest of auhtnetication with respect to it.
int addFpId = -1; //The fingerprint of user who needs to be enrolled (For admin access only)
int position = 0;
int sw_state = 0;
int sw = 2;
int white_led = 3;
int blue_led = 4;
int yellow_led = 5;
int green_led = 6;
int red_led = 7;
int buzzer = 8;
int lock = 9;

void setup() {
  pinMode(white_led, OUTPUT);
  pinMode(blue_led, OUTPUT);
  pinMode(yellow_led, OUTPUT);
  pinMode(green_led, OUTPUT);
  pinMode(red_led, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(lock, OUTPUT);
  pinMode(sw, INPUT);
  Serial.begin(9600);
  attachInterrupt(0, opendoor, HIGH);
  fps.Open();
  fps.SetLED(true);
  lcd.begin(20, 4);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("      WELCOME!      ");
  lcd.setCursor(0, 1);
  lcd.print(" PLEASE PRESS * KEY ");
  lcd.setCursor(0, 2);
  lcd.print(" TO INITIATE SYSTEM ");
  digitalWrite(lock, LOW);
  digitalWrite(yellow_led, HIGH);
  digitalWrite(white_led, HIGH);
  digitalWrite(blue_led, LOW);
  digitalWrite(green_led, LOW);
  digitalWrite(red_led, LOW);
}


//*********************RESET ARDUINO********************//
void(* resetFunc) (void) = 0;
//******************************************************//

//***************************************************************//
//*******************MAIN CODE***********************************//
//***************************************************************//

void loop() {
  char sig;
  if (locked) {
    lcd.setCursor(0, 3);
    lcd.print ("               ");
    lcd.setCursor(0, 3);
    lcd.print("STATUS: LOCKED");
    digitalWrite(yellow_led, HIGH);
    sig = myKeypad.waitForKey(); //Wait for a start signal
    if (sig == '*') {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(" PLACE A REGISTERED ");
      lcd.setCursor(0, 1);
      lcd.print("   FINGER ON THE    ");
      lcd.setCursor(0, 2);
      lcd.print(" SCANNER TO PROCEED ");
      delay(3000);
      // if a finger is on the sensor
      while (lookingForFinger) {     //run until a finger is pressed
        if (fps.IsPressFinger()) {   //Remove 'true' and add 'fps.IsPressFinger()' when fingerprint is attached
          //capture the finger print
          fps.CaptureFinger(false); //Remove the comment when fingerprint module is attached
          //get the id
          int id = fps.Identify1_N();//Remove '0' and add fps.Identify1_N() whe fingerpprint is attached
          //maximum finger print stored in 200.
          //Id > 20 is a not recognized one
          if (id < 20) {
            //finger print recognized: display the id
            lcd.setCursor(0, 0);
            lcd.print("FINGERPRINT VERIFIED");
            lcd.setCursor(0, 1);
            lcd.print("                    ");
            lcd.setCursor(0, 2);
            lcd.print("                    ");
            delay(500);
            fpId = id;
            if (fpId >= 0 && fpId <= 4) {
              if (fpId == 0) {
                lcd.setCursor(0, 1);
                lcd.print("ADMIN");
              }
              if (fpId == 1) {
                lcd.setCursor(0, 1);
                lcd.print("USER 1");
              }
              if (fpId == 2) {
                lcd.setCursor(0, 1);
                lcd.print("USER 2");
              }
              if (fpId == 3) {
                lcd.setCursor(0, 1);
                lcd.print("USER 3");
              }
              if (fpId == 4) {
                lcd.setCursor(0, 1);
                lcd.print("USER 4");
              }
            }
            else {
              lcd.setCursor(0, 1);
              lcd.print("GUEST USER:");
              lcd.print(fpId);
            }
            delay(2000);
            //Check for id and password starts here
            int counterForPass = 0;
            while (counterForPass < 3) {
              lcd.setCursor(0, 0);
              lcd.print("                    ");
              lcd.setCursor(0, 1);
              lcd.print("                    ");
              lcd.setCursor(0, 2);
              lcd.print("                    ");
              if (fpId >= 0 && fpId <= 4) {
                if (fpId == 0) {
                  lcd.setCursor(0, 0);
                  lcd.print("HELLO ADMIN !");
                }
                if (fpId == 1) {
                  lcd.setCursor(0, 0);
                  lcd.print("HELLO USER 1 !");
                }
                if (fpId == 2) {
                  lcd.setCursor(0, 0);
                  lcd.print("HELLO USER 2 !");
                }
                if (fpId == 3) {
                  lcd.setCursor(0, 0);
                  lcd.print("HELLO USER 3 !");
                }
                if (fpId == 4) {
                  lcd.setCursor(0, 0);
                  lcd.print("HELLO USER 4 !");
                }
              }
              else {
                lcd.setCursor(0, 0);
                lcd.print("HELLO GUEST USER ");
                lcd.print(fpId);
              }
              delay(500);
              lcd.setCursor(0, 1);
              lcd.print("ENTER YOUR PASSWORD:");
              char userPin [5];
              for (byte c = 0; c < 5; c++) {
                userPin[c] = myKeypad.waitForKey();
                lcd.setCursor(c, 2);
                lcd.print(userPin[c]);
              }
              delay(2000);
              lcd.setCursor(0, 0);
              lcd.print("                    ");
              lcd.setCursor(0, 1);
              lcd.print("                    ");
              lcd.setCursor(0, 2);
              lcd.print("                    ");
              int pinRec = checkUserPin(userPin);
              if (pinRec != -1) {
                delay(100);
                counterForPass = 5;
                lcd.setCursor(0, 0);
                lcd.print("ACCESS GRANTED");
                delay(2000);
                locked = false;

                // *********************Door Open**************************
                digitalWrite(lock, HIGH);
                digitalWrite(green_led, HIGH);
                digitalWrite(yellow_led, LOW);
                lcd.setCursor(0, 3);
                lcd.print("STATUS: UNLOCKED");
                delay(2000);
                //*********************************************************
                if (fpId == 0) {
                  delay(100);
                  lcd.setCursor(0, 0);
                  lcd.print("                    ");
                  lcd.setCursor(0, 1);
                  lcd.print("                    ");
                  lcd.setCursor(0, 2);
                  lcd.print("                    ");
                  lcd.setCursor(0, 0);
                  lcd.print("ADMIN ACCESS");
                  delay(500);
                  lcd.setCursor(0, 1);
                  lcd.print("ACTIVATED");
                  digitalWrite(blue_led, HIGH);
                  delay(2000);
                  adminAccess();
                }
              }
              else {
                lcd.setCursor(0, 1);
                lcd.print("                    ");
                lcd.setCursor(0, 2);
                lcd.print("                    ");
                lcd.setCursor(0, 1);
                lcd.print("INVALID PASSWORD");
                lcd.setCursor(0, 2);
                lcd.print("PLEASE TRY AGAIN");
                digitalWrite(red_led, HIGH);
                digitalWrite(buzzer, HIGH);
                delay(2000);
                digitalWrite(red_led, LOW);
                digitalWrite(buzzer, LOW);
                lcd.clear();
                if (counterForPass < 3) {
                  counterForPass++;
                }
                else {
                  lookingForFinger = false;
                  break;
                }
                if (counterForPass == 3) {
                  for (int alarm = 0; alarm <= 5; alarm++) {
                    lcd.setCursor(0, 0);
                    lcd.print("                    ");
                    lcd.setCursor(0, 1);
                    lcd.print("                    ");
                    lcd.setCursor(0, 2);
                    lcd.print("                    ");
                    lcd.setCursor(0, 0);
                    lcd.print("SUSPICIOUS ACTIVITY");
                    lcd.setCursor(0, 1);
                    lcd.print("      DETECTED     ");
                    delay(500);
                    digitalWrite(red_led, HIGH);
                    digitalWrite(buzzer, HIGH);
                    delay(500);
                    digitalWrite(red_led, LOW);
                    digitalWrite(buzzer, LOW);
                  }
                }
              }
            }
            break; //break the loop if finger is pressed
          }

          else {
            //finger print not recognized
            lcd.setCursor(0, 0);
            lcd.print("                    ");
            lcd.setCursor(0, 1);
            lcd.print("                    ");
            lcd.setCursor(0, 2);
            lcd.print("                    ");
            lcd.setCursor(0, 0);
            lcd.print("FINGERPRINT NOT");
            lcd.setCursor(0, 1);
            lcd.print("RECOGNIZED");
            digitalWrite(red_led, HIGH);
            digitalWrite(buzzer, HIGH);
            delay(2000);
            digitalWrite(red_led, LOW);
            digitalWrite(buzzer, LOW);
            lcd.clear();
            // ...
            // add you code here for the condition access disallowed

            // ...
            break;
          }
        }
      }
      //***************Automatic Door Close*****************
      if (locked == false) {
        for (int timer = 6; timer >= 0; timer--) {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("GOING TO SLEEP IN: ");
          lcd.setCursor(0, 1);
          lcd.print(timer);
          lcd.print("   SECONDS");
          delay(1000);
        }
        locked = true;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("TIMEOUT");
        delay(1000);
        digitalWrite(lock, LOW);
        digitalWrite(yellow_led, HIGH);
        digitalWrite(white_led, HIGH);
        digitalWrite(blue_led, LOW);
        digitalWrite(green_led, LOW);
        digitalWrite(red_led, LOW);
        lcd.setCursor(0, 0);
        lcd.print("                    ");
        lcd.setCursor(0, 1);
        lcd.print("                    ");
        lcd.setCursor(0, 2);
        lcd.print("                    ");
        lcd.setCursor(0, 3);
        lcd.print("                    ");
        lcd.setCursor(0, 3);
        lcd.print("STATUS: LOCKED");
        lcd.setCursor(6, 0);
        lcd.print("WELCOME!");
        lcd.setCursor(0, 1);
        lcd.print(" PLEASE PRESS * KEY ");
        lcd.setCursor(0, 2);
        lcd.print(" TO INITIATE SYSTEM ");
        delay(500);
      }

      //****************************************************
      else {
        delay(100);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("GOING TO SLEEP");
        delay(2000);
        digitalWrite(yellow_led, HIGH);
        digitalWrite(white_led, HIGH);
        digitalWrite(blue_led, LOW);
        digitalWrite(green_led, LOW);
        digitalWrite(red_led, LOW);
        lcd.setCursor(0, 0);
        lcd.print("                    ");
        lcd.setCursor(6, 0);
        lcd.print("WELCOME!");
        lcd.setCursor(0, 1);
        lcd.print(" PLEASE PRESS * KEY ");
        lcd.setCursor(0, 2);
        lcd.print(" TO INITIATE SYSTEM ");
        delay(500);
        if (locked == true) {
          lcd.setCursor(0, 3);
          lcd.print("                    ");
          lcd.setCursor(0, 3);
          lcd.print("STATUS: LOCKED");
          digitalWrite(lock, LOW);
          digitalWrite(yellow_led, HIGH);
          digitalWrite(white_led, HIGH);
          digitalWrite(blue_led, LOW);
          digitalWrite(green_led, LOW);
          digitalWrite(red_led, LOW);
        }
      }
      fps.Close(); //Remove the comment when fingerprint module is attached
    }
  }
}

//******************************************************//
//******************************************************//
//********************METHODS***************************//
//******************************************************//
//******************************************************//

//********************OPEN DOOR WITH SWITCH*************************//
void opendoor() {
  sw_state = digitalRead(sw);
  if (sw_state == HIGH) {
    digitalWrite(lock, HIGH);
    digitalWrite(green_led, HIGH);
    digitalWrite(yellow_led, LOW);
    lcd.setCursor(0, 3);
    lcd.print ("                 ");
    lcd.setCursor(0, 3);
    lcd.print("STATUS: UNLOCKED");
    locked = false;
  }
  else {
    digitalWrite(lock, LOW);
    digitalWrite(green_led, LOW);
    digitalWrite(yellow_led, HIGH);
    lcd.setCursor(0, 3);
    lcd.print ("                 ");
    lcd.setCursor(0, 3);
    lcd.print("STATUS: LOCKED");
    locked = true;
  }
}

//**********************CLEAR LINE************************//

void clearLine( int col, int row) {
  for (int c = 0; c < 20; c++) {
    lcd.setCursor(col, row);
    lcd.write(" ");
    col++;
    row++;
  }
}

//***********************PRINT USER***********************//

void printUser() {
  if (fpId >= 0 && fpId <= 4) {
    if (fpId == 0) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("ADMIN MODE");
      lcd.setCursor(0, 1);
      lcd.print("SHOWING USER'S LIST");
      lcd.setCursor(0, 2);
      lcd.print("ID 0: ADMIN");
      delay(500);
    }
    if (fpId == 1) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("ADMIN MODE");
      lcd.setCursor(0, 1);
      lcd.print("SHOWING USER'S LIST");
      lcd.setCursor(0, 2);
      lcd.print("ID 1: USER 1");
      delay(500);
    }
    if (fpId == 2) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("ADMIN MODE");
      lcd.setCursor(0, 1);
      lcd.print("SHOWING USER'S LIST");
      lcd.setCursor(0, 2);
      lcd.print("ID 2: USER 2");
      delay(500);
    }
    if (fpId == 3) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("ADMIN MODE");
      lcd.setCursor(0, 1);
      lcd.print("SHOWING USER'S LIST");
      lcd.setCursor(0, 2);
      lcd.print("ID 3: USER 3");
      delay(500);
    }
    if (fpId == 4) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("ADMIN MODE");
      lcd.setCursor(0, 1);
      lcd.print("SHOWING USER'S LIST");
      lcd.setCursor(0, 2);
      lcd.print("ID 4: USER 4");
      delay(500);
    }
  }
  else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ADMIN MODE");
    lcd.setCursor(0, 1);
    lcd.print("SHOWING USER'S LIST");
    lcd.setCursor(0, 2);
    lcd.print("GUEST USER: ID ");
    lcd.print(fpId);
    delay(500);
  }
}

//************************PRINT USER PIN*******************//

void printUserPin() {
  int mem;
  mem = 5;
  int count = (fpId * 11) + 5;
  for (int c = count; c < count + 5; c++) {
    lcd.setCursor(mem, 3);
    lcd.print((char)EEPROM.read(c));
    lcd.setCursor(mem + 1, 3);
    lcd.print(" ");
    delay(500);
    mem++;
  }
  delay(2000);
}

//*******************SHOW USER DATA**************************//

void showUserData() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ADMIN MODE");
  lcd.setCursor(0, 1);
  lcd.print("SHOWING USER'S LIST");
  delay(2000);
  while ((int)EEPROM.read(addFpId * 11) != 0) {
    printUser();
    delay(500);
    lcd.setCursor(0, 3);
    lcd.print("PIN: ");
    delay(500);
    printUserPin();
    clearLine(0, 3);
    fpId++;
    delay(2000);
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(fpId);
  lcd.setCursor(0, 1);
  lcd.print((int)EEPROM.read(addFpId * 11));
  delay(3000);
  lcd.setCursor(0, 0);
  lcd.print("ADMIN MODE");
  lcd.setCursor(0, 1);
  lcd.print("SHOWING USER'S LIST");
  lcd.setCursor(0, 2);
  lcd.print("COMPLETE");
}

//******************CHECK USER PIN*************************//

int checkUserPin(char userPin []) {
  int count = 0;
  int initiate;
  if (fpId >= 0) {
    initiate = (fpId * 11) + 5;
  }
  else return -1;
  char result[5];
  for (byte i = initiate; i < initiate + 5; i++) {
    char readValue;
    readValue = (char)EEPROM.read(i);
    if ((readValue > 47 && readValue < 58) || (readValue > 64 && readValue < 69)) {
      result[count] = readValue;
    }
    count++;
  }
  count = 0;
  for (int c = 0; c < 5; c++) {
    if (userPin[c] == result[c]) {
      count++;
    }
  }
  if (count == 5) return fpId;
  else return -1; //Make false when pin is found
}

//***********************ENROLL**************************//

bool enroll() {

  void(* resetFunc) (void) = 0;
  // get the first available id for new finger print
  int enrollid = 0;
  bool usedid = true;
  while (usedid == true)
  {
    usedid = fps.CheckEnrolled(enrollid);
    if (usedid == true) enrollid++;
  }
  //enrollment start here with the first free id
  fps.EnrollStart(enrollid);
  lcd.setCursor(0, 1);
  lcd.print("     ENROLLMENT     ");
  lcd.setCursor(0, 2);
  lcd.print("                    ");
  lcd.setCursor(0, 3);
  lcd.print("                    ");
  lcd.setCursor(0, 2);
  lcd.print("  PRESS ANY KEY TO  ");
  delay(200);
  lcd.setCursor(0, 3);
  lcd.print("CONTINUE & # TO EXIT");
  char sig = myKeypad.waitForKey();
  if (sig == '#') {
    for (int r = 0; r < 10; r++) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("RESETTING...");
      digitalWrite(red_led, HIGH);
      delay(200);
      digitalWrite(red_led, LOW);
      delay(200);
    }
    resetFunc();
  }
  else {
    lcd.setCursor(0, 2);
    lcd.print("                    ");
    lcd.setCursor(0, 3);
    lcd.print("                    ");
    lcd.setCursor(0, 2);
    lcd.print("PLACE YOUR FINGER");
    delay(200);
    lcd.setCursor(0, 3);
    lcd.print("ON THE SCANNER");
    delay(1000);
    // ***** FIRST MEASURE *****
    // wait for finger press
    while (fps.IsPressFinger() == false) delay(100);
    bool bret = fps.CaptureFinger(true);
    int iret = 0;
    if (bret != false)
    {
      //has a finger print captured
      delay(2000);
      lcd.setCursor(0, 2);
      lcd.print("                    ");
      lcd.setCursor(0, 3);
      lcd.print("                    ");
      lcd.setCursor(0, 2);
      lcd.print("REMOVE FINGER"); //Serial.println("Remove finger");
      delay(2000);
      // Enroll step 1
      fps.Enroll1();
      //wait for finger removed
      while (fps.IsPressFinger() == true) delay(100);
      // ***** SECOND MEASURE *****
      // Now we need to check the finger print
      // wait for finger press
      lcd.setCursor(0, 2);
      lcd.print("                    ");
      lcd.setCursor(0, 3);
      lcd.print("                    ");
      lcd.setCursor(0, 2);
      lcd.print("PLACE YOUR FINGER");
      delay(200);
      lcd.setCursor(0, 3);
      lcd.print("ON THE SCANNER");
      delay(1000);
      while (fps.IsPressFinger() == false) delay(100);
      bret = fps.CaptureFinger(true);
      if (bret != false)
      {
        delay(2000);
        lcd.setCursor(0, 2);
        lcd.print("                    ");
        lcd.setCursor(0, 3);
        lcd.print("                    ");
        lcd.setCursor(0, 2);
        lcd.print("REMOVE FINGER"); //Serial.println("Remove finger");
        //enroll step 2
        fps.Enroll2();
        //wait for finger removed
        while (fps.IsPressFinger() == true) delay(100);
        // ***** THIRD MEASURE *****
        //Check Again the finger print
        delay(2000);
        lcd.setCursor(0, 2);
        lcd.print("                    ");
        lcd.setCursor(0, 3);
        lcd.print("                    ");
        lcd.setCursor(0, 2);
        lcd.print("PLACE YOUR FINGER");
        delay(200);
        lcd.setCursor(0, 3);
        lcd.print("ON THE SCANNER");
        while (fps.IsPressFinger() == false) delay(100);
        bret = fps.CaptureFinger(true);
        if (bret != false)
        {
          delay(2000);
          lcd.setCursor(0, 2);
          lcd.print("                    ");
          lcd.setCursor(0, 3);
          lcd.print("                    ");
          lcd.setCursor(0, 2);
          lcd.print("REMOVE FINGER"); //Serial.println("Remove finger");
          iret = fps.Enroll3();
          if (iret == 0)
          {
            //*** SUCCESS third measure are the same -> NOW finger print is stored
            //capture the finger print
            delay(1000);
            if (exist())  return true;
            else return false;
          }
          else
          {
            //*** FAIL For some reason -> NOTHING STORED
            lcd.setCursor(0, 2);
            lcd.print("                    ");
            lcd.setCursor(0, 3);
            lcd.print("                    ");
            lcd.setCursor(0, 2);
            lcd.print("ENROLL FAILED");
            delay(2000);
          }
        }
        else {
          lcd.setCursor(0, 3);
          lcd.print("                    ");
          lcd.setCursor(0, 2);
          lcd.print("                    ");
          lcd.setCursor(0, 2);
          lcd.print("CAPTURE 3 FAILED");
          delay(2000);
          enroll();
        }
      }
      else {
        lcd.setCursor(0, 3);
        lcd.print("                    ");
        lcd.setCursor(0, 2);
        lcd.print("                    ");
        lcd.setCursor(0, 2);
        lcd.print("CAPTURE 2 FAILED");
        delay(2000);
        enroll();
      }
    }
    else {
      lcd.setCursor(0, 3);
      lcd.print("                    ");
      lcd.setCursor(0, 2);
      lcd.print("                    ");
      lcd.setCursor(0, 2);
      lcd.print("CAPTURE 1 FAILED");
      delay(2000);
      enroll();
    }
    return false;
  }
}

//*******************WRITE EPROM************************//

void writeEp(char userPin[]) {
  int count = 0;
  for (byte c = (addFpId * 11) + 5; c < (addFpId * 11) + 10; c++) {
    EEPROM.write(c, userPin[count]);
    count++;
  }
  lcd.setCursor(0, 2);
  lcd.print("                    ");
  lcd.setCursor(0, 2);
  lcd.print("USER PIN SAVED");
  delay(2000);
}


//*************************EXIST************************//

bool exist() {
  delay(1000);
  lcd.setCursor(0, 2);
  lcd.print("                    ");
  lcd.setCursor(0, 2);
  lcd.print("CONFIRM FINGER");
  if (fps.IsPressFinger()) {
    //capture the finger print
    fps.CaptureFinger(false);
    //get the id
    int id = fps.Identify1_N();
    //maximun finger print stored in 200.
    //Id > 200 is a not recognized one
    if (id < 20)
    {
      //finger print recognized: display the id
      addFpId = id;
      lcd.setCursor(0, 2);
      lcd.print("                    ");
      lcd.setCursor(0, 2);
      lcd.print("USER REGISTERED");
      delay(2000);
      lcd.setCursor(0, 3);
      lcd.print("USER ID: ");
      lcd.print(addFpId);
      delay(2000);
      return true;
    }
    else {
      delay(500);
      lcd.setCursor(0, 2);
      lcd.print("                    ");
      lcd.setCursor(0, 2);
      lcd.print("USER NOT REGISTERED");
      delay(2000);
    }
    return false;
  }
  else {
    exist();
  }
  delay(100);
}

//************************ADMIN ACCESS*******************//

void adminAccess() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ENTERING ADMIN MODE");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("CHOOSE ANY ONE :");
  delay(500);
  lcd.setCursor(0, 1);
  lcd.print("1.ADD USER");
  delay(500);
  lcd.setCursor(0, 2);
  lcd.print("2.SHOW USER'S LIST");
  delay(500);
  lcd.setCursor(0, 3);
  lcd.print("3.DELETE ALL ");
  delay(500);
  lcd.print("4.EXIT");
  byte option = myKeypad.waitForKey();
  option = option - 48;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("     ADMIN MODE     ");
  lcd.setCursor(0, 1);
  lcd.print("  OPTION: ");
  lcd.print(option);
  lcd.print(" CHOSEN  ");
  delay(2000);
  if (option > 0 && option < 5) {
    if (option == 3) {
      lcd.setCursor(0, 1);
      lcd.print("                    ");
      lcd.setCursor(0, 2);
      lcd.print("                    ");
      lcd.setCursor(0, 3);
      lcd.print("                    ");
      lcd.setCursor(0, 1);
      lcd.print("DELETING");
      lcd.setCursor(0, 2);
      lcd.print("FINGERPRITNS ...");
      fps.DeleteAll(); //Delete fingerprints
      delay(1000);
      lcd.setCursor(0, 1);
      lcd.print("                    ");
      lcd.setCursor(0, 2);
      lcd.print("                    ");
      lcd.setCursor(0, 1);
      lcd.print("DELETING");
      lcd.setCursor(0, 2);
      lcd.print("PASSWORDS ...");
      for (byte fingerprint = 1; fingerprint <= 10; fingerprint++) {
        //Deleting user pin and password
        for (int j = 0; j < 10; j++) {
          EEPROM.write((fingerprint * 11) + j, (char)0);
        }
      }
      lcd.setCursor(0, 1);
      lcd.print("                    ");
      lcd.setCursor(0, 2);
      lcd.print("                    ");
      lcd.setCursor(0, 3);
      lcd.print("                    ");
      lcd.setCursor(0, 1);
      lcd.print("ALL FINGERPRINTS AND");
      lcd.setCursor(0, 2);
      lcd.print(" PASSWORDS HAS BEEN ");
      lcd.setCursor(0, 3);
      lcd.print(" DELETED SUCCESFULLY");
      delay(2000);
      lcd.clear();
      enroll();
    }
    else if (option == 1) {
      lcd.setCursor(0, 1);
      lcd.print("                    ");
      lcd.setCursor(0, 2);
      lcd.print("                    ");
      lcd.setCursor(0, 3);
      lcd.print("                    ");
      lcd.setCursor(0, 1);
      lcd.print("      ADD USER      ");
      delay(2000);
      fps.Open();
      if (enroll() == true) {
        delay(100);
        lcd.setCursor(0, 2);
        lcd.print("                    ");
        lcd.setCursor(0, 3);
        lcd.print("                    ");
        lcd.setCursor(0, 2);
        lcd.print("ENTER PIN: ");
        char userPin [5];
        for (byte c = 0; c < 5; c++) {
          userPin[c] = myKeypad.waitForKey();
          lcd.setCursor(11 + c, 2);
          lcd.print(userPin[c]);
        }
        delay(2000);
        lcd.setCursor(0, 1);
        lcd.print("                    ");
        lcd.setCursor(0, 2);
        lcd.print("                    ");
        lcd.setCursor(0, 3);
        lcd.print("                    ");
        lcd.setCursor(0, 2);
        lcd.print("USER ID:");
        delay(500);
        lcd.print(addFpId);
        delay(2000);
        writeEp(userPin);
        lcd.setCursor(0, 1);
        lcd.print("                    ");
        lcd.setCursor(0, 2);
        lcd.print("                    ");
        lcd.setCursor(0, 2);
        lcd.print("REGISTRATION");
        lcd.setCursor(0, 3);
        lcd.print("SUCCESSFUL");
        delay(2000);
        adminAccess();
      }
      else {
        lcd.setCursor(0, 1);
        lcd.print("                    ");
        lcd.setCursor(0, 2);
        lcd.print("                    ");
        lcd.setCursor(0, 2);
        lcd.print("REGISTRATION FAILED");
        delay(2000);
        lcd.clear();
        adminAccess();
      }
    }
    else if (option == 2) {
      showUserData();
      delay(1000);
      adminAccess();
    }
    else if (option == 4) {
      return;
    }
  }
  else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("INVALID OPTION");
    delay(2000);
    lcd.clear();
    delay(100);
    adminAccess();
  }
}
//****************xxxxxxxxxx END xxxxxxxxxxxx******************//
