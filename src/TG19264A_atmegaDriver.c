/*
 * TG19264A_atmegaDriver.c
 *
 * Created: 10/18/2024 9:50:52 PM
 *  Author: pawel
 */ 

#define F_CPU 16000000UL

#include "..\include\TG19264A_atmegaDriver.h"
#include "..\include\fonts.h"
#include <inttypes.h>
#include <xc.h>
#include <util/delay.h>

#define BadValue 128
#define true 1
#define false 0

#define XPoints 192
#define XPointsPerChip 64
#define YPoints 64
#define YPointsPerPage 8

#define strobeEnable (PORTC ^= 1 << 3); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop")
#define strobeEnableFast (PORTC ^= 1 << 3); asm("nop"); asm("nop")
#define strobeReset (PORTC ^= 1 << 2)

#define cs1deselect (PORTD |= 1 << 6)
#define cs1select (PORTD &= ~(1 << 6))
#define cs2deselect (PORTC |= 1 << 6)
#define cs2select (PORTC &= ~(1 << 6))
#define cs3deselect (PORTC |= 1 << 7)
#define cs3select (PORTC &= ~(1 << 7))

#define RSreadState ( (1 << 4) & PINC)
#define RSdata (PORTC |= 1 << 4)
#define RScommand (PORTC &= ~(1 << 4))
#define RWread (PORTC |= 1 << 5)
#define RWwrite (PORTC &= ~(1 << 5))

uint8_t pageBuff[64]; //for library use only. Internal buffer!

//struct for acquiring information about bytes to send per chipId and chipID for start
typedef struct 
{
	uint8_t BytesPerChip[3];
	uint8_t StartchipID;
} chipTransferInfo;

//Gives needed information to functions about starting point, offset for mask and bytes to send;
typedef struct
{
	uint8_t page;
	uint8_t col;
	uint8_t offset;
	uint8_t BytesToSend;
} sendParam;

static void selectChip(uint8_t ID)
{
	if (leftSeg & ID)
		cs1select;
	if (midSeg & ID)
		cs2select;
	if (rightSeg & ID)
		cs3select;
}

static void deselectChip(uint8_t ID)
{
	if (leftSeg & ID)
		cs1deselect;
	if (midSeg & ID)
		cs2deselect;
	if (rightSeg & ID)
		cs3deselect;
}

static uint8_t getByte(void)
{
	uint8_t data;
	strobeEnable;
	data = PINA;
	strobeEnableFast;
	return data;
}

static void waitBusy(void)
{
	DDRA = 0x00;
	PORTA = 0xFF;
	RWread;
	uint8_t RSstate = RSreadState;
	RScommand;
	while (getByte() & (1 << 7));
	PORTA = 0x00;
	DDRA = 0xFF;
	RWwrite;
	if (RSstate)
		RSdata;
	else
		RScommand;
}

static void sendByte(uint8_t byte)
{
	strobeEnableFast;
	PORTA = byte;
	strobeEnableFast;
	waitBusy();
}

static void setAddress(uint8_t page, uint8_t col)
{
	RScommand;
	sendByte(0x40 | (0x3F & col));
	sendByte(0xB8 | (0x07 & page));
	RSdata;
}

/*
Sends data to selected address, chipID for selecting part 1=left 2=middle 4=right
sum for simultaneously turning few segments*/
static void sendData(uint8_t size, const uint8_t * buff)
{
	for (uint8_t i = 0; i < size; i++)
		sendByte(*buff++);
}

/***
test function to fill, chipID for selecting part 1=left 2=middle 4=right
sum for simultaneously turning few segments*/
static void testfillPage(uint8_t page, uint8_t pattern, uint8_t chipID)
{
	selectChip(chipID);
	setAddress(page,0);
	for (uint8_t i = 0; i < 64; i++)
	{
		sendByte(pattern);
	}
	deselectChip(chipID);
}

