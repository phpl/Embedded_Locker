#include "main.h"
#include "LPC17xx.h" // Device header
#include "core_cm3.h"
#include "PIN_LPC17xx.h"
#include "GPIO_LPC17xx.h" // Keil::Device:GPIO
#include "lcd_lib/LCD_ILI9325.h"
#include "lcd_lib/Open1768_LCD.h"
#include "lcd_lib/asciiLib.h"

void SysTick_Handler(void)
{
	msTicks++;
}

void delayMs(int val)
{
	msTicks = 0;
	while (msTicks < val)
		;
}

void uartInit()
{
	LPC_UART0->LCR = (1 << 0) | (1 << 1) | (1 << 7);
	LPC_UART0->DLM = 0;
	LPC_UART0->DLL = 13;
	LPC_UART0->LCR &= ~(1 << 7);
	PIN_Configure(0, 2, PIN_FUNC_1, PIN_PINMODE_PULLUP, PIN_PINMODE_NORMAL);
	PIN_Configure(0, 3, PIN_FUNC_1, PIN_PINMODE_PULLUP, PIN_PINMODE_NORMAL);
}

void timerInit()
{
	LPC_TIM0->PR = 0;
	LPC_TIM0->MR0 = 12500000; // co sekunde
	LPC_TIM0->MCR = (1 << 0) | (1 << 1);
	LPC_TIM0->TCR = 1;

	NVIC_EnableIRQ(TIMER0_IRQn);
}

void TIMER0_IRQHandler(void)
{
	LPC_TIM0->IR = 1;
	currendHistorySecondsMissed++;

	customMessageHandler();
	doorOpenedHandler();
	timeMissedHandler();
}

void customMessageHandler()
{
	if (customMessage)
	{
		customMessageSecondsMissed++;
	}

	if (customMessageSecondsMissed >= 8)
	{
		editMessage("");
		customMessageSecondsMissed = 0;
	}
}

void doorOpenedHandler()
{
	if (doorOpened)
	{
		currentDoorOpenedSecondMissed++;
	}

	if (currentDoorOpenedSecondMissed >= 15)
	{
		currentDoorOpenedSecondMissed = 0;
		doorOpened = false;
		sendString("Drzwi zamkniete");
		editMessage("");
	}
}

void timeMissedHandler()
{
	if (timeMissed)
	{
		timeMissedSecondsMissed++;
	}

	if (timeMissedSecondsMissed == 5)
	{
		cancelCodeWrite();
		timeMissed = false;
		timeMissedSecondsMissed = 0;
	}

	if ((currentKey > 0 || isServiceKey) && previousCodeLength == currentKey)
	{
		currentSecondsMissed++;
	}
	else if ((currentKey > 0 || isServiceKey) && previousCodeLength != currentKey)
	{
		previousCodeLength = currentKey;
		currentSecondsMissed = 0;
		currentMessage = "";
	}

	if (currentSecondsMissed == 15)
	{
		sendString("\nCzas minal\n");
		editMessage("Czas minal!");
		timeMissed = true;
		isServiceKey = false;
		currentSecondsMissed = 0;
	}
}

void sendString(char *s)
{
	int index = 0;
	char c = s[index];
	while (c != '\0')
	{
		if (LPC_UART0->LSR & (1 << 5))
		{
			sendToUART(c);
			++index;
			c = s[index];
		}
	}
}

void sendToUART(const int value)
{
	LPC_UART0->THR = value;
}

void lockerInit()
{
	lockerRowInit();
	lockerColumnInit();
	clearCode();
}

void lockerRowInit()
{
	lockerRowPinsInit();
	lockerRowDirsInit();
}

void lockerRowPinsInit()
{
	PIN_Configure(1, 0, 0, PIN_PINMODE_PULLUP, 0);
	PIN_Configure(1, 10, 0, PIN_PINMODE_PULLUP, 0);
	PIN_Configure(1, 8, 0, PIN_PINMODE_PULLUP, 0);
	PIN_Configure(1, 16, 0, PIN_PINMODE_PULLUP, 0);
}

