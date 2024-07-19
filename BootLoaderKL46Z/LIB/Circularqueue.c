/*
 * Circularqueue.c
 *
 *  Created on: Jul 14, 2024
 *      Author: SF
 */


#include "Circularqueue.h"

/*Check full of Queue*/
uint8_t CircurlarQueue_IsFull(CircurlarQueue_type *Queue)
{
  uint8_t Key = FALSE;
  if ((Queue->Front == Queue->Rear + 1) || (Queue->Front == 0 && Queue->Rear == Queue->Size -1))
  {
    Key = TRUE;
  }
   return Key;
}

/*Handling push data in queue*/
void CircurlarQueue_PushData(CircurlarQueue_type *Queue, uint8_t Input_Data)
{
  if(!CircurlarQueue_IsFull(Queue))
  {
    if (Queue->Front == -1)
    {
      Queue->Front = 0;
    }

    Queue->Rear = (Queue->Rear + 1)%Queue->Size;
    Queue->QueueArr[Queue->Rear] = Input_Data;
  }
}
 /*Handling peek data in queue*/
uint8_t CircurlarQueue_PopData(CircurlarQueue_type *Queue)
{
  uint8_t DataOut;
  if(!CircurlarQueue_IsEmpty(Queue))
  {
    DataOut = Queue->QueueArr[Queue->Front];
    if (Queue->Front == Queue->Rear)
    {
      Queue->Front = -1;
      Queue->Rear = -1;
    }
    else
    {
      Queue->Front = (Queue->Front + 1) % Queue->Size;
    }
  }
  return DataOut;
  }
