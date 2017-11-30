#include <rf430frl152h.h>
#include "types.h"

#ifndef NDEF_H
#define NDEF_H

//*****************************FUNCTION PROTOTYPES********************************/
void initISO15693(u16_t parameters );
u16_t BlockLockAPI(u16_t block, u08_t checkLock);

extern u08_t DS;

#define CLEAR_BLOCK_LOCKS                            	BIT3
#define FRAM_LOCK_BLOCK_AREA_SIZE  						38
#define FRAM_LOCK_BLOCKS								0xF840  //Address of ISO15693 lock blocks

<<<<<<< HEAD
#define NLEN 0x1C
#define NLENPOS 5
#define PLENPOS 11
#define PLEN 0x15
#define NDEFSTART 35
=======
#define NLEN 0x0C
#define PLEN 0x08
#define NDEFSTART 14
>>>>>>> master

#define ROM_EUSCI_SUPPORT_ENABLED       BIT2
#define EROM_EUSCI_SUPPORT_DISABLED     0
#define ROM_SENSOR_SUPPORT_ENABLED      BIT7
#define ROM_SENSOR_SUPPORT_DISABLED     0
#define	NFC_BRIDGE_DISABLED 			BIT6
#define	NFC_BRIDGE_ENABLED  			0
#define	EIGHT_BYTE_BLOCK    			BIT0
#define FOUR_BYTE_BLOCK_MASK			BIT0
#define	FOUR_BYTE_BLOCK     			0
#define	FIRST_ISO_PAGE_MASK    			BIT1
#define	FIRST_ISO_PAGE      			BIT1
#define	SECOND_ISO_PAGE     			0
#define FRAM_BLOCKS_8					0xF3


#define CHECK_LOCK              	1
#define LOCK_BLOCK              	0
#define LOCKED_FLAG                 BIT0

#endif
