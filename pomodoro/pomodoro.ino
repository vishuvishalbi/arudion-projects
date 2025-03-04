#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <Buzzer.h>
// Display setup
U8G2_SSD1306_128X64_NONAME_1_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);

// Button pins - adjust as needed for your board
#define START_PAUSE_BUTTON 0  // ESP32 onboard "BOOT" button
#define RESET_BUTTON 14       // Change this to your reset button pin

// Timer constants
#define WORK_TIME 25 * 60    // 25 minutes in seconds
#define BREAK_TIME 5 * 60    // 5 minutes in seconds
Buzzer buzzer(14);
// State management
enum TimerState {
  IDLE,
  WORKING,
  PAUSED,
  BREAK,
  COMPLETED
};

// Global variables
TimerState currentState = IDLE;
unsigned long timmerStart = 0;
unsigned long pausedTime = 0;
unsigned long remainingTime = WORK_TIME;
bool breakStarted = false;
bool buttonPressed = false;
bool resetButtonPressed = false;
int animationFrame = 0;

// Debounce management
unsigned long lastButtonPress = 0;
unsigned long lastAnimationUpdate = 0;
unsigned long debounceDelay = 50;  // 50ms debounce time

void showIdleScreen();
void showWorkingScreen();
void showPausedScreen();
void showBreakScreen();
void showCompletedScreen();

void setup() {
  Serial.begin(115200);
  
  // Initialize the OLED display
  u8g2.begin();
  u8g2.setFont(u8g2_font_6x10_tf);
  
  // Setup button pins
  pinMode(START_PAUSE_BUTTON, INPUT_PULLUP);
  pinMode(RESET_BUTTON, INPUT_PULLUP);
  
  // Show initial screen
  showIdleScreen();
}

void loop() {
  // Read buttons
  readButtons();
  
  // Handle timer logic based on state
  switch (currentState) {
    case IDLE:
      handleIdleState();
      break;
    case WORKING:
      handleWorkingState();
      break;
    case PAUSED:
      handlePausedState();
      break;
    case BREAK:
      handleBreakState();
      break;
    case COMPLETED:
      handleCompletedState();
      break;
  }
}

