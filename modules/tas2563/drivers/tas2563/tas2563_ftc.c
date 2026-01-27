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

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <zephyr/drivers/tas2563_driver.h>
#include "tas2563.h"
#include "tas2563_ftc_lib.h"
#include "tasdevice.h"
#include "OSL_wrapper.h"
#include "TAS2563_ftcfg.h"

#define TAS2563_PRM_ENFF_REG		TAS256X_REG(0x0, 0x0d, 0x54)
#define TAS2563_PRM_DISTCK_REG		TAS256X_REG(0x0, 0x0d, 0x58)
#define TAS2563_PRM_TE_SCTHR_REG	TAS256X_REG(0x0, 0x0f, 0x60)
#define TAS2563_PRM_PLT_FLAG_REG	TAS256X_REG(0x0, 0x0d, 0x74)
#define TAS2563_PRM_SINEGAIN_REG	TAS256X_REG(0x0, 0x0d, 0x7c)
/* prm_Int_B0 */
#define TAS2563_TE_TA1_REG			TAS256X_REG(0x0, 0x10, 0x0c)
/* prm_Int_A1 */
#define TAS2563_TE_TA1_AT_REG		TAS256X_REG(0x0, 0x10, 0x10)
/* prm_TE_Beta */
#define TAS2563_TE_TA2_REG			TAS256X_REG(0x0, 0x0f, 0x64)
/* prm_TE_Beta1 */
#define TAS2563_TE_AT_REG			TAS256X_REG(0x0, 0x0f, 0x68)
/* prm_TE_1_Beta1 */
#define TAS2563_TE_DT_REG			TAS256X_REG(0x0, 0x0f, 0x70)

static int tas2563_check_node(struct TFTCConfiguration *pFTCC)
{
	int ret = 0;

	return ret;
}

void tas2563_save_calibrated_data(
	struct TFTCConfiguration *pFTCC, int chn)
{
	struct TFCTBinFile *pTCalBin = &(pFTCC->mtCalBin);
	struct tas256x_priv* pTAS2563 = pFTCC->pTas256x_priv;
	struct os_interface* pOs_intf = pTAS2563->os_intf;

	char *pDevBlock = pTCalBin->pDevBlock;
	int rc = 0;
	pTAS2563->nv_handle = &chn;
	/* write number of commands for calibration block */
	rc = pOs_intf->nv_write(pDevBlock, sizeof(char),
		pTCalBin->mnDevBlockIndex, pTAS2563->nv_handle );
	if(rc != (int)pTCalBin->mnDevBlockIndex) {
		dev_err("%s:%u:rc = %d pTCalBin->mnBinIndex = %u\r\n",
			__func__, __LINE__, rc, pTCalBin->mnDevBlockIndex);
	}
}

// -------------------------------------------------------------------
// check_spk_bounds
// -------------------------------------------------------------------
// Description:
//      Checks if speaker paramters are within bounds.
// -------------------------------------------------------------------
unsigned char check_spk_bounds(struct TFTCConfiguration *pFTCC,
		double re1, int i)
{
	unsigned char result = RESULT_PASS;

	if (re1 > pFTCC->nTSpkCharDev[i].nReHi)
		result |= RE_FAIL_HI;
	if (re1 < pFTCC->nTSpkCharDev[i].nReLo)
		result |= RE_FAIL_LO;
	return result;
}

