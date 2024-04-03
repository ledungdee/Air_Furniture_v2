#include "PMS7003_HAL_STM32.h"
#include <string.h>
// Host Protocol
// StartByte1 - StartByte2 - COMMAND - DATA1 - DATA2 - VerifyByte1 - VerifyByte2
//  (0x42)	  -   (0x4d)   -  (CMD)  - DATAH - DATAL -   (LRCH)    -   (LRCL)
// Command definition	   -   0XE2  - 	 X	 -  X	 : read in passive mode
//						   -   0XE1  - 	 X	 -  0x00 : change to passive mode
//						   -   0XE1  - 	 X	 -  0x01 : change to active  mode
//						   -   0XE4  - 	 X	 -  0x00 : change to sleep   mode
//						   -   0XE4  - 	 X	 -  0x01 : change to wake up mode

uint8_t read_passive[7] = {0x42,0x4D,0xe2,0xFF,0xFF,0x03,0x6f};	 // LRC = sum all except verify bytes

uint8_t mode_passive[7] = {0x42,0x4D,0xe1,0xFF,0x00,0x02,0x6f};
uint8_t passive_ack[8]  = {0x42,0x4D,0x00,0x04,0xE1,0x00,0x01,0x74};// acknowledgment from sensor

uint8_t mode_active[7]  = {0x42,0x4D,0xe1,0xFF,0x01,0x02,0x70};
uint8_t active_ack[8]   = {0x42,0x4D,0x00,0x04,0xE1,0x01,0x01,0x75};

uint8_t cmd_sleep[7]    = {0x42,0x4D,0xe4,0xFF,0x00,0x02,0x72};
uint8_t sleep_ack[8]	= {0x42,0x4D,0x00,0x04,0xE4,0x00,0x01,0x77};// acknowledgment from sensor

uint8_t cmd_wakeup[7]   = {0x42,0x4D,0xe4,0xFF,0x01,0x02,0x73};

//PMS7003 transport protocol-Active Mode send back 32bytes data
uint8_t _sleepflag;

uint16_t _framelen;
uint16_t _reserved;
uint16_t _checkcode;

PMS_status PMS_Init(PMS_typedef *PMS_struct);
PMS_status PMS_swmode(PMS_typedef *PMS_struct, uint8_t PMS_MODE);
PMS_status PMS_sleep(PMS_typedef *PMS_struct);
PMS_status PMS_wakeup(PMS_typedef *PMS_struct);
PMS_status PMS_read(PMS_typedef *PMS_struct);
PMS_status _PMS_checksum(uint8_t buff[],uint8_t element);
PMS_status _PMS_checkarray(uint8_t rxbuf[], uint8_t ack[], uint8_t element);


PMS_status PMS_Init(PMS_typedef *PMS_struct)
{
	PMS_status initstatus    = 1;
	uint8_t mode_temp_holder = PMS_struct->PMS_MODE;

	initstatus &= PMS_wakeup(PMS_struct);

	//set up the mode
	switch (mode_temp_holder) {
		case PMS_MODE_ACTIVE:
			// sensor wake up is in active mode by default - do nothing here
			break;
		case PMS_MODE_PASSIVE:
			initstatus &= PMS_swmode(PMS_struct, PMS_MODE_PASSIVE);
			break;
		default:
			break;
	}
	return initstatus;
}

PMS_status PMS_sleep(PMS_typedef *PMS_struct)
{
	uint8_t max_try = 5;

	while(1)
	{
		uint8_t sleep_ackbuf[8] = {0};
		HAL_UART_Transmit(PMS_struct->PMS_huart,cmd_sleep,7,HAL_MAX_DELAY);
		__HAL_UART_CLEAR_FLAG(PMS_struct->PMS_huart, UART_FLAG_NE|UART_FLAG_ORE);
		__HAL_UART_FLUSH_DRREGISTER(PMS_struct->PMS_huart);
		HAL_UART_Receive(PMS_struct->PMS_huart,sleep_ackbuf,8,50);
		max_try--;
		if(_PMS_checkarray(sleep_ackbuf,sleep_ack,8) || !max_try)
		{
			break;
		}
	}
	if(max_try)
	{
		_sleepflag = 1;
		return PMS_OK;
	}
	else
	{
		return PMS_FAIL;
	}
}


PMS_status PMS_wakeup(PMS_typedef *PMS_struct)
{
	uint8_t max_try = 5;

	while(1)
	{
		uint8_t wakeup_ackbuf[32] = {0};
		HAL_UART_Transmit(PMS_struct->PMS_huart,cmd_wakeup,7,HAL_MAX_DELAY);
		__HAL_UART_CLEAR_FLAG(PMS_struct->PMS_huart, UART_FLAG_NE|UART_FLAG_ORE);
		__HAL_UART_FLUSH_DRREGISTER(PMS_struct->PMS_huart);
		HAL_UART_Receive(PMS_struct->PMS_huart,wakeup_ackbuf,32,5000);
		max_try--;
		if(_PMS_checksum(wakeup_ackbuf,32) || !max_try)
		{
			break;
		}
	}

	if(max_try)
	{
		PMS_struct->PMS_MODE = PMS_MODE_ACTIVE;
		_sleepflag = 0;
		return PMS_OK;
	}
	else
	{
		return PMS_FAIL;
	}
}


