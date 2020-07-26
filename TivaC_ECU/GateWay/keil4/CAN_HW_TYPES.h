#ifndef CAN_HW_TYPES_
#define CAN_HW_TYPES_

#ifndef CAN0_BASE
#define CAN0_BASE   0x40040000
#endif
#ifndef CAN1_BASE
#define CAN1_BASE 	0x40041000
#endif


#define SYSCTL_RCGC0 				0x400FE100

#define 	CAN_CTL_R			    0x00
#define		CAN_STS_R			    0x004
#define		CAN_ERR_R			    0x008
#define		CAN_BIT_R			    0x00C
#define 	CAN_INT_R			    0x010
#define 	CAN_TST_R				0x014
#define		CAN_BRPE			    0x018


#define		CAN_IF1CRQ_R			0x020
#define 	CAN_IF2CRQ_R			0x080
#define 	CAN_IF1CMSK_R    	0x024
#define		CAN_IF2CMSK_R	    0x084
#define		CAN_IF1MSK1_R			0x028
#define		CAN_IF2MSK1_R			0x088
#define		CAN_IF1MSK2_R			0x02C
#define		CAN_IF2MSK2_R			0x08C
#define		CAN_IF1ARB1_R			0x030
#define		CAN_IF2ARB1_R			0x090
#define		CAN_IF1ARB2_R			0x034
#define		CAN_IF2ARB2_R			0x094
#define		CAN_IF1MCTL_R			0x038
#define		CAN_IF2MCTL_R			0x098
#define		CAN_IF1DA1_R			0x03C
#define		CAN_IF1DA2_R			0x040
#define 	CAN_IF1DB1_R			0x044
#define		CAN_IF1DB2_R			0x048
#define		CAN_IF2DA1_R			0x09C
#define		CAN_IF2DA2_R			0x0A0
#define		CAN_IF2DB1_R			0x0A4
#define		CAN_IF2DB2_R			0x0A8


#define		CAN_TXRQ1_R				0x100
#define		CAN_TXRQ2_R				0x104
#define		CAN_NWDA1_R				0x120
#define		CAN_NWDA2_R				0x124
#define		CAN_MSG1INT_R			0x140
#define		CAN_MSG2INT_R			0x144
#define		CAN_MSG1VAL_R			0x160
#define		CAN_MSG2VAL_R			0x164




#endif

