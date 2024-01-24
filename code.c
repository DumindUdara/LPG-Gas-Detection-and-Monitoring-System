#include <built_in.h>

// Define HC-05 module connections
sbit BT_RX at RB1_bit;
sbit BT_TX at RB2_bit;
sbit BT_RST at RB4_bit;
sbit BT_EN at RB5_bit;

// Set HC-05 module connections
sbit BT_RX_Direction at TRISB1_bit;
sbit BT_TX_Direction at TRISB2_bit;
sbit BT_RST_Direction at TRISB4_bit;
sbit BT_EN_Direction at TRISB5_bit;

// Define LCD pins
sbit LCD_RS at RB0_bit;
sbit LCD_EN at RB1_bit;
sbit LCD_D4 at RB2_bit;
sbit LCD_D5 at RB3_bit;
sbit LCD_D6 at RB4_bit;
sbit LCD_D7 at RB5_bit;

// Define Buzzer pin
sbit BUZZER at RB6_bit;

// Define Relay pin
sbit GASVALVE at RD0_bit;

// Define pin direction
sbit LCD_RS_Direction at TRISB0_bit;
sbit LCD_EN_Direction at TRISB1_bit;
sbit LCD_D4_Direction at TRISB2_bit;
sbit LCD_D5_Direction at TRISB3_bit;
sbit LCD_D6_Direction at TRISB4_bit;
sbit LCD_D7_Direction at TRISB5_bit;
sbit BUZZER_Direction at TRISB6_bit;
sbit GASVALVE_Direction at TRISD0_bit;

char data_received = '\0';
int state = 0;

void Bluetoothcontrol(void) {
    static char lastCommand = '\0';  // Variable to store the last received command
    state = 1;  // Set the default state

    if (UART1_Data_Ready()) {
        data_received = UART1_Read();

        if (data_received == '1' && lastCommand != '1') {
            GASVALVE = 0;  // Turn off the relay only if the last command was not '1'
            lastCommand = '1';
            state = 0;
        } else if (data_received == '2' && lastCommand != '2') {
            GASVALVE = 1;  // Turn on the relay only if the last command was not '2'
            lastCommand = '2';
            state = 1;
        }

        Delay_ms(100);
    }
}



void main() {
    unsigned int adc_value;
    char GAS_String[10];
    char gasLevel[10];
    char gasLocation[10]= "GASV001";
    char prevGasLevel[10] = ""; // Variable to store the previous gas level

    UART1_Init(9600); // Initialize UART module with baud rate 9600 bps

    // Initialize HC-05 module
    BT_RX_Direction = 1; // Set RX pin as input
    BT_TX_Direction = 0; // Set TX pin as output
    BT_RST_Direction = 0; // Set RST pin as output
    BT_EN_Direction = 0;  // Set EN pin as output

    BT_RST = 0;   // Reset the Bluetooth module
    Delay_ms(500);
    BT_RST = 1;   // Release the reset
    // Initialize LCD and Buzzer
    Lcd_Init();
    BUZZER_Direction = 0; // Set Buzzer pin as output
    GASVALVE_Direction = 0; // Set Relay pin as output

    TRISA = 0xFF; // Configure Port A as input (for analog sensors)
    ADC_Init();   // Initialize ADC module

    Lcd_Out(1, 6, "SLTC");
    Lcd_Cmd(_LCD_CURSOR_OFF);
    Delay_ms(1000); // Add a delay for better visibility
    Lcd_Cmd(_LCD_CLEAR); // Clear the LCD
    Lcd_Out(1, 1, "LP Gas Detector");
    Lcd_Cmd(_LCD_CURSOR_OFF);
    Delay_ms(1000); // Add a delay for better visibility
    Lcd_Cmd(_LCD_CLEAR); // Clear the LCD
    GASVALVE = 1;
    while (1) {
        // Check for incoming Bluetooth data

        Bluetoothcontrol();
        Lcd_Out(1, 4, "Gas Level-");
        Lcd_Cmd(_LCD_CURSOR_OFF);
        // Read analog input from channel 0 (RA0)
        adc_value = ADC_Read(0);
        IntToStr(adc_value, GAS_String);
        // Map ADC value to gas level
        if (adc_value <= 80) {
            strcpy(gasLevel, "LOW");
            BUZZER = 0; // Turn off the buzzer

        } else if (adc_value <= 180) {
            GASVALVE = 1;

            strcpy(gasLevel, "MEDIUM");
            BUZZER = 1; // Turn on the buzzer
            Delay_ms(1000);
            BUZZER = 1; // Turn off the buzzer
            UART1_Write_Text(gasLocation);
            UART1_Write_Text(" - MEDIUM");


        } else {
            GASVALVE = 1;
            state = 1 ;
            strcpy(gasLevel, "HIGH");
            BUZZER = 1; // Turn on the buzzer for High gas level
            UART1_Write_Text(gasLocation);
            UART1_Write_Text(" - HIGH");
        }
        
        Lcd_Out(2, 1, GAS_String); // Display ADC value
        Lcd_Cmd(_LCD_CURSOR_OFF);
        Delay_ms(1000); // Delay for stability (adjust as needed)
        UART1_Write_Text("     ");
        // Update display only if gas level changes
        if (strcmp(gasLevel, prevGasLevel) != 0) {
            Lcd_Cmd(_LCD_CLEAR);
            strcpy(prevGasLevel, gasLevel); // Update the previous gas level
            Lcd_Out(2, 9, gasLevel);
        }

        BUZZER = 0; // Turn off the BUZZER
    }
}

