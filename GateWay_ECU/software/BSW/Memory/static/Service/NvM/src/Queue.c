/*
 * Queue.c
 *
 *
 *      Author: Yomna
 */


#include "Queue.h"
#include "NvM_Cfg.h"

/*[SWS_NvM_00195] The function NvM_ReadBlock shall take over the given parameters,
 *queue the read request in the job queue and return
 *Hint: this requirement is the same for all asynchronous single block requests.
 */
/* standard job queue */
 Job_Parameters Standard_Job_Queue[NVM_SIZE_STANDARD_JOB_QUEUE] ;
 Queue_Indices_Struct Stand_Queue_Indeces = {0, 0} ;


 /*[SWS_NvM_00378]
  * In case of priority based job processing order,
  * the NvM module shall use two queues, one for immediate write jobs (crash data) another for all other jobs.
  */
 /* immediate job queue */
 #if (NVM_JOB_PRIORITIZATION == STD_ON)
   static Job_Parameters Immediate_Job_Queue[NVM_SIZE_IMMEDIATE_JOB_QUEUE] ;
   static Queue_Indices_Struct Immed_Queue_Indeces = {0, 0} ;
 #endif

/*[SWS_NvM_00121] For blocks with a permanently configured RAM,
* the function NvM_SetRamBlockStatus shall request the recalculation of
* CRC in the background, i.e. the CRC recalculation shall be processed by
* the NvM_MainFunction, if the given “BlockChanged” parameter is TRUE and
* CRC calculation in RAM is configured.
*/
/* CRC job queue */
 NvM_BlockIdType CRC_Job_Queue[NVM_SIZE_CRC_JOB_QUEUE] ;
 Queue_Indices_Struct CRC_Queue_Indeces = {0, 0} ;


 /*Blocks configurations */
 extern NvMBlockDescriptorType NvMBlockDescriptor[NUMBER_OF_NVM_BLOCKS] ;
/*****************************************************************************************/
/*                                   Queue Flags                                         */
/*****************************************************************************************/
 boolean Standard_Queue_Empty = TRUE ;
 boolean Standard_Queue_FULL = FALSE ;

#if (NVM_JOB_PRIORITIZATION == STD_ON)
  boolean Immediate_Queue_Empty = TRUE ;
  boolean Immediate_Queue_FULL = FALSE ;
#endif

 boolean CRC_Queue_Empty = TRUE ;
 boolean CRC_Queue_Full = FALSE ;


/****************************************************************************************/
/*    Function Name           : Init_Queue                                              */
/*    Function Description    : put queue in it's initialized state                     */
/*    Parameter in            : none                                                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : none                                                    */
/****************************************************************************************/

void Init_Queues(void)
{
    /*counter to loop over queue elements*/
    NvM_BlockIdType counter ;

    #if(NVM_JOB_PRIORITIZATION == STD_ON)
      Immed_Queue_Indeces.Head = 0 ;
      Immed_Queue_Indeces.Tail = 0 ;

      Immediate_Queue_Empty = TRUE ;
      Immediate_Queue_FULL = FALSE ;
    #endif

    Stand_Queue_Indeces.Head = 0 ;
    Stand_Queue_Indeces.Tail = 0 ;

    Standard_Queue_Empty = TRUE ;
    Standard_Queue_FULL = FALSE ;

    CRC_Queue_Indeces.Head = 0 ;
    CRC_Queue_Indeces.Tail = 0 ;

    CRC_Queue_Empty = TRUE ;
    CRC_Queue_Full = FALSE ;


    #if(NVM_JOB_PRIORITIZATION == STD_ON)
        for(counter = 0; counter < NVM_SIZE_IMMEDIATE_JOB_QUEUE; counter++)
        {
            Immediate_Job_Queue[counter].ServiceId = 0 ;
            Immediate_Job_Queue[counter].Block_Id = 0 ;
            Immediate_Job_Queue[counter].RAM_Ptr = NULL ;
        }
    #endif

    for(counter = 0; counter < NVM_SIZE_STANDARD_JOB_QUEUE; counter++)
    {
        Standard_Job_Queue[counter].ServiceId = 0 ;
        Standard_Job_Queue[counter].Block_Id = 0 ;
        Standard_Job_Queue[counter].RAM_Ptr = NULL ;
    }

    for(counter = 0; counter < NVM_SIZE_CRC_JOB_QUEUE ; counter++)
    {
        CRC_Job_Queue[counter] = 0 ;
    }
}