int InitFTCC(struct TFTCConfiguration *pFTCC, struct smartpa_info a)
{
	int spkno = 0, ret = 0;
	pFTCC->nCalibrationTime = 2000;
	pFTCC->nPPC3_FS = 48000;

	pFTCC->bVerbose = false;
	pFTCC->bLoadCalibration = false;
	pFTCC->nActiveSpk_num = a.ndev;
	pFTCC->spkvendorid = a.spkvendorid;

	pFTCC->mnTFctResult = (struct TFCTResult *)calloc(
		pFTCC->nActiveSpk_num, sizeof(struct TFCTResult));
	if(NULL == pFTCC->mnTFctResult) {
		dev_err("%s:%u:memory allocate error\n",__func__,__LINE__);
		ret = -1;
		goto EXIT;
	}
	pFTCC->nTSpkCharDev = (struct TSPKCharData *)calloc(
		pFTCC->nActiveSpk_num, sizeof(struct TSPKCharData));
	if(NULL == pFTCC->nTSpkCharDev) {
		dev_err("%s:%u:memory allocate error\n",__func__,__LINE__);
		ret = -1;
		goto EXIT;
	}
	for(spkno = 0; spkno < pFTCC->nActiveSpk_num; spkno++) {
		pFTCC->mnTFctResult[spkno].bWrittenintoFile = false;
		pFTCC->nTSpkCharDev[spkno].nSpkTMax =
			TAS2563_ftcfg[spkno].nSpkTMax;
		pFTCC->nTSpkCharDev[spkno].nSpkReTolPer =
			TAS2563_ftcfg[spkno].nSpkReTolPer;
		pFTCC->nTSpkCharDev[spkno].nSpkReAlpha =
			TAS2563_ftcfg[spkno].nSpkReAlpha;
		pFTCC->nTSpkCharDev[spkno].nReHi = TAS2563_ftcfg[spkno].nReHi;
		pFTCC->nTSpkCharDev[spkno].nReLo = TAS2563_ftcfg[spkno].nReLo;
		pFTCC->nTSpkCharDev[spkno].nPPC3_Re0 =
			TAS2563_ftcfg[spkno].nPPC3_Re0;
		pFTCC->nTSpkCharDev[spkno].nPPC3_FWarp =
			TAS2563_ftcfg[spkno].nPPC3_FWarp;
		pFTCC->nTSpkCharDev[spkno].nPPC3_Bl =
			TAS2563_ftcfg[spkno].nPPC3_Bl;
		pFTCC->nTSpkCharDev[spkno].nPPC3_Mms =
			TAS2563_ftcfg[spkno].nPPC3_Mms;
		pFTCC->nTSpkCharDev[spkno].nPPC3_RTV =
			TAS2563_ftcfg[spkno].nPPC3_RTV;
		pFTCC->nTSpkCharDev[spkno].nPPC3_RTM =
			TAS2563_ftcfg[spkno].nPPC3_RTM;
		pFTCC->nTSpkCharDev[spkno].nPPC3_RTVA =
			TAS2563_ftcfg[spkno].nPPC3_RTVA;
		pFTCC->nTSpkCharDev[spkno].nPPC3_SysGain =
			TAS2563_ftcfg[spkno].nPPC3_SysGain;
		pFTCC->nTSpkCharDev[spkno].nPPC3_DevNonlinPer =
			TAS2563_ftcfg[spkno].nPPC3_DevNonlinPer;
		pFTCC->nTSpkCharDev[spkno].nPPC3_PIG =
			TAS2563_ftcfg[spkno].nPPC3_PIG;
		pFTCC->nTSpkCharDev[spkno].nf0Hi = TAS2563_ftcfg[spkno].nf0Hi;
		pFTCC->nTSpkCharDev[spkno].nf0Lo = TAS2563_ftcfg[spkno].nf0Lo;
		pFTCC->nTSpkCharDev[spkno].nQHi  = TAS2563_ftcfg[spkno].nQHi;
		pFTCC->nTSpkCharDev[spkno].nQLo = TAS2563_ftcfg[spkno].nQLo;
		pFTCC->nTSpkCharDev[spkno].nDevAddr =
			TAS2563_ftcfg[spkno].nDevAddr;
	}

EXIT:
	return ret;
}

uint32_t tas2563_coeff_read(struct TFTCConfiguration *pFTCC,
	int chn,  uint32_t reg)
{
	unsigned char pData[4] = {0x00, 0x00, 0x00, 0x00};
	int ret = tas2563_check_node(pFTCC);
	if(ret == 0)
	{
		tas256x_dev_bulk_read(pFTCC->pTas256x_priv,chn, reg, pData,
			sizeof(pData));//wait to change

	}

	return ((pData[0] << 24) | (pData[1] << 16) | (pData[2] << 8) |
		(pData[3]));
}

uint32_t tas2563_switch_device(
	struct TFTCConfiguration *pFTCC, uint8_t i2cslave)
{

	return 0;
}

void tas2563_coeff_write(struct TFTCConfiguration *pFTCC,
	int chn, uint32_t reg, uint32_t data)
{
	struct TFCTBinFile *pTCalBin = &(pFTCC->mtCalBin);
	char *pDevBlock = pTCalBin->pDevBlock;
	unsigned int nByte = 0;
	int ret = 0;

	// if the bin file is open, write the coefficients to the bin file
	if (pTCalBin->pDevBlock) {
		// if the bin file is open, write the coefficients to
		// the bin file
			unsigned int index = pTCalBin->mnDevBlockIndex;
			if(index + 7 > MAX_BIN_SIZE) goto EXIT;
			pDevBlock[index] = TAS256X_BOOK_ID(reg);
			index++;
			pDevBlock[index] = TAS256X_PAGE_ID(reg);
			index++;
			pDevBlock[index] = TAS256X_PAGE_REG(reg);
			index++;

			for (nByte = 0; nByte < 4; nByte++)
				pDevBlock[index + nByte] =
					(data >> ((3 - nByte) * 8)) & 0xFF;

			pTCalBin->mnDevBlockIndex += 7;

	} else {
		unsigned char pData[4] = {(data & 0xFF000000) >> 24,
					(data & 0x00FF0000) >> 16,
					(data & 0x0000FF00) >> 8,
					data & 0x000000FF};

		ret = tas2563_check_node(pFTCC);
		if(ret < 0) goto EXIT;
		tas256x_dev_bulk_write(pFTCC->pTas256x_priv,chn, reg, pData,
			sizeof(pData));//wait to change

	}
EXIT:
	return;
}

