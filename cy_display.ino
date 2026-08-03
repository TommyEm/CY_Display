/*
 * Countdown app for Arduino Uno
 */

#define IR_USE_AVR_TIMER1 // Change to Timer1 for IR, because passive buzzer already uses Timer2
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
#define IR_BUTTON_FUNC_PLAY 64
#define IR_BUTTON_FUNC_STOP 71
#define IR_BUTTON_FUNC_RESET 69

MD_Parola ledMatrix = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

LiquidCrystal lcd(LCD_RS_PIN, LCD_E_PIN, LCD_D4_PIN,
                  LCD_D5_PIN, LCD_D6_PIN, LCD_D7_PIN);

unsigned long lastSecondTick = millis();

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
String newSecondsBuffer;


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

void enterNewTime(int newDigit) {
  String newDigitStr = String(newDigit);
  
  if (newMinutesBuffer.length() < 2) {
    // Fill the minutes buffer
    newMinutesBuffer = newMinutesBuffer + newDigitStr;
    printOnLCD(newMinutesBuffer, 1);
  } else if (newMinutesBuffer.length() == 2 && newSecondsBuffer.length() < 2) {
    // After minutes, fill the seconds buffer
    if ((newSecondsBuffer.length() == 0 && newDigit < 6) || newSecondsBuffer.length() == 1) {
      // Accept only up to 5 for decimal
      newSecondsBuffer = newSecondsBuffer + newDigitStr;
    }
    printOnLCD(newMinutesBuffer + ":" + newSecondsBuffer, 1);
  }

  // Update actual settings
  if (newMinutesBuffer.length() == 2 && newSecondsBuffer.length() == 2) {
    settingTimerMinutes = newMinutesBuffer.toInt();
    settingTimerSeconds = newSecondsBuffer.toInt();

    // Reset
    isSettingUp = false;
    resetTimer();
    mode = 2;
  }
}

void handleIRCommand(long command) {
  String commandStr = String(command);
  Serial.println("mode: " + String(mode));
  Serial.println("command: " + String(command));

  if (mode == 1 && isSettingUp) {
    Serial.println("SETUP");
    settingTimerSeconds = 0;
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
      case IR_BUTTON_FUNC_RESET:
        resetTimer();
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
      case IR_BUTTON_FUNC_PLAY:
        if (mode == 1) {
          newMinutesBuffer = "";
          newSecondsBuffer = "";
          isSettingUp = true;
        } else if (mode == 2) {
          isPlaying = !isPlaying;
          tone(BUZZER_PIN, 1000, 300);
        }
        break;
      case IR_BUTTON_FUNC_STOP:
        isPlaying = false;
        tone(BUZZER_PIN, 1000, 300);
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
  printOnLCD("CY menu", 0);
  printOnLCD("Modes: 0 1 2", 1);
}

// Mode 2: Countdown settings
void settings() {
  if (isSettingUp) {
    printOnLCD("Enter time:", 0);
    if (newMinutesBuffer.length() == 0 && newSecondsBuffer.length() == 0) {
      // Init LCD but only when input hasn't started, otherwise interference can occur
      printOnLCD(newMinutesBuffer, 1);
    }
  } else {
    printOnLCD("Timer settings", 0);
    printOnLCD("Press play", 1);
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
}

void updateCountdown() {
  // WARNING!
  if (timerMinutes == 0 && timerSeconds <= 5) {
    tone(BUZZER_PIN, 1000, 300);
  }

  if (timerSeconds == 0 && timerMinutes == 0) {
    // Stop
    isPlaying = false;
    tone(BUZZER_PIN, 1000, 2000);
    return;
  } else if (timerSeconds == 0 && timerMinutes > 0) {
    timerMinutes--;
    timerSeconds = 59;
  } else {
    timerSeconds--;
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
  ledMatrix.print("> CY <");
}

void loop() {
  if (IrReceiver.decode()) {
    // Ignore repeat frames
    bool isRepeat = IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT;

    int command = IrReceiver.decodedIRData.command;

    if (!isRepeat) {
      handleIRCommand(command);
    }

    IrReceiver.resume();
  }

  unsigned long timeNow = millis();

  if (isPlaying && timeNow - lastSecondTick >= 1000) {
    lastSecondTick = timeNow;
    updateCountdown();
  }
  
  switch (mode) {
    case 0:
      menu();
      break;
    case 1:
      settings();
      break;
    case 2:
      countdown();
      break;
  }
}