/*
Reads data from selected address, chipID for selecting part 1=left 2=middle 4=right
sum for simultaneously turning few segments*/
static void readData(uint8_t size, uint8_t * buff)
{
	RWread;
	DDRA = 0x00;
	PORTA = 0xFF;
	RSdata;
	getByte();
	RScommand;
	while (getByte() & (1 << 7));
	for (uint8_t i = 0; i < size; i++)
	{
		RSdata;
		*buff++ = getByte();
		RScommand;
		while (getByte() & (1 << 7));
	}
	PORTA = 0x00;
	DDRA = 0xFF;
	RWwrite;
	RSdata;
}

/*
Gives possibility for shift up/down, chipID for selecting part 1=left 2=middle 4=right
sum for simultaneously turning few segments*/
static void setStartLine(uint8_t start, uint8_t chipID)
{
	RScommand;
	selectChip(chipID);
	sendByte(0xC0 | start);
	deselectChip(chipID);
	RSdata;
}


/*
Initializes ports for work with display
*/
void InitDisplay(void)
{
	DDRA = 0xFF;
	PORTA = 0x00;
	DDRC |= 0xFC;
	PORTC &= ~(0xFC);
	DDRD |= 1 << 6;
	PORTD &= (1 << 6);
	strobeReset;
	RSdata;
	RWwrite;
	_delay_ms(35);
	turnOnDisplay(0x7);
	setStartLine(0,0x7);
	cs1deselect;
	cs2deselect;
	cs3deselect;
}

void turnOnDisplay(uint8_t chipID)
{
	RScommand;
	selectChip(chipID);
	sendByte(0x3F);
	deselectChip(chipID);
	RSdata;
}

void turnOffDisplay(uint8_t chipID)
{
	RScommand;
	selectChip(chipID);
	sendByte(0x3E);
	deselectChip(chipID);
	RSdata;
}

uint8_t getStatDisplay(uint8_t chipID)
{
	DDRA = 0x00;
	PORTA = 0xFF;
	RWread;
	uint8_t RSstate = RSreadState;
	RScommand;
	selectChip(chipID);
	uint8_t res = getByte();
	deselectChip(chipID);
	PORTA = 0x00;
	DDRA = 0xFF;
	RWwrite;
	if (RSstate)
		RSdata;
	else
		RScommand;
	return res;
}

/************************************************************************/
/* return Bytes needed to send to specific chip in BytesToSendBuff
return chipIDStart which indicates starting display number and function returns
number of chips select changes needed                                */
/************************************************************************/
//static uint8_t getSendBytesInfo(uint8_t min, uint8_t max, uint8_t * BytesToSendBuff, uint8_t * chipIDStart)
static uint8_t getSendBytesInfo(uint8_t min, uint8_t max, chipTransferInfo * info)
{
	info->StartchipID = min / XPointsPerChip;
	uint8_t chipIDend = (max - 1)/XPointsPerChip;
	uint8_t csChanges = chipIDend - info->StartchipID;
	
	if (0 == csChanges)
	{
		info->BytesPerChip[info->StartchipID] = max - min;
	}
	else if (1 == csChanges)
	{
		info->BytesPerChip[info->StartchipID] = (info->StartchipID + 1) * XPointsPerChip - min;
		info->BytesPerChip[info->StartchipID + 1] = max - (info->StartchipID + 1) *XPointsPerChip;
	}
	else if (2 == csChanges)
	{
		info->BytesPerChip[info->StartchipID] = XPointsPerChip - min;
		info->BytesPerChip[info->StartchipID + 1] = XPointsPerChip;
		info->BytesPerChip[info->StartchipID + 2] = max - 2*XPointsPerChip;
	}
	else
		return BadValue;
		
	return csChanges + 1;
}


/************************************************************************/
/* special function for selecting only one chip with different approach
 1 == leftSeg, 2 == midSeg, 3== rightSeg                                */
/************************************************************************/
static void selectChipOne(uint8_t ID)
{
	if (0 == ID)
		cs1select;
	if (1 == ID)
		cs2select;
	if (2 == ID)
		cs3select;
}

/************************************************************************/
/* special function for deselecting only one chip with different approach
 1 == leftSeg, 2 == midSeg, 3== rightSeg                                */
/************************************************************************/
static void deselectChipOne(uint8_t ID)
{
	if (0 == ID)
		cs1deselect;
	if (1 == ID)
		cs2deselect;
	if (2 == ID)
		cs3deselect;
}

