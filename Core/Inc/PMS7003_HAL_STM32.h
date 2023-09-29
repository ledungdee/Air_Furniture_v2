#ifndef INC_PMS7003_HAL_STM32_H_
#define INC_PMS7003_HAL_STM32_H_


#include "main.h"

/*!<@PMS_MODE_define>*/
#define PMS_MODE_ACTIVE 1		//Mode active: sensor send data continuously (maximum ~800ms interval)
														//interval time can be varied depending on the change rate of collected datas
#define PMS_MODE_PASSIVE 0	//Mode passive: sensor send data when receiving a request
//-----------------------
typedef enum {
	PMS_OK = 1,
	PMS_FAIL = 0,
}PMS_status;


typedef struct {
	UART_HandleTypeDef *PMS_huart;
	uint8_t PMS_MODE;

	uint16_t PM1_0_factory;		 // concentration unit * μg/m3 - under factory environment
	uint16_t PM2_5_factory;
	uint16_t PM10_factory;
	uint16_t PM1_0_atmospheric; // concentration unit * μg/m3 - under atmospheric environment
	uint16_t PM2_5_atmospheric;
	uint16_t PM10_atmospheric;
	uint16_t density_0_3um; 	// number of particles with diameter beyond X(um) in 0.1L of air
	uint16_t density_0_5um;
	uint16_t density_1_0um;
	uint16_t density_2_5um;
	uint16_t density_5_0um;
	uint16_t density_10um;

}PMS_typedef;


extern PMS_status PMS_Init(PMS_typedef *PMS_struct);
extern PMS_status PMS_swmode(PMS_typedef *PMS_struct, uint8_t PMS_MODE);
extern PMS_status PMS_sleep(PMS_typedef *PMS_struct);
extern PMS_status PMS_wakeup(PMS_typedef *PMS_struct);
extern PMS_status PMS_read(PMS_typedef *PMS_struct);

#endif /* INC_PMS7003_HAL_STM32_H_ */
