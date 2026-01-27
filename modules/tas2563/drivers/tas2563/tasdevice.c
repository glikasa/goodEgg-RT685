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

#include<stdio.h>
#include <stdint.h>
#include <stdbool.h>
//#include <stdlib.h>
#ifndef I2C_ENABLE
	#include <string.h>
#endif
#include "tasdevice.h"
#include "OSL_wrapper.h"

extern const struct os_interface os_intf;
struct os_interface* tas256x_get_os_intf()
{
	return (struct os_interface*)&(os_intf);
}

int tas256x_change_book_page(struct tas256x_priv *p,
	int chn, int book, int page)
{
	int nResult = 0;

	struct os_interface* pOs_intf = p->os_intf;
	smartamp_cfg *tasdev = (smartamp_cfg *)p->tasdevice[chn];

	if ((tasdev->mnCurrentBook == book) && (tasdev->mnCurrentPage
		== page))
		goto end;

	if (tasdev->mnCurrentBook != book) {
		pOs_intf->dev_write(tasdev,TAS256X_BOOKCTL_PAGE,0);
		tasdev->mnCurrentPage = 0;
		pOs_intf->dev_write(tasdev,TAS256X_BOOKCTL_REG,book);
		tasdev->mnCurrentBook = book;
	}

	if (tasdev->mnCurrentPage != page) {
		pOs_intf->dev_write(tasdev,TAS256X_BOOKCTL_PAGE,page);
		tasdev->mnCurrentPage = page;
	}

end:
	return nResult;
}

int tas256x_dev_write(struct tas256x_priv *p, int chn,
	unsigned int reg, unsigned int value)
{
	int nResult = 0;
	struct os_interface* pOs_intf = p->os_intf;
	smartamp_cfg *tasdev = NULL;
	tasdev = p->tasdevice[chn];
	nResult = tas256x_change_book_page(p,chn,
		TAS256X_BOOK_ID(reg), TAS256X_PAGE_ID(reg));
	if (nResult < 0) {
		dev_err("%s, ERROR, L=%d, E=%d\n", __func__, __LINE__,
			nResult);
		goto end;
	}

	nResult = pOs_intf->dev_write(tasdev,TAS256X_PAGE_REG(reg),value);
	if (nResult < 0)
		dev_err("%s, ERROR, L=%d, E=%d\n",
			__func__, __LINE__, nResult);
	else
		dev_dbg("%s: BOOK:PAGE:REG 0x%02x:0x%02x:0x%02x, VAL: "
			"0x%02x\n", __func__, TAS256X_BOOK_ID(reg),
			TAS256X_PAGE_ID(reg), TAS256X_PAGE_REG(reg), value);

end:
	return nResult;
}

int tas256x_dev_update_bits(struct tas256x_priv* p, int chn,
	unsigned int reg, unsigned int mask, unsigned int value)
{
	unsigned int tmp;
	struct os_interface* pOs_intf = p->os_intf;
	smartamp_cfg *tasdev = (smartamp_cfg *)p->tasdevice[chn];

	int nResult = tas256x_change_book_page(p,chn,
		TAS256X_BOOK_ID(reg), TAS256X_PAGE_ID(reg));

	if (nResult < 0) {
		dev_err("%s, ERROR, L=%d, E=%d\n", __func__, __LINE__,
			nResult);
		goto end;
	}

	nResult = pOs_intf->dev_read(tasdev, TAS256X_PAGE_REG(reg));
	if (nResult == -1) {
		dev_err("%s, ERROR, L=%d, E=%d\n", __func__, __LINE__,
			nResult);
		goto end;
	}
	tmp = nResult & ~mask;
	tmp |= value & mask;
	if (tmp != nResult) {
		nResult = pOs_intf->dev_write(tasdev, TAS256X_PAGE_REG(reg),
			tmp);
		if (nResult == -1) {
			dev_err("%s, ERROR, L=%d, E=%d\n", __func__, __LINE__,
				nResult);
			goto end;
		}
	}
	dev_dbg("%s:%u BOOK:PAGE:REG 0x%02:0x%02:0x%02, mask: 0x%02x, "
		"val=0x%02x\n", __func__, __LINE__, TAS256X_BOOK_ID(reg),
		TAS256X_PAGE_ID(reg), TAS256X_PAGE_REG(reg), mask, value);
end:
	return nResult;
}

