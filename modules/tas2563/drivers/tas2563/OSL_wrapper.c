/*
* Copyright (C) 2021 ?2022 Texas Instruments Incorporated
*
* All rights reserved not granted herein.
* Limited License.
*
* Texas Instruments Incorporated grants a world-wide, royalty-free,
* non-exclusive license under copyrights and patents it now or
* hereafter owns or controls to make, have made, use, import, offer to
* sell and sell ("Utilize") this software subject to the terms herein.
* With respect to the foregoing patent license, such license is
* granted solely to the extent that any such patent is necessary to
* Utilize the software alone.  The patent license shall not apply to
* any combinations which include this software, other than
* combinations with devices manufactured by or for TI (?I Devices?.
* No hardware patent is licensed hereunder.
*
* Redistributions must preserve existing copyright notices and
* reproduce this license (including the above copyright notice and the
* disclaimer and (if applicable) source code license limitations
* below) in the documentation and/or other materials provided with the
* distribution
*
* Redistribution and use in binary form, without modification, are
* permitted provided that the following conditions are met:
*
*	* No reverse engineering, decompilation, or disassembly of this
*     software is permitted with respect to any software provided in
*     binary form.
*	* any redistribution and use are licensed by TI for use only with
*     TI Devices.
*	* Nothing shall obligate TI to provide you with source code for
*     the software licensed and provided to you in object code.
*
* If software source code is provided to you, modification and
* redistribution of the source code are permitted provided that the
* following conditions are met:
*
*   * any redistribution and use of the source code, including any
*     resulting derivative works, are licensed by TI for use only
*     with TI Devices.
*   * any redistribution and use of any object code compiled from the
*     source code and any resulting derivative works, are licensed by
*     TI for use only with TI Devices.
*
* Neither the name of Texas Instruments Incorporated nor the names of
* its suppliers may be used to endorse or promote products derived
* from this software without specific prior written permission.
*
* DISCLAIMER.
*
* THIS SOFTWARE IS PROVIDED BY TI AND TI? LICENSORS "AS IS" AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
* PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL TI AND TI? LICENSORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
* OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
* BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <zephyr/kernel.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/devicetree.h>
#include <zephyr/sys/util.h>
#include <stddef.h>
#include "OSL_wrapper.h"
#include "tasdevice.h"
#include "tas2563_driver.h"


#define TAS_SUCCESS	0
#define TAS_FAILURE	-EIO


void libsccb_init(void* dev_handle){}

//Single byte read
int libsccb_rd(void* dev_handle, int reg){

    uint8_t reg_addr = (uint8_t)reg;
    uint8_t val;
    int ret;
    // 'handle' is tasdevice[0], which is &data->hw_cfg
    smartamp_cfg *tasdev = (smartamp_cfg *)dev_handle;
    // Calculate the start of the 'data' struct based on where 'hw_cfg' is inside it
    struct tas2563_data *data = CONTAINER_OF(tasdev, struct tas2563_data, hw_cfg[0]);
    const struct tas2563_config *cfg = data->dev->config;

    // Zephyr performs a combined Write (reg address) then Read (1 byte)
    ret = i2c_write_read_dt(&cfg->i2c, &reg_addr, 1, &val, 1);
    if(ret == 0) {

	return (int)val;
    } else {

	//LOG_ERR("Failed to read i2c: address %02x, err: %d", reg_addr, ret);
	return ret;
    }
}

// int (*dev_write)(void* dev_handle, int reg, unsigned int Value)
//Single byte write
int libsccb_wr(void* dev_handle, int reg, unsigned int Value){

    // 'handle' is tasdevice[0], which is &data->hw_cfg
    smartamp_cfg *tasdev = (smartamp_cfg *)dev_handle;
    // Calculate the start of the 'data' struct based on where 'hw_cfg' is inside it
    struct tas2563_data *data = CONTAINER_OF(tasdev, struct tas2563_data, hw_cfg[0]);
    const struct tas2563_config *cfg = data->dev->config;

    uint8_t tx_buf[2];
    tx_buf[0] = (uint8_t)reg;
    tx_buf[1] = (uint8_t)Value;
    int ret = i2c_write_dt(&cfg->i2c, tx_buf, sizeof(tx_buf));
    return (ret == 0) ? TAS_SUCCESS : TAS_FAILURE;
}

// int (*dev_bulk_write)(void* dev_handle, int reg, int len, unsigned char* pData)
// Multiple bytes write
int libsccb_seqwrite(void* dev_handle, int reg, int len, unsigned char* pData){
    
    // 'handle' is tasdevice[0], which is &data->hw_cfg
    smartamp_cfg *tasdev = (smartamp_cfg *)dev_handle;
    // Calculate the start of the 'data' struct based on where 'hw_cfg' is inside it
    struct tas2563_data *data = CONTAINER_OF(tasdev, struct tas2563_data, hw_cfg[0]);
    const struct tas2563_config *cfg = data->dev->config;

    // Buffer size = 1 (register address) + len (data)
    uint8_t tx_buf[len + 1];
    tx_buf[0] = (uint8_t)reg;
    memcpy(&tx_buf[1], pData, len);

    int ret = i2c_write_dt(&cfg->i2c, tx_buf, sizeof(tx_buf));

    return (ret == 0) ? TAS_SUCCESS : TAS_FAILURE;
}

// int (*dev_bulk_read)(void* dev_handle, int reg, int len, unsigned char* pData)
//Multiple bytes read
int libsccb_seqread(void* dev_handle, int reg, int count, unsigned char* buf){

    // 'handle' is tasdevice[0], which is &data->hw_cfg
    smartamp_cfg *tasdev = (smartamp_cfg *)dev_handle;
    // Calculate the start of the 'data' struct based on where 'hw_cfg' is inside it
    struct tas2563_data *data = CONTAINER_OF(tasdev, struct tas2563_data, hw_cfg[0]);
    const struct tas2563_config *cfg = data->dev->config;
    uint8_t reg_addr = (uint8_t)reg;

    int ret = i2c_write_read_dt(&cfg->i2c, &reg_addr, 1, buf, count);
    return (ret == 0) ? TAS_SUCCESS : TAS_FAILURE;
}

// void (*msleep)(unsigned int msecs)
void ertos_timedelay(unsigned int msecs) {
	
    k_msleep(msecs);
}

void gpio_config() {

	//TODO:
	// RESET PIN setting, for tas2563 it should be high level
	// I2C-SPI PIN Setting, for tas2563 it should be low level
}

size_t nv_write(const void *ptr, size_t size, size_t nmemb, void *stream){

    return nmemb;
}

size_t nv_read(void *ptr, size_t size, size_t nmemb, void *stream){

    return nmemb;
}

#ifdef PRM_IN_NV
unsigned char* prm_download(void *handle, int chn,
	unsigned int *len)
{
	return NULL;
}

void prm_remove(unsigned char *pData)
{

}

const struct os_interface os_intf = {
	libsccb_rd,
	libsccb_wr,
	libsccb_seqwrite,
	libsccb_seqread,
	gpio_config,
	ertos_timedelay,
	nv_write,
	nv_read,
	prm_download,
	prm_remove
};
#else
const struct os_interface os_intf = {
	libsccb_rd,
	libsccb_wr,
	libsccb_seqwrite,
	libsccb_seqread,
	gpio_config,
	ertos_timedelay,
	nv_write,
	nv_read
};
#endif