void lockerRowDirsInit()
{
	GPIO_SetDir(1, 0, GPIO_DIR_OUTPUT);
	GPIO_SetDir(1, 10, GPIO_DIR_OUTPUT);
	GPIO_SetDir(1, 8, GPIO_DIR_OUTPUT);
	GPIO_SetDir(1, 16, GPIO_DIR_OUTPUT);
}

void lockerColumnInit()
{
	lockerColumnPinsInit();
	lockerColumnDirsInit();
}

void lockerColumnPinsInit()
{
	PIN_Configure(1, 4, 0, PIN_PINMODE_PULLDOWN, 0);
	PIN_Configure(1, 9, 0, PIN_PINMODE_PULLDOWN, 0);
	PIN_Configure(1, 15, 0, PIN_PINMODE_PULLDOWN, 0);
	PIN_Configure(1, 17, 0, PIN_PINMODE_PULLDOWN, 0);
}

void lockerColumnDirsInit()
{
	GPIO_SetDir(1, 4, GPIO_DIR_INPUT);
	GPIO_SetDir(1, 9, GPIO_DIR_INPUT);
	GPIO_SetDir(1, 15, GPIO_DIR_INPUT);
	GPIO_SetDir(1, 17, GPIO_DIR_INPUT);
}

uint16_t lockerRead()
{
	uint32_t readFromKeyboard = 0x0;

	for (int i = 3; i >= 0; --i)
	{
		setRowToRead(i);
		readFromKeyboard <<= 4;
		readFromKeyboard |= readColumn();
	}

	return readFromKeyboard;
}

void setRowToRead(int row)
{
	switch (row)
	{
	case 0:
		GPIO_PinWrite(1, 0, 1);
		GPIO_PinWrite(1, 10, 0);
		GPIO_PinWrite(1, 8, 0);
		GPIO_PinWrite(1, 16, 0);
		break;
	case 1:
		GPIO_PinWrite(1, 0, 0);
		GPIO_PinWrite(1, 10, 1);
		GPIO_PinWrite(1, 8, 0);
		GPIO_PinWrite(1, 16, 0);
		break;
	case 2:
		GPIO_PinWrite(1, 0, 0);
		GPIO_PinWrite(1, 10, 0);
		GPIO_PinWrite(1, 8, 1);
		GPIO_PinWrite(1, 16, 0);
		break;
	case 3:
		GPIO_PinWrite(1, 0, 0);
		GPIO_PinWrite(1, 10, 0);
		GPIO_PinWrite(1, 8, 0);
		GPIO_PinWrite(1, 16, 1);
		break;
	}
}

uint16_t readColumn()
{
	uint32_t readFromKeyboard = 0x0;

	readFromKeyboard |= GPIO_PinRead(1, 4) << 0;
	readFromKeyboard |= GPIO_PinRead(1, 9) << 1;
	readFromKeyboard |= GPIO_PinRead(1, 15) << 2;
	readFromKeyboard |= GPIO_PinRead(1, 17) << 3;

	return readFromKeyboard;
}

void lockerParse(uint32_t readFromKeyboard)
{
	ticks++;

	if (readFromKeyboard != 0 && readFromKeyboard == previousState && ticks == 10)
	{
		lockerSelect(readFromKeyboard);
	}
	else if (readFromKeyboard & ~previousState && ticks > 10)
	{
		previousState = 0;
		ticks = 0;
	}
	else
	{
		previousState = readFromKeyboard;
	}
}

