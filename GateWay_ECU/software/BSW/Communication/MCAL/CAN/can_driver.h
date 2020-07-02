#include "common.h"
#include "CAN_PORT.h"
/*Const definitions*/

#define CAN_MAX_STANDARD_ID		0x7FF
#define	CAN_MAX_EXTENDED_ID		0x1FFFFFFF
#define MAX_DATA_LENGTH       8

#define CAN_INT_ERROR   				CAN_CTL_EIE
#define CAN_INT_MASTER  				CAN_CTL_IE
#define CAN_INT_STATUS  				CAN_CTL_SIE
#define CAN_STATUS_BUS_OFF			CAN_STS_BOFF
#define CAN_STATUS_EPASS				CAN_STS_EPASS
#define CAN_STATUS_EWARN				CAN_STS_EWARN
#define CAN_STATUS_LEC_ACK			CAN_STS_LEC_ACK
#define CAN_STATUS_LEC_BIT0			CAN_STS_LEC_BIT0
#define CAN_STATUS_LEC_BIT1			CAN_STS_LEC_BIT1
#define CAN_STATUS_LEC_CRC			CAN_STS_LEC_CRC
#define CAN_STATUS_LEC_FORM			CAN_STS_LEC_FORM
#define CAN_STATUS_LEC_MASK			CAN_STS_LEC_M
#define CAN_STATUS_LEC_MSK
#define CAN_STATUS_LEC_NONE			CAN_STS_LEC_NONE
#define CAN_STATUS_LEC_STUFF		CAN_STS_LEC_STUFF
#define CAN_STATUS_RXOK					CAN_STS_RXOK
#define CAN_STATUS_TXOK					CAN_STS_TXOK
#define MSG_OBJ_DATA_LOST				CAN_IF1MCTL_MSGLST
#define MSG_OBJ_EXTENDED_ID			CAN_IF1ARB2_XTD
#define MSG_OBJ_FIFO						1
#define MSG_OBJ_NEW_DATA				CAN_IF1MCTL_NEWDAT
#define MSG_OBJ_NO_FLAGS				0
#define MSG_OBJ_REMOTE_FRAME		CAN_IF1MCTL_RMTEN
#define MSG_OBJ_RX_INT_ENABLE		CAN_IF1MCTL_RXIE
#define MSG_OBJ_STATUS_MASK			
#define MSG_OBJ_TX_INT_ENABLE		CAN_IF1MCTL_TXIE
#define MSG_OBJ_USE_DIR_FILTER	    CAN_IF1MSK2_MDIR
#define MSG_OBJ_USE_EXT_FILTER	    CAN_IF1MSK2_MXTD
#define MSG_OBJ_USE_ID_FILTER		CAN_IF1MCTL_UMASK


/*
    Enumerations to contrust on demand:
*/

typedef enum {
	CAN_INT_STS_CAUSE,
	CAN_INT_STS_OBJECT
}tCANIntStsReg;

typedef enum  {
	CAN_STS_CONTROL = 0,		//the main controller status
	CAN_STS_TXREQUEST = 1,	//bit mask of objects pending transmission
	CAN_STS_NEWDAT = 2,			//bit mask of objects with new data
	CAN_STS_MSGVAL = 3,			//bit mask of objects with valid configuration
}tCANStsReg;

typedef enum {
	
	CAN_MSG_OBJ_TYPE_TX = 0,
	CAN_MSG_OBJ_TYPE_RX = 1
	
}tMsgObjType;

typedef enum {
	ZERO_BYTE = 0,
	ONE_BYTE = 1,
	TWO_BYTE = 2,
	THREE_BYTE = 3,
	FOUR_BYTE = 4,
	FIVE_BYTE = 5,
	SIX_BYTE = 6,
	SEVEN_BYTE = 7,
	EIGHT_BYTE = 8,
}tCAN_MSG_LENGTH;
        
typedef struct {
	uint32_t prescaler ;
	uint32_t ui32SJW ;
	uint32_t ui32SyncPropPhase1Seg ;
	uint32_t ui32Phase2Seg ;
}tCANBitClkParms;

