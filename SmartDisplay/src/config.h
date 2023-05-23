/*
This file is part of CanFestival, a library implementing CanOpen Stack.

Copyright (C): Edouard TISSERANT and Francis DUPIN
AVR Port: Andreas GLAUSER and Peter CHRISTEN

See COPYING file for copyrights details.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef _CONFIG_H_
#define _CONFIG_H_


#define CAN_BAUDRATE                            500

// Needed defines by Canfestival lib
#define MAX_CAN_BUS_ID                          1
#define SDO_MAX_LENGTH_TRANSFER                 100
#define SDO_BLOCK_SIZE                          32
#define SDO_MAX_SIMULTANEOUS_TRANSFERS          5
#define NMT_MAX_NODE_ID                         128
#define SDO_TIMEOUT_MS                          100U
#define MAX_NB_TIMER                            16

// #define SDO_DYNAMIC_BUFFER_ALLOCATION           0
// #define SDO_DYNAMIC_BUFFER_ALLOCATION_SIZE      256 // 60 * 1024 // 320*480*2


// CANOPEN_BIG_ENDIAN is not defined
#define CANOPEN_LITTLE_ENDIAN                   1

#define US_TO_TIMEVAL_FACTOR                    8

#define REPEAT_SDO_MAX_SIMULTANEOUS_TRANSFERS_TIMES(repeat)\
repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat
#define REPEAT_NMT_MAX_NODE_ID_TIMES(repeat)\
repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat

#define EMCY_MAX_ERRORS                         8
#define REPEAT_EMCY_MAX_ERRORS_TIMES(repeat)\
repeat repeat repeat repeat repeat repeat repeat repeat


#endif /* _CONFIG_H_ */
