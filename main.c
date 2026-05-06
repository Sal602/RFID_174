#include "driverlib.h"
#include <msp430.h>
#include "Mfrc522.h"
#include <stdio.h>
#include <string.h>

// Pin Definitions for Servo and LEDs
#define SERVO_PIN       BIT3    // P1.3 for servo PWM (TA0.2)
#define BUTTON_PIN      BIT1    // P1.1 for S1 button
#define RED_LED_PIN     BIT0    // P1.0 for Red LED
#define GREEN_LED_PIN   BIT7    // P4.7 for Green LED

// Servo position values (adjust these based on your servo)
#define SERVO_0_DEG     350     // 0 degree position
#define SERVO_90_DEG    800    // 90 degree position
#define SERVO_180_DEG   2600    // 180 degree position

unsigned char serNum[5];
unsigned char servoState = 0;    // 0=idle, 1=moving to 90, 2=delay at 90, 3=moving to 180, 4=complete
unsigned int servoDelayTimer = 0;

// Authorized cards (4-byte UIDs)
const unsigned char authorizedCard1[4] = {0xBB, 0xEC, 0xC8, 0x06};
const unsigned char authorizedCard2[4] = {0xCA, 0xE6, 0x62, 0x06};

void initSystem(void);
void initPWM(void);
void initButton(void);
void initLEDs(void);
void setServoAngle(unsigned int angle);
void blinkRedLED(int times);
void processRFID(void);
void updateSystem(void);
int isAuthorizedCard(void);

// Main Function
int main(void)
{
    initSystem();       // Initialize clock and peripherals
    MFRC522_Init();     // Initialize RFID reader
    initPWM();          // Initialize servo PWM
    initButton();       // Initialize button
    initLEDs();         // Initialize LEDs
    
    // Start with servo at 0 degrees
    setServoAngle(SERVO_0_DEG);
    
    while(1)
    {
        processRFID();  // Check for RFID cards
        updateSystem(); // Update servo timer and LEDs
    }
    
    return 0;
}

// Initialization Functions
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
    TA0CCR2 = angle;
    __delay_cycles(200000);  // 200ms delay for servo to move
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

// Check if card is authorized
int isAuthorizedCard(void)
{
    int i;
    
    // Check against authorizedCard1
    for(i = 0; i < 4; i++)
        if(serNum[i] != authorizedCard1[i]) break;
    if(i == 4) return 1;
    
    // Check against authorizedCard2
    for(i = 0; i < 4; i++)
        if(serNum[i] != authorizedCard2[i]) break;
    if(i == 4) return 1;
    
    return 0;
}

// RFID Processing
void processRFID(void)
{
    uint8_t status;
    uint8_t str[MAX_LEN];
    uint8_t i;
    
    // Only check for new cards if servo is idle
    if(servoState == 0)
    {
        // Check for card
        status = MFRC522_Request(PICC_REQIDL, str);
        if (status == MI_OK)
        {
            // Get card serial number
            status = MFRC522_Anticoll(serNum);
            
            if (status == MI_OK)
            {
                // Check if card is authorized
                if(isAuthorizedCard())
                {
                    // Valid card detected - start servo sequence
                    servoState = 1;  // Move to 90 degrees
                    
                    // Turn on green LED, turn off red LED
                    P4OUT |= GREEN_LED_PIN;
                    P1OUT &= ~RED_LED_PIN;
                    
                    // Flash green LED 3 times to indicate access granted
                    for(i = 0; i < 3; i++)
                    {
                        P4OUT ^= GREEN_LED_PIN;
                        __delay_cycles(100000);
                        P4OUT ^= GREEN_LED_PIN;
                        __delay_cycles(100000);
                    }
                    
                    // Keep green LED solid
                    P4OUT |= GREEN_LED_PIN;
                    
                    // Move to 90 degrees immediately
                    setServoAngle(SERVO_90_DEG);
                    
                    // Set timer for 5 second delay AFTER reaching 90 degrees
                    servoDelayTimer = 500;  // 500 * 10ms = 5 seconds
                }
                else
                {
                    // Invalid card - blink red LED
                    blinkRedLED(3);
                }
                
                __delay_cycles(500000);  // 0.5 second delay between reads
            }
        }
    }
    
    MFRC522_Halt();
}

// System Update (Servo Timer Control)
void updateSystem(void)
{
    // Handle servo sequence
    if(servoState != 0 && servoDelayTimer > 0)
    {
        servoDelayTimer--;
        
        if(servoDelayTimer == 0)
        {
            if(servoState == 1)
            {
                // After 5 second delay at 90 degrees - move to 180 degrees
                setServoAngle(SERVO_180_DEG);
                servoState = 2;
                servoDelayTimer = 500;  // Wait 5 seconds at 180 degrees
            }
            else if(servoState == 2)
            {
                // After 5 second delay at 180 degrees - sequence complete
                servoState = 0;
                
                // Turn on red LED, turn off green LED
                P1OUT |= RED_LED_PIN;
                P4OUT &= ~GREEN_LED_PIN;
            }
        }
    }
    
    __delay_cycles(10000);  // 10ms delay for timing (each loop = ~10ms)
}