/****************************************************************************************/
/*    Function Name           : Search_Queue                                            */
/*    Function Description    : Search for a passed Block Id in the queue,              */
/*                              and find if it exists or not.                           */
/*                              If existed -> return E_OK                               */
/*                              If not existed -> return E_NOT_OK                       */
/*    Parameter in            : BlockId                                                 */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : Std_ReturnType                                          */
/****************************************************************************************/

Std_ReturnType Search_Queue(NvM_BlockIdType BlockId)
{
    /*counter to loop over queue elements*/
    uint16 counter ;
    /*Variable to save return value*/
    Std_ReturnType Return_Val = E_NOT_OK ;

    #if(NVM_JOB_PRIORITIZATION == STD_ON)
        for(counter = 0; counter < NVM_SIZE_IMMEDIATE_JOB_QUEUE; counter++){
           if(Immediate_Job_Queue[counter].Block_Id == BlockId){
               Return_Val = E_OK ;
               break ;
           }
        }
    #endif
    for(counter = 0; counter < NVM_SIZE_STANDARD_JOB_QUEUE; counter++){
       if(Standard_Job_Queue[counter].Block_Id == BlockId){
           Return_Val = E_OK ;
           break ;
       }
    }

    return Return_Val ;
}

/****************************************************************************************/
/*    Function Name           : Job_Enqueue                                             */
/*    Function Description    : Add jobs to the queue to be executed later              */
/*    Parameter in            : Job                                                     */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : Std_ReturnType                                          */
/****************************************************************************************/

Std_ReturnType Job_Enqueue(Job_Parameters Job)
{

  /*[SWS_NvM_00378]
   * In case of priority based job processing order,
   * the NvM module shall use two queues, one for immediate write jobs (crash data)
   * another for all other jobs
   */
  // Case1 : Immediate Job
  if(NvMBlockDescriptor[Job.Block_Id].NvMBlockJobPriority == 0){
    #if (NVM_JOB_PRIORITIZATION == STD_ON)
      if(Immediate_Queue_FULL == TRUE){
        return E_NOT_OK ;
      }

      Immediate_Job_Queue[Immed_Queue_Indeces.Tail] = Job ;

      if(Immediate_Queue_Empty == TRUE){
        Immediate_Queue_Empty = FALSE ;
      }

      Immed_Queue_Indeces.Tail++ ;

      //When Tail reaches queue end
      if(Immed_Queue_Indeces.Tail == NVM_SIZE_IMMEDIATE_JOB_QUEUE){
        /*Go to index 0 of the queue again
         *As the queue is circular
         */
        Immed_Queue_Indeces.Tail = 0 ;
      }

      //When Tail reaches Head while enqueing, the queue is full
      if(Immed_Queue_Indeces.Tail == Immed_Queue_Indeces.Head){
        Immediate_Queue_FULL = TRUE ;
      }
      return E_OK ;
    #endif
  }

  // Case2 : Standard Job
  else{

    if(Standard_Queue_FULL == TRUE){
      return E_NOT_OK ;
    }

    // if queue is empty, so insert your job directly
    if(Standard_Queue_Empty == TRUE){
      Standard_Job_Queue[Stand_Queue_Indeces.Tail] = Job ;
      Standard_Queue_Empty = FALSE ;
    }
    else{ // Queue is not full and not empty


      #if (NVM_JOB_PRIORITIZATION == STD_ON)

        uint16 counter ;   //internal variable to store the loop index
        //intermediate variable to store ID of the compared job in each cycle
        NvM_BlockIdType Compared_Job_Id ;

        /*insert the new job based on priority.
        *loop over queue elements starting from tail until you reach head,
        *or reach a higher priority job
        */
        for(counter = Stand_Queue_Indeces.Tail ; counter != Stand_Queue_Indeces.Head; counter--){
          // if counter = 0
          if(counter == 0){
            Compared_Job_Id = Standard_Job_Queue[NVM_SIZE_STANDARD_JOB_QUEUE -1].Block_Id ;
            if(NvMBlockDescriptor[Job.Block_Id].NvMBlockJobPriority < NvMBlockDescriptor[Compared_Job_Id].NvMBlockJobPriority){
              Standard_Job_Queue[counter] = Standard_Job_Queue[NVM_SIZE_STANDARD_JOB_QUEUE -1] ;
              counter = NVM_SIZE_STANDARD_JOB_QUEUE ;
            }
            else{
              break ;
            }
          }
          // if counter != 0
          else{
            Compared_Job_Id = Standard_Job_Queue[counter-1].Block_Id ;
            if(NvMBlockDescriptor[Job.Block_Id].NvMBlockJobPriority < NvMBlockDescriptor[Compared_Job_Id].NvMBlockJobPriority){
              Standard_Job_Queue[counter] = Standard_Job_Queue[counter-1] ;
            }
            else{
              break ;
            }
          }
        }
        Standard_Job_Queue[counter] = Job ;

    /*
     * [SWS_NvM_00379]
     * If priority based job processing is disabled via configuration,
     * the NvM module shall not support immediate write jobs. In this case,
     * the NvM module processes all jobs in FCFS order
     */
      #else
        Standard_Job_Queue[Stand_Queue_Indeces.Tail] = Job ;
      #endif
    }

    Stand_Queue_Indeces.Tail++ ;

    //When Tail reaches queue end
    if(Stand_Queue_Indeces.Tail == NVM_SIZE_STANDARD_JOB_QUEUE){
      Stand_Queue_Indeces.Tail = 0 ;
    }

      //When Tail reaches Head while enqueing, the queue is full
    if(Stand_Queue_Indeces.Tail == Stand_Queue_Indeces.Head){
      Standard_Queue_FULL = TRUE ;
    }
  }
  return E_OK;
}

