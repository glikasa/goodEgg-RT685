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

#ifndef __TAS2563_H__
#define __TAS2563_H__

// #define dev_err(...) fprintf (stderr, __VA_ARGS__)
// #define dev_info(...) fprintf (stdout, __VA_ARGS__)

/* Book Control Register (available in page0 of each book) */

#define TAS256X_REG(book, page, reg)  (((book * 256 * 128) + \
					(page * 128)) + reg)

#define TAS2563_PDM_CONFIG0  TAS256X_REG(0x0, 0x0, 0x41)
#define TAS2563_PDM_CONFIG0_PDM_GATE_PAD0_MASK	(0x1 << 6)
#define TAS2563_PDM_CONFIG0_PDM_GATE_PAD0_ENA	(0x1 << 6)
#define TAS2563_PDM_CONFIG0_PDM_GATE_PAD0_DIS	(0x0 << 6)

#define TAS2563_REG_MISC_DSP  TAS256X_REG(0x0, 0X1, 0x02)
#define TAS2563_REG_MISC_DSP_ROM_MODE_MASK  			(0xf << 1)
/*DSP mode*/
#define TAS2563_REG_MISC_DSP_ROM_MODE_RAM_MODE 			(0x0 << 1)
/*bypass mode*/
#define TAS2563_REG_MISC_DSP_ROM_MODE_ROM_MODE 			(0x1 << 1)

    /* Revision and PG ID */
#define TAS2563_REG_REV_PGID  		TAS256X_REG(0x0, 0x0, 0x7D)
#define TAS2563_REG_PDM_PAD0_SEL_IN_REG_BINARY_FACTOR_SPI_READ_FRAME_DEL	\
	TAS256X_REG(0x0, 0X1, 0x3E)
#define TAS2563_REG_SPI_READ_FRAME_DEL_MASK				(0x1 << 3)
#define TAS2563_REG_SPI_READ_FRAME_DEL_WITH_RAMS		(0x0 << 3)
#define TAS2563_REG_SPI_READ_FRAME_DEL_ALWYAS			(0x1 << 3)
#define TAS2563_REG_DEVICEID  		TAS256X_REG(0x0, 0xFD, 0x7C)
#define TAS2563_REG_UNLCK  			TAS256X_REG(0x0, 0xFD, 0x0D)


#define RESULT_PASS			(0x00)
#define RE_FAIL_HI			(0x01)
#define RE_FAIL_LO			(0x10)
#define RE_CHK_MSK			(0x11)
#define MAX_BIN_SIZE (64)

struct smartpa_info    {
	unsigned char spkvendorid;
	unsigned char ndev;
};

struct TFCTBinFile {
	char *pDevBlock;
	unsigned int mnDevBlockIndex;
};

struct TSPKCharData {
	double nSpkTMax;
	double nSpkReTolPer;
	double nSpkReAlpha;

	double nPPC3_Re0;
	double nPPC3_FWarp;
	double nPPC3_Bl;
	double nPPC3_Mms;
	double nPPC3_RTV;
	double nPPC3_RTM;
	double nPPC3_RTVA;
	double nPPC3_SysGain;
	double nPPC3_DevNonlinPer;
	double nPPC3_PIG;

	double nReHi;
	double nReLo;
	double nf0Hi;
	double nf0Lo;
	double nQHi;
	double nQLo;
	uint32_t mprm_EnFF;
	uint32_t mprm_DisTck;
	uint32_t mprm_TE_SCThr;
	uint32_t mprm_Plt_Flag;
	uint32_t mprm_SineGain;
	uint32_t mTE_TA1;
	uint32_t mTE_TA1_AT;
	uint32_t mTE_TA2;
	uint32_t mTE_AT;
	uint32_t mTE_DT;
	unsigned char nDevAddr;
};

struct TFCTResult {
	unsigned char mCalbirationResult;
	uint32_t dev_prm_pow;  // Total RMS power coefficient
	uint32_t dev_prm_tlimit; // Delta temperature limit coefficient
	double dev_re;
	uint8_t bWrittenintoFile;
};

struct TFTCConfiguration {
	uint8_t bVerbose;
	uint8_t bLoadCalibration;
	unsigned int nCalibrationTime;
	unsigned char spkvendorid;
	unsigned char cal_count;
	double nPPC3_FS;
	double temp_cal;
	unsigned char nActiveSpk_num;
	unsigned char *cmdName;
	struct TFCTBinFile mtCalBin;
	struct TFCTResult *mnTFctResult;
	struct TSPKCharData *nTSpkCharDev;
	struct tas256x_priv *pTas256x_priv;
};

int InitFTCC(struct TFTCConfiguration *pFTCC,
	struct smartpa_info a);
uint32_t tas2563_switch_device(
	struct TFTCConfiguration *pFTCC, uint8_t i2cslave);
int tas256x_dev_write(struct tas256x_priv *p,
	int chn,unsigned int reg, unsigned int value);
uint32_t tas2563_coeff_read(struct TFTCConfiguration *pFTCC,
	int chn, uint32_t reg);
void tas2563_coeff_write(struct TFTCConfiguration *pFTCC,
	int chn, uint32_t reg, uint32_t data);
unsigned char check_spk_bounds(struct TFTCConfiguration *pFTCC,
				double re1, int spkno);
int tas256x_dev_update_bits(struct tas256x_priv* p, int chn,
	unsigned int reg,
	unsigned int mask, unsigned int value);
void tas2563_save_calibrated_data(
	struct TFTCConfiguration *pFTCC, int chn);
/*void tas2563_write_calibrated_data(
	struct tas256x_priv* pTAS2563, int chn);*/
int tas256x_dev_read(struct tas256x_priv *p, int chn,
	unsigned int reg, int *pValue);
int tas256x_dev_bulk_write(struct tas256x_priv* p, int chn,
	unsigned int reg, unsigned char* pData, unsigned int nLength);
int tas256x_dev_bulk_read(struct tas256x_priv* pTAS2563,
	int chn, unsigned int reg, unsigned char* pData,
	unsigned int nLength);
int tas256x_change_book_page(struct tas256x_priv *p,
	int chn, int book, int page);
void *tasdevice_get_handle(void);
#endif
