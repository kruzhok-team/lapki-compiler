/*
 * mem_msd_glue.h
 *
 *  Created on: 30 ���. 2016 �.
 *      Author: Kreyl
 */

#ifndef MEM_MSD_GLUE_H__
#define MEM_MSD_GLUE_H__

#include "types.h"

namespace MsdMem {

extern uint32_t block_cnt, block_sz;

retv Read(uint32_t BlockAddress, uint8_t *ptr, uint32_t BlocksCnt);
retv Write(uint32_t BlockAddress, uint8_t *ptr, uint32_t BlocksCnt);

} // namespace

#endif // MEM_MSD_GLUE_H__