void playBuzzer() {
  buzzer.begin(10);

  buzzer.sound(NOTE_A3, 500); 
  buzzer.sound(NOTE_A3, 500);
  buzzer.sound(NOTE_A3, 500);
  buzzer.sound(NOTE_F3, 375);
  buzzer.sound(NOTE_C4, 125);

  buzzer.sound(NOTE_A3, 500);
  buzzer.sound(NOTE_F3, 375);
  buzzer.sound(NOTE_C4, 125);
  buzzer.sound(NOTE_A3, 1000);

  buzzer.sound(NOTE_E4, 500); 
  buzzer.sound(NOTE_E4, 500);
  buzzer.sound(NOTE_E4, 500);
  buzzer.sound(NOTE_F4, 375);
  buzzer.sound(NOTE_C4, 125);

  buzzer.sound(NOTE_GS3, 500);
  buzzer.sound(NOTE_F3, 375);
  buzzer.sound(NOTE_C4, 125);
  buzzer.sound(NOTE_A3, 1000);

  buzzer.sound(NOTE_A4, 500);
  buzzer.sound(NOTE_A3, 375);
  buzzer.sound(NOTE_A3, 125);
  buzzer.sound(NOTE_A4, 500);
  buzzer.sound(NOTE_GS4, 375);
  buzzer.sound(NOTE_G4, 125);

  buzzer.sound(NOTE_FS4, 125);
  buzzer.sound(NOTE_E4, 125);
  buzzer.sound(NOTE_F4, 250);
  buzzer.sound(0, 250);
  buzzer.sound(NOTE_AS3, 250);
  buzzer.sound(NOTE_DS4, 500);
  buzzer.sound(NOTE_D4, 375);
  buzzer.sound(NOTE_CS4, 125);

  buzzer.sound(NOTE_C4, 125);
  buzzer.sound(NOTE_B3, 125);
  buzzer.sound(NOTE_C4, 250);
  buzzer.sound(0, 250);
  buzzer.sound(NOTE_F3, 250);
  buzzer.sound(NOTE_GS3, 500);
  buzzer.sound(NOTE_F3, 375);
  buzzer.sound(NOTE_A3, 125);

  buzzer.sound(NOTE_C4, 500);
  buzzer.sound(NOTE_A3, 375);
  buzzer.sound(NOTE_C4, 125);
  buzzer.sound(NOTE_E4, 1000);

  buzzer.sound(NOTE_A4, 500);
  buzzer.sound(NOTE_A3, 375);
  buzzer.sound(NOTE_A3, 125);
  buzzer.sound(NOTE_A4, 500);
  buzzer.sound(NOTE_GS4, 375);
  buzzer.sound(NOTE_G4, 125);

  buzzer.sound(NOTE_FS4, 125);
  buzzer.sound(NOTE_E4, 125);
  buzzer.sound(NOTE_F4, 250);
  buzzer.sound(0, 250);
  buzzer.sound(NOTE_AS3, 250);
  buzzer.sound(NOTE_DS4, 500);
  buzzer.sound(NOTE_D4, 375);
  buzzer.sound(NOTE_CS4, 125);

  buzzer.sound(NOTE_C4, 125);
  buzzer.sound(NOTE_B3, 125);
  buzzer.sound(NOTE_C4, 250);
  buzzer.sound(0, 250);
  buzzer.sound(NOTE_F3, 250);
  buzzer.sound(NOTE_GS3, 500);
  buzzer.sound(NOTE_F3, 375);
  buzzer.sound(NOTE_C4, 125);

  buzzer.sound(NOTE_A3, 500);
  buzzer.sound(NOTE_F3, 375);
  buzzer.sound(NOTE_C4, 125);
  buzzer.sound(NOTE_A3, 1000);

  buzzer.end(2000);
}

void readButtons() {
  // Debounce start/pause button
  if ((millis() - lastButtonPress) > debounceDelay) {
    if (digitalRead(START_PAUSE_BUTTON) == LOW) {
      if (!buttonPressed) {
        buttonPressed = true;
        lastButtonPress = millis();
        handleStartPauseButton();
      }
    } else {
      buttonPressed = false;
    }
  }
  
  // Debounce reset button
  if (digitalRead(RESET_BUTTON) == LOW) {
    if (!resetButtonPressed) {
      resetButtonPressed = true;
      resetTimer();
    }
  } else {
    resetButtonPressed = false;
  }
}

void handleStartPauseButton() {
  switch (currentState) {
    case IDLE:
      // Start the timer
      currentState = WORKING;
      timmerStart = millis();
      remainingTime = WORK_TIME;
      break;
    case WORKING:
      // Pause the timer
      currentState = PAUSED;
      pausedTime = millis();
      remainingTime = WORK_TIME - ((millis() - timmerStart) / 1000);
      break;
    case PAUSED:
      // Resume the timer
      currentState = WORKING;
      timmerStart = millis() - ((WORK_TIME - remainingTime) * 1000);
      break;
    case COMPLETED:
      // Start a new work session
      currentState = IDLE;
      showIdleScreen();
      break;
  }
}

void resetTimer() {
  currentState = IDLE;
  remainingTime = WORK_TIME;
  showIdleScreen();
}

void handleIdleState() {
  // Only update the screen when needed
  static unsigned long lastDisplayUpdate = 0;
  if (millis() - lastDisplayUpdate > 500) { // Update every 500ms
    showIdleScreen();
    lastDisplayUpdate = millis();
  }
}

void handleWorkingState() {
  // Calculate remaining time
  unsigned long elapsedSeconds = (millis() - timmerStart) / 1000;
  if (elapsedSeconds >= WORK_TIME) {
    // Work session is complete
    currentState = BREAK;
    breakStarted = true;
    timmerStart = millis();
    remainingTime = BREAK_TIME;
    
    // Update display for break time
    showBreakScreen();
  } else {
    remainingTime = WORK_TIME - elapsedSeconds;
    showWorkingScreen();
  }
}