void tas2563_ftc_start(struct TFTCConfiguration *pFTCC,
	int spkno)
{
	uint32_t prm_EnFF = 0x40000000;
	uint32_t prm_DisTck = 0x40000000;
	uint32_t prm_TE_SCThr = 0x7fffffff;
	uint32_t prm_Plt_Flag = 0x40000000;
	uint32_t prm_SineGain = 0x0a3d70a4;
	uint32_t te_TA1 = 0x0036915e; /* 100 ms*/
	uint32_t te_TA1_AT = 0x0036915e; /* 100 ms*/
	uint32_t te_TA2 = 0x0006d372; /* 100 ms*/
	uint32_t te_AT = 0x0036915e; /* 100 ms*/
	uint32_t te_DT = 0x0036915e; /* 100 ms*/
	struct TSPKCharData *pSpk = pFTCC->nTSpkCharDev;

	// FB Enable (prm_EnFF)
	pSpk[spkno].mprm_EnFF = tas2563_coeff_read(pFTCC,spkno,
		TAS2563_PRM_ENFF_REG);
	// FTE = Off (prm_DisTck)
	pSpk[spkno].mprm_DisTck = tas2563_coeff_read(pFTCC,spkno,
		TAS2563_PRM_DISTCK_REG);
	// SCE = Off (prm_TE_SCThr)
	pSpk[spkno].mprm_TE_SCThr = tas2563_coeff_read(pFTCC,spkno,
		TAS2563_PRM_TE_SCTHR_REG);
	// PE Enable = On (prm_Plt_Flag)
	pSpk[spkno].mprm_Plt_Flag = tas2563_coeff_read(pFTCC,spkno,
		TAS2563_PRM_PLT_FLAG_REG);
	// PTG = 0.8 (prm_SineGain)
	pSpk[spkno].mprm_SineGain = tas2563_coeff_read(pFTCC,spkno,
		TAS2563_PRM_SINEGAIN_REG);
	// TA1 in Temperature estimator
	pSpk[spkno].mTE_TA1 = tas2563_coeff_read(pFTCC,spkno,
		TAS2563_TE_TA1_REG);
	// TA1_AT in Temperature estimator
	pSpk[spkno].mTE_TA1_AT = tas2563_coeff_read(pFTCC,spkno,
		TAS2563_TE_TA1_AT_REG);
	// TA2 in Temperature estimator
	pSpk[spkno].mTE_TA2 = tas2563_coeff_read(pFTCC,spkno,
		TAS2563_TE_TA2_REG);
	// AT in Temperature estimator
	pSpk[spkno].mTE_AT = tas2563_coeff_read(pFTCC,spkno,
		TAS2563_TE_AT_REG);
	// DT in Temperature estimator
	pSpk[spkno].mTE_DT = tas2563_coeff_read(pFTCC,spkno,
		TAS2563_TE_DT_REG);

	// FB Enable = Off (prm_EnFF)
	tas2563_coeff_write(pFTCC,spkno, TAS2563_PRM_ENFF_REG, prm_EnFF);
	// FTE = Off (prm_DisTck)
	tas2563_coeff_write(pFTCC,spkno, TAS2563_PRM_DISTCK_REG,
		prm_DisTck);
	// SCE = Off (prm_TE_SCThr)
	tas2563_coeff_write(pFTCC,spkno, TAS2563_PRM_TE_SCTHR_REG,
		prm_TE_SCThr);
	// PE Enable = On (prm_Plt_Flag)
	tas2563_coeff_write(pFTCC,spkno, TAS2563_PRM_PLT_FLAG_REG,
		prm_Plt_Flag);
	// PTG = 0.8 (prm_SineGain) peak -22dBfs
	tas2563_coeff_write(pFTCC,spkno, TAS2563_PRM_SINEGAIN_REG,
		prm_SineGain);

	// TA1 in Temperature Estimator set to 100ms
	tas2563_coeff_write(pFTCC,spkno, TAS2563_TE_TA1_REG, te_TA1);
	// TA1_AT in Temperature Estimator set to 100ms
	tas2563_coeff_write(pFTCC,spkno, TAS2563_TE_TA1_AT_REG,
		te_TA1_AT);
	// TA2 in Temperature Estimator set to 100ms
	tas2563_coeff_write(pFTCC,spkno, TAS2563_TE_TA2_REG, te_TA2);
	// AT in Temperature Estimator set to 100ms
	tas2563_coeff_write(pFTCC,spkno, TAS2563_TE_AT_REG, te_AT);
	// DT in Temperature Estimator set to 100ms
	tas2563_coeff_write(pFTCC,spkno, TAS2563_TE_DT_REG, te_DT);
}

