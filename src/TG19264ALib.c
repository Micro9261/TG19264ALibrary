/*
 * TG19264A_atmegaDriver.c
 *
 * Created: 10/18/2024 9:50:52 PM
 *  Author: Micro9261
 */ 

#include <inttypes.h>
#include "TG19264Fonts.h"
#include "TG19264ALib.h"
#include "TG19264Config.h"

#define BadValue 128
#define true 1
#define false 0
#define HIGH 1
#define LOW 0
#define BUSY_FLAG 7

#define XPoints 192
#define XPointsPerChip 64
#define YPoints 64
#define YPointsPerPage 8


#define strobe_enable (E_PIN_PORT ^= HIGH <<  E_PIN_NUM); DELAY_200NS
#define strobe_enable_fast (E_PIN_PORT ^= HIGH <<  E_PIN_NUM); DELAY_STROBE_FAST
#define strobe_reset (RES_PIN_PORT ^= HIGH <<  RES_PIN_NUM)

#define cs1_deselect (CS1_PIN_PORT |= HIGH << CS1_PIN_NUM)
#define cs1_select (CS1_PIN_PORT &= ~(HIGH << CS1_PIN_NUM))
#define cs2_deselect (CS2_PIN_PORT |= HIGH << CS2_PIN_NUM)
#define cs2_select (CS2_PIN_PORT &= ~(HIGH << CS2_PIN_NUM))
#define cs3_deselect (CS3_PIN_PORT |= HIGH << CS3_PIN_NUM)
#define cs3_select (CS3_PIN_PORT &= ~(HIGH << CS3_PIN_NUM))

#define read_rs ( (HIGH << RS_PIN_NUM) & RS_PIN_READ)
#define set_type_data (RS_PIN_PORT |= HIGH << RS_PIN_NUM)
#define set_type_cmd (RS_PIN_PORT &= ~(HIGH << RS_PIN_NUM))
#define set_state_read (RW_PIN_PORT |= HIGH << RW_PIN_NUM)
#define set_state_write (RW_PIN_PORT &= ~(HIGH << RW_PIN_NUM))

static uint8_t page_buff[64]; //for library use only. Internal buffer!

//struct for acquiring information about bytes to send per chipId and chipID for start
typedef struct 
{
	uint8_t bytes_per_chip[3];
	uint8_t start_id;
} tx_info_st;

//Gives needed information to functions about starting point, offset for mask and bytes to send;
typedef struct
{
	uint8_t page;
	uint8_t col;
	int8_t offset;
	uint8_t bytes_to_send;
} tx_param_st;

//used for selecting one chip 
static void select_chip(uint8_t ID)
{
	if (TG_left_disp & ID)
		cs1_select;
	if (TG_mid_disp & ID)
		cs2_select;
	if (TG_right_disp & ID)
		cs3_select;
}

//used for deselecting one chip
static void deselect_chip(uint8_t ID)
{
	if (TG_left_disp & ID)
		cs1_deselect;
	if (TG_mid_disp & ID)
		cs2_deselect;
	if (TG_right_disp & ID)
		cs3_deselect;
}

//reads byte from display
static uint8_t get_byte(void)
{
	uint8_t data;
	strobe_enable;
	data = DATA_READ;
	strobe_enable_fast;
	return data;
}

//wait till display ready to write
static void wait_busy(void)
{
	DATA_DDR = INPUT_8BIT;
	DATA_PORT = PULLUP_8BIT;
	set_state_read;
	uint8_t rs_state = read_rs;
	set_type_cmd;
	while (get_byte() & (HIGH << BUSY_FLAG));
	DATA_PORT = LOW;
	DATA_DDR = OUTPUT_8BIT;
	set_state_write;
	if (rs_state)
		set_type_data;
	else
		set_type_cmd;
}

static void send_byte(uint8_t byte)
{
	strobe_enable_fast;
	DATA_PORT = byte;
	strobe_enable_fast;
	wait_busy();
}

static void set_address(uint8_t page, uint8_t col)
{
	set_type_cmd;
	send_byte(0x40 | (0x3F & col));
	send_byte(0xB8 | (0x07 & page));
	set_type_data;
}

/*
Sends data to selected address, chipID for selecting part 1=left 2=middle 4=right
sum for simultaneously turning few segments*/
static void send_data(uint8_t size, const uint8_t * buff)
{
	for (uint8_t i = 0; i < size; i++)
		send_byte(*buff++);
}