void handlePausedState() {
  showPausedScreen();
}

void handleBreakState() {
  if (breakStarted) {
    playBuzzer();
    breakStarted = false;
  }
  // Calculate remaining break time
  unsigned long elapsedSeconds = (millis() - timmerStart) / 1000;
  if (elapsedSeconds >= BREAK_TIME) {
    // Break is complete
    currentState = COMPLETED;
    showCompletedScreen();
  } else {
    remainingTime = BREAK_TIME - elapsedSeconds;
    showBreakScreen();
  }
}

void handleCompletedState() {
  // Only update the screen when needed
  static unsigned long lastDisplayUpdate = 0;
  if (millis() - lastDisplayUpdate > 1000) { // Update every second
    showCompletedScreen();
    lastDisplayUpdate = millis();
  }
}

void showIdleScreen() {
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(15, 10, "POMODORO TIMER");
    
    u8g2.setFont(u8g2_font_inb16_mn); // Larger font for timer
    u8g2.drawStr(35, 35, "25:00");
    
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(5, 55, "Press button to start");
  } while (u8g2.nextPage());
}

void showWorkingScreen() {
  int minutes = remainingTime / 60;
  int seconds = remainingTime % 60;
  char timeStr[6];
  sprintf(timeStr, "%02d:%02d", minutes, seconds);
  
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(30, 10, "WORKING TIME");
    
    // Display the timer
    u8g2.setFont(u8g2_font_inb16_mn);
    u8g2.drawStr(35, 30, timeStr);
    
    // Animation - progress bar
    u8g2.drawFrame(10, 45, 108, 10);
    int progressWidth = map(remainingTime, 0, WORK_TIME, 0, 106);
    u8g2.drawBox(11, 46, progressWidth, 8);
    
    // Animation - bouncing dot
    if (millis() - lastAnimationUpdate > 200) {
      animationFrame = (animationFrame + 1) % 10;
      lastAnimationUpdate = millis();
    }
    
    int dotX = 10 + animationFrame * 10;
    u8g2.drawDisc(dotX, 40, 2);
  } while (u8g2.nextPage());
}

void showPausedScreen() {
  int minutes = remainingTime / 60;
  int seconds = remainingTime % 60;
  char timeStr[6];
  sprintf(timeStr, "%02d:%02d", minutes, seconds);
  
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(45, 10, "PAUSED");
    
    // Display the timer
    u8g2.setFont(u8g2_font_inb16_mn);
    u8g2.drawStr(35, 30, timeStr);
    
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(10, 45, "Press to resume");
    u8g2.drawStr(10, 55, "Or reset button");
  } while (u8g2.nextPage());
}

void showBreakScreen() {
  int minutes = remainingTime / 60;
  int seconds = remainingTime % 60;
  char timeStr[6];
  sprintf(timeStr, "%02d:%02d", minutes, seconds);
  
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(35, 10, "BREAK TIME");
    
    // Display the timer
    u8g2.setFont(u8g2_font_inb16_mn);
    u8g2.drawStr(35, 30, timeStr);
    
    // Animation - expanding/contracting circle
    if (millis() - lastAnimationUpdate > 500) {
      animationFrame = (animationFrame + 1) % 4;
      lastAnimationUpdate = millis();
    }
    
    u8g2.drawCircle(64, 48, 8 + animationFrame*2);
  } while (u8g2.nextPage());
}

void showCompletedScreen() {
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(20, 15, "SESSION COMPLETE");
    
    // Blinking text
    if ((millis() / 500) % 2 == 0) {
      u8g2.drawStr(15, 35, "Press button to start");
      u8g2.drawStr(20, 45, "the next session");
    }
  } while (u8g2.nextPage());
}