void tas2563_ftc_stop(struct TFTCConfiguration *pFTCC,
	int spkno)
{
	struct TSPKCharData *pSpk = pFTCC->nTSpkCharDev;
	// FB Enable  (prm_EnFF)
	tas2563_coeff_write(pFTCC,spkno, TAS2563_PRM_ENFF_REG,
		pSpk[spkno].mprm_EnFF);
	// FTE (prm_DisTck)
	tas2563_coeff_write(pFTCC,spkno, TAS2563_PRM_DISTCK_REG,
		pSpk[spkno].mprm_DisTck);
	// SCE (prm_TE_SCThr)
	tas2563_coeff_write(pFTCC,spkno, TAS2563_PRM_TE_SCTHR_REG,
		pSpk[spkno].mprm_TE_SCThr);
	// PE Enable(prm_Plt_Flag)
	tas2563_coeff_write(pFTCC,spkno, TAS2563_PRM_PLT_FLAG_REG,
		pSpk[spkno].mprm_Plt_Flag);
	// PTG (prm_SineGain)
	tas2563_coeff_write(pFTCC,spkno, TAS2563_PRM_SINEGAIN_REG,
		pSpk[spkno].mprm_SineGain);
	// TA1 in Temperature Estimator
	tas2563_coeff_write(pFTCC,spkno, TAS2563_TE_TA1_REG,
		pSpk[spkno].mTE_TA1);
	// TA1_AT in Temperature Estimator
	tas2563_coeff_write(pFTCC,spkno, TAS2563_TE_TA1_AT_REG,
		pSpk[spkno].mTE_TA1_AT);
	// TA2 in Temperature Estimator
	tas2563_coeff_write(pFTCC,spkno, TAS2563_TE_TA2_REG,
		pSpk[spkno].mTE_TA2);
	// AT in Temperature Estimator
	tas2563_coeff_write(pFTCC,spkno, TAS2563_TE_AT_REG,
		pSpk[spkno].mTE_AT);
	// DT in Temperature Estimator
	tas2563_coeff_write(pFTCC,spkno, TAS2563_TE_DT_REG,
		pSpk[spkno].mTE_DT);
}

// -------------------------------------------------------------------
// coeff_fixed_to_float
// -------------------------------------------------------------------
// Description:
//      Converts coefficients from fixed format to double.
//
// Inputs:
//      coeff - 32-bit coefficient in 5.27 format.
// -------------------------------------------------------------------
static double coeff_fixed_to_float(uint32_t coeff)
{
    double d_coeff = 0;
    if ((coeff & 0x80000000) != 0)
    {
        // 2 Complement
        d_coeff = -1 * (double)(~(coeff - 1));
    }
    else
    {
        d_coeff = (double)(coeff);
    }

    // All the coeffs are 5.x
    return d_coeff / 0x08000000;
}

// -------------------------------------------------------------------
// calc_prm_pow
// -------------------------------------------------------------------
// Description:
//      Calculates total rms power coefficient
//
// Inputs:
//      re and delta_t_max
//
// Outputs:
//      prm_pow
// -------------------------------------------------------------------
uint32_t calc_prm_pow(double re, double delta_t_max, double nRTV,
	double nRTM, double nRTVA, double nSysGain)
{
    double powerLimit;
    double pwrRootTot;

    uint32_t prm_PwrRMSTot;

    //Calculate power limit
    powerLimit = delta_t_max  / ((nRTV + nRTM)*nRTVA /
    	((nRTV + nRTM) + nRTVA));

    pwrRootTot = sqrt(re*powerLimit) / nSysGain;

    pwrRootTot = pwrRootTot / 32;

    // Convert to fixed point
    if (pwrRootTot >= 1)
        prm_PwrRMSTot = 0x7FFFFFFF;
    else
        prm_PwrRMSTot = pwrRootTot * 0x80000000;

    return prm_PwrRMSTot;
}

#define TAS2563_PRM_R0_REG		TAS256X_REG(0x00, 0x0f, 0x34)
#define TAS2563_PRM_R0_LOW_REG	TAS256X_REG(0x00, 0x0f, 0x48)
#define TAS2563_PRM_INVR0_REG		TAS256X_REG(0x00, 0x0f, 0x40)

