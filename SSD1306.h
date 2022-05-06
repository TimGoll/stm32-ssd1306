/**
  ******************************************************************************
  * @file           SSD1306.h
  * @author         Tim Goll, Aleksander Alekseev (afiskon), Olivier Van den Fede (4ilo)
  * @date           14.04.2022
  * @brief          Handles the communication with OLED displays via I2C.
  ******************************************************************************
  * This library provides the interface and helper functions to write characters,
  * lines and bitmaps onto a compatible display.
  *
  ******************************************************************************
  */

#pragma once

#include "stm32h7xx_hal.h"

#include "SSD1306_fonts.h"

#include <string.h> // For memcpy
#include <math.h> // for abs
#include <stdlib.h> // for malloc

// Enumeration for screen colors
typedef enum {
	SSD1306_COLOR_BLACK = 0x00, // SSD1306_COLOR_BLACK color, no pixel
	SSD1306_COLOR_WHITE = 0x01  // Pixel is set. Color depends on OLED
} SSD1306_Color_t;

/**
 * @brief A struct that has all display specific data.
 */
typedef struct {
	I2C_HandleTypeDef *i2cHandle; ///< I2C handle
	uint16_t address; ///< The address of the specific i2c device

	uint8_t width; ///< The width of the display
	uint8_t height; ///< The height of the display

	uint8_t offset_x; ///< Horizontal offset; isn't needed for most displays

	uint8_t should_mirrror_vertical; ///< Mirrors the screen content vertically if needed
	uint8_t should_mirrror_horizontal; ///< Mirrors the screen content horizontally if needed
	uint8_t should_inverse_color; ///< Inverses the color if needed

	uint8_t cur_x; ///< The current horizontal pixel coordinate
	uint8_t cur_y; ///< The current vertical pixel coordinate

	uint16_t buffer_size; ///< The size of the buffer

	uint8_t *buffer; ///< A pointer to an array with a frame buffer

	uint8_t is_dirty; ///< If a pixel got updated since the last screen update, the buffer is dirty
	uint8_t is_initialized; ///< Holds the initialization state
} SSD1306_t;

/**
 * @brief A data type that is a vertex with two coordinates.
 */
typedef struct {
	uint8_t x; ///< The horizontal position
	uint8_t y; ///< The vertical position
} SSD1306_Vertex_t;

/**
 * @brief A symbol data type.
 */
typedef struct {
	const uint8_t width; ///< Bitmap width in pixels
	const uint8_t height; ///< Bitmap height in pixels
	const uint8_t *data; ///< Pointer to bitmap data array
} SSD1306_Bitmap_t;

/**
 * Initializes one external display. The set data is then added to the displays data struct.
 *
 * @param [in] *dev The display data struct that contains everything regarding the specific display instance
 * @param [in] *i2cHandle The handler to the i2c port which is connected to the external display
 * @param [in] address The 7-bit i2c address
 * @param [in] width The width of the display
 * @param [in] height The height of the display
 * @param [in] offset_x Horizontal offset
 * @param [in] should_mirrror_vertical Mirrors the screen content vertically
 * @param [in] should_mirrror_horizontal Mirrors the screen content horizontally
 * @param [in] should_inverse_color Inverses the color if needed
 */
void SSD1306_Initialize(SSD1306_t *dev, I2C_HandleTypeDef *i2cHandle, uint16_t address, uint8_t width, uint8_t height, uint8_t offset_x, uint8_t should_mirrror_vertical, uint8_t should_mirrror_horizontal, uint8_t should_inverse_color);

/**
 * Sets the contrast of the display. Contrast increases as the value increases.
 *
 * @param [in] *dev The display data struct that contains everything regarding the specific display instance
 * @param[in] value The contrast to set
 * @note RESET = 7Fh.
 */
void SSD1306_SetContrast(SSD1306_t *dev, uint8_t value);