/***
test function to fill, chipID for selecting part 1=left 2=middle 4=right
sum for simultaneously turning few segments*/
static void test_fill_page(uint8_t page, uint8_t pattern, uint8_t chipID)
{
	select_chip(chipID);
	set_address(page,0);
	for (uint8_t i = 0; i < 64; i++)
	{
		send_byte(pattern);
	}
	deselect_chip(chipID);
}

/*
Reads data from selected address, chipID for selecting part 1=left 2=middle 4=right
sum for simultaneously turning few segments*/
static void read_data(uint8_t size, uint8_t * buff)
{
	set_state_read;
	DATA_DDR = INPUT_8BIT;
	DATA_PORT = PULLUP_8BIT;
	set_type_data;
	get_byte();
	set_type_cmd;
	while (get_byte() & (HIGH << BUSY_FLAG));
	for (uint8_t i = 0; i < size; i++)
	{
		set_type_data;
		*buff++ = get_byte();
		set_type_cmd;
		while (get_byte() & (HIGH << BUSY_FLAG));
	}
	DATA_PORT = LOW;
	DATA_DDR = OUTPUT_8BIT;
	set_state_write;
	set_type_data;
}

/*
Gives possibility for shift up/down, chipID for selecting part 1=left 2=middle 4=right
sum for simultaneously turning few segments*/
static void set_start_line(uint8_t start, uint8_t chip_id)
{
	set_type_cmd;
	select_chip(chip_id);
	send_byte(0xC0 | start);
	deselect_chip(chip_id);
	set_type_data;
}


/*
Initializes ports for work with display
*/
void TG_init(void)
{
	//initialize MCU interface
	DATA_DDR = OUTPUT_8BIT;
	DATA_PORT = LOW;
	//RS config
	RS_PIN_DDR |= OUTPUT << RS_PIN_NUM;
	RS_PIN_PORT &= ~(HIGH << RS_PIN_NUM);
	//RW config
	RW_PIN_DDR |= OUTPUT << RW_PIN_NUM;
	RW_PIN_PORT &= ~(HIGH << RW_PIN_NUM);
	//Enable config
	E_PIN_DDR |= OUTPUT << E_PIN_NUM;
	E_PIN_PORT &= ~(HIGH << E_PIN_NUM);
	//CS1 config
	CS1_PIN_DDR |= OUTPUT << CS1_PIN_NUM;
	CS1_PIN_PORT &= ~(HIGH << CS1_PIN_NUM);
	//CS2 config
	CS2_PIN_DDR |= OUTPUT << CS2_PIN_NUM;
	CS2_PIN_PORT &= ~(HIGH << CS2_PIN_NUM);
	//CS3 config
	CS3_PIN_DDR |= OUTPUT << CS3_PIN_NUM;
	CS3_PIN_PORT &= ~(HIGH << CS3_PIN_NUM);
	//RES config
	RES_PIN_DDR |= OUTPUT << RES_PIN_NUM;
	RES_PIN_PORT &= ~(HIGH << RES_PIN_NUM);
	
	//initialize display
	strobe_reset;
	set_type_data;
	set_state_write;
	DELAY_MS(35);
	TG_turn_on(0x7);
	set_start_line(0,0x7);
	cs1_deselect;
	cs2_deselect;
	cs3_deselect;
}

void TG_turn_on(uint8_t chip_id)
{
	set_type_cmd;
	select_chip(chip_id);
	send_byte(0x3F);
	deselect_chip(chip_id);
	set_type_data;
}

void TG_turn_off(uint8_t chip_id)
{
	set_type_cmd;
	select_chip(chip_id);
	send_byte(0x3E);
	deselect_chip(chip_id);
	set_type_data;
}

uint8_t TG_get_stat(uint8_t chip_id)
{
	DATA_DDR = INPUT_8BIT;
	DATA_PORT = PULLUP_8BIT;
	set_state_read;
	uint8_t rs_state = read_rs;
	set_type_cmd;
	select_chip(chip_id);
	uint8_t res = get_byte();
	deselect_chip(chip_id);
	DATA_PORT = LOW;
	DATA_DDR = OUTPUT_8BIT;
	set_state_write;
	if (rs_state)
		set_type_data;
	else
		set_type_cmd;
	return res;
}

