
/*
 * MemIf_Types.h
 *
 *  Created on: Nov 24, 2019
 *      Author: Sahar
 */

#ifndef MEMIF_TYPES_H_
#define MEMIF_TYPES_H_

#include "Platform_Types.h"

typedef uint8 MemIf_JobResultType;
#define     MEMIF_JOB_OK                (0U)
#define     MEMIF_JOB_FAILED            (1U)
#define     MEMIF_JOB_PENDING           (2U)
#define     MEMIF_JOB_CANCELED          (3U)
#define     MEMIF_BLOCK_INCONSISTENT    (4U)
#define     MEMIF_BLOCK_INVALID         (5U)

typedef uint8 MemIf_StatusType;
/* The underlying abstraction module or device driver has not been initialized (yet).*/
#define     MEMIF_UNINIT            (0U)

/*The underlying abstraction module or device driver is currently idle. */
#define     MEMIF_IDLE              (1U)

/*The underlying abstraction module or device driver is currently busy.*/
#define     MEMIF_BUSY              (2U)

/*The underlying abstraction module is busy with internal management operations.
 *  The underlying device driver can be busy or idle.
 */
#define     MEMIF_BUSY_INTERNAL     (3U)

typedef uint8 MemIf_ModeType;
#define MEMIF_MODE_SLOW             (0U)
#define MEMIF_MODE_FAST             (1U)






#endif /* MEMIF_TYPES_H_ */


