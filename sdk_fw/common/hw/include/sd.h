/*
 * sd.h
 *
 *  Created on: 2020. 2. 21.
 *      Author: Baram
 */

#ifndef SRC_COMMON_HW_INCLUDE_SD_H_
#define SRC_COMMON_HW_INCLUDE_SD_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "hw_def.h"

#ifdef _USE_HW_SD

#define SD_PIN_DETECTED  HW_SD_PIN_DETECTED
#define SD_PIN_PWREN     HW_SD_PIN_PWREN

typedef struct
{
  uint32_t card_type;                    /*!< Specifies the card Type                         */
  uint32_t card_version;                 /*!< Specifies the card version                      */
  uint32_t card_class;                   /*!< Specifies the class of the card class           */
  uint32_t rel_card_Add;                 /*!< Specifies the Relative Card Address             */
  uint32_t block_numbers;                /*!< Specifies the Card Capacity in blocks           */
  uint32_t block_size;                   /*!< Specifies one block size in bytes               */
  uint32_t log_block_numbers;            /*!< Specifies the Card logical Capacity in blocks   */
  uint32_t log_block_size;               /*!< Specifies logical block size in bytes           */
  uint32_t card_size;
  uint32_t freq;

} sd_info_t;



bool sdInit(void);
bool sdIsInit(void);
bool sdDeInit(void);
void sdTest(void);

bool sdReadBlocks(uint32_t block_addr, uint8_t *p_data, uint32_t num_of_blocks, uint32_t timeout_ms);
bool sdWriteBlocks(uint32_t block_addr, uint8_t *p_data, uint32_t num_of_blocks, uint32_t timeout_ms);
bool sdEraseBlocks(uint32_t start_addr, uint32_t end_addr);
bool sdIsBusy(void);
bool sdIsDetected(void);
bool sdGetInfo(sd_info_t *p_info);


#endif /* _USE_HW_LED */

#ifdef __cplusplus
}
#endif



#endif /* SRC_COMMON_HW_INCLUDE_SD_H_ */
