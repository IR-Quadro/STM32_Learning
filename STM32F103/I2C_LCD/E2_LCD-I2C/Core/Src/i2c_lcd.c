#include "i2c_lcd.h"
#include "i2c.h"

// Function to initialize the LCD
void lcd_init(void)
{
    // 4-bit initialization
    HAL_Delay(50);  // Wait for >40ms
    lcd_send_cmd(0x30);
    HAL_Delay(5);   // Wait for >4.1ms
    lcd_send_cmd(0x30);
    HAL_Delay(1);   // Wait for >100us
    lcd_send_cmd(0x30);
    HAL_Delay(10);
    lcd_send_cmd(0x20);  // 4-bit mode
    HAL_Delay(10);

    // Display initialization
    lcd_send_cmd(LCD_CMD_FUNCTION_SET);  // Function set: DL=0 (4-bit mode), N=1 (2-line display), F=0 (5x8 characters)
    HAL_Delay(1);
    lcd_send_cmd(LCD_CMD_DISPLAY_OFF);   // Display off
    HAL_Delay(1);
    lcd_send_cmd(LCD_CMD_CLEAR_DISPLAY); // Clear display
    HAL_Delay(2);
    lcd_send_cmd(LCD_CMD_ENTRY_MODE_SET); // Entry mode set: I/D=1 (increment cursor), S=0 (no shift)
    HAL_Delay(1);
    lcd_send_cmd(LCD_CMD_DISPLAY_ON);    // Display on: D=1, C=0, B=0 (Cursor and blink)
    HAL_Delay(1);
}

// Function to send a command to the LCD
HAL_StatusTypeDef lcd_send_cmd(uint8_t cmd)
{
	uint8_t data_u, data_l;
    uint8_t data_t[4];

    data_u = (cmd & 0xF0); // Upper nibble
    data_l = ((cmd << 4) & 0xF0); // Lower nibble

    data_t[0] = data_u | 0x0C; // Enable high, RS low
    data_t[1] = data_u | 0x08; // Enable low, RS low
    data_t[2] = data_l | 0x0C; // Enable high, RS low
    data_t[3] = data_l | 0x08; // Enable low, RS low

    if (HAL_I2C_Master_Transmit(&hi2c1, SLAVE_ADDRESS_LCD << 1, (uint8_t *)data_t, 4, HAL_MAX_DELAY) != HAL_OK)
    {
        // Handle error
        return HAL_ERROR;
    }
    return HAL_OK;
}

// Function to send data to the LCD
HAL_StatusTypeDef lcd_send_data(uint8_t data)
{
	uint8_t data_u, data_l;
    uint8_t data_t[4];

    data_u = (data & 0xF0); // Upper nibble
    data_l = ((data << 4) & 0xF0); // Lower nibble

    data_t[0] = data_u | 0x0D; // Enable high, RS high
    data_t[1] = data_u | 0x09; // Enable low, RS high
    data_t[2] = data_l | 0x0D; // Enable high, RS high
    data_t[3] = data_l | 0x09; // Enable low, RS high

    if (HAL_I2C_Master_Transmit(&hi2c1, SLAVE_ADDRESS_LCD << 1, (uint8_t *)data_t, 4, HAL_MAX_DELAY) != HAL_OK)
    {
        // Handle error
        return HAL_ERROR;
    }
    return HAL_OK;
}

// Function to clear the LCD screen
void lcd_clear(void)
{
    if (lcd_send_cmd(LCD_CMD_CLEAR_DISPLAY) != HAL_OK)
    {
        // Handle error
    }
    HAL_Delay(50); // Wait for the command to complete
}

// Function to set the cursor position on the LCD
void lcd_put_cursor(int row, int col)
{
    if (row == 0)
    {
        col |= LCD_CMD_SET_CURSOR; // Set position for row 0
    }
    else if (row == 1)
    {
        col |= (LCD_CMD_SET_CURSOR | 0x40); // Set position for row 1
    }

    lcd_send_cmd(col); // Send command to set cursor position
}

// Function to send a string to the LCD
void lcd_send_string(char *str)
{
    while (*str) lcd_send_data(*str++); // Send each character of the string
}


// Function to scan I2C addresses
uint8_t scan_i2c_address(void)
{
    HAL_StatusTypeDef result;
    uint8_t i;
    printf("Scanning I2C addresses...\n");
    for (i = 1; i < 128; i++)
    {
        result = HAL_I2C_IsDeviceReady(&hi2c1, (uint16_t)(i << 1), 1, 10);
        if (result == HAL_OK)
        {
            printf("I2C device found at address 0x%02X\n", i);
            return i;
        }
    }
    printf("No I2C devices found.\n");
    return 0;
}



// ==============================> The functions added by me in this library. <============================//


