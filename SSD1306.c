#include "SSD1306.h"

void SSD1306_Initialize(SSD1306_t *dev, I2C_HandleTypeDef *i2cHandle, uint16_t address, uint8_t width, uint8_t height, uint8_t offset_x, uint8_t should_mirrror_vertical, uint8_t should_mirrror_horizontal, uint8_t should_inverse_color) {
	dev->i2cHandle = i2cHandle;
	dev->address = address << 1; // shift by one to make room for read/write bit
	dev->width = width;
	dev->height = height;
	dev->offset_x = offset_x;
	dev->should_mirrror_vertical = should_mirrror_vertical;
	dev->should_mirrror_horizontal = should_mirrror_horizontal;
	dev->should_inverse_color = should_inverse_color;

	dev->buffer_size = dev->width * dev->height / 8;

	// set up screen buffer
	dev->buffer = malloc(dev->buffer_size * sizeof(uint8_t));

	// initialize the OLED
	SSD1306_SetDisplayOn(dev, 0); //display off

	// 00b, Horizontal Addressing Mode; 01b, Vertical Addressing Mode;
	// 10b, Page Addressing Mode (RESET); 11b, Invalid
	SSD1306_WriteCommand(dev, 0x20); // set Memory Addressing Mode
	SSD1306_WriteCommand(dev, 0x00);

	SSD1306_WriteCommand(dev, 0xB0); // set Page Start Address for Page Addressing Mode,0-7

	if (dev->should_mirrror_vertical) {
		SSD1306_WriteCommand(dev, 0xC0); // Mirror vertically
	} else {
		SSD1306_WriteCommand(dev, 0xC8); // set COM Output Scan Direction
	}

	SSD1306_WriteCommand(dev, 0x00); // set low column address
	SSD1306_WriteCommand(dev, 0x10); // set high column address

	SSD1306_WriteCommand(dev, 0x40); // set start line address

	SSD1306_SetContrast(dev, 0xFF);

	if (dev->should_mirrror_horizontal) {
		SSD1306_WriteCommand(dev, 0xA0); // mirror horizontally
	} else {
		SSD1306_WriteCommand(dev, 0xA1); // set segment re-map 0 to 127
	}

	if (dev->should_inverse_color) {
		SSD1306_WriteCommand(dev, 0xA7); // set inverse color
	} else {
		SSD1306_WriteCommand(dev, 0xA6); // set normal color
	}

	if (dev->height == 128) {
		SSD1306_WriteCommand(dev, 0xFF); // found in the Luma Python lib for SH1106.
	} else {
		SSD1306_WriteCommand(dev, 0xA8); // set multiplex ratio(1 to 64)
	}

	if (dev->height == 32) {
		SSD1306_WriteCommand(dev, 0x1F);
	} else if (dev->height == 64) {
		SSD1306_WriteCommand(dev, 0x3F);
	} else {
		SSD1306_WriteCommand(dev, 0x3F);
	}

	SSD1306_WriteCommand(dev, 0xA4); // 0xa4,Output follows RAM content;0xa5,Output ignores RAM content

	SSD1306_WriteCommand(dev, 0xD3); // set display offset
	SSD1306_WriteCommand(dev, 0x00); // not offset

	SSD1306_WriteCommand(dev, 0xD5); // set display clock divide ratio/oscillator frequency
	SSD1306_WriteCommand(dev, 0xF0); // set divide ratio

	SSD1306_WriteCommand(dev, 0xD9); // set pre-charge period
	SSD1306_WriteCommand(dev, 0x22);

	SSD1306_WriteCommand(dev, 0xDA); // set com pins hardware configuration

	if (dev->height == 32) {
		SSD1306_WriteCommand(dev, 0x02);
	} else if (dev->height == 64) {
		SSD1306_WriteCommand(dev, 0x12);
	} else {
		SSD1306_WriteCommand(dev, 0x12);
	}

	SSD1306_WriteCommand(dev, 0xDB); // set vcomh
	SSD1306_WriteCommand(dev, 0x20); // 0x20,0.77xVcc

	SSD1306_WriteCommand(dev, 0x8D); // set DC-DC enable
	SSD1306_WriteCommand(dev, 0x14);
	SSD1306_SetDisplayOn(dev, 1); // turn on SSD1306 panel

	// Clear screen
	SSD1306_Fill(dev, SSD1306_COLOR_BLACK);

	// Flush buffer to screen
	SSD1306_UpdateScreen(dev);

	// Set default values for screen object
	dev->cur_x = 0;
	dev->cur_y = 0;

	dev->is_initialized = 1;
}

void SSD1306_SetContrast(SSD1306_t *dev, uint8_t value) {
	SSD1306_WriteCommand(dev, 0x81); // contrast control register
	SSD1306_WriteCommand(dev, value);
}

