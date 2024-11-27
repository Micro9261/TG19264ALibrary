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
void TG_init(void);

/************************************************************************/
/*Clears display in selected rectangle area that starts at 
PointA(posX,posY) and ends at PointB(posX,PosY)							*/
/************************************************************************/
void TG_clear_area(uint8_t posXpointA, uint8_t posYpointA, uint8_t posXpointB, uint8_t posYpointB);

/************************************************************************/
/* Clears full display                                                  */
/************************************************************************/
void TG_clear_full(void);

/************************************************************************/
/* Draws image from buff pointer. Starts at (posX,posY), prints sizeX x sizeY
characters                                                     */
/************************************************************************/
void TG_image(uint8_t posX, uint8_t posY, uint8_t sizeX, uint8_t sizeY, const uint8_t * buff);

/************************************************************************/
/* Draws rectangle at (posX,posY) of size sizeX x sizeY                 */
/************************************************************************/
void TG_rectangle(uint8_t posX, uint8_t posY, uint8_t sizeX, uint8_t sizeY);

/************************************************************************/
/* Draws line from point A(posXpointA,posYpointA) to 
point B(posXpointB,posYpointB)                                          */
/************************************************************************/
void TG_line(uint8_t posXpointA, uint8_t posYpointA, uint8_t posXpointB, uint8_t posYpointB);

/************************************************************************/
/* Writes text from (posX,posY) with given height of letters and space between them            */
/************************************************************************/
void TG_printf(uint8_t posX, uint8_t posY, uint8_t height, uint8_t space, const char * text);

/************************************************************************/
/* Turns display off, chipID for selecting part 1=left 2=middle 4=right 
sum for simultaneously turning few segments*/
/************************************************************************/
void TG_turn_off(uint8_t chipID);

/************************************************************************/
/* Turns display on, chipID for selecting part 1=left 2=middle 4=right 
sum for simultaneously turning few segments*/
/************************************************************************/
void TG_trun_on(uint8_t chipID);

/************************************************************************/
/* Get display status, chipID for selecting part 1=left 2=middle 4=right
sum not allowed!*/
/************************************************************************/
uint8_t TG_get_stat(uint8_t chipID);

/************************************************************************/
/* Testing Display communication                                        */
/************************************************************************/
void TG_test(void);

/************************************************************************/
/* Changes states for all pixels using XOR operation                     */
/************************************************************************/
void TG_reverse_all(void);

#endif /* TG19264A_ATMEGADRIVER_H_ */