typedef struct {	
	uint32_t Msg_ID;              //ARB
	uint32_t Flags;
}tCANConfigTXMsgObj;

typedef struct
{
	uint8_t* Msg_Data ;
	uint8_t Msg_Length ;
}str_TransmitMessageInfo ;

typedef struct {
		uint32_t Msg_ID;
	uint32_t Msg_ID_MSK;
	uint32_t Flags;
}tCANConfigRXMsgObj;


typedef struct {
	
	uint8_t Msg_Length;
	uint8_t Msg_Data[MAX_DATA_LENGTH];		// Array	
}tCANReadRXData;


typedef enum {
	STANDARD_FRAME = 0,
	EXTENDED_FRAME = 1
}tCANFRAME_TYPE;

typedef enum {
	MsgObj1 = 0x01,
	MsgObj2 = 0x02,
	MsgObj3 = 0x03,
	MsgObj4 = 0x04,
	MsgObj5 = 0x05,
	MsgObj6 = 0x06,
	MsgObj7 = 0x07,
	MsgObj8 = 0x08,
	MsgObj9 = 0x09,
	MsgObj10 = 0x0A,
	MsgObj11 = 0x0B,
	MsgObj12 = 0x0C,
	MsgObj13 = 0x0D,
	MsgObj14 = 0x0E,
	MsgObj15 = 0x0F,
	MsgObj16 = 0x10,
	MsgObj17 = 0x11,
	MsgObj18 = 0x12,
	MsgObj19 = 0x13,
	MsgObj20 = 0x14,
	MsgObj21 = 0x15,
	MsgObj22 = 0x16,
	MsgObj23 = 0x17,
	MsgObj24 = 0x18,
	MsgObj25 = 0x19,
	MsgObj26 = 0x1A,
	MsgObj27 = 0x1B,
	MsgObj28 = 0x1C,
	MsgObj29 = 0x1D,
	MsgObj30 = 0x1E,
	MsgObj31 = 0x1F,
	MsgObj32 = 0x20,
	
}MsgObjID;

/* Prototypes*/

void GPIO_Init (Port_Name Port_Base);
void CANBitTimingSet (uint32_t Base, tCANBitClkParms *psClkParms );
void CANDisable (uint32_t Base);
void CANEnable (uint32_t Base);
bool_t CANErrCntrGet (uint32_t ui32Base, uint32_t pui32RxCount, uint32_t pui32TxCount);
void CANInit(uint32_t Base,uint8_t Mode);


void CANIntClear (uint32_t ui32Base, uint32_t ui32IntClr);
void CANIntDisable (uint32_t ui32Base, uint32_t ui32IntFlags);
void CANIntEnable (uint32_t ui32Base, uint32_t ui32IntFlags);
void CANIntRegister (uint32_t ui32Base, void (*pfnHandler)(void));
uint32_t CANIntStatus (uint32_t ui32Base, tCANIntStsReg eIntStsReg);
void CANIntUnregister (uint32_t ui32Base);


void CANTransmitMessageSet (uint32_t Base, uint8_t ObjID, tCANConfigTXMsgObj *MsgObject);
void CANReceiveMessageSet (uint32_t Base, uint8_t ObjID, tCANConfigRXMsgObj *MsgObject);
void CANMessageGet (uint32_t Base, MsgObjID ObjID, tCANReadRXData *psMsgObject, bool_t bClrPendingInt);
void CANMessageClear(uint32_t Base, uint32_t ObjID);
bool_t CANRetryGet (uint32_t Base);
void CANRetrySet (uint32_t Base, bool_t bAutoRetry);
uint32_t CANStatusGet (uint32_t Base, tCANStsReg eStatusReg);
void CAN_Write(uint32_t Base, uint8_t ObjID, str_TransmitMessageInfo *MsgObject);
uint8_t TransmitOk(uint8_t MsgObjID,uint32_t BaseAddress) ;
uint8_t ReceiveOk(uint8_t MsgObjID,uint32_t BaseAddress)  ;

