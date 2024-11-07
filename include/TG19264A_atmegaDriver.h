/*
 * TG19264A_atmegaDriver.h
 *
 * Created: 10/18/2024 9:51:20 PM
 *  Author: Micro9261
 */ 


#ifndef TG19264A_ATMEGADRIVER_H_
#define TG19264A_ATMEGADRIVER_H_

#define leftSeg 0x1
#define midSeg 0x2
#define rightSeg 0x4

#include <inttypes.h>
/************************************************************************/
/*Initialization of display interface and clearing screen               */
/************************************************************************/
void InitDisplay(void);

/************************************************************************/
/*Clears display in selected rectangle area that starts at (posX,posY)   */
/************************************************************************/
void clearDisplay(uint8_t posX, uint8_t posY, uint8_t sizeX, uint8_t sizeY);

/************************************************************************/
/* Draws image from buff pointer. Starts at (posX,posY), prints sizeX x sizeY
characters                                                     */
/************************************************************************/
void drawImage(uint8_t posX, uint8_t posY, uint8_t sizeX, uint8_t sizeY, const uint8_t * buff);

/************************************************************************/
/* Draws rectangle at (posX,posY) of size sizeX x sizeY                 */
/************************************************************************/
void drawRectangle(uint8_t posX, uint8_t posY, uint8_t sizeX, uint8_t sizeY);

/************************************************************************/
/* Draws line from point A(posXpointA,posYpointA) to 
point B(posXpointB,posYpointB)                                          */
/************************************************************************/
void drawLine(uint8_t posXpointA, uint8_t posYpointA, uint8_t posXpointB, uint8_t posYpointB);

/************************************************************************/
/* Writes text from (posX,posY) with given height of letters and space between them            */
/************************************************************************/
void writeDisplay(uint8_t posX, uint8_t posY, uint8_t height, uint8_t space, const char * text);

/************************************************************************/
/* Turns display off, chipID for selecting part 1=left 2=middle 4=right 
sum for simultaneously turning few segments*/
/************************************************************************/
void turnOffDisplay(uint8_t chipID);

/************************************************************************/
/* Turns display on, chipID for selecting part 1=left 2=middle 4=right 
sum for simultaneously turning few segments*/
/************************************************************************/
void turnOnDisplay(uint8_t chipID);

/************************************************************************/
/* Get display status, chipID for selecting part 1=left 2=middle 4=right
sum not allowed!*/
/************************************************************************/
uint8_t getStatDisplay(uint8_t chipID);

/************************************************************************/
/* Testing Display communication                                        */
/************************************************************************/
void TestDisplay(void);

/************************************************************************/
/* Changes states for all pixels using XOR operation                     */
/************************************************************************/
void reverseDisplayCol(void);

#endif /* TG19264A_ATMEGADRIVER_H_ */