/************************************************************************/
/* function to create mask with bits written from LSB                   */
/************************************************************************/
static inline uint8_t makeMask(uint8_t bits)
{
	uint8_t specMask = 0x0;
	while(bits--)
	specMask |= 1 << (7 - bits);
	return specMask;
}

/************************************************************************/
/* function to create mask with bits written from MSB                   */
/************************************************************************/
static inline uint8_t makeReverseMask(uint8_t bits)
{
	uint8_t specMask = 0x0;
	while (bits--)
	{
		specMask <<=1;
		specMask |= 1 << 0;
	}
	return specMask;
}

/************************************************************************/
/* fills page  from given col by pattern                                */
/************************************************************************/
//static inline void fillPage(uint8_t page, uint8_t col, uint8_t pattern, uint8_t numOfBytes)
static inline void fillPage(const sendParam * param)
{
	setAddress(param->page,param->col);
	for (uint8_t i = 0; i < param->BytesToSend; i++)
		sendByte(param->offset);
}


/************************************************************************/
/* Clears selected area by mask bits, invert for Up rows.               */
/************************************************************************/
//static inline void clearMaskPage(uint8_t page, uint8_t col, uint8_t bits, uint8_t BytesToSend, uint8_t invert)
static inline void clearMaskPage(const sendParam * param, uint8_t invert)
{
	setAddress(param->page, param->col);
	readData(param->BytesToSend,pageBuff);
	uint8_t maskUp = makeMask(param->offset);
	if (invert)
		maskUp = ~maskUp;
	for (uint8_t i = 0; i < param->BytesToSend; i++)
		pageBuff[i] = pageBuff[i] & maskUp;
	setAddress(param->page, param->col);
	for (uint8_t i = 0; i < param->BytesToSend; i++)
		sendByte(pageBuff[i]);
}

/************************************************************************/
/*Clears display in selected rectangle area that starts at (posX,posY)   */
/************************************************************************/
void clearDisplay(uint8_t posX, uint8_t posY, uint8_t sizeX, uint8_t sizeY)
{
	if (posX > XPoints - 1 || posY > YPoints - 1)
		return;
	uint8_t colStart = posX%XPointsPerChip;
	uint8_t pageSup = (posY + sizeY - 1)/YPointsPerPage;
	uint8_t pageInf = posY/YPointsPerPage;
	uint8_t rowsSup = (posY + sizeY)%YPointsPerPage;
	uint8_t rowsInf = posY%YPointsPerPage;
	uint8_t pagesToChange = pageSup - pageInf + 1;
	pageSup = 0x07 & ~pageSup;		//change direction of pages from 7->0, to 0->7
	
	chipTransferInfo TransInfo;
	uint8_t csChangesNeeded = getSendBytesInfo(posX,posX+sizeX,&TransInfo);
	uint8_t colToSend;
	uint8_t iteration = 0;
	sendParam TxInfo;
	while (csChangesNeeded--)
	{
		if (0 == iteration++)
		colToSend = colStart;
		else
		colToSend = 0;
		
		selectChipOne(TransInfo.StartchipID);
		for(uint8_t i = 0; i < pagesToChange; i++)
		{
			TxInfo.BytesToSend = TransInfo.BytesPerChip[TransInfo.StartchipID];
			TxInfo.col = colToSend;
			TxInfo.page = pageSup + i;
			if (0 == i && rowsSup != 0)
			{
				TxInfo.offset = rowsSup;
				clearMaskPage(&TxInfo, true); //reverse mask 
			}
			else if (0 == i && rowsInf != 0)
			{
				TxInfo.offset = rowsInf;
				clearMaskPage(&TxInfo, false); //don't reverse mask
			}
			else
			{
				TxInfo.offset = 0x00; // used as pattern for every byte not offset!
				fillPage(&TxInfo);
			}
		}
		deselectChipOne(TransInfo.StartchipID++);
	}
}

