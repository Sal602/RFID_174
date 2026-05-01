#include "driverlib.h"
#include <msp430.h>
#include "Mfrc522.h"
#include <stdio.h>

#include <msp430.h>
#include "Mfrc522.h"
#include <string.h>

//==============================================================================
// Pin Definitions
//==============================================================================
#define RED_LED_PIN     BIT0    // P1.0 for Red LED
#define GREEN_LED_PIN   BIT7    // P4.7 for Green LED
#define BUTTON_PIN      BIT1    // P1.1 for S1 button
#define SERVO_PIN       BIT3    // P1.3 for servo PWM (not used in test)

//==============================================================================
// Function Prototypes
//==============================================================================
void initSystem(void);
void initUART(void);
void initLEDs(void);
void sendChar(char c);
void sendString(char *str);
void sendHexNumber(unsigned char num);
void sendDecimalNumber(unsigned char num);
void processRFID(void);

//==============================================================================
// Main Function
//==============================================================================
int main(void)
{
    initSystem();       // Initialize clock and watchdog
    initUART();         // Initialize serial communication
    initLEDs();         // Initialize LEDs
    MFRC522_Init();     // Initialize RFID reader
    
    // Send startup message
    sendString("\r\n\r\n=== RC522 Card Reader Test ===\r\n");
    sendString("Place card near reader to see UID\r\n");
    sendString("================================\r\n\n");
    
    while(1)
    {
        processRFID();  // Continuously check for cards
        __delay_cycles(100000);  // 100ms delay between reads
    }
    
    return 0;
}

//==============================================================================
// System Initialization
//==============================================================================
void initSystem(void)
{
    WDTCTL = WDTPW | WDTHOLD;  // Stop watchdog
    
    // Configure clock to 1MHz
    UCSCTL3 = SELREF_2;
    UCSCTL4 = SELA_2 | SELS_3 | SELM_3;
}

void initLEDs(void)
{
    // Red LED on P1.0
    P1DIR |= RED_LED_PIN;
    P1OUT &= ~RED_LED_PIN;
    
    // Green LED on P4.7
    P4DIR |= GREEN_LED_PIN;
    P4OUT &= ~GREEN_LED_PIN;
}

//==============================================================================
// UART Functions for Debug Output
//==============================================================================
void initUART(void)
{
    // Configure P3.3 and P3.4 for UART (connects to USB debugger)
    P3SEL |= BIT3 | BIT4;
    
    // Configure UCA0 as UART mode
    UCA0CTL1 = UCSWRST;
    UCA0CTL0 = 0x00;
    UCA0CTL1 = UCSSEL_2;    // SMCLK
    UCA0BR0 = 0x68;         // 9600 baud
    UCA0BR1 = 0x00;
    UCA0MCTL = UCBRS_1;
    UCA0CTL1 &= ~UCSWRST;
}

// Override the low-level write function for printf
int write(int fd, const char *buf, int len)
{
    int i;
    for(i = 0; i < len; i++) {
        while (!(UCA0IFG & UCTXIFG));
        UCA0TXBUF = buf[i];
    }
    return len;
}
void sendChar(char c)
{
    while (!(UCA0IFG & UCTXIFG));  // Wait for TX buffer ready
    UCA0TXBUF = c;
}

void sendString(char *str)
{
    while (*str) {
        sendChar(*str++);
    }
}

void sendHexNumber(unsigned char num)
{
    char hexChars[] = "0123456789ABCDEF";
    sendChar(hexChars[(num >> 4) & 0x0F]);  // High nibble
    sendChar(hexChars[num & 0x0F]);          // Low nibble
}

void sendDecimalNumber(unsigned char num)
{
    // Convert to 3-digit decimal (0-255)
    sendChar('0' + (num / 100));
    sendChar('0' + ((num % 100) / 10));
    sendChar('0' + (num % 10));
}

//==============================================================================
// RFID Processing
//==============================================================================
void processRFID(void)
{
    uint8_t status;
    uint8_t str[MAX_LEN];
    uint8_t i;
    unsigned char uid[5];
    
    status = MFRC522_Request(PICC_REQIDL, str);
    
    if (status == MI_OK)
    {
        status = MFRC522_Anticoll(uid);
        
        if (status == MI_OK)
        {
            // This prints to CCS console!
            printf("\r\nCard Detected! UID: ");
            for(i = 0; i < 5; i++) {
                printf("%d ", uid[i]);   // Print as decimal instead of hex
            }
            printf("\r\nDecimal: {%d, %d, %d, %d, %d}\r\n", 
                   uid[0], uid[1], uid[2], uid[3], uid[4]);
        }
    }
    MFRC522_Halt();
}