/************************************************************************/
/* returns number of chip changes needed, and tx_info_st. Takes position
of first pixel (min) and last (max) on X axis        */
/************************************************************************/
//static uint8_t getSendBytesInfo(uint8_t min, uint8_t max, uint8_t * BytesToSendBuff, uint8_t * chipIDStart)
static uint8_t calc_tx_info(uint8_t min, uint8_t max, tx_info_st * info)
{
	info->start_id = min / XPointsPerChip;
	uint8_t chipIDend = (max - 1)/XPointsPerChip;
	uint8_t csChanges = chipIDend - info->start_id;
	
	if (0 == csChanges)
	{
		info->bytes_per_chip[info->start_id] = max - min;
	}
	else if (1 == csChanges)
	{
		info->bytes_per_chip[info->start_id] = (info->start_id + 1) * XPointsPerChip - min;
		info->bytes_per_chip[info->start_id + 1] = max - (info->start_id + 1) *XPointsPerChip;
	}
	else if (2 == csChanges)
	{
		info->bytes_per_chip[info->start_id] = XPointsPerChip - min;
		info->bytes_per_chip[info->start_id + 1] = XPointsPerChip;
		info->bytes_per_chip[info->start_id + 2] = max - 2*XPointsPerChip;
	}
	else
		return BadValue;
		
	return csChanges + 1;
}


/************************************************************************/
/* special function for selecting only one chip with different approach
 1 == leftSeg, 2 == midSeg, 3== rightSeg                                */
/************************************************************************/
static void select_1_chip(uint8_t chip_id)
{
	switch(chip_id)
	{
		case 0 : cs1_select;
				break;
		case 1 : cs2_select;
				break;
		case 2: cs3_select;
				break;
	}
}

/************************************************************************/
/* special function for deselecting only one chip with different approach
 1 == leftSeg, 2 == midSeg, 3== rightSeg                                */
/************************************************************************/
static void deselect_1_chip(uint8_t chip_id)
{
	switch(chip_id)
	{
		case 0 : cs1_deselect;
				break;
		case 1 : cs2_deselect;
				break;
		case 2: cs3_deselect;
				break;
	}
}

/************************************************************************/
/* function to create mask with bits written from MSB                   */
/************************************************************************/
static inline uint8_t make_mask(uint8_t bits)
{
	uint8_t spec_mask = 0x0;
	while(bits--)
	spec_mask |= HIGH << (7 - bits);
	return spec_mask;
}

/************************************************************************/
/* function to create mask with bits written from LSB                   */
/************************************************************************/
static inline uint8_t make_rev_mask(uint8_t bits)
{
	uint8_t spec_mask = 0x0;
	while (bits--)
	{
		spec_mask <<=1;
		spec_mask |= HIGH;
	}
	return spec_mask;
}

/************************************************************************/
/* fills page  from given col by pattern                                */
/************************************************************************/
static inline void fill_page(const tx_param_st * param)
{
	set_address(param->page,param->col);
	for (uint8_t i = 0; i < param->bytes_to_send; i++)
		send_byte(param->offset);
}


/************************************************************************/
/* Clears selected area by mask bits, invert for Up rows.               */
/************************************************************************/
static void clear_page_mask(const tx_param_st * param, uint8_t invert)
{
	set_address(param->page, param->col);
	read_data(param->bytes_to_send,page_buff);
	uint8_t maskUp = make_mask(param->offset);
	if (invert)
		maskUp = ~maskUp;
	for (uint8_t i = 0; i < param->bytes_to_send; i++)
		page_buff[i] = page_buff[i] & maskUp;
	set_address(param->page, param->col);
	send_data(param->bytes_to_send,page_buff);
}