/************************************************************************/
/* Changes states for all pixels using XOR operation                     */
/************************************************************************/
void reverseDisplayCol(void)
{
	for (uint8_t chip = 0; chip < 3; chip++)
	{
		selectChipOne(chip);
		for (uint8_t page = 0; page < YPoints/YPointsPerPage; page++)
		{
			setAddress(page, 0);
			readData(XPointsPerChip,pageBuff);
			for (uint8_t i = 0; i < XPointsPerChip; i++)
				pageBuff[i] ^= 0xFF;
			setAddress(page,0);
			for (uint8_t i = 0; i < XPointsPerChip; i++)
				sendByte(pageBuff[i]);
		}
		deselectChipOne(chip);
	}
}

//Creates images using mask based on bits needed to be cleared, work only on selected rows from up or down (selection by inverting mask)
//static inline void DrawMaskPage(uint8_t page, uint8_t col, uint8_t bits, uint8_t BytesToSend, uint8_t invert, const uint8_t * image)
static inline void DrawMaskPage(const sendParam * param, uint8_t invert, const uint8_t * image)
{
	setAddress(param->page, param->col);
	readData(param->BytesToSend,pageBuff);
	uint8_t maskUp = makeMask(param->offset);
	if (invert)
		maskUp = ~maskUp;
	for (uint8_t i = 0; i < param->BytesToSend; i++)
	{
		pageBuff[i] = pageBuff[i] & maskUp;
		if (invert)
			pageBuff[i] |= (image[i] << (8 - param->offset) );
		else
			pageBuff[i] |= (image[i] >> param->offset );
	}
	setAddress(param->page, param->col);
	for (uint8_t i = 0; i < param->BytesToSend; i++)
		sendByte(pageBuff[i]);
}

//Creates images using mask based on bits needed to be cleared, work on all rows
//static inline void DrawPage(uint8_t page, uint8_t col, uint8_t bits, uint8_t BytesToSend, const uint8_t * imageBefore, const uint8_t * imageNow)
static inline void DrawPage(const sendParam * param, const uint8_t * imageBefore, const uint8_t * imageNow)
{
	setAddress(param->page, param->col);
	uint8_t topMask = makeMask(param->offset);
	uint8_t offsetDown;
	if (param->offset == 0)
		offsetDown = 0;
	else
		offsetDown = 8 - param->offset;
	for (uint8_t i = 0; i < param->BytesToSend; i++)
	{
		pageBuff[i] = (imageNow[i] & ~topMask) >> offsetDown;
		if (param->offset != 0)
			pageBuff[i] |= (imageBefore[i] & topMask) << param->offset;
	}
	sendData(param->BytesToSend,pageBuff);
}

/*
Prints image from buff in given X,Y coordinates with defined sizeX x sizeY image size
*/
void drawImage(uint8_t posX, uint8_t posY, uint8_t sizeX, uint8_t sizeY, const uint8_t * buff)
{
	if (posX + sizeX > XPoints || posY + sizeY> YPoints)
		return;
	
	uint8_t colStart = posX%XPointsPerChip;
	uint8_t pageSup = (posY + sizeY - 1)/YPointsPerPage;
	uint8_t pageInf = posY/YPointsPerPage;
	uint8_t rowsSup = (posY + sizeY)%YPointsPerPage;
	uint8_t rowsInf = posY%YPointsPerPage;
	uint8_t pagesToChange = pageSup - pageInf + 1;
	pageSup = 0x07 & ~pageSup;
	
	chipTransferInfo TransInfo;
	uint8_t csChangesNeeded = getSendBytesInfo(posX,posX+sizeX,&TransInfo);
	uint8_t colToSend;
	uint8_t iteration = 0;
	
	uint8_t startXPointer = 0;
	uint16_t startYPointer = 0;
	sendParam TxInfo;
	while (csChangesNeeded--)
	{
		if (0 == iteration++)
		colToSend = colStart;
		else
		colToSend = 0;
		
		selectChipOne(TransInfo.StartchipID);
		startYPointer = 0;
		for(uint8_t i = 0; i < pagesToChange; i++)
		{
			TxInfo.BytesToSend = TransInfo.BytesPerChip[TransInfo.StartchipID];
			TxInfo.col = colToSend;
			TxInfo.page = pageSup + i;
			TxInfo.offset = rowsSup;
			const uint8_t* setBuffAddress  = buff + startXPointer + startYPointer;
			if (0 == i && rowsSup != 0)
				DrawMaskPage(&TxInfo, true, setBuffAddress);
			else if (pagesToChange - 1 == i && rowsInf != 0)
			{
				TxInfo.offset = rowsInf;
				DrawMaskPage(&TxInfo, false, setBuffAddress - sizeY);
			}
			else
				DrawPage(&TxInfo, setBuffAddress - sizeY, setBuffAddress);
			startYPointer += sizeY;
		}
		deselectChipOne(TransInfo.StartchipID++);
		startXPointer += TxInfo.BytesToSend;
	}
}

