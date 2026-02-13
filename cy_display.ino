/*
 * Created by ArduinoGetStarted.com
 *
 * This example code is in the public domain
 *
 * Tutorial page: https://arduinogetstarted.com/tutorials/arduino-led-matrix
 */

#include <IRremote.h>
#include <LiquidCrystal.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include "MD_MAX72xx_custom_font.h"

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define CS_PIN 3

#define LCD_RS_PIN A5
#define LCD_E_PIN A4
#define LCD_D4_PIN 6
#define LCD_D5_PIN 7
#define LCD_D6_PIN 8
#define LCD_D7_PIN 9

#define IR_RECEIVE_PIN 5

#define BUZZER_PIN 2

#define IR_BUTTON_0 22
#define IR_BUTTON_1 12
#define IR_BUTTON_2 24
#define IR_BUTTON_3 94
#define IR_BUTTON_4 8
#define IR_BUTTON_5 28
#define IR_BUTTON_6 90
#define IR_BUTTON_7 66
#define IR_BUTTON_8 82
#define IR_BUTTON_9 74
#define IR_BUTTON_FUNC_PLAY_PAUSE 64
#define IR_BUTTON_FUNC_STOP 71
#define IR_BUTTON_FUNC_RESET 69

MD_Parola ledMatrix = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

LiquidCrystal lcd(LCD_RS_PIN, LCD_E_PIN, LCD_D4_PIN,
                  LCD_D5_PIN, LCD_D6_PIN, LCD_D7_PIN);

unsigned long lastTimeLEDMatrixUpdated = millis();
unsigned long debounceDelayLEDMatrix = 1000;

// Modes:
// 0 -> Menu
// 1 -> Countdown config
// 2 -> Countdown
int mode = 0;

// Settings
int settingTimerMinutes = 5;
int settingTimerSeconds = 0;

bool isSettingUp = false;
bool isPlaying = false;
int timerMinutes = settingTimerMinutes;
int timerSeconds = settingTimerSeconds;
String newMinutesBuffer;


void printOnLCD(String text, int cursorLine) {
  for (int i = text.length(); i < 16; i++ ) {
    text += " ";
  }
  // set cursor line O or 1
  lcd.setCursor(0, cursorLine);
  // print text
  lcd.print(text);
  if (cursorLine == 0) {
    cursorLine = 1;
  } else {
    cursorLine = 0;
  }
}

void resetTimer() {
  // printOnLCD("Reset", 0);
  isPlaying = false;
  timerMinutes = settingTimerMinutes;
  timerSeconds = settingTimerSeconds;
}

void enterNewTime(int newMinutes) {
  String newMinutesStr = String(newMinutes);
  
  // Update settings buffer
  if (newMinutesBuffer.length() <= 2) {
    newMinutesBuffer = newMinutesBuffer + newMinutesStr;
  } else {
    newMinutesBuffer = newMinutesStr;
  }
  printOnLCD(newMinutesBuffer, 1);

  // Update actual settings
  int newMinutesBufferSize = newMinutesBuffer.length();
  if (newMinutesBufferSize == 2) {
    settingTimerMinutes = newMinutesBuffer.toInt();

    // Reset
    isSettingUp = false;
    resetTimer();
    mode = 2;
  }
}

void handleIRCommand(long command) {
  String commandStr = String(command);
  Serial.println("mode");
  Serial.println(mode);
  Serial.println(command);

  if (command == IR_BUTTON_FUNC_RESET) {
    resetTimer();
  } else if (mode == 1 && isSettingUp) {
    Serial.println("SETUP");
    switch (command) {
      case IR_BUTTON_0:
        enterNewTime(0);
        break;
      case IR_BUTTON_1:
        enterNewTime(1);
        break;
      case IR_BUTTON_2:
        enterNewTime(2);
        break;
      case IR_BUTTON_3:
        enterNewTime(3);
        break;
      case IR_BUTTON_4:
        enterNewTime(4);
        break;
      case IR_BUTTON_5:
        enterNewTime(5);
        break;
      case IR_BUTTON_6:
        enterNewTime(6);
        break;
      case IR_BUTTON_7:
        enterNewTime(7);
        break;
      case IR_BUTTON_8:
        enterNewTime(8);
        break;
      case IR_BUTTON_9:
        enterNewTime(9);
        break;
      case IR_BUTTON_FUNC_STOP:
        isSettingUp = false;
        mode = 2;
        break;
    }

  } else {
    switch (command) {
      case IR_BUTTON_0:
        mode = 0;
        break;
      case IR_BUTTON_1:
        mode = 1;
        break;
      case IR_BUTTON_2:
        mode = 2;
        break;
      case IR_BUTTON_FUNC_PLAY_PAUSE:
        if (mode == 1) {
          newMinutesBuffer = "";
          isSettingUp = true;
        } else {
          // printOnLCD("Countdown PLAY", 0);
          tone(BUZZER_PIN, 1000, 300);
          isPlaying = true;
        }
        break;
      case IR_BUTTON_FUNC_STOP:
        // printOnLCD("Countdown PAUSE", 0);
        isPlaying = false;
        break;
      case IR_BUTTON_FUNC_RESET:
        resetTimer();
        break;
      default:
        printOnLCD(commandStr, 1);
    }
  }
}

