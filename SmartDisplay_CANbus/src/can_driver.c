#include "canfestival.h"

uint8_t sendAck = 0;
Message msgAck = {0};

void pushCan();

/**
  * @brief  canSend
	* @param  CANx:CAN1 or CAN2   m:can message
  * @retval 0ㄩSuccess
  */
unsigned char canSend(CAN_PORT Canport, Message *m)	                
{
  pushCan(m);
  
  return 0;
}