/*
//==============================================================================
// Pin Definitions for Servo and LEDs
//==============================================================================
#define SERVO_PIN       BIT3    // P1.3 for servo PWM (TA0.2)
#define BUTTON_PIN      BIT1    // P1.1 for S1 button
#define RED_LED_PIN     BIT0    // P1.0 for Red LED
#define GREEN_LED_PIN   BIT7    // P4.7 for Green LED

#define SERVO_0_DEG     350
#define SERVO_180_DEG   2600

//==============================================================================
// Global Variables
//==============================================================================
unsigned char authorizedCardDetected = 0;
unsigned char serNum[5];

// Authorized cards (REPLACE WITH YOUR ACTUAL CARD NUMBERS)
const unsigned char authorizedCard1[5] = {148, 176, 135, 240, 83};
const unsigned char authorizedCard2[5] = {148, 217, 159, 240, 34};

//==============================================================================
// Function Prototypes
//==============================================================================
void initSystem(void);
void initPWM(void);
void initButton(void);
void initLEDs(void);
void setServoAngle(unsigned int angle);
void blinkRedLED(int times);
void processRFID(void);
void updateSystem(void);

//==============================================================================
// Main Function
//==============================================================================
int main(void)
{
    initSystem();       // Initialize clock and peripherals
    MFRC522_Init();     // Initialize RFID reader
    initPWM();          // Initialize servo PWM
    initButton();       // Initialize button
    initLEDs();         // Initialize LEDs
    
    while(1)
    {
        processRFID();  // Check for RFID cards
        updateSystem(); // Update servo and LEDs
    }
    
    return 0;
}

//==============================================================================
// Initialization Functions
//==============================================================================
void initSystem(void)
{
    WDTCTL = WDTPW | WDTHOLD;  // Stop watchdog
    
    // Configure clock to 1MHz
    UCSCTL3 = SELREF_2;
    UCSCTL4 = SELA_2 | SELS_3 | SELM_3;
}

void initLEDs(void)
{
    P1DIR |= RED_LED_PIN;
    P4DIR |= GREEN_LED_PIN;
    P1OUT |= RED_LED_PIN;   // Start with RED ON
    P4OUT &= ~GREEN_LED_PIN;
}

void initButton(void)
{
    P1DIR &= ~BUTTON_PIN;
    P1REN |= BUTTON_PIN;
    P1OUT |= BUTTON_PIN;     // Pull-up
}

void initPWM(void)
{
    P1DIR |= SERVO_PIN;
    P1SEL |= SERVO_PIN;
    
    TA0CCR0 = 20000;
    TA0CCR2 = SERVO_0_DEG;
    TA0CCTL2 = OUTMOD_7;
    TA0CTL = TASSEL_2 | MC_1;
}

void setServoAngle(unsigned int angle)
{
    if (angle == 0)
        TA0CCR2 = SERVO_0_DEG;
    else
        TA0CCR2 = SERVO_180_DEG;
    
    __delay_cycles(200000);  // 200ms for servo to move
}

void blinkRedLED(int times)
{
    int i;
    for(i = 0; i < times; i++)
    {
        P1OUT |= RED_LED_PIN;
        __delay_cycles(200000);
        P1OUT &= ~RED_LED_PIN;
        __delay_cycles(200000);
    }
}

//==============================================================================
// RFID Processing
//==============================================================================
void processRFID(void)
{
    uint8_t status;
    uint8_t str[MAX_LEN];
    uint8_t i;
    int cardAuthorized = 0;
    
    // Check for card
    status = MFRC522_Request(PICC_REQIDL, str);
    if (status == MI_OK)
    {
        // Get card serial number
        status = MFRC522_Anticoll(serNum);
        
        if (status == MI_OK)
        {
            // Check if card matches authorized cards
            cardAuthorized = 1;
            for(i = 0; i < 5; i++)
            {
                if(serNum[i] != authorizedCard1[i])
                {
                    cardAuthorized = 0;
                    break;
                }
            }
            
            if(!cardAuthorized)
            {
                cardAuthorized = 1;
                for(i = 0; i < 5; i++)
                {
                    if(serNum[i] != authorizedCard2[i])
                    {
                        cardAuthorized = 0;
                        break;
                    }
                }
            }
            
            if(cardAuthorized)
            {
                authorizedCardDetected = 1;
                P4OUT |= GREEN_LED_PIN;   // Green ON
                P1OUT &= ~RED_LED_PIN;    // Red OFF
                
                // Flash green LED 3 times
                for(i = 0; i < 3; i++)
                {
                    P4OUT ^= GREEN_LED_PIN;
                    __delay_cycles(100000);
                    P4OUT ^= GREEN_LED_PIN;
                    __delay_cycles(100000);
                }
            }
            else
            {
                authorizedCardDetected = 0;
                blinkRedLED(5);  // Unauthorized - blink red 5 times
                P1OUT |= RED_LED_PIN;
                P4OUT &= ~GREEN_LED_PIN;
            }
            
            __delay_cycles(1000000);  // 1 second delay
        }
    }
    
    MFRC522_Halt();
}

//==============================================================================
// System Update (Button and Servo Control)
//==============================================================================
void updateSystem(void)
{
    unsigned char buttonPressed = ((P1IN & BUTTON_PIN) == 0);
    
    if(authorizedCardDetected && buttonPressed)
    {
        setServoAngle(180);  // Move servo to 180 degrees
    }
    else if(authorizedCardDetected && !buttonPressed)
    {
        setServoAngle(0);    // Move servo back to 0 degrees
    }
    
    __delay_cycles(10000);  // Small delay
}
/*
// Function declarations
void initSystem(void);
void initPWM(void);
void initButton(void);
void initLEDs(void);
void setLEDs(unsigned char buttonPressed);
void servoControl(void);

int main(void)
{
    initSystem();
    initLEDs();          // Initialize LEDs first
    initPWM();
    initButton();
    
    while(1) {
        servoControl();
    }
    
    return 0;
}

// Initialize system clock and watchdog
void initSystem(void)
{
    WDTCTL = WDTPW | WDTHOLD;  // Stop watchdog timer
    
    // Configure clock to 1MHz
    UCSCTL3 = SELREF_2;
    UCSCTL4 = SELA_2 | SELS_3 | SELM_3;
}

// Initialize LEDs
void initLEDs(void)
{
    // Red LED on P1.0
    P1DIR |= BIT0;      // Set P1.0 as output
    P1OUT &= ~BIT0;     // Start with RED OFF
    
    // Green LED on P4.7 (MSP430F5529LP LaunchPad)
    P4DIR |= BIT7;      // Set P4.7 as output
    P4OUT &= ~BIT7;     // Start with GREEN OFF
    
    // If your board has Green LED on P1.6 instead, use:
    // P1DIR |= BIT6;
    // P1OUT &= ~BIT6;
}

// Control LEDs based on button state
void setLEDs(unsigned char buttonPressed)
{
    if (buttonPressed) {
        // Button pressed: Green ON, Red OFF
        P4OUT |= BIT7;      // Green LED ON
        P1OUT &= ~BIT0;     // Red LED OFF
        
        // If Green LED is on P1.6:
        // P1OUT |= BIT6;    // Green ON
        // P1OUT &= ~BIT0;   // Red OFF
    } else {
        // Button not pressed: Red ON, Green OFF
        P1OUT |= BIT0;      // Red LED ON
        P4OUT &= ~BIT7;     // Green LED OFF
        
        // If Green LED is on P1.6:
        // P1OUT |= BIT0;    // Red ON
        // P1OUT &= ~BIT6;   // Green OFF
    }
}

// Initialize PWM on P1.3 for servo
void initPWM(void)
{
    P1DIR |= BIT3;
    P1SEL |= BIT3;
    
    TA0CCR0 = 20000;
    TA0CCR2 = 350;              // Start at 0 degrees
    TA0CCTL2 = OUTMOD_7;
    TA0CTL = TASSEL_2 | MC_1;
}

// Initialize S1 button on P1.1
void initButton(void)
{
    P1DIR &= ~BIT1;
    P1REN |= BIT1;
    P1OUT |= BIT1;              // Pull-up
}

// Control servo and LEDs based on button state
void servoControl(void)
{
    unsigned char buttonPressed;
    
    // Check if button is pressed (P1.1 = 0)
    buttonPressed = ((P1IN & BIT1) == 0);
    
    // Update LEDs based on button state
    setLEDs(buttonPressed);
    
    // Control servo based on button state
    if (buttonPressed) {
        TA0CCR2 = 2600;         // Button pressed: 180 degrees
    } else {
        TA0CCR2 = 350;          // Button released: 0 degrees
    }
    
    __delay_cycles(10000);      // Small delay
}
*/