/*
 * TG19264A_atmegaDriver.h
 *
 * Created: 10/18/2024 9:51:20 PM
 *  Author: Micro9261
 */ 


#ifndef TG19264A_ATMEGADRIVER_H_
#define TG19264A_ATMEGADRIVER_H_

#define TG_left_disp (0x1)
#define TG_mid_disp (0x2)
#define TG_right_disp (0x4)

#include <inttypes.h>
/************************************************************************/
/*Initialization of display interface and clearing screen               */
/************************************************************************/
void TG_init(void);

/************************************************************************/
/*Clears display in selected rectangle area that starts at 
PointA(X,Y) and ends at PointB(X,Y)							*/
/************************************************************************/
void TG_clear_area(uint8_t A_x, uint8_t A_y, uint8_t B_x, uint8_t B_y);

/************************************************************************/
/* Clears full display                                                  */
/************************************************************************/
void TG_clear_full(void);

/************************************************************************/
/* Draws image from buff pointer. Starts at (posX,posY), prints sizeX x sizeY
characters                                                     */
/************************************************************************/
void TG_image(uint8_t x, uint8_t y, uint8_t x_size, uint8_t y_size, const uint8_t * img_ptr);

/************************************************************************/
/* Draws rectangle at (posX,posY) of size sizeX x sizeY                 */
/************************************************************************/
void TG_rectangle(uint8_t x, uint8_t y, uint8_t x_size, uint8_t y_size);

/************************************************************************/
/* Draws line from point A(posXpointA,posYpointA) to 
point B(posXpointB,posYpointB)                                          */
/************************************************************************/
void TG_line(uint8_t A_x, uint8_t A_y, uint8_t B_x, uint8_t B_y);

/************************************************************************/
/* Writes text from (posX,posY) with given height of letters and space between them in pixels */
/************************************************************************/
void TG_printf(uint8_t x, uint8_t y, uint8_t height, uint8_t space, const char * txt);

/************************************************************************/
/* Turns display off, chipID for selecting part 1=left 2=middle 4=right 
sum for simultaneously turning few segments*/
/************************************************************************/
void TG_turn_off(uint8_t chip_id);

/************************************************************************/
/* Turns display on, chipID for selecting part 1=left 2=middle 4=right 
sum for simultaneously turning few segments*/
/************************************************************************/
void TG_turn_on(uint8_t chip_id);

/************************************************************************/
/* Get display status, chipID for selecting part 1=left 2=middle 4=right
sum not allowed!*/
/************************************************************************/
uint8_t TG_get_stat(uint8_t chip_id);

/************************************************************************/
/* Testing Display communication                                        */
/************************************************************************/
void TG_test(void);

/************************************************************************/
/* Changes states for all pixels using XOR operation                     */
/************************************************************************/
void TG_reverse_all(void);

#endif /* TG19264A_ATMEGADRIVER_H_ */