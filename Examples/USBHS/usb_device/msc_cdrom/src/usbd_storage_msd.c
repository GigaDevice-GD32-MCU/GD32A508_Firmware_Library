/*!
    \file    usbd_storage_msd.c
    \brief   this file provides the disk operations functions

    \version 2025-08-08 V1.3.0, firmware for GD32A508
*/

/*
    Copyright (c) 2025, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors
       may be used to endorse or promote products derived from this software without
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
OF SUCH DAMAGE.
*/

#include "usb_conf.h"
#include "sram_msd.h"
#include "usbd_msc_mem.h"

/* USB Mass storage standard inquiry data */
const int8_t STORAGE_InquiryData[] = {
    /* LUN 0 */
    0x05U,                        /* 0x05 for CD-ROM; 0x00 for usb mass storage */
    0x80U,                        /* RMB BYTE is set by inquiry data */
    0x02U,                        /* ANSI SCSI 2 */
    0x00U,                        /* Data format = 0 */
    0x1FU,                        /* Additional length */
    0x00U,
    0x00U,
    0x00U,
    /* vendor information = GD32 */
    'G', 'D', '3', '2', 0x20U, 0x20U, 0x20U, 0x20U,
    /* product identification = CDROM DEMO */
    'C', 'D', 'R', 'O', 'M', 0x20, 'D', 'E', 'M', '0', 0x20U, 0x20U, 0x20U, 0x20U, 0x20U, 0x20U,
    /* Product revision = 1.0 */
    0x31U, 0x2EU, 0x30U, 0x20U
};

/* USB CDROM TOC(Table Of Content) Data */
uint8_t usbcdrom_read_toc_data[] = {
    0x00U,
    0x12U,                        /* number of bytes below */
    0x01U,                        /* first track */
    0x01U,                        /* last track */
    0x00U,                        /* reserved */
    0x14U,                        /* (adr, control) */
    0x01U,                        /* (track being described) */
    0x00U,                        /* (reserved) */
    0x00U, 0x00U, 0x00U, 0x00U,   /* (start logical block address 0) */
    0x00U,                        /* (reserved) */
    0x14U,                        /* (adr, control) */
    0xAAU,                        /* (track being described (leadout)) */
    0x00U,                        /* (reserved) */
    0x00U, 0x00U, 0xA2U, 0x8AU    /* (start logical block address 41610) */
};

/* local function prototypes ('static') */
static int8_t STORAGE_Init(uint8_t Lun);
static int8_t STORAGE_IsReady(uint8_t Lun);
static int8_t STORAGE_IsWriteProtected(uint8_t Lun);
static int8_t STORAGE_GetMaxLun(void);
static int8_t STORAGE_Read(uint8_t Lun, uint8_t *buf, uint32_t BlkAddr, uint16_t BlkLen);
static int8_t STORAGE_Write(uint8_t Lun, uint8_t *buf, uint32_t BlkAddr, uint16_t BlkLen);

usbd_mem_cb USBD_Internal_Storage_fops = {
    .mem_init      = STORAGE_Init,
    .mem_ready     = STORAGE_IsReady,
    .mem_protected = STORAGE_IsWriteProtected,
    .mem_read      = STORAGE_Read,
    .mem_write     = STORAGE_Write,
    .mem_maxlun    = STORAGE_GetMaxLun,

    .mem_toc_data     = usbcdrom_read_toc_data,
    .mem_inquiry_data = {(uint8_t *)STORAGE_InquiryData},
    .mem_block_size   = {ISRAM_BLOCK_SIZE},
    .mem_block_len    = {ISRAM_BLOCK_NUM}
};

usbd_mem_cb *usbd_mem_fops = &USBD_Internal_Storage_fops;

/*!
    \brief      initialize the storage medium
    \param[in]  Lun: logical unit number
    \param[out] none
    \retval     status
*/
static int8_t STORAGE_Init(uint8_t Lun)
{
    return 0U;
}

/*!
    \brief      check whether the medium is ready
    \param[in]  Lun: logical unit number
    \param[out] none
    \retval     status
*/
static int8_t STORAGE_IsReady(uint8_t Lun)
{
    return 0U;
}

/*!
    \brief      check whether the medium is write-protected
    \param[in]  Lun: logical unit number
    \param[out] none
    \retval     status
*/
static int8_t STORAGE_IsWriteProtected(uint8_t Lun)
{
    return 0U;
}

/*!
    \brief      read data from the medium
    \param[in]  Lun: logical unit number
    \param[in]  buf: pointer to the buffer to save data
    \param[in]  BlkAddr: address of 1st block to be read
    \param[in]  BlkLen: number of blocks to be read
    \param[out] none
    \retval     status
*/
static int8_t STORAGE_Read(uint8_t Lun, uint8_t *buf, uint32_t BlkAddr, uint16_t BlkLen)
{
    if(0U != SRAM_ReadMultiBlocks(buf, BlkAddr, ISRAM_BLOCK_SIZE, (uint32_t)BlkLen)) {
        return -1;
    }

    return 0U;
}

/*!
    \brief      write data to the medium
    \param[in]  Lun: logical unit number
    \param[in]  buf: pointer to the buffer to write
    \param[in]  BlkAddr: address of 1st block to be written
    \param[in]  BlkLen: number of blocks to be write
    \param[out] none
    \retval     status
*/
static int8_t STORAGE_Write(uint8_t Lun, uint8_t *buf, uint32_t BlkAddr, uint16_t BlkLen)
{
    if(0U != SRAM_WriteMultiBlocks(buf, BlkAddr, ISRAM_BLOCK_SIZE, (uint32_t)BlkLen)) {
        return -1;
    }

    return 0U;
}

/*!
    \brief      get number of supported logical unit
    \param[in]  none
    \param[out] none
    \retval     number of logical unit
*/
static int8_t STORAGE_GetMaxLun(void)
{
    return (MEM_LUN_NUM - 1U);
}
