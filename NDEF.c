#include "types.h"
#include "NDEF.h"
#include "patch.h"
#include <string.h>

#define FIRMWARE_CONTROL_ADDRESS 	0xF867
#pragma RETAIN(Firmware_System_Control_Byte);
#pragma location = FIRMWARE_CONTROL_ADDRESS

volatile const u08_t Firmware_System_Control_Byte = ROM_SENSOR_SUPPORT_DISABLED + EROM_EUSCI_SUPPORT_DISABLED + NFC_BRIDGE_DISABLED + FOUR_BYTE_BLOCK + FIRST_ISO_PAGE;

#pragma RETAIN(DS)
#pragma location = 0x1C00
u08_t DS;
#pragma RETAIN(RF)
#pragma location = 0x1C6A
const u08_t RF;
#pragma RETAIN(NRX)
#pragma location = 0x1CA4 //rx
const u08_t NRX[34];
#pragma RETAIN(NTX)
#pragma location = 0x1CC6 //tx
const u08_t NTX[33];
#pragma RETAIN(EL)
#pragma location = 0x1CF2
const u08_t EL;
#pragma RETAIN(PF)
#pragma location = 0x1C0A
const u16_t PF[48];



#define NDEF_START_ADDRESS	0xF868
#pragma PERSISTENT(NFC_NDEF_Message)
#pragma RETAIN(NFC_NDEF_Message)
#pragma location = NDEF_START_ADDRESS


u08_t NFC_NDEF_Message[104] = {
		// Block 0
		0xE1, 		// NDEF Magic Number
		0x40, 		// Version Number, read/write access conditions
		0x79, 		//0x7E,		// 1008 bytes / 8 = 126 blocks
		0x00,		//0x04,//8 byte extended memory //0x00,		// does not support read multiple blocks (limited to only 3 blocks)

		// Block 1
		0x03,		// NDEF Message present
		0x0c,		// Length , 22 bytes
		0xD1,		// Record header
		0x01,		// type length

		// Block 2
		0x08,		// Payload length
		0x54,		// Record Type text)
		0x02, 		// SOT
		0x65,		// 'e'

		0x6E,		// 'n'

		0x54,

		0x03,		// end of text
		0x00,		// Empty don't care
};

void initISO15693(u16_t parameters ){
	RF13MCTL |= RF13MTXEN + RF13MRXEN + RF13MRFTOEN;
	RF13MINT |= RF13MRXIE + RX13MRFTOIE;
	if (parameters & CLEAR_BLOCK_LOCKS )  {
		memset ((u08_t *) FRAM_LOCK_BLOCKS, 0xFF, FRAM_LOCK_BLOCK_AREA_SIZE);     //block is locked with a zero bit, clears FRAM and RAM lock blocks
	}
}

u16_t BlockLockAPI(u16_t block, u08_t checkLock)
{
	u16_t locked;

	locked = BlockLockROM_Patched(block, checkLock );
	return locked;
}

