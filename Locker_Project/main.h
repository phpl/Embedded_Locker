#ifndef MY_PROJECT_LOCKER_MAIN_HEADER
#define MY_PROJECT_LOCKER_MAIN_HEADER

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

volatile uint32_t msTicks = 0;

uint32_t previousState = 0;

int ticks = 0;
int keyCodeLength = 255;
int currentKey = 0;
int sizeX = 320;
int sizeY = 240;
int previousCodeLength = 0;
int previousLength = 0;
int messageX = 30;
int messageY = 20;
int beginYHistory = 240;
int unlockSecondsMissed = 0;
int currentSecondsMissed = 0;
int currentDoorOpenedSecondMissed = 0;
int currendHistorySecondsMissed = 0;
int timeMissedSecondsMissed = 0;
int customMessageSecondsMissed = 0;
int currentCodesLength = 0;
int currentUnlocks = 0;

char keyCode[11];
char codes[50][11];
char historyOfUnlocks[8][11];

char* serviceKeyCode = "F997";
char* currentMessage = "";

bool canOpenDoor = false;
bool isServiceKey = false;
bool timeMissed = false;
bool doorOpened = false;
bool messageChanged = false;
bool codeChanged = false;
bool customMessage = false;


void uartInit(void);
void timerInit(void);
void lockerInit(void);
void lockerRowInit(void);
void lockerRowPinsInit(void);
void lockerRowDirsInit(void);
void lockerColumnInit(void);
void lockerColumnPinsInit(void);
void lockerColumnDirsInit(void);
void sendString(char* s);
void sendToUART(const int value);
void SysTick_Handler(void);
void delayMs(int val);
void acceptCodeWrite(void);
void cancelCodeWrite(void);
void setRowToRead(int row);
void lockerSelect(uint32_t readFromKeyboard);
void lockerParse(uint32_t readFromKeyboard);
void clearCode(void);
void addToKeys(char* keyCode);
void initConfigurations(void);
void printLetterToScreen(int x_pos_init, int y_pos_init, char c);
void printStringToScreen(char* word, int x, int y);
void drawScreen(void);
void editMessage(char* newMessage);
void openDoor(void);
void fillAsterisks(char* temp);
void printUnlocksHistory(void);
void addUnlockToHistory(void);
void shiftValuesInHistoryArray(void);
void customMessageHandler(void);
void doorOpenedHandler(void);
void timeMissedHandler(void);

bool validateServiceKey(char* keyCode);
bool compareKeys(char *keyCode, char* keyToCompare);
bool validateKey(char* keyCode);

uint16_t readColumn(void);
uint16_t lockerRead(void);
#endif