// Mode 0: Menu
void menu() {
  printOnLCD("CY_BORG menu", 0);
  printOnLCD("Modes: 0 1 2", 1);
}

// Mode 2: Countdown settings
void countdownSettings() {
  printOnLCD("Timer settings", 0);
  printOnLCD("Press play", 1);

  if (isSettingUp) {
    printOnLCD("Enter time:", 0);
    printOnLCD(newMinutesBuffer, 1);

    // The commands are handled in handleIRCommand
  }
}

// Mode 2: Countdown
void countdown() {
  // Pad timer units
  String minutesStr = String(timerMinutes);
  String secondsStr = String(timerSeconds);

  if (timerMinutes < 10) {
    minutesStr = "0" + minutesStr;
  }
  if (timerSeconds < 10) {
    secondsStr = "0" + secondsStr;
  }
  String timerText = minutesStr + ":" + secondsStr;
  
  // Display
  ledMatrix.print(timerText);
  printOnLCD("Countdown:", 0);
  printOnLCD(timerText, 1);

  // Countdown
  if (isPlaying == true) {
    // WARNING!
    if (timerMinutes == 0 && timerSeconds <= 5) {
      tone(BUZZER_PIN, 1000, 300);
    }

    if (timerSeconds == 0 && timerMinutes == 0) {
      // Stop
      isPlaying = false;
      tone(BUZZER_PIN, 1000, 2000);
      // mode = 0;
    } else if (timerSeconds == 0 && timerMinutes > 0) {
      timerMinutes--;
      timerSeconds = 59;
    } else {
      timerSeconds--;
    }
  }
}

void setup() {
  Serial.begin(115200);

  // Reset mode
  mode = 0;

  pinMode(BUZZER_PIN, OUTPUT);

  IrReceiver.begin(IR_RECEIVE_PIN);
  
  lcd.begin(16, 2);
  
  ledMatrix.begin();
  ledMatrix.setFont(monospace);
  ledMatrix.setIntensity(0);
  ledMatrix.displayClear();

  ledMatrix.setTextAlignment(PA_CENTER);
  // ledMatrix.print("Loading");
  // delay(1000);
  ledMatrix.print("CY");

  // tone(BUZZER_PIN, 1000, 1000);
}

void loop() {
  unsigned long timeNow = millis();

  // ledMatrix.setTextAlignment(PA_LEFT);
  // ledMatrix.print("Left"); // display text

  // ledMatrix.setTextAlignment(PA_CENTER);
  // ledMatrix.print("Center"); // display text

  // ledMatrix.setTextAlignment(PA_RIGHT);
  // ledMatrix.print("Right"); // display text

  // ledMatrix.setTextAlignment(PA_CENTER);
  // ledMatrix.setInvert(true);
  // ledMatrix.print("Invert"); // display text inverted

  // ledMatrix.setInvert(false);
  // ledMatrix.print(1234); // display number


  if (timeNow - lastTimeLEDMatrixUpdated > debounceDelayLEDMatrix) {
    lastTimeLEDMatrixUpdated = timeNow;
    if (IrReceiver.decode()) {
      IrReceiver.resume();
      int command = IrReceiver.decodedIRData.command;
      handleIRCommand(command);
    }
  
    switch (mode) {
      case 1:
        countdownSettings();
        break;
      case 2:
        countdown();
        break;
      case 0:
      default:
        menu();
    }
  }
}
