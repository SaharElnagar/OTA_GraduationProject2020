/*
 * Queue.h
 *
 *
 *      Author: Yomna
 */

#ifndef BSW_STATIC_SERVICE_NVM_INC_QUEUE_H_
#define BSW_STATIC_SERVICE_NVM_INC_QUEUE_H_

#include "Std_Types.h"
#include "NvM_PrivateTypes.h"

/*Size of CRC job queue*/
#define NVM_SIZE_CRC_JOB_QUEUE      (10U)

void Init_Queues(void) ;
Std_ReturnType Search_Queue(NvM_BlockIdType BlockId) ;
Std_ReturnType Job_Enqueue(Job_Parameters Job) ;
Std_ReturnType Job_Dequeue(void) ;
void Get_SingleJob(Job_Parameters* Job) ;
Std_ReturnType CRCJob_Enqueue(NvM_BlockIdType BlockId) ;
Std_ReturnType CRCJob_Dequeue(void) ;


#endif /* BSW_STATIC_SERVICE_NVM_INC_QUEUE_H_ */
