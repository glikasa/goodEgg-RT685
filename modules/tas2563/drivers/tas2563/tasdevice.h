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

#ifndef __TASDEVICE_H__
#define __TASDEVICE_H__

#define dev_err(...) printf (__VA_ARGS__)
#define dev_info(...) printf (__VA_ARGS__)
#define dev_dbg(...)
#define ERROR_DEV_COMM	(0x0000000)
/* Book Control Register (available in page0 of each book) */
#define TAS256X_BOOK_ID(reg)			(reg / (256 * 128))
#define TAS256X_PAGE_REG(reg)			((reg % (256 * 128)) % 128)
#define TAS256X_BOOKCTL_PAGE	(0)
#define TAS256X_BOOKCTL_REG     (127)
#define TAS256X_PAGE_ID(reg)			((reg % (256 * 128)) / 128)

enum {
	SMARTPA_POWEROFF,
	SMARTPA_POWERON
};

typedef enum profileId {
	MUSIC,
	VOICE,
	CALIBRATION,
	BYPASS,
#ifdef PDM_ENABLE
	INDEPENDANT_CAPTURE,
	MIXTURE_CAPTURE
#endif
} profileId_t;

#ifdef I2C_ENABLE
typedef struct {
	int id; //TODO check tas256x i2c address
} t_libsccb_cfg;
#else
typedef struct {
	int pinsel;
	int pin_cs;
} t_libsccb_cfg;
#endif

typedef struct {
	int mnCurrentBook;
	int mnCurrentPage;
	void *pMnDspfw_runtime_info;
	t_libsccb_cfg *pSccb_cfg;
} smartamp_cfg;

typedef unsigned char cfg_u8;

typedef union {
	struct {
		cfg_u8 offset;
		cfg_u8 value;
	};
	struct {
		cfg_u8 command;
		cfg_u8 param;
	};
} cfg_reg;

#define CFG_META_SWITCH (255)
#define CFG_META_DELAY  (254)
#define CFG_META_BURST  (253)
struct tas256x_priv {
	void **tasdevice;
	void *nv_handle;
	void *private;
	int dev_state;
	struct os_interface* os_intf;
	unsigned int mnErrCode;
	unsigned char ndev;
};

int tas256x_dev_write(struct tas256x_priv *p, int chn,
	unsigned int reg, unsigned int value);
void tas256x_transmit_registers(struct tas256x_priv
	*p, int chn, cfg_reg *r, int n);
struct os_interface* tas256x_get_os_intf();
#endif