/************************************************************************/
/*Clears display in selected rectangle area that starts at
PointA(posX,posY) and ends at PointB(posX,PosY)						   */
/************************************************************************/
void TG_clear_area(uint8_t A_x, uint8_t A_y, uint8_t B_x, uint8_t B_y)
{
	if (A_x >= XPoints || B_x >= XPoints
	|| A_y >= YPoints || B_y >= YPoints)
	return;
	
	uint8_t set_x, set_y; //coordinates of start point
	uint8_t end_x, end_y; //coordinates of end point
	if (A_x > B_x) // sets coordinates for writing to display from left to right;
	{
		set_x = B_x;
		set_y = B_y;
		end_x = A_x;
		end_y = A_y;
	}
	else
	{
		set_x = A_x;
		set_y = A_y;
		end_x = B_x;
		end_y = B_y;
	}
	uint8_t col_start = set_x%XPointsPerChip;
	uint8_t page_max = end_y/YPointsPerPage;
	uint8_t page_min = set_y/YPointsPerPage;
	uint8_t row_max = (end_y - 1)%YPointsPerPage;
	uint8_t row_min = set_y%YPointsPerPage;
	uint8_t page_changes = page_max - page_min + 1;
	page_max = 0x07 & ~page_max;		//change direction of pages from 7->0, to 0->7
	
	tx_info_st tx_info;
	uint8_t cs_changes = calc_tx_info(set_x,end_x + 1,&tx_info);
	uint8_t col_set;
	uint8_t iteration = 0;
	tx_param_st tx_param;
	while (cs_changes--)
	{
		if (0 == iteration++)
		col_set = col_start;
		else
		col_set = 0;
		
		select_1_chip(tx_info.start_id);
		for(uint8_t i = 0; i < page_changes; i++)
		{
			tx_param.bytes_to_send = tx_info.bytes_per_chip[tx_info.start_id];
			tx_param.col = col_set;
			tx_param.page = page_max + i;
			if (0 == i && row_max != 0)
			{
				tx_param.offset = row_max;
				clear_page_mask(&tx_param, true); //reverse mask 
			}
			else if (0 == i && row_min != 0)
			{
				tx_param.offset = row_min;
				clear_page_mask(&tx_param, false); //don't reverse mask
			}
			else
			{
				tx_param.offset = 0x00; // used as pattern for every byte, not offset!
				fill_page(&tx_param);
			}
		}
		deselect_1_chip(tx_info.start_id++);
	}
}

// used by clearDisplayFull
static inline void send_pattern(uint8_t size, uint8_t pattern)
{
	for (uint8_t i = 0; i < size; i++)
		send_byte(pattern);
}


/************************************************************************/
/* Clears full display                                                  */
/************************************************************************/
void TG_clear_full(void)
{
	uint8_t chip_id = 0;
	while (chip_id < 3)
	{
		select_1_chip(chip_id);
		for (uint8_t i=0; i < 8; i++)
		{
			set_address(i,0);
			send_pattern(XPointsPerChip, 0x0);
		}
		deselect_1_chip(chip_id++);
	}
}

/************************************************************************/
/* Changes states for all pixels using XOR operation                     */
/************************************************************************/
void TG_reverse_all(void)
{
	for (uint8_t chip = 0; chip < 3; chip++)
	{
		select_1_chip(chip);
		for (uint8_t page = 0; page < YPoints/YPointsPerPage; page++)
		{
			set_address(page, 0);
			read_data(XPointsPerChip,page_buff);
			for (uint8_t i = 0; i < XPointsPerChip; i++)
				page_buff[i] ^= 0xFF;
			set_address(page,0);
			for (uint8_t i = 0; i < XPointsPerChip; i++)
				send_byte(page_buff[i]);
		}
		deselect_1_chip(chip);
	}
}

//Creates images using mask based on bits needed to be cleared, work only on selected rows from up or down (selection by reversing mask)
static inline void draw_page_mask(const tx_param_st * param, uint8_t rev, const uint8_t * img_ptr)
{
	set_address(param->page, param->col);
	read_data(param->bytes_to_send,page_buff);
	uint8_t upper_mask = make_mask(param->offset);
	if (rev)
		upper_mask = ~upper_mask;
	for (uint8_t i = 0; i < param->bytes_to_send; i++)
	{
		page_buff[i] = page_buff[i] & upper_mask;
		if (rev)
			page_buff[i] |= (img_ptr[i] << (8 - param->offset) );
		else
			page_buff[i] |= (img_ptr[i] >> param->offset );
	}
	set_address(param->page, param->col);
	send_data(param->bytes_to_send,page_buff);
}