// -------------------------------------------------------------------
// set_re
// -------------------------------------------------------------------
// Description:
//      Writes an Re value to the TAS2555.
//
// Inputs:
//      PPC3_Re0 - Nominal Re obtained during characterization.
//      re - New Re value to be written to the TAS2555.
//      alpha - Voice coil temperature coefficient of resistance.
//      t_cal - Calibration temperature.
// -------------------------------------------------------------------
void set_re(struct TFTCConfiguration *pFTCC, int spkno, double re,
	double alpha)
{
    uint32_t prm_R0 = 0;
    uint32_t prm_R0_low = 0;
    uint32_t prm_InvR0 = 0;

    double r0, r0_low, inv_r0;


    // Calculate PRM_R0
    r0 = re / 16;
    if (r0 >= 1)
        prm_R0 = 0x7FFFFFFF;
    else
        prm_R0 = r0 * 0x80000000;

    // Calculate PRM_R0_LOW
    r0_low = re*(1 + alpha*(-60)) / 16;
    if (r0_low >= 1)
        prm_R0_low = 0x7FFFFFFF;
    else
        prm_R0_low = r0_low * 0x80000000;

    // Calculate prm_InvR0
    inv_r0 = (1.0 / re);
    if (inv_r0 >= 1)
        prm_InvR0 = 0x7FFFFFFF;
    else
        prm_InvR0 = inv_r0 * 0x80000000;

    // Write values to TAS2563
		tas2563_coeff_write(pFTCC,spkno, TAS2563_PRM_R0_REG, prm_R0);
		tas2563_coeff_write(pFTCC,spkno, TAS2563_PRM_R0_LOW_REG,
			prm_R0_low);
		tas2563_coeff_write(pFTCC,spkno, TAS2563_PRM_INVR0_REG,
			prm_InvR0);
}

#define TAS2563_RUNTIME_RE_REG_TF		TAS256X_REG(0x64, 0x02, 0x70)
#define TAS2563_RUNTIME_RE_REG		TAS256X_REG(0x64, 0x02, 0x48)
// -------------------------------------------------------------------
// get_re
// -------------------------------------------------------------------
// Description:
//      Used to measure actual Re from the speaker.
//
// Inputs:
//      gpFTCC->nPPC3_Re0 - Nominal Re obtained during
//			characterization.
// -------------------------------------------------------------------
double get_re(struct TFTCConfiguration *pFTCC,int spkno)
{
	uint32_t prm_R0;
	double re;

	// Determine whether the speakers are connected?
	prm_R0 = tas2563_coeff_read(pFTCC,spkno,
		TAS2563_RUNTIME_RE_REG_TF);

	if (prm_R0 & 0x80000000) {
		return 0x7fffffff;
	} else {
		//Get re value
		prm_R0 = tas2563_coeff_read(pFTCC,spkno,
			TAS2563_RUNTIME_RE_REG);
		re = coeff_fixed_to_float(prm_R0);
	}

    return re;
}

// -------------------------------------------------------------------
// calc_prm_tlimit
// -------------------------------------------------------------------
// Description:
//      Calculates the thermal limit coefficient
//
// Inputs:
//      delta_t_max
//
// Outputs:
//      prm_TLimit
// -------------------------------------------------------------------
uint32_t calc_prm_tlimit(double delta_t_max, double alpha,
	double nDevNonlinPer, double nRTV, double nRTM,
	double nRTVA, double nPIG)
{
	// Thermal controller
	double TC_Temp_Device_NonLin;
	double TC_TLimit;
	double PI_gain;

	uint32_t prm_TLimit;

	// Calculation to Thermal Limit
	TC_Temp_Device_NonLin = floor(((1 / alpha) + delta_t_max)*
		(nDevNonlinPer / 100));

	// Read PI Gain from PPC3 configuration file
	PI_gain = nPIG;

	// Calculate TC_Limit
	TC_TLimit = (delta_t_max - TC_Temp_Device_NonLin)*
		(((2 * PI_gain) + 1) / (2 * PI_gain));
	TC_TLimit = TC_TLimit / 256;

	if (TC_TLimit >= 1)
		prm_TLimit = 0x7FFFFFFF;
	else
		prm_TLimit = TC_TLimit * 0x80000000;

	return prm_TLimit;
}

#define TAS2563_PRM_PWRRMSTOT_REG	TAS256X_REG(0x00, 0x0d, 0x3c)
#define TAS2563_PRM_TLIMIT_REG	TAS256X_REG(0x00, 0x10, 0x14)
// -------------------------------------------------------------------
// set_temp_cal
// -------------------------------------------------------------------
// Description:
//      Set Temp Calibrated values
//
// Inputs:
//      prm_pow and prm_tlimit
// -------------------------------------------------------------------
void set_temp_cal(struct TFTCConfiguration *pFTCC,uint32_t spkno,
	uint32_t prm_PwrRMSTot, uint32_t prm_TLimit)
{
	tas2563_coeff_write(pFTCC,spkno, TAS2563_PRM_PWRRMSTOT_REG,
		prm_PwrRMSTot);
	tas2563_coeff_write(pFTCC,spkno, TAS2563_PRM_TLIMIT_REG,
		prm_TLimit);
}