int tas256x_dev_bulk_write(struct tas256x_priv* p, int chn,
	unsigned int reg, unsigned char* pData, unsigned int nLength)
{
	int nResult = 0;
	struct os_interface* pOs_intf = p->os_intf;
	smartamp_cfg *tasdev = (smartamp_cfg *)p->tasdevice[chn];
	if (nLength > 1024) {
		dev_err("Error: %s:%u size too large\n", __func__, __LINE__);
		return -1;
	}

	nResult = tas256x_change_book_page(p,chn, TAS256X_BOOK_ID(reg),
		TAS256X_PAGE_ID(reg));
	if (nResult < 0)
		goto end;

	nResult = pOs_intf->dev_bulk_write(tasdev, reg, nLength, pData);

	if (nResult < 0)
		dev_err("%s, ERROR, L=%d, E=%d\n",
			__func__, __LINE__, nResult);
	else
		dev_dbg("%s: BOOK:PAGE:REG %u:%u:%u, len: 0x%02x\n",
			__func__, TAS256X_BOOK_ID(reg), TAS256X_PAGE_ID(reg),
			TAS256X_PAGE_REG(reg), nLength);

end:
	return nResult;


}

#ifdef I2C_ENABLE
int tas256x_dev_read(struct tas256x_priv *p, int chn,
	unsigned int reg, int *pValue)
{
	int nResult = 0;
	struct os_interface* pOs_intf = p->os_intf;
	smartamp_cfg *tasdev = p->tasdevice[chn];
	nResult = tas256x_change_book_page(p,chn,
		TAS256X_BOOK_ID(reg), TAS256X_PAGE_ID(reg));
	if (nResult < 0) {
		dev_err("%s, ERROR, L=%d, E=%d\n",
			__func__, __LINE__, nResult);
		goto end;
	}
	nResult = pOs_intf->dev_read(tasdev, TAS256X_PAGE_REG(reg));
	if (nResult < 0){
		dev_err("%s, ERROR, L=%d, E=%d\n",
			__func__, __LINE__, nResult);
	}else{
		dev_dbg("%s: BOOK:PAGE:REG 0x%02x:0x%02x:0x%02x\n", __func__,
			TAS256X_BOOK_ID(reg), TAS256X_PAGE_ID(reg),
			TAS256X_PAGE_REG(reg));
		*pValue = nResult;
		nResult = 0;
	}

end:
	return nResult;
}

int tas256x_dev_bulk_read(struct tas256x_priv* pTAS2563,
	int chn, unsigned int reg, unsigned char* pData,
	unsigned int nLength)
{
	int nResult = 0;
	int i = 0;
	struct os_interface* pOs_intf = NULL;
	smartamp_cfg *tasdev = (smartamp_cfg *)pTAS2563->tasdevice[chn];

	pOs_intf = pTAS2563->os_intf;

	nResult = tas256x_change_book_page(pTAS2563,chn,
		TAS256X_BOOK_ID(reg), TAS256X_PAGE_ID(reg));
	if (nResult < 0) {
		dev_err("%s:%u:reg = 0x%x, nLenth = %d\n",
			__func__, __LINE__, reg, nLength);
		goto end;
	}

	dev_dbg("%s:%u:reg = 0x%x, Lenth = %d",
		__func__, __LINE__, reg, nLength);

#define STRIDE (4)
	/* Read chunk bytes defined by STRIDE */
	for (i = 0; i < (int)(nLength / STRIDE); i++) {
		nResult = pOs_intf->dev_bulk_read(tasdev,
			TAS256X_PAGE_REG((reg + i * STRIDE)), STRIDE,
			&pData[i * STRIDE]);
		if (nResult < 0) {
			dev_err("%s, %d, I2C or SPI error %d\r\n",
				__func__, __LINE__, nResult);
			pTAS2563->mnErrCode |= ERROR_DEV_COMM;
		}
		else
			pTAS2563->mnErrCode &= ~ERROR_DEV_COMM;
	}

	/* Read remaining bytes */
	if ((nLength % STRIDE) != 0) {
		nResult = pOs_intf->dev_bulk_read(tasdev,
			TAS256X_PAGE_REG(reg + i * STRIDE), (nLength % STRIDE),
			&pData[i * STRIDE]);
		if (nResult < 0) {
			dev_err("%s, %d, I2C or SPI error %d\r\n",
				__func__, __LINE__, nResult);
			pTAS2563->mnErrCode |= ERROR_DEV_COMM;
		}
		else
			pTAS2563->mnErrCode &= ~ERROR_DEV_COMM;
	}

end:
	return nResult;
}
#else
int tas256x_dev_read(struct tas256x_priv *p, int chn,
	unsigned int reg, int *pValue)
{
	int nResult = 0;
	struct os_interface* pOs_intf = p->os_intf;
	smartamp_cfg *tasdev = p->tasdevice[chn];
	unsigned char nBuf[2];

	nResult = tas256x_change_book_page(p,chn,
		TAS256X_BOOK_ID(reg), TAS256X_PAGE_ID(reg));
	if (nResult < 0) {
		dev_err("%s, ERROR, L=%d, E=%d\n",
			__func__, __LINE__, nResult);
		goto end;
	}

	nResult = pOs_intf->dev_bulk_read(tasdev, TAS256X_PAGE_REG(reg),
		2, &nBuf[0]);
	if (nResult < 0){
		dev_err("%s, ERROR, L=%d, E=%d\n",
			__func__, __LINE__, nResult);
	}else{
		dev_dbg("%s: BOOK:PAGE:REG 0x%02x:0x%02x:0x%02x\n", __func__,
			TAS256X_BOOK_ID(reg), TAS256X_PAGE_ID(reg),
			TAS256X_PAGE_REG(reg));
		*pValue = nBuf[1];
		nResult = 0;
	}

end:
	return nResult;
}

