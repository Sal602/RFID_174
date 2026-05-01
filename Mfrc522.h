#ifndef MFRC522_H
#define MFRC522_H

#include <msp430.h>
#include <stdint.h>
#include <string.h>

//==============================================================================
// RC522 Register Definitions
//==============================================================================
#define Page0			0x00
#define CommandReg		0x01
#define CommIEnReg		0x02
#define DivIEnReg		0x03
#define CommIrqReg		0x04
#define DivIrqReg		0x05
#define ErrorReg		0x06
#define Status1Reg		0x07
#define Status2Reg		0x08
#define FIFODataReg		0x09
#define FIFOLevelReg		0x0A
#define WaterLevelReg		0x0B
#define ControlReg		0x0C
#define BitFramingReg		0x0D
#define CollReg			0x0E
#define ModeReg			0x11
#define TxModeReg		0x12
#define RxModeReg		0x13
#define TxControlReg		0x14
#define TxAutoReg		0x15
#define TxSelReg		0x16
#define RxSelReg		0x17
#define RxThresholdReg		0x18
#define DemodReg		0x19
#define MfTxReg			0x1C
#define MfRxReg			0x1D
#define SerialSpeedReg		0x1F
#define CRCResultRegM		0x21
#define CRCResultRegL		0x22
#define ModWidthReg		0x24
#define RFCfgReg		0x26
#define GsNReg			0x27
#define CWGsPReg		0x28
#define ModGsPReg		0x29
#define TModeReg		0x2A
#define TPrescalerReg		0x2B
#define TReloadRegH		0x2C
#define TReloadRegL		0x2D
#define TCounterValueRegH	0x2E
#define TCounterValueRegL	0x2F
#define TestSel1Reg		0x31
#define TestSel2Reg		0x32
#define TestPinEnReg		0x33
#define TestPinValueReg		0x34
#define TestBusReg		0x35
#define AutoTestReg		0x36
#define VersionReg		0x37
#define AnalogTestReg		0x38
#define TestDAC1Reg		0x39
#define TestDAC2Reg		0x3A
#define TestADCReg		0x3B

//==============================================================================
// RC522 Command Definitions  
//==============================================================================
#define PCD_IDLE			0x00
#define PCD_AUTHENT			0x0E
#define PCD_RECEIVE			0x08
#define PCD_TRANSMIT			0x04
#define PCD_TRANSCEIVE			0x0C
#define PCD_RESETPHASE			0x0F
#define PCD_CALCCRC			0x03

#define PICC_REQIDL			0x26
#define PICC_ANTICOLL			0x93
#define PICC_SElECTTAG			0x93
#define PICC_AUTHENT1A			0x60
#define PICC_AUTHENT1B			0x61
#define PICC_READ			0x30
#define PICC_WRITE			0xA0
#define PICC_HALT			0x50
#define PICC_WUPA			0x52

#define MI_OK				0x26
#define MI_NOTAGERR			0xCC
#define MI_ERR				0xBB

#define MAX_LEN				16

//==============================================================================
// Pin Definitions for MSP430F5529LP
//==============================================================================
#define RC522_CS_PORT   P4OUT
#define RC522_CS_PIN    BIT0
#define RC522_CS_HIGH   P4OUT |= BIT0
#define RC522_CS_LOW    P4OUT &= ~BIT0

#define RC522_RST_PORT  P2OUT
#define RC522_RST_PIN   BIT0
#define RC522_RST_HIGH  P2OUT |= BIT0
#define RC522_RST_LOW   P2OUT &= ~BIT0

//==============================================================================
// Function Prototypes
//==============================================================================
void MFRC522_Init(void);
void MFRC522_WriteReg(uint8_t addr, uint8_t val);
uint8_t MFRC522_ReadReg(uint8_t addr);
void MFRC522_SetBitMask(uint8_t reg, uint8_t mask);
void MFRC522_ClearBitMask(uint8_t reg, uint8_t mask);
void MFRC522_AntennaOn(void);
void MFRC522_AntennaOff(void);
void MFRC522_Reset(void);
uint8_t MFRC522_Request(uint8_t reqMode, uint8_t *TagType);
uint8_t MFRC522_ToCard(uint8_t command, uint8_t *sendData, uint8_t sendLen, 
                       uint8_t *backData, uint16_t *backLen);
uint8_t MFRC522_Anticoll(uint8_t *serNum);
void MFRC522_CalulateCRC(uint8_t *pIndata, uint8_t len, uint8_t *pOutData);
uint8_t MFRC522_SelectTag(uint8_t *serNum);
uint8_t MFRC522_Auth(uint8_t authMode, uint8_t BlockAddr, 
                     uint8_t *Sectorkey, uint8_t *serNum);
uint8_t MFRC522_ReadBlock(uint8_t blockAddr, uint8_t *recvData);
uint8_t MFRC522_WriteBlock(uint8_t blockAddr, uint8_t *writeData);
void MFRC522_Halt(void);

#endif