void SSD1306_SetDisplayOn(SSD1306_t *dev, uint8_t on) {
	if (on) {
		SSD1306_WriteCommand(dev, 0xAF);
	} else {
		SSD1306_WriteCommand(dev, 0xAE);
	}
}

void SSD1306_FillBuffer(SSD1306_t *dev, uint8_t* buf, uint32_t len) {
	if (len <= dev->buffer_size) {
		memcpy(dev->buffer, buf, len);

		dev->is_dirty = 1;
	}
}

void SSD1306_Fill(SSD1306_t *dev, SSD1306_Color_t color) {
	for (uint16_t i = 0; i < dev->buffer_size; i++) {
		dev->buffer[i] = (color == SSD1306_COLOR_BLACK) ? 0x00 : 0xFF;
	}

	dev->is_dirty = 1;
}

void SSD1306_UpdateScreen(SSD1306_t *dev) {
	// only proceed if frame buffer is dirty
	if (!dev->is_dirty) {
		return;
	}

	// Write data to each page of RAM. Number of pages
	// depends on the screen height:
	//
	//  * 32px   ==  4 pages
	//  * 64px   ==  8 pages
	//  * 128px  ==  16 pages
	for (uint8_t i = 0; i < dev->height / 8; i++) {
		SSD1306_WriteCommand(dev, 0xB0 + i); // Set the current RAM page address.
		SSD1306_WriteCommand(dev, 0x00);
		SSD1306_WriteCommand(dev, 0x10);
		SSD1306_WriteData(dev, &(dev->buffer[dev->width * i]), dev->width);
	}

	dev->is_dirty = 0;
}

void SSD1306_DrawPixel(SSD1306_t *dev, uint8_t x, uint8_t y, SSD1306_Color_t color) {
	// Don't write outside the buffer
	if (x >= dev->width || y >= dev->height) {
		return;
	}

	// Draw in the right color
	if (color == SSD1306_COLOR_WHITE) {
		dev->buffer[x + (y / 8) * dev->width] |= 1 << (y % 8);
	} else {
		dev->buffer[x + (y / 8) * dev->width] &= ~(1 << (y % 8));
	}

	dev->is_dirty = 1;
}

char SSD1306_DrawChar(SSD1306_t *dev, char ch, SSD1306_Font_t font, SSD1306_Color_t color) {
	// Check remaining space
	if (dev->width < (dev->cur_x + font.char_width) || dev->height < (dev->cur_y + font.char_height)) {
		// Not enough space on current line
		return 0;
	}

	// Check if character is valid
	if (ch > 126) {
		return 0;
	}

	// check if normal, special or no valid char
	const uint16_t *char_start = NULL;

	if (ch < 32) {
		if (ch < SSD1606_FONT_START_SPECIAL_CHARS || ch >= font.s_data_amount + SSD1606_FONT_START_SPECIAL_CHARS) {
			return 0;
		}

		char_start = font.s_data + (ch - SSD1606_FONT_START_SPECIAL_CHARS) * font.char_height;
	} else {
		char_start = font.data + (ch - 32) * font.char_height;
	}


	// Use the font to write
	for (uint8_t i = 0; i < font.char_height; i++) {
		uint16_t b = *(char_start + i);

		for (uint8_t j = 0; j < font.char_width; j++) {
			if((b << j) & 0x8000)  {
				SSD1306_DrawPixel(dev, dev->cur_x + j, (dev->cur_y + i), (SSD1306_Color_t) color);
			} else {
				SSD1306_DrawPixel(dev, dev->cur_x + j, (dev->cur_y + i), (SSD1306_Color_t) !color);
			}
		}
	}

	// The current space is now taken
	dev->cur_x += font.char_width;

	// Return written char for validation
	return ch;
}

char SSD1306_DrawString(SSD1306_t *dev, char* str, SSD1306_Font_t font, SSD1306_Color_t color) {
	while (1) {
		if (*str == '\0') {
			break;
		}

		if (SSD1306_DrawChar(dev, *str, font, color) != *str) {
			break;
		}

		str++;
	}

	return *str;
}

void SSD1306_DrawBitmap(SSD1306_t *dev, SSD1306_Bitmap_t bitmap, SSD1306_Color_t color) {
	// Check remaining space
	if (dev->width < (dev->cur_x + bitmap.width) || dev->height < (dev->cur_y + bitmap.height)) {
		return;
	}

	uint8_t bytes_per_row = (bitmap.width + 7) / 8; // Bitmap scan line pad = whole byte
	uint8_t byte = 0;

	for (uint8_t j = 0; j < bitmap.height; j++) {
		for (uint8_t i = 0; i < bitmap.width; i++) {
			if (i & 7) {
				byte <<= 1;
			} else {
				byte = (*(uint8_t *)(&bitmap.data[j * bytes_per_row + i / 8]));
			}

			if (byte & 0x80) {
				SSD1306_DrawPixel(dev, dev->cur_x + i, dev->cur_y + j, (SSD1306_Color_t) color);
			} else {
				SSD1306_DrawPixel(dev, dev->cur_x + i, dev->cur_y + j, (SSD1306_Color_t) !color);
			}
		}
	}

	// The current space is now taken
	dev->cur_x += bitmap.width;
}