/**
 * Sets Display ON/OFF.
 *
 * @param [in] *dev The display data struct that contains everything regarding the specific display instance
 * @param[in] on The display state, 0 for OFF, any for ON
 * @note RESET = 7Fh.
 */
void SSD1306_SetDisplayOn(SSD1306_t *dev, uint8_t on);

/**
 * Fills the screen buffer with values from a given buffer of a fixed length.
 *
 * @param [in] *dev The display data struct that contains everything regarding the specific display instance
 */
void SSD1306_FillBuffer(SSD1306_t *dev, uint8_t* buf, uint32_t len);

/**
 * Fills the whole screen with the given color.
 *
 * @param [in] *dev The display data struct that contains everything regarding the specific display instance
 */
void SSD1306_Fill(SSD1306_t *dev, SSD1306_Color_t color);

/**
 * Writes the screen buffer with changed to the screen.
 *
 * @param [in] *dev The display data struct that contains everything regarding the specific display instance
 */
void SSD1306_UpdateScreen(SSD1306_t *dev);

/**
 * Draws one pixel in the screen buffer.
 *
 * @param [in] *dev The display data struct that contains everything regarding the specific display instance
 */
void SSD1306_DrawPixel(SSD1306_t *dev, uint8_t x, uint8_t y, SSD1306_Color_t color);

/**
 * Draws one char to the screen buffer.
 *
 * @param [in] *dev The display data struct that contains everything regarding the specific display instance
 *
 * @retval char Returns the character if successful, 0 if not
 */
char SSD1306_DrawChar(SSD1306_t *dev, char ch, SSD1306_Font_t font, SSD1306_Color_t color);

/**
 * Writes full string to screen buffer.
 *
 * @param [in] *dev The display data struct that contains everything regarding the specific display instance
 *
 * @retval char Returns the first character of the string after successful write
 */
char SSD1306_DrawString(SSD1306_t *dev, char* str, SSD1306_Font_t font, SSD1306_Color_t color);

void SSD1306_DrawBitmap(SSD1306_t *dev, SSD1306_Bitmap_t bitmap, SSD1306_Color_t color);

/**
 * Sets the cursor position to the provided values.
 *
 * @param [in] *dev The display data struct that contains everything regarding the specific display instance
 */
void SSD1306_SetCursor(SSD1306_t *dev, uint8_t x, uint8_t y);

/**
 * Draws a line using the Bresenhem's algorithm.
 *
 * @param [in] *dev The display data struct that contains everything regarding the specific display instance
 */
void SSD1306_DrawLine(SSD1306_t *dev, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, SSD1306_Color_t color);

/**
 * Draws a poly line.
 *
 * @param [in] *dev The display data struct that contains everything regarding the specific display instance
 */
void SSD1306_DrawPolyline(SSD1306_t *dev, SSD1306_Vertex_t *par_vertex, uint16_t par_size, SSD1306_Color_t color);

/**
 * Draws a circle using the Bresenhem's algorithm.
 *
 * @param [in] *dev The display data struct that contains everything regarding the specific display instance
 */
void SSD1306_DrawCircle(SSD1306_t *dev, uint8_t x, uint8_t y, uint8_t radius, SSD1306_Color_t color);

/**
 * Draws a rectangle.
 *
 * @param [in] *dev The display data struct that contains everything regarding the specific display instance
 */
void SSD1306_DrawRectangle(SSD1306_t *dev, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, SSD1306_Color_t color);

uint8_t SSD1606Font_AddSpecialChar(SSD1306_Font_t *font, const uint16_t *s_char);

/**
 * Sends a byte to the command register.
 *
 * @intfunction
 *
 * @param [in] *dev The display data struct that contains everything regarding the specific display instance
 */
void SSD1306_WriteCommand(SSD1306_t *dev, uint8_t data);

/**
 * Writes a sequence of bytes to the display.
 *
 * @intfunction
 *
 * @param [in] *dev The display data struct that contains everything regarding the specific display instance
 */
void SSD1306_WriteData(SSD1306_t *dev, uint8_t* buffer, size_t buff_size);
