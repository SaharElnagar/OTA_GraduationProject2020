
#ifndef STD_TYPES_H
#define STD_TYPES_H

#include "Platform_Types.h"


#ifndef NULL
#define	NULL	0
#endif

typedef struct {
	uint16 vendorID;
	uint16 moduleID;
	uint8  instanceID;
/** Vendor numbers */
	uint8 sw_major_version;
	uint8 sw_minor_version;
	uint8 sw_patch_version;
/** Autosar spec. numbers */
	uint8 ar_major_version;
	uint8 ar_minor_version;
	uint8 ar_patch_version;
} Std_VersionInfoType;



typedef uint8 Std_ReturnType;
#define E_OK                    ((Std_ReturnType)0U)
#define E_NOT_OK                ((Std_ReturnType)1U)
#define E_PENDING               ((Std_ReturnType)2U)


#define STD_HIGH		0x01
#define STD_LOW			0x00

#define STD_ACTIVE		0x01
#define STD_IDLE		0x00

#define STD_ON			0x01
#define STD_OFF			0x00

#define STD_TYPES_AR_RELEASE_MAJOR_VERSION   (4U)
#define STD_TYPES_AR_RELEASE_MINOR_VERSION   (3U)
#define STD_TYPES_AR_RELEASE_PATCH_VERSION   (1U)

#endif

