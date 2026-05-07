/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2007        */
/*-----------------------------------------------------------------------*/
/* This is a stub disk I/O module that acts as front end of the existing */
/* disk I/O modules and attach it to FatFs module with common interface. */
/*-----------------------------------------------------------------------*/

#include "gd_lib.h"
#include "ffconf.h"
#include "diskio.h"
#include "string.h"

#include "mem_msd_glue.h"

__attribute__((unused))
extern void PrintfC(const char *format, ...);

/*-----------------------------------------------------------------------*/
/* Initialize a Drive                                                    */
extern "C"
DSTATUS disk_initialize (
    BYTE drv                /* Physical drive nmuber (0..) */
)
{
    return 0;
}

/*-----------------------------------------------------------------------*/
/* Return Disk Status                                                    */
extern "C"
DSTATUS disk_status (
    BYTE drv        /* Physical drive nmuber (0..) */
)
{
    return 0;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
extern "C"
DRESULT disk_read (
    BYTE drv,        /* Physical drive nmuber (0..) */
    BYTE *buff,        /* Data buffer to store read data */
    DWORD sector,    /* Sector address (LBA) */
    BYTE count        /* Number of sectors to read (1..255) */
)
{
    if(MsdMem::Read(sector, buff, count) == retv::Ok) return RES_OK;
    else return RES_ERROR;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */

#if _READONLY == 0
extern "C"
DRESULT disk_write (
    BYTE drv,            /* Physical drive nmuber (0..) */
    const BYTE *buff,    /* Data to be written */
    DWORD sector,        /* Sector address (LBA) */
    BYTE count           /* Number of sectors to write (1..255) */
)
{
    if(MsdMem::Write(sector, (uint8_t*)buff, count) == retv::Ok) return RES_OK;
    else return RES_ERROR;
}
#endif /* _READONLY */

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
extern "C"
DRESULT disk_ioctl (
    BYTE drv,        /* Physical drive nmuber (0..) */
    BYTE ctrl,        /* Control code */
    void *buff        /* Buffer to send/receive control data */
)
{
    switch (ctrl) {
    case CTRL_SYNC:
        return RES_OK;
    case GET_SECTOR_COUNT: /* Get number of sectors on the drive */
        *((DWORD *)buff) = MsdMem::block_cnt;
        return RES_OK;
    case GET_SECTOR_SIZE: /* Get size of sector for generic read/write */
        *((WORD *)buff) = MsdMem::block_sz;
        return RES_OK;
    case GET_BLOCK_SIZE: /* Get internal block size in unit of sector */
        *((DWORD *)buff) = 256; /* 512b blocks in one erase block */
        return RES_OK;
    default:
        return RES_PARERR;
    }
  return RES_PARERR;
}

extern "C"
DWORD get_fattime(void) {
    return ((uint32_t)0 | (1 << 16)) | (1 << 21); /* wrong but valid time */
}