/************************************************************************/
/*Creates images using mask based on bits needed to be cleared and pointers
to last page and page used now, works on all rows*/
/************************************************************************/
static inline void draw_page(const tx_param_st * param, const uint8_t * img_before_ptr, const uint8_t * img_now_ptr)
{
	set_address(param->page, param->col);
	uint8_t top_mask = make_mask(param->offset);
	uint8_t bottom_mask;
	if (param->offset == 0)
		bottom_mask = 0;
	else
		bottom_mask = 8 - param->offset;
	for (uint8_t i = 0; i < param->bytes_to_send; i++)
	{
		page_buff[i] = (img_now_ptr[i] & ~top_mask) >> bottom_mask;
		if (param->offset != 0)
			page_buff[i] |= (img_before_ptr[i] & top_mask) << param->offset;
	}
	send_data(param->bytes_to_send,page_buff);
}

/*
Prints image from buff in given X,Y coordinates with defined sizeX x sizeY image size
*/
void TG_image(uint8_t x, uint8_t y, uint8_t x_size, uint8_t y_size, const uint8_t * img_ptr)
{
	if (x + x_size > XPoints || y + y_size> YPoints)
		return;
	
	uint8_t col_start = x%XPointsPerChip;
	uint8_t page_max = (y + y_size - 1)/YPointsPerPage;
	uint8_t page_min = y/YPointsPerPage;
	uint8_t row_max = (y + y_size)%YPointsPerPage;
	uint8_t row_min = y%YPointsPerPage;
	uint8_t page_changes = page_max - page_min + 1;
	page_max = 0x07 & ~page_max;
	
	tx_info_st tx_info;
	uint8_t cs_changes = calc_tx_info(x,x+x_size,&tx_info);
	uint8_t col_set;
	uint8_t iteration = 0;
	
	uint8_t x_start = 0;
	uint16_t y_start = 0;
	tx_param_st TxInfo;
	while (cs_changes--)
	{
		if (0 == iteration++)
		col_set = col_start;
		else
		col_set = 0;
		
		select_1_chip(tx_info.start_id);
		y_start = 0;
		for(uint8_t i = 0; i < page_changes; i++)
		{
			TxInfo.bytes_to_send = tx_info.bytes_per_chip[tx_info.start_id];
			TxInfo.col = col_set;
			TxInfo.page = page_max + i;
			TxInfo.offset = row_max;
			const uint8_t* cur_buff_ptr  = img_ptr + x_start + y_start;
			if (0 == i && row_max != 0)
				draw_page_mask(&TxInfo, true, cur_buff_ptr);
			else if (page_changes - 1 == i && row_min != 0)
			{
				TxInfo.offset = row_min;
				draw_page_mask(&TxInfo, false, cur_buff_ptr - y_size);
			}
			else
				draw_page(&TxInfo, cur_buff_ptr - y_size, cur_buff_ptr);
			y_start += y_size;
		}
		deselect_1_chip(tx_info.start_id++);
		x_start += TxInfo.bytes_to_send;
	}
}

typedef struct
{
	uint8_t line_type; // if 0 line is more horizontal than vertical, 1 otherwise
	int8_t y_dir; // 1 when A_posY > B_posY, -1 when A_posY < B_posY, 0 when A_posY == B_posY
	uint8_t first_seg_len; // how many bits to write first
	int8_t pixel_changes; //used for counting pixels before change
	uint8_t row_type_ptr; // used when verHor == 1 (indicates how many writes verticaly) CHANGED!!!! //points which type of row is printing now
	int8_t row_type_cnt[3]; // 0 left, 1 middle, 2 right
	uint8_t row_len[2]; // 0 normal, 1 special
} line_param_st;