/*
TODO number 3 priority;
Draws line from pointA to pointB
*/
void drawLine(uint8_t posXpointA, uint8_t posYpointA, uint8_t posXpointB, uint8_t posYpointB)
{
	uint8_t startX, startY;
	uint8_t endX, endY;
	if (posXpointA < posXpointB)
	{
		startX = posXpointB;
		endX = posXpointA;
		startY = posYpointB;
		endY = posYpointA;
	}
	else
	{
		startX = posXpointA;
		endX = posXpointB;
		startY = posYpointA;
		endY = posYpointB;
	}
	uint8_t dotsX = endX - startX;
	int8_t dotsY = endY - startY;
	
	chipTransferInfo TransInfo;
	uint8_t cschangesNeeded = getSendBytesInfo(startX, endX, &TransInfo);
	uint8_t colToSend;
	uint8_t iteration = 0;
	
}

/*
Draws desired rectangle
*/
void drawRectangle(uint8_t posX, uint8_t posY, uint8_t sizeX, uint8_t sizeY)
{
	drawLine(posX, posX, posX, posY+sizeY);
	drawLine(posX, posY+sizeY, posX +sizeX, posY+sizeY);
	drawLine(posX+sizeX, posY+sizeY, posX+sizeX, posY);
	drawLine(posX+sizeX, posY, posX, posY);
}


/************************************************************************/
/* Writes text from (posX,posY) with given height of letters            */
/************************************************************************/
void writeDisplay(uint8_t posX, uint8_t posY, uint8_t height, uint8_t space, const char * text)
{
	const uint8_t (*ptr)[5];
	uint8_t charWidth = 0;
	uint8_t charHight = 0;
	if (height == 7)
	{
		ptr = Font8x5;
		charHight = 8;
		charWidth = 5;
	}
	while (*text != '\0')
	{
		if (posY + charHight >= YPoints)
			posY -= (posY + charHight - YPoints);
		if (posX + charWidth >= XPoints)
		{
			posX = 0;
			posY -= charHight;
		}
		drawImage(posX,posY,charWidth,charHight,ptr[(uint8_t)*text]);
		posX += charWidth + space;
		text++;
	}
}

/*
Prints test data on display
*/
void TestDisplay(void)
{
	for(int i =0; i < 8; i++)
	{
		testfillPage(i,makeMask(i),leftSeg);
		testfillPage(i,0xFF,midSeg);
		testfillPage(i,0xFF,rightSeg);
	}
	setStartLine(0,0x7);
	_delay_ms(1000);
	for (uint8_t i =0; i < 8; i++)
	{
		selectChipOne(0);
		setAddress(i,0);
		readData(64,pageBuff);
		deselectChipOne(0);
		selectChipOne(2);
		setAddress(i,0);
		sendData(64,pageBuff);
		deselectChipOne(2);
	}
	_delay_ms(1000);
	selectChipOne(0);
	for (uint8_t i =0; i < 8; i++)
	{
		setAddress(i,0);
		readData(32,pageBuff);
		for (uint8_t y=0; y < 32; y++)
			pageBuff[y] = ~pageBuff[y];
		setAddress(i,0);
		sendData(32,pageBuff);
	}
	deselectChipOne(0);
	_delay_ms(1000);
	turnOffDisplay(midSeg);
	clearDisplay(2,2,188,60);
	_delay_ms(1000);
	reverseDisplayCol();
	turnOnDisplay(midSeg);
}
