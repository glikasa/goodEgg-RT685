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
* combinations with devices manufactured by or for TI (揟I Devices?.
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
* THIS SOFTWARE IS PROVIDED BY TI AND TI扴 LICENSORS "AS IS" AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
* PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL TI AND TI扴 LICENSORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
* OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
* BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __REGISTER_SETTING_H__
#define __REGISTER_SETTING_H__

cfg_reg shut_down[] = {
	//BOOK0 PAGE0
	{{ 0x00, 0x00 }},
	{{ 0x7f, 0x00 }},
	{{ 0x00, 0x00 }},
	{{ 0x02, 0x0e }},
};

#if defined FOUR_PAS
cfg_reg bypass_power_up_prim[] = {
	//BOOK0 PAGE0
	{{ 0x00, 0x00 }},
	{{ 0x7f, 0x00 }},
	{{ 0x00, 0x00 }},
    //{ 0x06, 0x08 }}, //RX Rising edge
	{{ 0x0a, 0x13 }},
#ifdef RX_16BIT
	{{ 0x08, 0x10 }}, // RX:16-bit, L channel
#else
	{{ 0x08, 0x5e }}, // RX:32-bit, L channel
#endif
	{{ 0x0b, 0x02 }}, // disable Vsns
	{{ 0x0c, 0x00 }}, // disable Isns, Isns: slot 2
	{{ 0x30, 0x99 }},
	{{ 0x02, 0x0c }},
	{{ 0x03, 0x02 }} //change AMP_LEVEL to lowest
};

cfg_reg bypass_power_up_sec[] = {
	//BOOK0 PAGE0
	{{ 0x00, 0x00 }},
	{{ 0x7f, 0x00 }},
	{{ 0x00, 0x00 }},
    //{ 0x06, 0x08 }}, //RX Rising edge
	{{ 0x0a, 0x13 }},
#ifdef RX_16BIT
	{{ 0x08, 0x20 }}, //  RX:16-bit, R channel
#else
	{{ 0x08, 0x6e }}, //  RX:32-bit, R channel
#endif
	{{ 0x0b, 0x02 }}, // disable Vsns
	{{ 0x0c, 0x00 }}, // disable Isns, Isns: slot 2
	{{ 0x30, 0x99 }},
	{{ 0x02, 0x0c }},
	{{ 0x03, 0x02 }} //change AMP_LEVEL to lowest
};

cfg_reg bypass_power_up_tert[] = {
	//BOOK0 PAGE0
	{{ 0x00, 0x00 }},
	{{ 0x7f, 0x00 }},
	{{ 0x00, 0x00 }},
    //{ 0x06, 0x08 }}, //RX Rising edge
	{{ 0x0a, 0x13 }},
#ifdef RX_16BIT
	{{ 0x08, 0x10 }}, // RX:16-bit, L channel
#else
	{{ 0x08, 0x5e }}, // RX:32-bit, L channel
#endif
	{{ 0x0b, 0x02 }}, // disable Vsns
	{{ 0x0c, 0x00 }}, // disable Isns, Isns: slot 2
	{{ 0x30, 0x99 }},
	{{ 0x02, 0x0c }},
	{{ 0x03, 0x02 }} //change AMP_LEVEL to lowest
};

cfg_reg bypass_power_up_quat[] = {
	//BOOK0 PAGE0
	{{ 0x00, 0x00 }},
	{{ 0x7f, 0x00 }},
	{{ 0x00, 0x00 }},
    //{ 0x06, 0x08 }}, //RX Rising edge
	{{ 0x0a, 0x13 }},
#ifdef RX_16BIT
	{{ 0x08, 0x20 }}, //  RX:16-bit, R channel
#else
	{{ 0x08, 0x6e }}, //  RX:32-bit, R channel
#endif
	{{ 0x0b, 0x02 }}, // disable Vsns
	{{ 0x0c, 0x00 }}, // disable Isns, Isns: slot 2
	{{ 0x30, 0x99 }},
	{{ 0x02, 0x0c }},
	{{ 0x03, 0x02 }} //change AMP_LEVEL to lowest
};
cfg_reg power_up_prim[] = {
	//BOOK0 PAGE0
	{{ 0x00, 0x00 }},
	{{ 0x7f, 0x00 }},
	{{ 0x00, 0x00 }},
    //{ 0x06, 0x08 }}, //RX Rising edge
#ifdef RX_16BIT
	{{ 0x08, 0x70 }}, // RX:16-bit, L channel TX:8-bit
	{{ 0x0c, 0x40 }}, // enable Isns, Isns: slot 0
	{{ 0x0b, 0x41 }}, // enable Vsns
#else
	{{ 0x08, 0x7e }}, // RX:32-bit, L channel TX:16-bit
	{{ 0x0c, 0x40 }}, // enable Isns, Isns: slot 0
	{{ 0x0b, 0x42 }}, // enable Vsns
#endif
	{{ 0x30, 0x99 }},
	{{ 0x0a, 0x13 }},
	{{ 0x41, 0x41 }},
	{{ 0x07, 0x02 }},
	{{ 0x02, 0x00 }},
	{{ 0x03, 0x02 }}
};

cfg_reg power_up_sec[] = {
	//BOOK0 PAGE0
	{{ 0x00, 0x00 }},
	{{ 0x7f, 0x00 }},
	{{ 0x00, 0x00 }},
    //{ 0x06, 0x08 }}, //RX Rising edge
#ifdef RX_16BIT
	{{ 0x08, 0x70 }}, // RX:16-bit, R channel TX:8-bit
	{{ 0x0c, 0x42 }}, // enable Isns, Isns: slot 2
	{{ 0x0b, 0x43 }}, // enable Vsns
#else
	{{ 0x08, 0x7e }}, // RX:32-bit, R channel TX:16-bit
	{{ 0x0c, 0x44 }}, // enable Isns, Isns: slot 4
	{{ 0x0b, 0x46 }}, // enable Vsns
#endif
	{{ 0x30, 0x99 }},
	{{ 0x0a, 0xf3 }},
	{{ 0x41, 0x41 }},
	{{ 0x07, 0x02 }},
	{{ 0x02, 0x00 }},
	{{ 0x03, 0x02 }}
};

cfg_reg power_up_tert[] = {
	//BOOK0 PAGE0
	{{ 0x00, 0x00 }},
	{{ 0x7f, 0x00 }},
	{{ 0x00, 0x00 }},
    //{ 0x06, 0x08 }}, //RX Rising edge
#ifdef RX_16BIT
	{{ 0x08, 0x70 }}, // RX:16-bit, L channel TX:8-bit
	{{ 0x0c, 0x40 }}, // enable Isns, Isns: slot 0
	{{ 0x0b, 0x41 }}, // enable Vsns
#else
	{{ 0x08, 0x7e }}, // RX:32-bit, L channel TX:16-bit
	{{ 0x0c, 0x40 }}, // enable Isns, Isns: slot 0
	{{ 0x0b, 0x42 }}, // enable Vsns
#endif
	{{ 0x30, 0x99 }},
	{{ 0x0a, 0x13 }},
	{{ 0x41, 0x41 }},
	{{ 0x07, 0x02 }},
	{{ 0x02, 0x00 }},
	{{ 0x03, 0x02 }}
};

cfg_reg power_up_quat[] = {
	//BOOK0 PAGE0
	{{ 0x00, 0x00 }},
	{{ 0x7f, 0x00 }},
	{{ 0x00, 0x00 }},
    //{ 0x06, 0x08 }}, //RX Rising edge
#ifdef RX_16BIT
	{{ 0x08, 0x70 }}, // RX:16-bit, R channel TX:8-bit
	{{ 0x0c, 0x42 }}, // enable Isns, Isns: slot 2
	{{ 0x0b, 0x43 }}, // enable Vsns
#else
	{{ 0x08, 0x7e }}, // RX:32-bit, R channel TX:16-bit
	{{ 0x0c, 0x44 }}, // enable Isns, Isns: slot 4
	{{ 0x0b, 0x46 }}, // enable Vsns
#endif
	{{ 0x30, 0x99 }},
	{{ 0x0a, 0xf3 }},
	{{ 0x41, 0x41 }},
	{{ 0x07, 0x02 }},
	{{ 0x02, 0x00 }},
	{{ 0x03, 0x02 }}
};
#elif defined WOOFER_TWEETERS
cfg_reg bypass_power_up_prim[] = {
	//BOOK0 PAGE0
	{{ 0x00, 0x00 }},
	{{ 0x7f, 0x00 }},
	{{ 0x00, 0x00 }},
    //{ 0x06, 0x08 }}, //RX Rising edge
	{{ 0x0a, 0x13 }},
#ifdef RX_16BIT
	{{ 0x08, 0x10 }}, // RX:16-bit, L channel
#else
	{{ 0x08, 0x5e }}, // RX:32-bit, L channel
#endif
	{{ 0x0b, 0x02 }}, // disable Vsns
	{{ 0x0c, 0x00 }}, // disable Isns, Isns: slot 2
	{{ 0x30, 0x99 }},
	{{ 0x02, 0x0c }},
	{{ 0x03, 0x02 }} //change AMP_LEVEL to lowest
};

cfg_reg bypass_power_up_sec[] = {
	//BOOK0 PAGE0
	{{ 0x00, 0x00 }},
	{{ 0x7f, 0x00 }},
	{{ 0x00, 0x00 }},
    //{ 0x06, 0x08 }}, //RX Rising edge
	{{ 0x0a, 0x13 }},
#ifdef RX_16BIT
	{{ 0x08, 0x20 }}, //  RX:16-bit, R channel
#else
	{{ 0x08, 0x6e }}, //  RX:32-bit, R channel
#endif
	{{ 0x0b, 0x02 }}, // disable Vsns
	{{ 0x0c, 0x00 }}, // disable Isns, Isns: slot 2
	{{ 0x30, 0x99 }},
	{{ 0x02, 0x0c }},
	{{ 0x03, 0x02 }} //change AMP_LEVEL to lowest
};

cfg_reg bypass_power_up_tert[] = {
	//BOOK0 PAGE0
	{{ 0x00, 0x00 }},
	{{ 0x7f, 0x00 }},
	{{ 0x00, 0x00 }},
    //{ 0x06, 0x08 }}, //RX Rising edge
	{{ 0x0a, 0x13 }},
#ifdef RX_16BIT
	{{ 0x08, 0x20 }}, //  RX:16-bit, R channel
#else
	{{ 0x08, 0x6e }}, //  RX:32-bit, R channel
#endif
	{{ 0x0b, 0x02 }}, // disable Vsns
	{{ 0x0c, 0x00 }}, // disable Isns, Isns: slot 2
	{{ 0x30, 0x99 }},
	{{ 0x02, 0x0c }},
	{{ 0x03, 0x02 }} //change AMP_LEVEL to lowest
};

cfg_reg power_up_prim[] = {
	//BOOK0 PAGE0
	{{ 0x00, 0x00 }},
	{{ 0x7f, 0x00 }},
	{{ 0x00, 0x00 }},
    //{ 0x06, 0x08 }}, //RX Rising edge
#ifdef RX_16BIT
	{{ 0x08, 0x70 }}, // RX:16-bit, L channel TX:8-bit
	{{ 0x0c, 0x40 }}, // enable Isns, Isns: slot 0
	{{ 0x0b, 0x41 }}, // enable Vsns
#else
	{{ 0x08, 0x7e }}, // RX:32-bit, L channel TX:16-bit
	{{ 0x0c, 0x40 }}, // enable Isns, Isns: slot 0
	{{ 0x0b, 0x42 }}, // enable Vsns
#endif
	{{ 0x30, 0x99 }},
	{{ 0x0a, 0x13 }},
	{{ 0x41, 0x41 }},
	{{ 0x07, 0x02 }},
	{{ 0x02, 0x00 }},
	{{ 0x03, 0x02 }}
};

cfg_reg power_up_sec[] = {
	//BOOK0 PAGE0
	{{ 0x00, 0x00 }},
	{{ 0x7f, 0x00 }},
	{{ 0x00, 0x00 }},
    //{ 0x06, 0x08 }}, //RX Rising edge
#ifdef RX_16BIT
	{{ 0x08, 0x70 }}, // RX:16-bit, R channel TX:8-bit
	{{ 0x0c, 0x42 }}, // enable Isns, Isns: slot 2
	{{ 0x0b, 0x43 }}, // enable Vsns
#else
	{{ 0x08, 0x7e }}, // RX:32-bit, R channel TX:16-bit
	{{ 0x0c, 0x44 }}, // enable Isns, Isns: slot 4
	{{ 0x0b, 0x46 }}, // enable Vsns
#endif
	{{ 0x30, 0x99 }},
	{{ 0x0a, 0xf3 }},
	{{ 0x41, 0x41 }},
	{{ 0x07, 0x02 }},
	{{ 0x02, 0x00 }},
	{{ 0x03, 0x02 }}
};
cfg_reg power_up_tert[] = {
	//BOOK0 PAGE0
	{{ 0x00, 0x00 }},
	{{ 0x7f, 0x00 }},
	{{ 0x00, 0x00 }},
    //{ 0x06, 0x08 }}, //RX Rising edge
#ifdef RX_16BIT
	{{ 0x08, 0x70 }}, // RX:16-bit, R channel TX:8-bit
	{{ 0x0c, 0x42 }}, // enable Isns, Isns: slot 2
	{{ 0x0b, 0x43 }}, // enable Vsns
#else
	{{ 0x08, 0x7e }}, // RX:32-bit, R channel TX:16-bit
	{{ 0x0c, 0x44 }}, // enable Isns, Isns: slot 4
	{{ 0x0b, 0x46 }}, // enable Vsns
#endif
	{{ 0x30, 0x99 }},
	{{ 0x0a, 0xf3 }},
	{{ 0x41, 0x41 }},
	{{ 0x07, 0x02 }},
	{{ 0x02, 0x00 }},
	{{ 0x03, 0x02 }}
};
#elif defined STEREO
cfg_reg bypass_power_up_prim[] = {
	//BOOK0 PAGE0
	{{ 0x00, 0x00 }},
	{{ 0x7f, 0x00 }},
	{{ 0x00, 0x00 }},
    //{ 0x06, 0x08 }}, //RX Rising edge
	{{ 0x0a, 0x13 }},
#ifdef RX_16BIT
	{{ 0x08, 0x10 }}, // RX:16-bit, L channel
#else
	{{ 0x08, 0x5e }}, // RX:32-bit, L channel
#endif
	{{ 0x0b, 0x02 }}, // disable Vsns
	{{ 0x0c, 0x00 }}, // disable Isns, Isns: slot 2
	{{ 0x30, 0x99 }},
	{{ 0x02, 0x0c }},
	{{ 0x03, 0x02 }} //change AMP_LEVEL to lowest
};

cfg_reg bypass_power_up_sec[] = {
	//BOOK0 PAGE0
	{{ 0x00, 0x00 }},
	{{ 0x7f, 0x00 }},
	{{ 0x00, 0x00 }},
    //{ 0x06, 0x08 }}, //RX Rising edge
	{{ 0x0a, 0x13 }},
#ifdef RX_16BIT
	{{ 0x08, 0x20 }}, //  RX:16-bit, R channel
#else
	{{ 0x08, 0x6e }}, //  RX:32-bit, R channel
#endif
	{{ 0x0b, 0x02 }}, // disable Vsns
	{{ 0x0c, 0x00 }}, // disable Isns, Isns: slot 2
	{{ 0x30, 0x99 }},
	{{ 0x02, 0x0c }},
	{{ 0x03, 0x02 }} //change AMP_LEVEL to lowest
};

cfg_reg power_up_prim[] = {
	//BOOK0 PAGE0
	{{ 0x00, 0x00 }},
	{{ 0x7f, 0x00 }},
	{{ 0x00, 0x00 }},
    //{ 0x06, 0x08 }}, //RX Rising edge
#ifdef RX_16BIT
	{{ 0x08, 0x70 }}, // RX:16-bit, L channel TX:8-bit
	{{ 0x0c, 0x40 }}, // enable Isns, Isns: slot 0
	{{ 0x0b, 0x41 }}, // enable Vsns
#else
	{{ 0x08, 0x7e }}, // RX:32-bit, L channel TX:16-bit
	{{ 0x0c, 0x40 }}, // enable Isns, Isns: slot 0
	{{ 0x0b, 0x42 }}, // enable Vsns
#endif
	{{ 0x30, 0x99 }},
	{{ 0x0a, 0x13 }},
	{{ 0x41, 0x41 }},
	{{ 0x07, 0x02 }},
	{{ 0x02, 0x00 }},
	{{ 0x03, 0x02 }}
};

cfg_reg power_up_sec[] = {
	//BOOK0 PAGE0
	{{ 0x00, 0x00 }},
	{{ 0x7f, 0x00 }},
	{{ 0x00, 0x00 }},
    //{ 0x06, 0x08 }}, //RX Rising edge
#ifdef RX_16BIT
	{{ 0x08, 0x70 }}, // RX:16-bit, R channel TX:8-bit
	{{ 0x0c, 0x42 }}, // enable Isns, Isns: slot 2
	{{ 0x0b, 0x43 }}, // enable Vsns
#else
	{{ 0x08, 0x7e }}, // RX:32-bit, R channel TX:16-bit
	{{ 0x0c, 0x44 }}, // enable Isns, Isns: slot 4
	{{ 0x0b, 0x46 }}, // enable Vsns
#endif
	{{ 0x30, 0x99 }},
	{{ 0x0a, 0xf3 }},
	{{ 0x41, 0x41 }},
	{{ 0x07, 0x02 }},
	{{ 0x02, 0x00 }},
	{{ 0x03, 0x02 }}
};
#else
cfg_reg bypass_power_up_prim[] = {
	//BOOK0 PAGE0
    {{ 0x00, 0x00 }},
    {{ 0x7f, 0x00 }},
    {{ 0x00, 0x00 }},
    //{ 0x06, 0x08 }}, //RX Rising edge
    {{ 0x0a, 0x13 }},
#ifdef RX_16BIT
    {{ 0x08, 0x70 }}, // RX&TX:16-bit, R channel //
#else
	{{ 0x08, 0x7e }},
#endif
    {{ 0x0b, 0x02 }}, // disable Vsns
    {{ 0x0c, 0x00 }}, // disable Isns, Isns: slot 2
	{{ 0x30, 0x99 }},
	{{ 0x02, 0x0c }},
    {{ 0x03, 0x02 }} //change AMP_LEVEL to lowest
};

cfg_reg power_up_prim[] = {
	//BOOK0 PAGE0
    {{ 0x00, 0x00 }},
    {{ 0x7f, 0x00 }},
    {{ 0x00, 0x00 }},
    //{ 0x06, 0x08 }}, //RX Rising edge
    {{ 0x0b, 0x44 }}, // enable Vsns
#ifdef RX_16BIT
    {{ 0x08, 0x70 }}, // RX:16-bit, R channel TX:8-bit
    {{ 0x0c, 0x41 }}, // enable Isns, Isns: slot 1
#else
	{{ 0x08, 0x7e }},
    {{ 0x0c, 0x46 }}, // enable Isns, Isns: slot 6
#endif
	{{ 0x30, 0x99 }},
	{{ 0x0a, 0x13 }},
	{{ 0x41, 0x41 }},
	{{ 0x07, 0x02 }},
	{{ 0x02, 0x00 }},
    {{ 0x03, 0x02 }}
};
#endif

#ifdef PDM_ENABLE
cfg_reg standalone_record_on[] = {
    //BOOK0 PAGE0
	{{ 0x00, 0x00 }},
	{{ 0x7f, 0x00 }},
	{{ 0x00, 0x00 }},
	{{ 0x0b, 0x40 }}, // enable Vsns
#ifdef RX_16BIT
	{{ 0x08, 0x20 }}, // RX:16-bit, R channel TX:8-bit
	{{ 0x0c, 0x41 }}, // enable Isns, Isns: slot 1
#else
	{{ 0x08, 0x6e }},
	{{ 0x0c, 0x42 }}, // enable Isns, Isns: slot 2
#endif
	{{ 0x30, 0x99 }},
	{{ 0x41, 0x16 }}, // PDM_CONFIG0: Enable pmic1&pmic2
	{{ 0x0a, 0x13 }},
	{{ 0x02, 0x80 }}, // Enable PDM_I2S mode, Isns & Vsns powerdown
};

cfg_reg mixture_playback_record[] = {
    //BOOK0 PAGE0
	{{ 0x00, 0x00 }},
	{{ 0x7f, 0x00 }},
	{{ 0x00, 0x00 }},
	{{ 0x0b, 0x40 }}, // enable Vsns
	{{ 0x41, 0x16 }}, // PDM_CONFIG0: Enable pmic1&pmic2
	{{ 0x30, 0x99 }},
	{{ 0x08, 0x6e }},
	{{ 0x0c, 0x42 }}, // enable Isns, Isns: slot 2
	{{ 0x0a, 0x13 }},
	{{ 0x02, 0x00 }}
};
#else
#if defined FOUR_PAS
cfg_reg voice_power_up_prim[] = {
	//BOOK0 PAGE0
	{{ 0x00, 0x00 }},
	{{ 0x7f, 0x00 }},
	{{ 0x00, 0x00 }},
    //{ 0x06, 0x08 }}, //RX Rising edge
	{{ 0x0b, 0x44 }}, // enable Vsns
	{{ 0x08, 0x6e }},
	{{ 0x0c, 0x46 }}, // enable Isns, Isns: slot 6
	{{ 0x30, 0x99 }},
	{{ 0x0a, 0xf1 }},
	{{ 0x07, 0x00 }},
	{{ 0x02, 0x00 }}
};
cfg_reg voice_power_up_sec[] = {
	//BOOK0 PAGE0
	{{ 0x00, 0x00 }},
	{{ 0x7f, 0x00 }},
	{{ 0x00, 0x00 }},
    //{ 0x06, 0x08 }}, //RX Rising edge
	{{ 0x0b, 0x44 }}, // enable Vsns
	{{ 0x08, 0x6e }},
	{{ 0x0c, 0x46 }}, // enable Isns, Isns: slot 6
	{{ 0x30, 0x99 }},
	{{ 0x0a, 0xf1 }},
	{{ 0x07, 0x00 }},
	{{ 0x02, 0x00 }}
};
cfg_reg voice_power_up_tert[] = {
	//BOOK0 PAGE0
	{{ 0x00, 0x00 }},
	{{ 0x7f, 0x00 }},
	{{ 0x00, 0x00 }},
    //{ 0x06, 0x08 }}, //RX Rising edge
	{{ 0x0b, 0x44 }}, // enable Vsns
	{{ 0x08, 0x6e }},
	{{ 0x0c, 0x46 }}, // enable Isns, Isns: slot 6
	{{ 0x30, 0x99 }},
	{{ 0x0a, 0xf1 }},
	{{ 0x07, 0x00 }},
	{{ 0x02, 0x00 }}
};
cfg_reg voice_power_up_quat[] = {
	//BOOK0 PAGE0
	{{ 0x00, 0x00 }},
	{{ 0x7f, 0x00 }},
	{{ 0x00, 0x00 }},
    //{ 0x06, 0x08 }}, //RX Rising edge
	{{ 0x0b, 0x44 }}, // enable Vsns
	{{ 0x08, 0x6e }},
	{{ 0x0c, 0x46 }}, // enable Isns, Isns: slot 6
	{{ 0x30, 0x99 }},
	{{ 0x0a, 0xf1 }},
	{{ 0x07, 0x00 }},
	{{ 0x02, 0x00 }}
};
#elif defined WOOFER_TWEETERS
cfg_reg voice_power_up_prim[] = {
	//BOOK0 PAGE0
	{{ 0x00, 0x00 }},
	{{ 0x7f, 0x00 }},
	{{ 0x00, 0x00 }},
    //{ 0x06, 0x08 }}, //RX Rising edge
	{{ 0x0b, 0x44 }}, // enable Vsns
	{{ 0x08, 0x6e }},
	{{ 0x0c, 0x46 }}, // enable Isns, Isns: slot 6
	{{ 0x30, 0x99 }},
	{{ 0x0a, 0xf1 }},
	{{ 0x07, 0x00 }},
	{{ 0x02, 0x00 }}
};
cfg_reg voice_power_up_sec[] = {
	//BOOK0 PAGE0
	{{ 0x00, 0x00 }},
	{{ 0x7f, 0x00 }},
	{{ 0x00, 0x00 }},
    //{ 0x06, 0x08 }}, //RX Rising edge
	{{ 0x0b, 0x44 }}, // enable Vsns
	{{ 0x08, 0x6e }},
	{{ 0x0c, 0x46 }}, // enable Isns, Isns: slot 6
	{{ 0x30, 0x99 }},
	{{ 0x0a, 0xf1 }},
	{{ 0x07, 0x00 }},
	{{ 0x02, 0x00 }}
};
cfg_reg voice_power_up_tert[] = {
	//BOOK0 PAGE0
	{{ 0x00, 0x00 }},
	{{ 0x7f, 0x00 }},
	{{ 0x00, 0x00 }},
    //{ 0x06, 0x08 }}, //RX Rising edge
	{{ 0x0b, 0x44 }}, // enable Vsns
	{{ 0x08, 0x6e }},
	{{ 0x0c, 0x46 }}, // enable Isns, Isns: slot 6
	{{ 0x30, 0x99 }},
	{{ 0x0a, 0xf1 }},
	{{ 0x07, 0x00 }},
	{{ 0x02, 0x00 }}
};
#elif defined STEREO
cfg_reg voice_power_up_prim[] = {
	//BOOK0 PAGE0
	{{ 0x00, 0x00 }},
	{{ 0x7f, 0x00 }},
	{{ 0x00, 0x00 }},
    //{ 0x06, 0x08 }}, //RX Rising edge
	{{ 0x0b, 0x40 }}, // enable Vsns
	{{ 0x08, 0x6e }},
	{{ 0x0c, 0x06 }}, // enable Isns, Isns: slot 6
	{{ 0x30, 0x99 }},
	{{ 0x0a, 0xf1 }},
	{{ 0x07, 0x00 }},
	{{ 0x02, 0x00 }}
};
cfg_reg voice_power_up_sec[] = {
	//BOOK0 PAGE0
	{{ 0x00, 0x00 }},
	{{ 0x7f, 0x00 }},
	{{ 0x00, 0x00 }},
    //{ 0x06, 0x08 }}, //RX Rising edge
	{{ 0x0b, 0x44 }}, // enable Vsns
	{{ 0x08, 0x6e }},
	{{ 0x0c, 0x06 }}, // enable Isns, Isns: slot 6
	{{ 0x30, 0x99 }},
	{{ 0x0a, 0xf1 }},
	{{ 0x07, 0x00 }},
	{{ 0x02, 0x00 }}
};
#else
cfg_reg voice_power_up_prim[] = {
	//BOOK0 PAGE0
    {{ 0x00, 0x00 }},
    {{ 0x7f, 0x00 }},
    {{ 0x00, 0x00 }},
    //{ 0x06, 0x08 }}, //RX Rising edge
    {{ 0x0b, 0x40 }}, // enable Vsns
	{{ 0x08, 0x6e }},
    {{ 0x0c, 0x06 }}, // enable Isns, Isns: slot 6
	{{ 0x30, 0x99 }},
	{{ 0x0a, 0xf1 }},
	{{ 0x07, 0x00 }},
	{{ 0x02, 0x00 }}
};
#endif
#endif
#endif