/* XM_339 */
//#define	TAS2563_XM_K_REG	TAS256X_REG(0x64, 0x02, 0x68)
/* XM_340 */
#define	TAS2563_XM_A1_REG	TAS256X_REG(0x64, 0x02, 0x4c)
/* XM_341 */
#define	TAS2563_XM_A2_REG	TAS256X_REG(0x64, 0x02, 0x64)

int get_f0_Q(struct TFTCConfiguration *pFTCC, uint32_t spkno,
	double nPPC3_FWarp, double nPPC3_nFS, double nPPC3_Bl,
	double nPPC3_Mms, double *pnF0, double *pnQ)
{
	int nResult = 0;
	double K, K_Dash, fp, gamma, Den;
	double K_K;		//(K * K)
	double Kd_Kd;	//(K_dash * K_Dash)
	double KmKd_KmKd;	//((K - K_Dash) * (K - K_Dash))
	double KpKd_KpKd;	//((K + K_Dash) * (K + K_Dash))
	int Re_Int, a1_Int, a2_Int/*, k_Int*/;
	double Re_dbl, a1_dbl, a2_dbl, /*k_dbl,*/ Mms, a1_dash, a2_dash,
		f_out, Q_out;

	if (!pnF0 && !pnQ)
		goto end;

	if (nPPC3_nFS == 0.0) {
		nResult = -1;
		goto end;
	}

	fp = nPPC3_FWarp;
	if (fp == 0.0) {
		K = 2.0 * nPPC3_nFS / 8;
		K_Dash = 2.0 * nPPC3_nFS;
	} else {
		K = 2 * PI * fp / tan(PI * fp / (nPPC3_nFS / 8));
		K_Dash = 2 * PI * fp / tan(PI * fp / nPPC3_nFS);
	}

	Re_Int = tas2563_coeff_read(pFTCC,spkno, TAS2563_RUNTIME_RE_REG);
	Re_dbl = coeff_fixed_to_float(Re_Int);
	if (Re_dbl == 0.0) {
		nResult = -2;
		goto end;
	}

	Mms = nPPC3_Mms * 0.001;
	if (Mms == 0.0) {
		nResult = -3;
		goto end;
	}

	gamma = pow(nPPC3_Bl, 2) / (Mms * Re_dbl);

	K_K = pow(K, 2);
	Kd_Kd = pow(K_Dash, 2);
	KpKd_KpKd = pow(K + K_Dash, 2);
	KmKd_KmKd = pow(K - K_Dash, 2);

	a1_Int = tas2563_coeff_read(pFTCC,spkno, TAS2563_XM_A1_REG);
	a2_Int = tas2563_coeff_read(pFTCC,spkno, TAS2563_XM_A2_REG);
	a1_dbl = coeff_fixed_to_float(a1_Int) * (-1.0);
	a2_dbl = coeff_fixed_to_float(a2_Int) * (-1.0);

	Den = KpKd_KpKd + (K_K - Kd_Kd) * a1_dbl + KmKd_KmKd * a2_dbl
			+ (1 - a1_dbl + a2_dbl) * gamma * K_Dash;
	if (Den == 0.0) {
		nResult = -4;
		goto end;
	}

	a1_dash = (K_K - Kd_Kd) + (K_K + Kd_Kd) * a1_dbl +
		(K_K - Kd_Kd) * a2_dbl;
	a1_dash = (2 * a1_dash) / Den;

	a2_dash = KmKd_KmKd + (K_K - Kd_Kd) * a1_dbl + KpKd_KpKd * a2_dbl
				- (1 - a1_dbl + a2_dbl) * gamma * K_Dash;
	a2_dash /= Den;

	f_out = (4 * nPPC3_nFS * nPPC3_nFS * (1 + a1_dash + a2_dash)) /
		(1- a1_dash + a2_dash);
	if (f_out < 0) {
		nResult = -5;
		goto end;
	}
	f_out = sqrt(f_out) / (2 * PI);

	if (pnF0 != NULL)
		*pnF0 = f_out;

	if (pnQ != NULL) {
		Q_out = 2 * PI * f_out * (1 - a1_dash + a2_dash);
		Q_out = Q_out / (4 * nPPC3_nFS * (1 - a2_dash));
		*pnQ = Q_out;
	}

end:

	return nResult;
}