/************************************************************************/
/* draws line horizontally                                              */
/************************************************************************/
static inline void draw_hor(tx_param_st * param, line_param_st * step)
{
	if (0 == step->y_dir)
	{
		set_address(param->page, param->col);
		read_data(param->bytes_to_send, page_buff);
		for (uint8_t i = 0; i < param->bytes_to_send; i++)
			page_buff[i] |= (1 << param->offset);
		set_address(param->page, param->col);
		send_data(param->bytes_to_send, page_buff);
		param->col = 0;
	}
	else
	{
		uint8_t all_bytes_sent = 0;
		int8_t rows_checked = param->offset;
		uint8_t bytes_to_send = param->bytes_to_send;
		// = step->rowTypePtr;
		uint8_t bytes_sent = 0;
		uint8_t bytes_change = 0;
		uint8_t change = 0; // change from toWrite
		while (all_bytes_sent < bytes_to_send)
		{
			uint8_t tmp_change = 0;
			uint8_t type_ptr = step->row_type_ptr;
			uint8_t row_len;
			//if ( !(PINC & 1 << 7) && step->firstSegLen != 0)
				//PORTB ^= 1 << 0;
			if (0 != step->first_seg_len)
			{
				all_bytes_sent =step->first_seg_len;
				rows_checked += step->y_dir;
				step->first_seg_len = 0;
				if (step->row_type_cnt[step->row_type_ptr] == 1)
					type_ptr++;
				else if (step->row_type_cnt[step->row_type_ptr] > 1)
				{
					tmp_change = 1;
					step->row_type_cnt[step->row_type_ptr]--;
				}
			}
				//allBytesSent += step->rowLen[ 1 == typePtr ? 0 : 1];
				
			while (all_bytes_sent < bytes_to_send)
			{
				row_len = step->row_len [ 1 == type_ptr ? 0 : 1];
				int8_t bytes_to_write = bytes_to_send - all_bytes_sent;
				change = (bytes_to_write%row_len != 0);
				//int8_t tmpRowsCheck = rowsCheck;
				if (1 == step->y_dir)
				{
					if (rows_checked + step->row_type_cnt[type_ptr] > 7)
					{
						all_bytes_sent += row_len * (8 - rows_checked);
						rows_checked += (bytes_to_write/row_len + change);
						break;
					}
					else
					{
						all_bytes_sent +=	step->row_type_cnt[type_ptr] * row_len;
						rows_checked += step->row_type_cnt[type_ptr++];
					}
				}
				else // if (-1 == step->ydir)
				{
					if (rows_checked - step->row_type_cnt[type_ptr] < 0)
					{
						all_bytes_sent += row_len * (rows_checked + 1);
						rows_checked -= (bytes_to_write/row_len) + change;
						
						break;
					}
					else
					{
						all_bytes_sent +=	step->row_type_cnt[type_ptr] * row_len;
						rows_checked -= step->row_type_cnt[type_ptr++];
					}
				}
			}
			if (tmp_change == 1)
			{
				step->row_type_cnt[step->row_type_ptr]++;
				tmp_change = 0;
			}
			if (all_bytes_sent >= bytes_to_send)
			{
				step->first_seg_len = all_bytes_sent - bytes_to_send;
				all_bytes_sent = bytes_to_send;
				//if (step->firstSegLen == 0)
					//_delay_ms(1000);
			}
			bytes_change = all_bytes_sent - bytes_sent;
			set_address(param->page, param->col);
			read_data(bytes_change, page_buff);
			for (uint8_t i = 0; i < bytes_change; i++)
			{
				page_buff[i] |= (1 << param->offset);
				step->pixel_changes--;
				if (0 >= step->pixel_changes)
				{
					step->row_type_cnt[step->row_type_ptr]--;
					if (0 >= step->row_type_cnt[step->row_type_ptr])
						step->row_type_ptr++;
					param->offset += step->y_dir;
					
					step->pixel_changes = step->row_len[ 1 == step->row_type_ptr ? 0 : 1 ];
				}
			}
			set_address(param->page,param->col);
			send_data(bytes_change, page_buff);
			if (0 > rows_checked && !(change && bytes_to_send == all_bytes_sent))
			{
				param->offset = 7;// + (0 == step->pixelsToChange ? 1 : 0);
				rows_checked = param->offset; //?
				param->page--;
			}
			else if (7 < rows_checked && !(change && bytes_to_send == all_bytes_sent))
			{
				param->offset = 0;//  - (0 == step->pixelsToChange ? 1 : 0);
				rows_checked = param->offset;  //?
				param->page++;
			}
			//param->col += allBytesSent - bytesSent;
			bytes_sent = all_bytes_sent;
			param->col = bytes_sent;
			if (step->first_seg_len > step->row_len[ 1 == step->row_type_ptr ? 0 : 1 ])
				step->first_seg_len %= step->row_len[ 1 == step->row_type_ptr ? 0 : 1 ];
		}
		param->col = 0;
	}
}