void SSD1306_SetCursor(SSD1306_t *dev, uint8_t x, uint8_t y) {
	dev->cur_x = x;
	dev->cur_y = y;
}

void SSD1306_DrawLine(SSD1306_t *dev, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, SSD1306_Color_t color) {
	uint8_t delta_x = abs(x2 - x1);
	uint8_t delta_y = abs(y2 - y1);

	int8_t sign_x = ((x1 < x2) ? 1 : -1);
	int8_t sign_y = ((y1 < y2) ? 1 : -1);

	int16_t error_1 = delta_x - delta_y;
	int16_t error_2 = 0;

	SSD1306_DrawPixel(dev, x2, y2, color);

	while ((x1 != x2) || (y1 != y2)) {
		SSD1306_DrawPixel(dev, x1, y1, color);

		error_2 = error_1 * 2;

		if (error_2 > -delta_y) {
			error_1 -= delta_y;
			x1 += sign_x;
		}

		if (error_2 < delta_x) {
			error_1 += delta_x;
			y1 += sign_y;
		}
	}

	return;
}

void SSD1306_DrawPolyline(SSD1306_t *dev, SSD1306_Vertex_t *par_vertex, uint16_t par_size, SSD1306_Color_t color) {
	if(par_vertex == 0) {
		return;
	}

	for(uint16_t i = 1; i < par_size; i++) {
		SSD1306_DrawLine(dev, par_vertex[i - 1].x, par_vertex[i - 1].y, par_vertex[i].x, par_vertex[i].y, color);
	}
}

void SSD1306_DrawCircle(SSD1306_t *dev, uint8_t x, uint8_t y, uint8_t radius, SSD1306_Color_t color) {
	if (x >= dev->width || y >= dev->height) {
		return;
	}

	int16_t delta_x = -radius;
	int16_t delta_y = 0;
	int16_t error_1 = 2 - 2 * radius;
	int16_t error_2 = 0;

	do {
		SSD1306_DrawPixel(dev, x - delta_x, y + delta_y, color);
		SSD1306_DrawPixel(dev, x + delta_x, y + delta_y, color);
		SSD1306_DrawPixel(dev, x + delta_x, y - delta_y, color);
		SSD1306_DrawPixel(dev, x - delta_x, y - delta_y, color);

		error_2 = error_1;

		if (error_2 <= delta_y) {
			delta_y++;
			error_1 = error_1 + (delta_y * 2 + 1);

			if (-delta_x == delta_y && error_2 <= delta_x) {
				error_2 = 0;
			}
		}

		if (error_2 > delta_x) {
			delta_x++;
			error_1 = error_1 + (delta_x * 2 + 1);
		}
	} while (delta_x <= 0);
}

void SSD1306_DrawRectangle(SSD1306_t *dev, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, SSD1306_Color_t color) {
	SSD1306_DrawLine(dev, x1, y1, x2, y1, color);
	SSD1306_DrawLine(dev, x2, y1, x2, y2, color);
	SSD1306_DrawLine(dev, x2, y2, x1, y2, color);
	SSD1306_DrawLine(dev, x1, y2, x1, y1, color);
}

// FONT FUNCTIONS

uint8_t SSD1606Font_AddSpecialChar(SSD1306_Font_t *font, const uint16_t *s_char) {
	if (font->s_data_amount >= SSD1606_FONT_MAX_SPECIAL_CHARS) {
		return 0;
	}

	// copy to buffer
	uint16_t offset = font->s_data_amount * font->char_height;

	for (uint16_t i = 0; i < font->char_height; i++) {
		font->s_data[offset + i] = s_char[i];
	}

	font->s_data_amount++;

	return font->s_data_amount - 1 + SSD1606_FONT_START_SPECIAL_CHARS;
}

// HELPERS //

void SSD1306_WriteCommand(SSD1306_t *dev, uint8_t data) {
	HAL_I2C_Mem_Write(dev->i2cHandle, dev->address, 0x00, 1, &data, 1, HAL_MAX_DELAY);
}

void SSD1306_WriteData(SSD1306_t *dev, uint8_t* buffer, size_t buff_size) {
	HAL_I2C_Mem_Write(dev->i2cHandle, dev->address, 0x40, 1, buffer, buff_size, HAL_MAX_DELAY);
}
