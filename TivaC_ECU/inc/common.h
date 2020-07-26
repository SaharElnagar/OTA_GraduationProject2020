#ifndef COMMON_H_
#define COMMON_H_


typedef unsigned char uint8_t;
typedef  char sint8_t;
typedef unsigned short  uint16_t;
typedef  signed short  sint16_t;
typedef signed char int8_t;
typedef uint8_t bool_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

#define  TRUE 			1
#define  FALSE			0


#define 	BIT0		0
#define 	BIT1		1
#define 	BIT2		2
#define 	BIT3		3
#define 	BIT4		4
#define 	BIT5		5
#define 	BIT6		6
#define 	BIT7		7
#define 	BIT8		8
#define 	BIT9		9
#define 	BIT10		10
#define 	BIT11		11
#define 	BIT12		12
#define 	BIT13		13
#define 	BIT14		14
#define 	BIT15		15
#define 	BIT16		16
#define 	BIT17		17
#define 	BIT18		18
#define 	BIT19		19
#define 	BIT20		20
#define 	BIT21		21
#define 	BIT22		22
#define 	BIT23		23
#define 	BIT24		24
#define 	BIT25		25
#define 	BIT26		26
#define 	BIT27		27
#define 	BIT28		28
#define 	BIT29		29
#define 	BIT30		30
#define 	BIT30		30
#define 	BIT31		31
#define 	BIT32		32


#define Test_Mode 1
#define Real_Mode 0 

//#define ACCESS_REG_8BIT(REG)         (*(volatile uint8*)REG)

//#define ACCESS_REG_16BIT(REG)         (*(volatile uint16*)REG)

//#define READ_REG_8BIT(REG)           (*(volatile uint8*)(REG))

//#define READ_REG_16BIT(REG)           (*(volatile uint16*)(REG))

//#define WRITE_REG_8BIT(REG,Val)      (ACCESS_REG_8BIT(REG))=(Val);

#define ACCESS_REG(REG)								(*(volatile uint32_t*)(REG))

#define	READ_REG(REG)									(*(volatile uint32_t*) (REG))

#define	WRITE_REG(REG, VAL)			 			((*(volatile uint32_t*) (REG)) = VAL)

#define SET_BIT(REG,BIT_OFFSET)		 		((*(volatile uint32_t*) (REG)) |= (BIT_OFFSET))

#define CLEAR_BIT(REG,BIT_OFFSET)     ((*(volatile uint32_t*) (REG)) &= (~(BIT_OFFSET)))

#define BIT_IS_SET(REG,BIT_OFFSET)    ((*(volatile uint32_t*) (REG)) & (BIT_OFFSET))

#define BIT_IS_CLEAR(REG,BIT_OFFSET)  (!(BIT_IS_SET((REG),BIT_OFFSET)))

#define TOGGLE_BIT(REG,BIT_OFFSET)    ((*(volatile uint32_t*) (REG)) = (*(volatile uint32_t*) (REG)) ^ (BIT_OFFSET))



#endif /* COMMON_H_ */