/************************************************************************/
/* draws line vertically                                              */
/************************************************************************/
static inline void draw_ver(tx_param_st * param, line_param_st * step)
{
	if (1 == param->bytes_to_send) //only true when Ay == By
	{
		while (0 != step->row_len[0])
		{
			uint8_t data;
			set_address(param->page, param->col);
			read_data(1, &data);
			while (param->offset >= 0 && param->offset <= 7)
			{
				data |= (1 << param->offset);
				param->offset += step->y_dir;
				step->row_len[0]--;
				if (0 == step->row_len[0])
					break;
			}
			set_address(param->page,param->col);
			send_byte(data);
			if (param->offset < 0)
			{
				param->offset = 7;
				param->page--;
			}
			else if (param->offset > 7)
			{
				param->offset = 0;
				param->page++;
			}
		}
	}
	else
	{
		uint8_t row_len = step->row_len[ 1 == step->row_type_ptr ? 0 : 1];
		uint8_t bytes_sent = 0;
		while (bytes_sent < param->bytes_to_send)
		{
			uint8_t data;
			set_address(param->page, param->col);
			read_data(1,&data);
			while(param->offset >= 0 && param->offset <= 7 && row_len != 0)
			{
				data |= (1 << param->offset);
				param->offset += step->y_dir;
				row_len--;
			}
			set_address(param->page, param->col);
			send_byte(data);
			if (0 == row_len)
			{
				step->row_type_cnt[step->row_type_ptr]--;
				bytes_sent++;
				param->col++;
				if (0 == step->row_type_cnt[step->row_type_ptr])
				{
					step->row_type_ptr++;
					if (step->row_type_ptr > 2)
						break;
				}
				if (step->row_type_cnt[step->row_type_ptr] > 0)
					row_len = step->row_len[1 == step->row_type_ptr ? 0 : 1];
			}
			if (param->offset < 0)
			{
				param->offset = 7;
				param->page--;
			}
			else if (param->offset > 7)
			{
				param->offset = 0;
				param->page++;
			}
		}
		param->col = 0;
	}
}

/************************************************************************/
/* Draws line on given chip display using drawVer() and drawHor() fun   */
/************************************************************************/
static inline void line_select_fun(tx_param_st * tx_param, line_param_st * line_param )
{
	if (line_param->line_type)
		draw_ver(tx_param, line_param);
	else
		draw_hor(tx_param, line_param);
}

/*
Draws line from pointA to pointB
*/
void TG_line(uint8_t A_x, uint8_t A_y, uint8_t B_x, uint8_t B_y)
{
	//if values above max value, don't execute cmd
	if (A_x >= XPoints || B_x >= XPoints
		|| A_y >= YPoints || B_y >= YPoints)
		return;
	
	uint8_t set_x, set_y; //coordinates of start point
	uint8_t end_x, end_y; //coordinates of end point
	if (A_x > B_x) // sets coordinates for writing to display from left to right;
	{
		set_x = B_x;
		set_y = B_y;
		end_x = A_x;
		end_y = A_y;
	}
	else
	{
		set_x = A_x;
		set_y = A_y;
		end_x = B_x;
		end_y = B_y;
	}
	
	line_param_st line_param;
	//check direction of writing pages, -1 => form 0 ~ 7 if 1 => from 7 ~ 0, if 0 don't change;
	line_param.y_dir = end_y - set_y ? (end_y - set_y < 0) ? 1 : -1 : 0; // if endY == setY yDir = 0, else if result negative yDir = 1, else yDir = -1
	uint8_t dots_x = end_x - set_x + 1;
	int8_t dots_y = end_y - set_y;
	if (dots_y < 0)
		dots_y  = -1 * dots_y;
	dots_y++;
	if (dots_y > dots_x)
	{
		line_param.row_len[0] = dots_y / dots_x;
		line_param.row_len[1] = line_param.row_len[0] + 1;
		line_param.row_type_cnt[0] = line_param.row_type_cnt[2] = (dots_y % dots_x) / 2;
		if ( (dots_y % dots_x) % 2 != 0)
			line_param.row_type_cnt[2]++;
		if (0 != line_param.row_type_cnt[0])
			line_param.row_type_ptr = 0;
		else
			line_param.row_type_ptr = 1;
		line_param.row_type_cnt[1] = dots_x - line_param.row_type_cnt[0] - line_param.row_type_cnt[2];
		line_param.line_type = 1;
	}
	else
	{
		line_param.row_len[0] = dots_x / dots_y;
		line_param.row_len[1] = line_param.row_len[0] + 1;
		line_param.row_type_cnt[0] = (dots_x % dots_y) / 2;
		line_param.row_type_cnt[2] = (dots_x % dots_y) / 2;
		if ( (dots_x % dots_y) % 2 != 0)
			line_param.row_type_cnt[2]++;
		if (0 != line_param.row_type_cnt[0])
			line_param.row_type_ptr = 0;
		else
			line_param.row_type_ptr = 1;
		line_param.row_type_cnt[1] = dots_y - line_param.row_type_cnt[0] - line_param.row_type_cnt[2];
		line_param.line_type = 0;
	}
	line_param.first_seg_len = 1 == line_param.row_type_ptr ? line_param.row_len[0] : line_param.row_len[1]; //if special rows, write special pixels length else normal length
	line_param.pixel_changes = line_param.first_seg_len;
	//gets information about chipID write sequence
	tx_info_st tx_info;
	end_x++;
	uint8_t cs_changes = calc_tx_info(set_x,end_x, &tx_info);
	end_x--; //refactor
	tx_param_st txInfo; //offset used for row number
	txInfo.offset = set_y % YPointsPerPage;
	txInfo.offset = 0x7 & ~txInfo.offset;
	txInfo.page = set_y / YPointsPerPage;
	txInfo.page = 0x07 & ~txInfo.page;		//change direction of pages from 7->0, to 0->7
	txInfo.col = set_x % XPointsPerChip;
	while (cs_changes--)
	{
		txInfo.bytes_to_send = tx_info.bytes_per_chip[tx_info.start_id];
		select_1_chip(tx_info.start_id);
		line_select_fun(&txInfo,&line_param);
		deselect_1_chip(tx_info.start_id++);
	}
}