PMS_status PMS_swmode(PMS_typedef *PMS_struct, uint8_t PMS_MODE)
{
	if((PMS_MODE != PMS_struct->PMS_MODE) && !_sleepflag)
	{
		uint8_t max_try = 5;
		switch(PMS_MODE){
		case PMS_MODE_ACTIVE:
			while(1)
			{
				uint8_t active_ackbuf[8] = {0};
				HAL_UART_Transmit(PMS_struct->PMS_huart,mode_active,7,HAL_MAX_DELAY);
				__HAL_UART_CLEAR_FLAG(PMS_struct->PMS_huart, UART_FLAG_NE|UART_FLAG_ORE);
				__HAL_UART_FLUSH_DRREGISTER(PMS_struct->PMS_huart);
				HAL_UART_Receive(PMS_struct->PMS_huart,active_ackbuf,8,50);
				max_try--;
				if(_PMS_checkarray(active_ackbuf,active_ack,8) || !max_try)
				{
					break;
				}
			}
			break;
		case PMS_MODE_PASSIVE:
			while(1)
			{
				uint8_t passive_ackbuf[8] = {0};
				HAL_UART_Transmit(PMS_struct->PMS_huart,mode_passive,7,HAL_MAX_DELAY);
				__HAL_UART_CLEAR_FLAG(PMS_struct->PMS_huart, UART_FLAG_NE|UART_FLAG_ORE);
				__HAL_UART_FLUSH_DRREGISTER(PMS_struct->PMS_huart);
				HAL_UART_Receive(PMS_struct->PMS_huart,passive_ackbuf,8,50);
				max_try--;
				if(_PMS_checkarray(passive_ackbuf,passive_ack,8) || !max_try)
				{
					break;
				}
			}
			break;
		default:
			break;
		}

		if(max_try)
		{
			PMS_struct->PMS_MODE = PMS_MODE;
			return PMS_OK;
		}
		else
		{
			return PMS_FAIL;
		}
	}
	return PMS_OK;
}

PMS_status PMS_read(PMS_typedef *PMS_struct)
{

	if(!_sleepflag)
	{
		uint8_t rxbuf[32] = {0};
		uint8_t max_try = 2;
		switch(PMS_struct->PMS_MODE)
		{
		case PMS_MODE_PASSIVE:
			while(1)
			{
				HAL_UART_Transmit(PMS_struct->PMS_huart,read_passive,7,HAL_MAX_DELAY);
				__HAL_UART_CLEAR_FLAG(PMS_struct->PMS_huart, UART_FLAG_NE|UART_FLAG_ORE);
				__HAL_UART_FLUSH_DRREGISTER(PMS_struct->PMS_huart);
				HAL_UART_Receive(PMS_struct->PMS_huart,rxbuf,32,100);
				max_try--;
				if( (_PMS_checksum(rxbuf,32)) || (!max_try))
				{
					break;
				}
			}
			break;
		case PMS_MODE_ACTIVE:
			while(1)
			{
				__HAL_UART_CLEAR_FLAG(PMS_struct->PMS_huart, UART_FLAG_NE|UART_FLAG_ORE);
				__HAL_UART_FLUSH_DRREGISTER(PMS_struct->PMS_huart);
				HAL_UART_Receive(PMS_struct->PMS_huart,rxbuf,32,1500);
				max_try--;
				if( (_PMS_checksum(rxbuf,32)) || (!max_try))
				{
					break;
				}
			}
			break;
		default:
			break;
		}
		if(max_try)
		{
			PMS_struct->PM1_0_factory		= (rxbuf[4]	<<8) + rxbuf[5];
			PMS_struct->PM2_5_factory 		= (rxbuf[6]	<<8) + rxbuf[7];
			PMS_struct->PM10_factory 		= (rxbuf[8]	<<8) + rxbuf[9];
			PMS_struct->PM1_0_atmospheric 	= (rxbuf[10]<<8) + rxbuf[11];
			PMS_struct->PM2_5_atmospheric 	= (rxbuf[12]<<8) + rxbuf[13];
			PMS_struct->PM10_atmospheric	= (rxbuf[14]<<8) + rxbuf[15];
			PMS_struct->density_0_3um		= (rxbuf[16]<<8) + rxbuf[17];
			PMS_struct->density_0_5um 		= (rxbuf[18]<<8) + rxbuf[19];
			PMS_struct->density_1_0um 		= (rxbuf[20]<<8) + rxbuf[21];
			PMS_struct->density_2_5um		= (rxbuf[22]<<8) + rxbuf[23];
			PMS_struct->density_5_0um 		= (rxbuf[24]<<8) + rxbuf[25];
			PMS_struct->density_10um 		= (rxbuf[26]<<8) + rxbuf[27];
			return PMS_OK;
		}
		else
		{
			return PMS_FAIL;
		}
	}
	return PMS_FAIL;
}

PMS_status _PMS_checksum(uint8_t *rxbuff,uint8_t element)
{
	if(rxbuff[0]==0x42)
	{
		uint16_t sum = 0;
		uint16_t checksum = 0;
		for (uint8_t var = 0; var < element-3; var++)
		{
			 sum += (uint16_t)rxbuff[var];
		}
		checksum = rxbuff[element-2];
		checksum = checksum << 8UL;
		checksum |= rxbuff[element-1];

		if (sum == checksum)
		{
			return PMS_OK;
		}
		else
		{
			return PMS_FAIL;
		}
	}
	else
	{
		return PMS_FAIL;
	}
}

PMS_status _PMS_checkarray(uint8_t *rxbuf, uint8_t *ack, uint8_t element)
{
  uint8_t pos;
  for(pos = 0; pos < element; pos++)
  {
    if (rxbuf[pos] != ack[pos])
    return PMS_FAIL;
  }
  return PMS_OK;
}
