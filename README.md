# SSD1306 Library for STM32

This library is heavily [based on the library by afiskin](https://github.com/afiskon/stm32-ssd1306). It was modified to be more dynamic in regards to support of multiple displays at once. While doing so, I got rid of most of the `#define`s.

To use a display, first a new variable of the type `SSD1306_t` has to be created. Then the initialize function can be called. The setup looks like this:

```C
// define struct for OLED display
SSD1306_t ssd1306;

SSD1306_Initialize(&ssd1306, &hi2c1, 0x3C, 128, 32, 0, 0, 0, 0);
```

The reference of said variable is then passed to the initialize function, together with a reference to the I2C object. It is followed by the address (`0x3C` is the default address of most modules) and the dimensions of the display. The following four zeros are used for specific config. More info on that can be found in the header filer.

To put content on the screen any draw function ca be used. To update it to the real display the function `SSD1306_UpdateScreen` has to be called. This function pushes the frame buffer to the display. Calling the function without changing the buffer beforehand does nothing.

A full example would look like this:

```C
// define struct for OLED display
SSD1306_t ssd1306;

SSD1306_Initialize(&ssd1306, &hi2c1, 0x3C, 128, 32, 0, 0, 0, 0);
SSD1306_DrawLine(&ssd1306, 2, 2, 55, 2, SSD1306_COLOR_WHITE);
SSD1306_UpdateScreen(&ssd1306);
```

Before fonts can be used, they have to be enabled. Use defines to enable their setup:

```C
#define SSD1306_INCLUDE_FONT_6x8 // creates the 6x8 font
#define SSD1306_INCLUDE_FONT_7x10 // creates the 7x10 font
#define SSD1306_INCLUDE_FONT_11x18 // creates the 11x18 font
#define SSD1306_INCLUDE_FONT_16x26 // creates the 16x26 font
```

## Documentation

The documentation can be found in the [header file `SSD1306.h`](SSD1306.h).

## License

[MIT](LICENSE)
