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

#ifndef TAS2563_LIB_FTC_H_
#define TAS2563_LIB_FTC_H_

#include <stdint.h>
#include <stdbool.h>

#include "tas2563.h"

#define PI                   (3.14159)

int get_lib_ver(void);
void tas2563_ftc_start(struct TFTCConfiguration *pFTCC,
	int spkno);
void set_re(struct TFTCConfiguration *pFTCC, int spkno, double re,
	double alpha);
void set_temp_cal(struct TFTCConfiguration *pFTCC,uint32_t spkno,
	uint32_t prm_pow, uint32_t prm_tlimit);
double get_re(struct TFTCConfiguration *pFTCC,int spkno);
double CalcRe(double re_ppc3, uint32_t prm_r0);
uint32_t calc_prm_pow(double re, double delta_t_max, double nRTV,
			double nRTM, double nRTVA, double nSysGain);
uint32_t calc_prm_tlimit(double delta_t_max, double alpha,
			double nDevNonlinPer, double nRTV,
			double nRTM, double nRTVA, double nPIG);
void tas2563_ftc_stop(struct TFTCConfiguration *pFTCC,
	int spkno);

/* below functions are used in SPK measurement only */
int get_Re_deltaT(struct TFTCConfiguration *pFTCC,
	double nPPC3_alpha,	double *pnRe, double *pnDeltaT);
int get_f0_Q(struct TFTCConfiguration *pFTCC, uint32_t spkno,
	double nPPC3_FWarp, double nPPC3_nFS,
	double nPPC3_Bl, double nPPC3_Mms, double *pnF0, double *pnQ);

#endif /* TAS2563_LIB_FTC_H_ */
