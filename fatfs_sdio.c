/* 
 * The MIT License (MIT)
 *
 * Copyright (c) 2022 nzc21489
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#pragma GCC optimize("O2")
#include <stdio.h>
#include "pico/sdio.h"
#include "ff.h"
#include "diskio.h"
#include <string.h>
DSTATUS Stat = STA_NOINIT;

DSTATUS disk_status(
    BYTE pdrv /* [IN] Physical drive number */
)
{
    if (pdrv != 0)
    {
        return STA_NOINIT; /* Supports only single drive */
    }
    else
    {
        return Stat;
    }
}

DSTATUS disk_initialize(
    BYTE pdrv /* Physical drive nmuber (0) */
)
{
    if (pdrv != 0)
    {
        return STA_NOINIT;
    }

#ifdef sdio_4pin
    if (sd_init_4pins() == 0)
#else
    if (sd_init_1pin() == 0)
#endif
    {
        sd_set_clock_divider(3);
        Stat &= ~STA_NOINIT; /* Clear STA_NOINIT */
    }
    else
    {
        Stat = STA_NOINIT;
    }
    return Stat;
}

DRESULT disk_read(
    BYTE pdrv,    /* Physical drive nmuber (0) */
    BYTE *buff,   /* Pointer to the data buffer to store read data */
    LBA_t sector, /* Start sector number (LBA) */
    UINT count    /* Sector count (1..128) */
)
{
    if (pdrv != 0)
    {
        return STA_NOINIT;
    }

    sd_set_clock_divider(3);

    if (Stat == 0)
    {
        for (int i = 0; i < count; i++)
        {
            uint32_t tmp_bff[512/4];
            if (sd_readblocks_sync(&tmp_bff[0], sector + i, 1) == 0)
            {
                memcpy((uint8_t *)buff + 512 * i, tmp_bff, 512);
            }
            else
            {
                printf("read error\n");
                return RES_ERROR;
            }
        }

        return RES_OK;
    }
    else
    {
        printf("Stat error\n");
        return RES_ERROR;
    }
}

DRESULT disk_write(
    BYTE pdrv,        /* Physical drive nmuber (0) */
    const BYTE *buff, /* Pointer to the data to be written */
    LBA_t sector,     /* Start sector number (LBA) */
    UINT count        /* Sector count (1..128) */
)
{
    if (pdrv != 0)
    {
        return STA_NOINIT;
    }

    sd_set_clock_divider(30);
    
    if (Stat == 0)
    {
        if (sd_writeblocks_async((uint32_t *)buff, sector, count) != 0)
        {
            return RES_ERROR;
        }

        static int timeout = 10;
        int rc;
        while (!sd_write_complete(&rc))
        {
            printf("Waiting for completion\n");
            if (!--timeout)
            {
                return RES_ERROR;
            }
        }
        return RES_OK;
    }
    else
    {
        return RES_ERROR;
    }
}

DRESULT disk_ioctl(
    BYTE pdrv, /* Physical drive nmuber (0) */
    BYTE cmd,  /* Control code */
    void *buff /* Buffer to send/receive data block */
)
{
    switch (cmd)
    {
    case CTRL_SYNC:
        return RES_OK;
    case GET_SECTOR_COUNT:
        *((DWORD *)buff) = 0;
        return RES_ERROR;
    case GET_SECTOR_SIZE:
        *((WORD *)buff) = 512;
        return RES_OK;
    case GET_BLOCK_SIZE:
        return RES_ERROR;
    }
    return RES_ERROR;
}

DWORD get_fattime(void)
{
    return 0;
}