void exTas256x_calib_start(char *spk_vendor)
{
	struct tas256x_priv* pTAS256X =
		(struct tas256x_priv*)tasdevice_get_handle();
	struct TFTCConfiguration *pFTCC = NULL;
	struct smartpa_info a;
	struct TFCTResult *nTFctResult = NULL;
	struct TSPKCharData *nTSpkCharDev = NULL;
	int i = 0, ret = 0;

	if(pTAS256X->private != NULL) {
		dev_info("%s:%u:Calibation has been started!\n",
			__func__, __LINE__);
		goto EXIT;
	}
	if(SMARTPA_POWERON == pTAS256X->dev_state) {
		/* STEP 1: Play silence externally */
		pFTCC = (struct TFTCConfiguration *)calloc(1,
			sizeof(struct TFTCConfiguration));
		if(!pFTCC) {
			dev_err("%s:%u: calloc failed\n",__func__, __LINE__);
			ret = -1;
			goto EXIT;
		}

		pFTCC->temp_cal = DEFAULT_ENV_TEMPERATURE;
		memset(&a, 0x0, sizeof(struct smartpa_info));
		if(spk_vendor != NULL) {
			if (strstr(spk_vendor,"GD_GD_GD_GD")!=NULL) {
				a.spkvendorid = SW;
				dev_info("%s:%u:spk verndor is SW!\n",
					__func__,__LINE__);
			} else {
				a.spkvendorid = GOERTEK;
			}
		} else {
			a.spkvendorid = GOERTEK;
		}
		a.ndev = pTAS256X->ndev;

		ret = InitFTCC(pFTCC, a);
		if(ret < 0) {
			dev_err("%s:%u: pFTCC init error\n",__func__, __LINE__);
			goto EXIT;
		}
		nTFctResult = pFTCC->mnTFctResult;
		nTSpkCharDev = pFTCC->nTSpkCharDev;

		pFTCC->pTas256x_priv = pTAS256X;

		pFTCC->bLoadCalibration = true;

		/* STEP 2: start calibration process */
		for(i = 0; i < pFTCC->nActiveSpk_num; i++) {
			// Default Re
			nTFctResult[i].dev_re = nTSpkCharDev[i].nPPC3_Re0;
			tas2563_switch_device(pFTCC, nTSpkCharDev[i].nDevAddr);
			tas2563_ftc_start(pFTCC, i);
		}
	}

EXIT:
	if(ret == 0) pTAS256X->private = pFTCC;
	else if(pFTCC) free(pFTCC);
	return;
}

void exTas256x_calib_stop(void)
{
	struct tas256x_priv* pTAS256X =
		(struct tas256x_priv*)tasdevice_get_handle();
	struct TFTCConfiguration *pFTCC = NULL;
	struct TFCTResult *nTFctResult = NULL;
	struct TSPKCharData *nTSpkCharDev = NULL;
	struct TFCTBinFile *pTCalBin = NULL;
	int i = 0;

	if(pTAS256X->private) {
		pFTCC = pTAS256X->private;
		/* STEP 4: Get actual Re from TAS2563 */
		for(i = 0; i < pFTCC->nActiveSpk_num; i++) {
			tas2563_switch_device(pFTCC, nTSpkCharDev[i].nDevAddr);
			nTFctResult[i].dev_re = get_re(pFTCC,i);
			dev_info("\r\n chn: %d re: %f \r\n",
				i, nTFctResult[i].dev_re );
		}

		nTFctResult = pFTCC->mnTFctResult;
		nTSpkCharDev = pFTCC->nTSpkCharDev;

		for(i = 0; i < pFTCC->nActiveSpk_num; i++) {
			/* STEP 5: check speaker bounds */
			nTFctResult[i].mCalbirationResult =
				check_spk_bounds(pFTCC, nTFctResult[i].dev_re, i);
			/* STEP 6: Set temperature limit to target TMAX */
			if ((nTFctResult[i].mCalbirationResult & RE_CHK_MSK)
				== RESULT_PASS) {
				tas2563_switch_device(pFTCC,
					nTSpkCharDev[i].nDevAddr);
				nTFctResult[i].dev_prm_pow =
					calc_prm_pow(nTFctResult[i].dev_re,
					nTSpkCharDev[i].nSpkTMax - pFTCC->temp_cal,
					nTSpkCharDev[i].nPPC3_RTV,
					nTSpkCharDev[i].nPPC3_RTM,
					nTSpkCharDev[i].nPPC3_RTVA,
					nTSpkCharDev[i].nPPC3_SysGain);
				nTFctResult[i].dev_prm_tlimit =
					calc_prm_tlimit(nTSpkCharDev[i].nSpkTMax -
						pFTCC->temp_cal,
					nTSpkCharDev[i].nSpkReAlpha,
					nTSpkCharDev[i].nPPC3_DevNonlinPer,
					nTSpkCharDev[i].nPPC3_RTV,
					nTSpkCharDev[i].nPPC3_RTM,
					nTSpkCharDev[i].nPPC3_RTVA,
					nTSpkCharDev[i].nPPC3_PIG);
				set_re(pFTCC,i, nTFctResult[i].dev_re,
					nTSpkCharDev[i].nSpkReAlpha);
				set_temp_cal(pFTCC,i, nTFctResult[i].dev_prm_pow,
					nTFctResult[i].dev_prm_tlimit);
			}
		}

		for(i = 0; i < pFTCC->nActiveSpk_num; i++) {
			tas2563_switch_device(pFTCC,
				pFTCC->nTSpkCharDev[i].nDevAddr);
			tas2563_ftc_stop(pFTCC, i);
		}
		/*Step 6.1: Stop playing */
		pFTCC->cal_count = (pFTCC->cal_count + 1) % CALIBRATION_COUNT;
		/* STEP 8: Save .bin */
		pTCalBin = &(pFTCC->mtCalBin);
		pTCalBin->pDevBlock = (char *)calloc(MAX_BIN_SIZE,
			sizeof(char));
		if(NULL == pTCalBin->pDevBlock) {
			dev_err("%s:%u:calloc memory error\n",__func__,__LINE__);
			goto EXIT;
		}
		pTCalBin->mnDevBlockIndex = 0;

		for(i = 0; i < pFTCC->nActiveSpk_num; i++) {
			memset(pTCalBin->pDevBlock, 0x0, 36);
			pTCalBin->mnDevBlockIndex = 0;
			if ((nTFctResult[i].mCalbirationResult & RE_CHK_MSK)
				== RESULT_PASS) {
				pTCalBin->pDevBlock[pTCalBin->mnDevBlockIndex] = 1;
				pTCalBin->mnDevBlockIndex = 1;
				tas2563_switch_device(pFTCC,
					nTSpkCharDev[i].nDevAddr);
				set_re(pFTCC,i, nTFctResult[i].dev_re,
					nTSpkCharDev[i].nSpkReAlpha);
				set_temp_cal(pFTCC,i, nTFctResult[i].dev_prm_pow,
					nTFctResult[i].dev_prm_tlimit);
			} else {
				pTCalBin->mnDevBlockIndex = 36;
			}
			if(nTFctResult[i].bWrittenintoFile == false) {
				tas2563_save_calibrated_data(pFTCC,i);
				nTFctResult[i].bWrittenintoFile = true;
			}
		}
	}else {
		dev_err("%s:%u:PA handle has not been ready!\n",
			__func__,__LINE__);
	}
EXIT:
	if(pFTCC) {
		if(pFTCC->mnTFctResult) free(pFTCC->mnTFctResult);
		if(pFTCC->nTSpkCharDev) free(pFTCC->nTSpkCharDev);
		if(pTCalBin->pDevBlock) {
			free(pTCalBin->pDevBlock);
			pTCalBin->pDevBlock = NULL;
		}
		free(pFTCC);
		pTAS256X->private = NULL;
	}

	return;
}