/****************************************************************************************/
/*    Function Name           : Job_Dequeue                                             */
/*    Function Description    : Remove a single job from the queue                      */
/*    Parameter in            : none                                                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : Std_ReturnType                                          */
/****************************************************************************************/

Std_ReturnType Job_Dequeue(void)
{
 #if (NVM_JOB_PRIORITIZATION == STD_ON)
   //Immediate queue is not empty, so dequeue immediate job
   if(Immediate_Queue_Empty == FALSE){

     Immediate_Job_Queue[Immed_Queue_Indeces.Head].Block_Id = 0 ;
     Immediate_Job_Queue[Immed_Queue_Indeces.Head].ServiceId = 0 ;
     Immediate_Job_Queue[Immed_Queue_Indeces.Head].RAM_Ptr = NULL ;

     if(Immediate_Queue_FULL == TRUE){
       Immediate_Queue_FULL = FALSE ;
     }

     Immed_Queue_Indeces.Head++ ;

     if(Immed_Queue_Indeces.Head == NVM_SIZE_IMMEDIATE_JOB_QUEUE){
       Immed_Queue_Indeces.Head = 0 ;
     }

     if(Immed_Queue_Indeces.Head == Immed_Queue_Indeces.Tail){
       Immediate_Queue_Empty = TRUE ;
     }
     return E_OK ;
  }
 #endif

  //Immediate queue is empty and standard queue is empty, so return error
  if(Standard_Queue_Empty == TRUE){
    return E_NOT_OK ;
  }
  //Immediate queue is empty and standard queue is not empty,
  //so dequeue standard jobs
  else{

    Standard_Job_Queue[Stand_Queue_Indeces.Head].Block_Id = 0 ;
    Standard_Job_Queue[Stand_Queue_Indeces.Head].ServiceId = 0 ;
    Standard_Job_Queue[Stand_Queue_Indeces.Head].RAM_Ptr = NULL ;

    if(Standard_Queue_FULL == TRUE){
      Standard_Queue_FULL = FALSE ;
    }

    Stand_Queue_Indeces.Head++ ;

    if(Stand_Queue_Indeces.Head == NVM_SIZE_STANDARD_JOB_QUEUE){
      Stand_Queue_Indeces.Head = 0 ;
    }

    if(Stand_Queue_Indeces.Head == Stand_Queue_Indeces.Tail){
      Standard_Queue_Empty = TRUE ;
    }

    return E_OK ;
  }

}