/*
Draws desired rectangle
*/
void TG_rectangle(uint8_t x, uint8_t y, uint8_t x_size, uint8_t y_size)
{
	TG_line(x, x, x, y + y_size);
	TG_line(x, y + y_size, x + x_size, y + y_size);
	TG_line(x + x_size, y + y_size, x + x_size, y);
	TG_line(x + x_size, y, x, y);
}


/************************************************************************/
/* Writes text from (posX,posY) with given height of letters  and space */
/************************************************************************/
void TG_printf(uint8_t x, uint8_t y, uint8_t height, uint8_t space, const char * txt)
{
	const uint8_t (*font_ptr)[5];
	uint8_t font_width = 0;
	uint8_t font_height = 0;
	if (height == 7)
	{
		font_ptr = default_f;
		font_height = 8;
		font_width = 5;
	}
	while (*txt != '\0')
	{
		if (y + font_height >= YPoints)
			y -= (y + font_height - YPoints);
		if (x + font_width >= XPoints || (*txt == '\n'))
		{
			x = 0;
			y -= font_height;
		}
		TG_image(x, y, font_width, font_height, font_ptr[(uint8_t)*txt]);
		x += font_width + space;
		txt++;
	}
}

/*
Prints test data on display
*/
void TG_test(void)
{
	for(int i =0; i < 8; i++)
	{
		test_fill_page(i,make_mask(i),TG_left_disp);
		test_fill_page(i,0xFF,TG_mid_disp);
		test_fill_page(i,0xFF,TG_right_disp);
	}
	set_start_line(0,0x7);
	DELAY_MS(1000);
	for (uint8_t i =0; i < 8; i++)
	{
		select_1_chip(0);
		set_address(i,0);
		read_data(64,page_buff);
		deselect_1_chip(0);
		select_1_chip(2);
		set_address(i,0);
		send_data(64,page_buff);
		deselect_1_chip(2);
	}
	DELAY_MS(1000);
	select_1_chip(0);
	for (uint8_t i =0; i < 8; i++)
	{
		set_address(i,0);
		read_data(32,page_buff);
		for (uint8_t y=0; y < 32; y++)
			page_buff[y] = ~page_buff[y];
		set_address(i,0);
		send_data(32,page_buff);
	}
	deselect_1_chip(0);
	DELAY_MS(1000);
	TG_turn_off(TG_mid_disp);
	TG_clear_area(2,2,188,60);
	DELAY_MS(1000);
	TG_reverse_all();
	TG_turn_on(TG_mid_disp);
}