void lockerSelect(uint32_t readFromKeyboard)
{
	if (currentKey >= 11)
	{
		return;
	}

	switch (readFromKeyboard)
	{
	case 1 << 0:
		keyCode[currentKey++] = '1';
		break;
	case 1 << 1:
		keyCode[currentKey++] = '2';
		break;
	case 1 << 2:
		keyCode[currentKey++] = '3';
		break;
	case 1 << 3:
		//A
		acceptCodeWrite();
		break;
	case 1 << 4:
		keyCode[currentKey++] = '4';
		break;
	case 1 << 5:
		keyCode[currentKey++] = '5';
		break;
	case 1 << 6:
		keyCode[currentKey++] = '6';
		break;
	case 1 << 7:
		keyCode[currentKey++] = 'B';
		break;
	case 1 << 8:
		keyCode[currentKey++] = '7';
		break;
	case 1 << 9:
		keyCode[currentKey++] = '8';
		break;
	case 1 << 10:
		keyCode[currentKey++] = '9';
		break;
	case 1 << 11:
		//C
		cancelCodeWrite();
		break;
	case 1 << 12:
		keyCode[currentKey++] = '0';
		break;
	case 1 << 13:
		keyCode[currentKey++] = 'F';
		break;
	case 1 << 14:
		keyCode[currentKey++] = 'E';
		break;
	case 1 << 15:
		keyCode[currentKey++] = 'D';
		break;
	}
	sendString("\n\n");
	sendString(keyCode);
	sendString("\n\n");

	if (currentKey > 0)
	{
		if (previousLength != currentKey)
		{
			codeChanged = true;
		}
		else
		{
			previousLength = currentKey;
		}
	}
}

void acceptCodeWrite()
{
	if (isServiceKey)
	{
		addToKeys(keyCode);
		isServiceKey = false;
	}
	else
	{
		canOpenDoor = validateKey(keyCode);
		isServiceKey = validateServiceKey(keyCode);

		if (isServiceKey)
		{
			sendString("\nWpisano kod serwisowy!\n");
			editMessage("Wpisano kod serwisowy!");
			customMessage = true;
		}
		else if (!canOpenDoor)
		{
			sendString("\nWpisano zly kod, sprobuj ponownie!\n");
			editMessage("Wpisano zly kod!");
			customMessage = true;
		}
	}

	clearCode();
}

void cancelCodeWrite()
{
	sendString("\nAnulowano wpisywanie kodu!\n");
	editMessage("Anulowano wpisywanie kodu!");
	customMessage = true;
	clearCode();
}

void clearCode()
{
	memset(keyCode, 0, sizeof keyCode);
	currentKey = 0;
	previousCodeLength = 0;
}

bool validateKey(char *keyCode)
{
	for (int i = 0; i < currentCodesLength; ++i)
	{
		if (compareKeys(keyCode, codes[i]))
		{
			return true;
		}
	}
	return false;
}

bool validateServiceKey(char *keyCode)
{
	return compareKeys(keyCode, serviceKeyCode);
}

bool compareKeys(char *keyCode, char *keyToCompare)
{
	bool isKeyValid = true;

	if (strcmp(keyCode, keyToCompare) != 0)
	{
		isKeyValid = false;
	}

	return isKeyValid;
}

void addToKeys(char *keyCode)
{
	if (currentCodesLength == 50)
	{
		sendString("\nWpisano za dużo kodow\n");
		editMessage("Wpisano za duzo kodow!");
		customMessage = true;
		return;
	}

	if (strcmp(keyCode, serviceKeyCode) == 0)
	{
		sendString("\nBlad! Kod serwisowy!\n");
		editMessage("Blad! Kod serwisowy!");
		customMessage = true;
		return;
	}

	strcpy(codes[currentCodesLength], keyCode);
	currentCodesLength++;
	sendString("\nDodany nowy kod\n");
	editMessage("Dodano nowy kod!");
	customMessage = true;
}

void printStringToScreen(char *word, int x, int y)
{
	int i = 0;
	while (word[i] != '\0')
	{
		printLetterToScreen(x, y, word[i]);
		x += 10;
		++i;
	}
}

