## قسمت دوم

راه اندازی LCD کاراکتری با مبدل I2C

---

## I2C LCD Library Functions

This library provides functions to control LCD displays communicating via the I2C protocol.

### Core Functions

* `lcd_init()`: This function initializes the LCD for 4-bit operation, a two-line display, and a 5x8 pixel font. It also performs initial setup steps such as turning off the display, clearing the display, setting the entry mode, and turning on the display.

* `lcd_send_cmd(uint8_t cmd)`: This function sends a control command to the LCD. Commands are used to set the display mode, clear the screen, set the cursor position, etc.

* `lcd_send_data(uint8_t data)`: This function sends a byte of data to the LCD for display. This is typically used to send the ASCII code of characters you want to display.

* `lcd_clear()`: This function clears the LCD screen and returns the cursor to the home position (row 0, column 0).

* `lcd_put_cursor(int row, int col)`: This function sets the cursor position on the LCD.
    * `row`: The row number (0 for the first row, 1 for the second row).
    * `col`: The column number (starting from 0).

* `lcd_send_string(char *str)`: This function sends and displays a string of characters to the LCD. It sends characters one by one until it encounters a null terminator (`\0`) in the string.

* `scan_i2c_address()`: This function scans the I2C bus for available devices and prints the addresses where a device responds. This is useful for finding the I2C address of your LCD.

### Custom Functions Added to the Library

This section details the functions that have been added to the library to provide enhanced capabilities for interacting with the LCD display.

* `lcd_send_custom_char(uint8_t location, const uint8_t *charmap)`: This function is used to send a custom character pattern to the LCD's CGRAM (Character Generator RAM) and display it.
    * `location`: An 8-bit integer specifying the memory location in CGRAM where the custom character will be stored (values 0 to 7).
    * `charmap`: A pointer to a constant 8-byte array that defines the pixel pattern of the custom character. Each byte in the array represents a row of pixels.

    **How to Use:**
    ```c
    const uint8_t customChar[] = {
      0b00000,
      0b00100,
      0b01010,
      0b00100,
      0b00100,
      0b01110,
      0b10001,
      0b00000
    };

    lcd_send_custom_char(0, customChar); // Send the custom character to location 0
    lcd_put_cursor(0, 0);
    lcd_send_data(0); // Display the character stored at location 0
    ```

* `lcd_scroll_text(const char *text, uint8_t row, uint16_t delay_ms)`: This function scrolls a long text horizontally from left to right across a specified row of the LCD.
    * `text`: A pointer to the string that needs to be scrolled.
    * `row`: The row number on the LCD where the text will be scrolled (typically 0 or 1).
    * `delay_ms`: The delay time in milliseconds between each scroll step. This value determines the scrolling speed.

    **How to Use:**
    ```c
    lcd_scroll_text("This is a long scrolling text!", 0, 200);
    ```
    If the length of the text is less than or equal to the number of LCD columns, the text will simply be displayed on the specified row without scrolling.

* `lcd_scroll_text_from_right(const char *text, uint8_t row, uint16_t delay_ms)`: This function scrolls a text horizontally from right to left across a specified row of the LCD. The text starts off-screen from the right and gradually scrolls into view on the left.
    * `text`: A pointer to the string that needs to be scrolled from right to left.
    * `row`: The row number on the LCD where the text will be scrolled (typically 0 or 1).
    * `delay_ms`: The delay time in milliseconds between each scroll step. This value determines the scrolling speed.

    **How to Use:**
    ```c
    lcd_scroll_text_from_right("Text scrolling from right!", 1, 150);
    ```
    If the length of the text is less than or equal to the number of LCD columns, the text will be displayed right-aligned on the specified row without scrolling.


