/*
 * Circularqueue.h
 *
 *  Created on: Jul 14, 2024
 *      Author: SF
 */

#ifndef CIRCULARQUEUE_H_
#define CIRCULARQUEUE_H_

#include "stdint.h"


#define MAX_SIZE_QUEUE	        (600u) //528u
#define TRUE					(1u)
#define FALSE					(0u)
#define QUEUE_EMPTY 			(0U)
#define QUEUE_EMPTY_PEEK_VALUE 	(9U)

/* define queue types using struct */
typedef struct
{
  int8_t Front, Rear,Size;
  uint8_t *QueueArr;
} CircurlarQueue_type;

/*Function Prototypes */
/*Check full of Queue*/
uint8_t CircurlarQueue_IsFull(CircurlarQueue_type *Queue);

/*Check empty of Queue*/
/*Check empty of Queue*/
static inline uint8_t CircurlarQueue_IsEmpty(CircurlarQueue_type *Queue)
{
  uint8_t Key = FALSE;
  if (Queue->Front == -1)
  {
    Key = TRUE;
  }
  return Key;
}
/*Handling push data in queue*/
void CircurlarQueue_PushData(CircurlarQueue_type *Queue, uint8_t Input_Data);

/*Handling peek data in queue*/
uint8_t CircurlarQueue_PopData(CircurlarQueue_type *Queue);

#endif /* CIRCULARQUEUE_H_ */