void printLetterToScreen(int x_pos_init, int y_pos_init, char c)
{
	uint8_t tab[16];

	GetASCIICode(1, tab, c);

	int y_pos = y_pos_init;
	int x_pos = x_pos_init;

	lcdWriteReg(ADRX_RAM, y_pos);
	lcdWriteReg(ADRY_RAM, x_pos);

	for (unsigned j = 0; j < 8; ++j)
	{
		for (unsigned i = 0; i < 16; ++i)
		{
			uint8_t value = tab[i] & (1 << (7 - j));
			if (value)
			{
				lcdWriteReg(ADRX_RAM, y_pos);
				lcdWriteReg(ADRY_RAM, x_pos);
				lcdWriteReg(DATA_RAM, LCDWhite);
			}
			--y_pos;
		}
		++x_pos;
		y_pos = y_pos_init;
	}
}

void drawScreen()
{
	lcdWriteReg(ADRX_RAM, 0);
	lcdWriteReg(ADRY_RAM, 0);
	lcdWriteReg(DATA_RAM, LCDWhite);

	int quarterY = sizeY / 4;
	char asterisks[256];
	fillAsterisks(asterisks);

	for (unsigned int i = 0; i < sizeX; ++i)
	{
		for (unsigned int j = 0; j < sizeY; ++j)
		{
			if (j < quarterY - 1 || j >= beginYHistory - 20)
			{
				lcdWriteData(LCDBlack);
			}
			else
			{
				lcdWriteData(LCDPastelBlue);
			}
		}
	}

	printStringToScreen("LOCKER PROJECT", sizeX / 2, sizeY * 3 / 5);
	printUnlocksHistory();
	printStringToScreen(asterisks, messageX, messageY + 20);
	printStringToScreen(currentMessage, messageX, messageY);
}

void fillAsterisks(char *arrayToFill)
{
	for (int i = 0; i < currentKey; i++)
	{
		arrayToFill[i] = '*';
	}
	arrayToFill[currentKey] = '\0';
}

void printUnlocksHistory()
{
	int y = beginYHistory;
	printStringToScreen("Czas otworzenia drzwi [s]:", messageX, y);
	y -= 20;
	for (int i = 0; i < currentUnlocks; ++i)
	{
		printStringToScreen(historyOfUnlocks[i], messageX, y);
		y -= 20;
	}
}

void initConfigurations()
{
	SysTick_Config(SystemCoreClock / 1000);
	uartInit();
	timerInit();
	lcdConfiguration();
	lockerInit();
	init_ILI9325();
}

void editMessage(char *newMessage)
{
	currentMessage = newMessage;
	messageChanged = true;
}

void openDoor()
{
	sendString("\n\nDrzwi otwarte!\n\n");
	editMessage("Drzwi otwarte!");
	canOpenDoor = false;
	doorOpened = true;
	unlockSecondsMissed = currendHistorySecondsMissed;
	addUnlockToHistory();
}

void addUnlockToHistory()
{
	if (currentUnlocks >= 8)
	{
		shiftValuesInHistoryArray();
	}

	char unlockSeconds[255];
	sprintf(unlockSeconds, "%d", unlockSecondsMissed);
	strcpy(historyOfUnlocks[currentUnlocks], unlockSeconds);

	if (currentUnlocks < 8)
	{
		currentUnlocks++;
	}
}

void shiftValuesInHistoryArray()
{
	for (int i = 0; i < currentUnlocks; ++i)
	{
		strcpy(historyOfUnlocks[i], historyOfUnlocks[i + 1]);
	}
}

int main(void)
{
	initConfigurations();
	uint32_t valFromKeyboard;
	addToKeys("4");
	drawScreen();
	while (1)
	{
		if (messageChanged || codeChanged)
		{
			drawScreen();
			messageChanged = false;
			codeChanged = false;
		}

		valFromKeyboard = lockerRead();
		delayMs(1);
		lockerParse(valFromKeyboard);

		if (canOpenDoor)
		{
			openDoor();
		}
	}
}