int tas256x_dev_bulk_read(struct tas256x_priv* pTAS2563,
	int chn, unsigned int reg, unsigned char* pData,
	unsigned int nLength)
{
	int nResult = 0;
	int i = 0;
	unsigned char nBuf[129];
	struct os_interface* pOs_intf = NULL;
	smartamp_cfg *tasdev = (smartamp_cfg *)pTAS2563->tasdevice[chn];
	unsigned int len = nLength+1;

	if (nLength > 128) {
		dev_err("%s len Error %d\n", __func__, nLength);
		nResult = -1;
	}

	pOs_intf = pTAS2563->os_intf;

	nResult = tas256x_change_book_page(pTAS2563,chn,
		TAS256X_BOOK_ID(reg), TAS256X_PAGE_ID(reg));
	if (nResult < 0) {
		dev_err("%s:%u:reg = 0x%x, nLenth = %d\n",
			__func__, __LINE__, reg, nLength);
		goto end;
	}

#define STRIDE (4)
	/* Read chunk bytes defined by STRIDE */
	for (i = 0; i < (int)(len / STRIDE); i++) {
		nResult = pOs_intf->dev_bulk_read(tasdev,
			TAS256X_PAGE_REG((reg + i * STRIDE)), STRIDE,
			&nBuf[i * STRIDE]);
		if (nResult < 0) {
			dev_err("%s, %d, I2C or SPI error %d\r\n",
				__func__, __LINE__, nResult);
			pTAS2563->mnErrCode |= ERROR_DEV_COMM;
			goto end;
		}
		else
			pTAS2563->mnErrCode &= ~ERROR_DEV_COMM;
	}

	/* Read remaining bytes */
	if ((len % STRIDE) != 0) {
		nResult = pOs_intf->dev_bulk_read(tasdev,
			TAS256X_PAGE_REG(reg + i * STRIDE), (len % STRIDE),
			&nBuf[i * STRIDE]);
		if (nResult < 0) {
			dev_err("%s, %d, I2C or SPI error %d\r\n",
				__func__, __LINE__, nResult);
			pTAS2563->mnErrCode |= ERROR_DEV_COMM;
			goto end;
		}
		else
			pTAS2563->mnErrCode &= ~ERROR_DEV_COMM;
	}
	memcpy(pData, &nBuf[1], nLength);
end:
	return nResult;
}
#endif

void tas256x_transmit_registers(struct tas256x_priv
	*p, int chn, cfg_reg *r, int n)
{
	int i = 0, j = 0;
	struct os_interface* pOs_intf = p->os_intf;
	unsigned char *pData = NULL;
	smartamp_cfg *tasdev = (smartamp_cfg *)p->tasdevice[chn];
	while (i < n) {
		switch (r[i].command) {
		case CFG_META_SWITCH:
			// Used in legacy applications.  Ignored here.
			break;
		case CFG_META_DELAY:
			pOs_intf->msleep(r[i].param);
			break;
		case CFG_META_BURST:
			j = (r[i].param / 2) + 1;
			if(i + j < n) {
				pData = &(r[i+1].value);
				pOs_intf->dev_bulk_write(tasdev,
					(unsigned int)r[i+1].offset,
					(unsigned int)r[i].param, pData);

			}
			i +=  j;
			break;
		default:
			pOs_intf->dev_write(tasdev, (unsigned int)r[i].offset,
				(unsigned int)r[i].value);
			break;
		}
		i++;
	}
	tasdev->mnCurrentBook = -1;
	tasdev->mnCurrentPage = -1;

}
