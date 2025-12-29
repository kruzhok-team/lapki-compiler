/*
 * mem_msd_glue.cpp
 *
 *  Created on: 30 ���. 2016 �.
 *      Author: Kreyl
 */

#include "mem_msd_glue.h"
#include "shell.h"
#include "SpiFlash.h"

//#include "diskio.h"

extern SpiFlash spi_flash;

namespace MsdMem {

uint32_t block_cnt, block_sz;

retv Read(uint32_t BlockAddress, uint8_t *ptr, uint32_t BlocksCnt) {
//    Printf("R %u %u\r", BlockAddress, BlocksCnt);
    return spi_flash.ReadQ(BlockAddress * block_sz, ptr, BlocksCnt * block_sz);

//    Mem.Read(BlockAddress * MSD_BLOCK_SZ, ptr, BlocksCnt * MSD_BLOCK_SZ);
//    if(disk_read(0, ptr, BlockAddress, BlocksCnt) == RES_OK) return retvOk;
//    else return retvFail;
//    memcpy(ptr, &buf[BlockAddress*MsdBlockSz], BlocksCnt*MsdBlockSz);
//    return retv::Ok;
}

retv Write(uint32_t BlockAddress, uint8_t *ptr, uint32_t BlocksCnt) {
//    Printf("W %u %u\r", BlockAddress, BlocksCnt);
    uint32_t PageCnt = (BlocksCnt * block_sz) / SPIFLASH_PAGE_SZ;
    // Erase sectors
    uint32_t Addr = BlockAddress * block_sz;
    while(BlocksCnt) {
        if(spi_flash.EraseSector4k(Addr) != retv::Ok) return retv::Fail;
        Addr += block_sz;
        BlocksCnt--;
    }
    // Write data page by page
    Addr = BlockAddress * block_sz;
    while(PageCnt) {
//        if(spi_flash.WritePage(Addr, ptr, SPIFLASH_PAGE_SZ) != retv::Ok) {
        if(spi_flash.WritePageQ(Addr, ptr, SPIFLASH_PAGE_SZ) != retv::Ok) return retv::Fail;
        Addr += SPIFLASH_PAGE_SZ;
        ptr += SPIFLASH_PAGE_SZ;
        PageCnt--;
    }
    return retv::Ok;
//    return spi_flash.wr
//    memcpy(&buf[BlockAddress*MsdBlockSz], ptr, BlocksCnt*MsdBlockSz);
//    if(disk_write(0, ptr, BlockAddress, BlocksCnt) == RES_OK) return retvOk;
//    else return retvFail;
//    while(BlocksCnt != 0) {
        // Calculate Mem Sector addr
//        uint32_t SectorStartAddr = BlockAddress * MEM_SECTOR_SZ;
        // Write renewed sector
//        if(Mem.EraseAndWriteSector4k(SectorStartAddr, ptr) != OK) return FAILURE;
        // Process variables
//        BlockAddress += MSD_BLOCK_SZ;
//        BlocksCnt--;
//    }
//    return retv::Ok;
}

} // namespace