// Fuction to send a custom char to the LCD
void lcd_send_custom_char(uint8_t location, const uint8_t *charmap)
{
	location &= 0x07;
	lcd_send_cmd(0x40 | (location << 3));
	
	for(uint8_t i = 0 ; i < 8 ; i++)
	{
		lcd_send_data(charmap[i]);
	}
}


// Fuction to Scroll a String to LCD
void lcd_scroll_text(const char *text, uint8_t row, uint16_t delay_ms)
{
    // Calculate the length of the input text
    uint8_t len = strlen(text);
    
    // If the text is shorter or equal to the number of LCD columns, print it directly
    if(len <= LCD_COLS)
    {
        // Set the cursor to the specified row, column 0 (start of the line)
        lcd_put_cursor(row, 0);
        
        // Send the string to the LCD
        lcd_send_string((char *)text);
    }
    else
    {
        // If the text is longer than the screen, scroll it across the LCD
        // Loop over the text, displaying a portion of it at a time
        for(uint8_t i = 0; i <= len - LCD_COLS; i++)
        {
            // Set the cursor to the specified row, column 0 (start of the line)
            lcd_put_cursor(row, 0);
            
            // Loop through the characters in the current scroll window (LCD_COLS characters)
            for(uint8_t j = 0; j < LCD_COLS; j++)
            {
                // Send each character of the current window to the LCD
                lcd_send_data(text[i + j]);
            }
            
            // Delay for the specified amount of time before displaying the next part of the text
            HAL_Delay(delay_ms);
        }
    }
}



// Scrolls the given text from right to left on a specific LCD row.
// The text starts off-screen (from the right) and scrolls leftwards into view.
void lcd_scroll_text_from_right(const char *text, uint8_t row, uint16_t delay_ms)
{
    // Get the length of the input text
    uint8_t len = strlen(text);

    // Calculate the total length of the scroll buffer:
    // Extra (LCD_COLS - 1) spaces are added before the text to simulate entry from the right
    uint8_t buffer_len = len + LCD_COLS - 1;

    // Create a temporary buffer to hold spaces + text
    char buffer[buffer_len];

    // Fill the beginning of the buffer with spaces to simulate off-screen entry
    for (uint8_t i = 0; i < LCD_COLS - 1; i++)
    {
        buffer[i] = ' ';
    }

    // Copy the original text after the initial spaces
    strcpy(buffer + LCD_COLS - 1, text);

    // If the text fits within the LCD width, simply display it aligned from the right
    if (len <= LCD_COLS)
    {
        // Move the cursor so that the text is right-aligned on the LCD
        lcd_put_cursor(row, LCD_COLS - len); 
        lcd_send_string((char *)text); // Display the text
    }
    else
    {
        // Scroll the text across the LCD screen from right to left
        for (uint8_t i = 0; i <= buffer_len - LCD_COLS; i++)
        {
            // Always write to the beginning of the specified row
            lcd_put_cursor(row, 0);

            // Display 16 characters (LCD_COLS) from the buffer starting at position i
            for (uint8_t j = 0; j < LCD_COLS; j++)
            {
                lcd_send_data(buffer[i + j]);
            }

            // Delay between scroll steps
            HAL_Delay(delay_ms);
        }
    }
}

// =========================> non-Bloking Scroll <======================== //

typedef struct {
	
	const char *text;
	uint8_t row;
	uint8_t index;
	uint8_t isActive;
	uint16_t delay_ms;
	uint32_t last_tick;
	
} LcdScrollContext;

LcdScrollContext scroll_ctx;


void LcdStartScroll(const char *text, uint8_t row, uint16_t delay_ms)
{
	scroll_ctx.text = text;
	scroll_ctx.row  = row;
	scroll_ctx.index = 0;
	scroll_ctx.last_tick = HAL_GetTick();
	scroll_ctx.delay_ms  = delay_ms;
	scroll_ctx.isActive = 1;
}

void LcdUpdateScroll()
{
	if(!scroll_ctx.isActive) return;
	
	uint8_t len = strlen(scroll_ctx.text);
	uint8_t buffer_len = len + LCD_COLS - 1;
	char buffer[buffer_len];
	
	for(uint8_t i = 0 ; i < LCD_COLS - 1 ; i++)
	{
		buffer[i] = ' ';
	}
	
	strcpy(buffer + LCD_COLS - 1, scroll_ctx.text);
	
	if(HAL_GetTick() - scroll_ctx.last_tick >= scroll_ctx.delay_ms)
	{
		scroll_ctx.last_tick = HAL_GetTick();
		
		if(scroll_ctx.index <= buffer_len - LCD_COLS)
		{
			lcd_put_cursor(scroll_ctx.row, 0);
			
			for(uint8_t j = 0 ; j < LCD_COLS ; j++)
			{
				lcd_send_data(buffer[scroll_ctx.index + j]);
			}
			
			scroll_ctx.index++;
		}
		
		else
		{
			scroll_ctx.isActive = 0;
		}
	}
}