/****************************************************************************************/
/*    Function Name           : Get_SingleJob                                           */
/*    Function Description    : copy a single job parameters from queue to be executed  */
/*    Parameter in            : none                                                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : none                                                    */
/****************************************************************************************/

void Get_SingleJob(Job_Parameters* Job)
{
    #if(NVM_JOB_PRIORITIZATION == STD_ON)

        if(Immediate_Queue_Empty == FALSE){

            *Job = Immediate_Job_Queue[Immed_Queue_Indeces.Head] ;
        }
        else if(Standard_Queue_Empty == FALSE){

            *Job = Standard_Job_Queue[Stand_Queue_Indeces.Head] ;
        }
        else {

            Job->ServiceId = 0 ;
            Job->Block_Id = 0 ;
            Job->RAM_Ptr = NULL ;
        }
        #else

             if(Standard_Queue_Empty == FALSE){

                *Job = Standard_Job_Queue[Stand_Queue_Indeces.Head] ;
             }
             else {

                 Job->ServiceId = 0 ;
                 Job->Block_Id = 0 ;
                 Job->RAM_Ptr = NULL ;
             }

         #endif

}

/****************************************************************************************/
/*    Function Name           : CRCJob_Enqueue                                          */
/*    Function Description    : Enqueue CRC Jobs to be processed inside main function.  */
/*    Parameter in            : BlockId                                                 */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : Std_ReturnType                                          */
/*    Requirement             :                                                         */
/*    Notes                   :                                                         */
/****************************************************************************************/

Std_ReturnType CRCJob_Enqueue(NvM_BlockIdType BlockId)
{
    Std_ReturnType Return_Val ;

    if(CRC_Queue_Full == TRUE)
    {
        Return_Val = E_NOT_OK ;
    }
    else
    {
        /* insert the new CRC job*/
        CRC_Job_Queue[CRC_Queue_Indeces.Tail] = BlockId ;
        /* increment the tail pointer */
        CRC_Queue_Indeces.Tail++ ;

        /* When Tail reaches queue end */
        if(CRC_Queue_Indeces.Tail == NVM_SIZE_CRC_JOB_QUEUE)
        {
            /* Return the tail pointer to the queue start again */
            CRC_Queue_Indeces.Tail = 0 ;
        }

        /* if the queue was empty, mark it as not empty */
        if(CRC_Queue_Empty == TRUE)
        {
            CRC_Queue_Empty = FALSE ;
        }

        /* When Tail reaches Head while Enqueuing, the queue is full */
        if(CRC_Queue_Indeces.Tail == CRC_Queue_Indeces.Head)
        {
            CRC_Queue_Full = TRUE ;
        }
        Return_Val = E_OK ;
    }

    return Return_Val ;

}

/****************************************************************************************/
/*    Function Name           : CRCJob_Dequeue                                          */
/*    Function Description    : Dequeue CRC Jobs                                        */
/*    Parameter in            : none                                                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : Std_ReturnType                                          */
/*    Requirement             :                                                         */
/*    Notes                   :                                                         */
/****************************************************************************************/

 Std_ReturnType CRCJob_Dequeue( void )
{
     Std_ReturnType Return_Val ;


     if(CRC_Queue_Empty == TRUE)
     {
         Return_Val = E_NOT_OK ;
     }
     else
     {
         CRC_Job_Queue[CRC_Queue_Indeces.Head] = 0 ;
         CRC_Queue_Indeces.Head++ ;

         /* if the queue was full, mark it as not full */
         if(CRC_Queue_Full == TRUE)
         {
             CRC_Queue_Full = FALSE ;
         }

         /* When Head reaches queue end */
         if(CRC_Queue_Indeces.Head == NVM_SIZE_CRC_JOB_QUEUE)
         {
             /* Return the head pointer to the queue start again */
             CRC_Queue_Indeces.Head = 0 ;
         }

         /* When Head reaches Tail while Dequeuing, the queue is empty */
         if(CRC_Queue_Indeces.Head == CRC_Queue_Indeces.Tail)
         {
             CRC_Queue_Empty = TRUE ;
         }

         Return_Val = E_OK ;
     }

     return Return_Val ;
}