int exTas256x_get_f0(unsigned int *f0_array)
{
	int ret = 0;
	struct TFTCConfiguration *pFTCC = NULL;
	if(f0_array != NULL) {
		struct tas256x_priv* pTAS256X =
			(struct tas256x_priv*)tasdevice_get_handle();
		struct smartpa_info a;
		/*struct TFCTResult *nTFctResult = NULL;*/
		struct TSPKCharData *nTSpkCharDev = NULL;
		unsigned int i = 0;
		double nDevAF0, nDevAQ;
		pFTCC = (struct TFTCConfiguration *)calloc(1,
			sizeof(struct TFTCConfiguration));
		if(!pFTCC) {
			dev_err("%s:%u: calloc failed\n",__func__, __LINE__);
			goto EXIT;
		}
		a.ndev = pTAS256X->ndev;

		ret = InitFTCC(pFTCC, a);
		pFTCC->pTas256x_priv = pTAS256X;
		nTSpkCharDev = pFTCC->nTSpkCharDev;
		for(i = 0; i < pFTCC->nActiveSpk_num; i++) {
			f0_array[i] = 0;
			tas2563_switch_device(pFTCC, nTSpkCharDev[i].nDevAddr);

			ret = get_f0_Q(pFTCC,i, nTSpkCharDev[i].nPPC3_FWarp,
				pFTCC->nPPC3_FS, nTSpkCharDev[i].nPPC3_Bl,
				nTSpkCharDev[i].nPPC3_Mms, &nDevAF0, &nDevAQ);
			if(ret < 0) {
				dev_err("ERROR:%s:L:%u: failed rc = %d!\n",
					__func__, __LINE__, ret);
				continue;
			}
			if(nDevAF0 > 0 || nDevAF0 < 4000) {
				f0_array[i] = (unsigned int)(nDevAF0);// * 0x80000);
			}
		}
	} else ret = -1;
EXIT:
	if(pFTCC->mnTFctResult) free(pFTCC->mnTFctResult);
	if(pFTCC->nTSpkCharDev) free(pFTCC->nTSpkCharDev);

	if(pFTCC) free(pFTCC);
	return ret;
}
