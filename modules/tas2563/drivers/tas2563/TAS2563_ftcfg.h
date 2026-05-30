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

#ifndef __TAS2563_FTCFG_H__
#define __TAS2563_FTCFG_H__
struct TSPKCharData TAS2563_ftcfg[] = {
{
.nSpkTMax = 100,
.nSpkReTolPer = 10,
.nSpkReAlpha = 0.0033,
.nPPC3_Re0  = 6.7, //
.nPPC3_FWarp = 891,
.nPPC3_Bl = 0.814,
.nPPC3_Mms = 0.0667,
.nPPC3_RTV = 63.4,
.nPPC3_RTM = 566.7,
.nPPC3_RTVA = 136,
.nPPC3_SysGain = 8.92,
.nPPC3_DevNonlinPer = 1.5,
.nPPC3_PIG = 2,
.nReHi = 7.975,
.nReLo = 6.525,
.nf0Hi = 924,
.nf0Lo = 616,
.nQHi  = 1.704,
.nQLo = 1.136,
.nDevAddr  = 0x98
},
#ifdef STEREO
{
.nSpkTMax = 100,
.nSpkReTolPer = 10,
.nSpkReAlpha = 0.0033,
.nPPC3_Re0  = 6.7, //
.nPPC3_FWarp = 891,
.nPPC3_Bl = 0.814,
.nPPC3_Mms = 0.0667,
.nPPC3_RTV = 63.4,
.nPPC3_RTM = 566.7,
.nPPC3_RTVA = 136,
.nPPC3_SysGain = 8.92,
.nPPC3_DevNonlinPer = 1.5,
.nPPC3_PIG = 2,
.nReHi = 7.975,
.nReLo = 6.525,
.nf0Hi = 924,
.nf0Lo = 616,
.nQHi  = 1.704,
.nQLo = 1.136,
.nDevAddr  = 0x9A
},
#elif defined WOOFER_TWEETERS
{
.nSpkTMax = 100,
.nSpkReTolPer = 10,
.nSpkReAlpha = 0.0033,
.nPPC3_Re0  = 6.7, //
.nPPC3_FWarp = 891,
.nPPC3_Bl = 0.814,
.nPPC3_Mms = 0.0667,
.nPPC3_RTV = 63.4,
.nPPC3_RTM = 566.7,
.nPPC3_RTVA = 136,
.nPPC3_SysGain = 8.92,
.nPPC3_DevNonlinPer = 1.5,
.nPPC3_PIG = 2,
.nReHi = 7.975,
.nReLo = 6.525,
.nf0Hi = 924,
.nf0Lo = 616,
.nQHi  = 1.704,
.nQLo = 1.136,
.nDevAddr  = 0x9A
},
{
.nSpkTMax = 100,
.nSpkReTolPer = 10,
.nSpkReAlpha = 0.0033,
.nPPC3_Re0  = 6.7, //
.nPPC3_FWarp = 891,
.nPPC3_Bl = 0.814,
.nPPC3_Mms = 0.0667,
.nPPC3_RTV = 63.4,
.nPPC3_RTM = 566.7,
.nPPC3_RTVA = 136,
.nPPC3_SysGain = 8.92,
.nPPC3_DevNonlinPer = 1.5,
.nPPC3_PIG = 2,
.nReHi = 7.975,
.nReLo = 6.525,
.nf0Hi = 924,
.nf0Lo = 616,
.nQHi  = 1.704,
.nQLo = 1.136,
.nDevAddr  = 0x9C
},
#elif defined FOUR_PAS
{
.nSpkTMax = 100,
.nSpkReTolPer = 10,
.nSpkReAlpha = 0.0033,
.nPPC3_Re0  = 6.7, //
.nPPC3_FWarp = 891,
.nPPC3_Bl = 0.814,
.nPPC3_Mms = 0.0667,
.nPPC3_RTV = 63.4,
.nPPC3_RTM = 566.7,
.nPPC3_RTVA = 136,
.nPPC3_SysGain = 8.92,
.nPPC3_DevNonlinPer = 1.5,
.nPPC3_PIG = 2,
.nReHi = 7.975,
.nReLo = 6.525,
.nf0Hi = 924,
.nf0Lo = 616,
.nQHi  = 1.704,
.nQLo = 1.136,
.nDevAddr  = 0x9A
},
{
.nSpkTMax = 100,
.nSpkReTolPer = 10,
.nSpkReAlpha = 0.0033,
.nPPC3_Re0  = 6.7, //
.nPPC3_FWarp = 891,
.nPPC3_Bl = 0.814,
.nPPC3_Mms = 0.0667,
.nPPC3_RTV = 63.4,
.nPPC3_RTM = 566.7,
.nPPC3_RTVA = 136,
.nPPC3_SysGain = 8.92,
.nPPC3_DevNonlinPer = 1.5,
.nPPC3_PIG = 2,
.nReHi = 7.975,
.nReLo = 6.525,
.nf0Hi = 924,
.nf0Lo = 616,
.nQHi  = 1.704,
.nQLo = 1.136,
.nDevAddr  = 0x9C
},
{
.nSpkTMax = 100,
.nSpkReTolPer = 10,
.nSpkReAlpha = 0.0033,
.nPPC3_Re0  = 6.7, //
.nPPC3_FWarp = 891,
.nPPC3_Bl = 0.814,
.nPPC3_Mms = 0.0667,
.nPPC3_RTV = 63.4,
.nPPC3_RTM = 566.7,
.nPPC3_RTVA = 136,
.nPPC3_SysGain = 8.92,
.nPPC3_DevNonlinPer = 1.5,
.nPPC3_PIG = 2,
.nReHi = 7.975,
.nReLo = 6.525,
.nf0Hi = 924,
.nf0Lo = 616,
.nQHi  = 1.704,
.nQLo = 1.136,
.nDevAddr  = 0x9E
},
#endif
};
#endif