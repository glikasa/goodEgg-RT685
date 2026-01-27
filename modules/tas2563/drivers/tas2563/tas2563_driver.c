#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/i2s.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/devicetree.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/tas2563_driver.h>

/* TAS2563 Specific Headers */
#include "tasdevice.h"
#include "tas2563.h"
#include "tas2563_ftc_lib.h"
#include "prm_reg/register_setting.h"
#include "OSL_wrapper.h"

#ifdef PDM_ENABLE
#include "prm_pdm_tuning_mode.h"
#include "cfg0_pdm_music_COEFF.h"
#include "cfg1_pdm_calibration_COEFF.h"
#else
#include "prm_reg/prm_tuning_mode.h"
#include "coef/cfg0_music_COEFF_prim.h"
#include "coef/cfg2_Voice_COEFF_prim.h"
#include "coef/cfg1_calibration_COEFF.h"
#ifdef STEREO
#include "cfg0_music_COEFF_sec.h"
#include "cfg2_Voice_COEFF_sec.h"
#define PIN_GPIO_66 (66)
#elif defined WOOFER_TWEETERS
#include "cfg0_music_COEFF_sec.h"
#include "cfg2_Voice_COEFF_sec.h"
#include "cfg0_music_COEFF_tert.h"
#include "cfg2_Voice_COEFF_tert.h"
#define PIN_GPIO_66 (66)
#define PIN_GPIO_86 (86)
#elif defined FOUR_PAS
#include "cfg0_music_COEFF_sec.h"
#include "cfg2_Voice_COEFF_sec.h"
#include "cfg0_music_COEFF_tert.h"
#include "cfg2_Voice_COEFF_tert.h"
#include "cfg0_music_COEFF_quat.h"
#include "cfg2_Voice_COEFF_quat.h"
#define PIN_GPIO_66 (66)
#define PIN_GPIO_86 (86)
#define PIN_GPIO_88 (88)
#endif
#endif




#define DT_DRV_COMPAT ti_tas2563
LOG_MODULE_REGISTER(tas2563);

// typedef struct {
//
//     int mProfileId;
//
//     bool bPrmOptimized;
//
// } dspfw_runtime_info;
/* --- Configuration Structures --- */

struct tas2563_config {
    struct i2c_dt_spec i2c;
    const struct device *i2s;  
    struct gpio_dt_spec int_gpio;
    struct gpio_dt_spec reset_gpio;
    int initial_book;
    int initial_page;
    dspfw_runtime_info *dsp_info;
    void *sccb_cfg; 
};

struct tas2563_data {
    const struct device *dev;           
    struct tas256x_priv lib_priv;       
    smartamp_cfg *tas_ptrs[1];          
    smartamp_cfg hw_cfg[1];             
};

/* --- I2S Parameters --- */

#define SAMPLE_FREQUENCY    44100
#define SAMPLE_BIT_WIDTH    16
#define NUMBER_OF_CHANNELS  1
#define BLOCK_SIZE          1024 
#define BLOCK_COUNT         12 
#define TIMEOUT             10000

K_MEM_SLAB_DEFINE_STATIC(mem_slab, BLOCK_SIZE, BLOCK_COUNT, 4);

/* --- Static Driver State --- */

static dspfw_runtime_info h_dspfw_runtime_info[] = {
    { .mProfileId = -1, .bPrmOptimized = false }
};

static t_libsccb_cfg h_tas256x_sccb[] = {
    { .id = 0x4c } 
};

/* --- Helper Functions --- */

static void tas2563_write_calibrated_data(struct tas256x_priv* pTAS2563, int chn)
{
    struct os_interface* pOs_intf = pTAS2563->os_intf;
    int i = 0, rc = 0;
    unsigned char DevBlock[36];
    
    rc = pOs_intf->nv_read(DevBlock, sizeof(char), 36, &chn);
    if (rc != 36) {
        LOG_ERR("Calibration read error: %d", rc);
        return;
    }

    if (DevBlock[0] != 0x0) {
        for (i = 1; i < 36; i += 7) {
            tas256x_dev_bulk_write(pTAS2563, chn,
                TAS256X_REG(DevBlock[i], DevBlock[i+1], DevBlock[i+2]),
                &DevBlock[i+3], 4);
        }
    }
}

/* --- Operational Logic (DSP & Speaker Control) --- */

void tas256x_dspon(struct tas256x_priv* pTAS256X, unsigned int profile)
{
    if (profile == BYPASS) return;

    for (int i = 0; i < pTAS256X->ndev; i++) {
        smartamp_cfg *tasdev = (smartamp_cfg *)pTAS256X->tasdevice[i];
        dspfw_runtime_info *pDspruntimeinfo = tasdev->pMnDspfw_runtime_info;

        if (pDspruntimeinfo->mProfileId == profile) continue;

        switch (profile) {
        case MUSIC:
            tas256x_transmit_registers(pTAS256X, i, cfg0_music_coeff_registers_prim,
                sizeof(cfg0_music_coeff_registers_prim) / sizeof(cfg0_music_coeff_registers_prim[0]));
            break;
        case CALIBRATION:
            tas256x_transmit_registers(pTAS256X, i, cfg1_calibration_coeff_registers,
                sizeof(cfg1_calibration_coeff_registers) / sizeof(cfg1_calibration_coeff_registers[0]));
            break;
        default:
            LOG_WRN("Profile %d not fully implemented", profile);
            break;
        }
    }
}

void tas256x_poweron(struct tas256x_priv* pTAS256X, unsigned int profile)
{
    for (int i = 0; i < pTAS256X->ndev; i++) {
        smartamp_cfg *tasdev = (smartamp_cfg *)pTAS256X->tasdevice[i];
        
	dspfw_runtime_info *pDspruntimeinfo = tasdev->pMnDspfw_runtime_info;
        switch (profile) {
        case MUSIC:
        case CALIBRATION:
            tas256x_transmit_registers(pTAS256X, i, power_up_prim,
                sizeof(power_up_prim) / sizeof(power_up_prim[0]));
            break;
        case BYPASS:
            tas256x_dev_update_bits(pTAS256X, i, TAS2563_REG_MISC_DSP,
                TAS2563_REG_MISC_DSP_ROM_MODE_MASK, TAS2563_REG_MISC_DSP_ROM_MODE_ROM_MODE);
            tas256x_transmit_registers(pTAS256X, i, bypass_power_up_prim,
                sizeof(bypass_power_up_prim) / sizeof(bypass_power_up_prim[0]));
            break;
        }
        pDspruntimeinfo->mProfileId = profile;
    }
}

// void tas256x_poweron(struct tas256x_priv* pTAS256X,
// 	unsigned int profile)
// {
// 	int i = 0;
// 	for(i = 0; i < pTAS256X->ndev; i++) {
// 		smartamp_cfg *tasdev =
// 			(smartamp_cfg *)pTAS256X->tasdevice[i];
// 		dspfw_runtime_info *pDspruntimeinfo
// 			= tasdev->pMnDspfw_runtime_info;
// 		switch (profile) {
// 		case MUSIC:
// 		case CALIBRATION:
// 			switch(i) {
// #if defined STEREO
// 			case 0:
// 				tas256x_transmit_registers(pTAS256X, i, power_up_prim,
// 					sizeof(power_up_prim) / sizeof(power_up_prim[0]));
// 				break;
// 			case 1:
// 				tas256x_transmit_registers(pTAS256X, i, power_up_sec,
// 					sizeof(power_up_sec) / sizeof(power_up_sec[0]));
// 				break;
// #elif defined WOOFER_TWEETERS
// 			case 0:
// 				tas256x_transmit_registers(pTAS256X, i, power_up_prim,
// 					sizeof(power_up_prim) / sizeof(power_up_prim[0]));
// 				break;
// 			case 1:
// 				tas256x_transmit_registers(pTAS256X, i, power_up_sec,
// 					sizeof(power_up_sec) / sizeof(power_up_sec[0]));
// 				break;
// 			case 2:
// 				tas256x_transmit_registers(pTAS256X, i, power_up_tert,
// 					sizeof(power_up_tert) / sizeof(power_up_tert[0]));
// 				break;
// #elif defined FOUR_PAS
// 			case 0:
// 				tas256x_transmit_registers(pTAS256X, i, power_up_prim,
// 					sizeof(power_up_prim) / sizeof(power_up_prim[0]));
// 				break;
// 			case 1:
// 				tas256x_transmit_registers(pTAS256X, i, power_up_sec,
// 					sizeof(power_up_sec) / sizeof(power_up_sec[0]));
// 				break;
// 			case 2:
// 				tas256x_transmit_registers(pTAS256X, i, power_up_tert,
// 					sizeof(power_up_tert) / sizeof(power_up_tert[0]));
// 				break;
// 			case 3:
// 				tas256x_transmit_registers(pTAS256X, i, power_up_quat,
// 					sizeof(power_up_quat) / sizeof(power_up_quat[0]));
// 				break;
// #else
// 			case 0:
// 				tas256x_transmit_registers(pTAS256X, i, power_up_prim,
// 					sizeof(power_up_prim) / sizeof(power_up_prim[0]));
// 				break;
// #endif
// 			default:
// 				dev_err("%s:%u: no such channel(%d)\n", __func__,
// 					__LINE__, i);
// 				break;
// 			}
// 			break;
// #ifndef PDM_ENABLE
// 		case VOICE:
// 			break;
// #endif
// #ifdef PDM_ENABLE
// 		case INDEPENDANT_CAPTURE:
// 			tas256x_transmit_registers(pTAS256X, i,
// 				standalone_record_on, sizeof(standalone_record_on) /
// 				sizeof(standalone_record_on[0]));
// 			break;
// 		case MIXTURE_CAPTURE:
// 			tas256x_transmit_registers(pTAS256X, i,
// 				mixture_playback_record,
// 				sizeof(mixture_playback_record) /
// 				sizeof(mixture_playback_record[0]));
// 			break;
// #endif
// 		case BYPASS:
// 			tas256x_dev_update_bits(pTAS256X, i,TAS2563_REG_MISC_DSP,
// 				TAS2563_REG_MISC_DSP_ROM_MODE_MASK,
// 				TAS2563_REG_MISC_DSP_ROM_MODE_ROM_MODE);
// 			switch(i) {
// 			case 0:
// 				tas256x_transmit_registers(pTAS256X, i,
// 					bypass_power_up_prim,
// 					sizeof(bypass_power_up_prim) /
// 					sizeof(bypass_power_up_prim[0]));
// 				break;
// #ifdef STEREO
// 			case 1:
// 				tas256x_transmit_registers(pTAS256X, i,
// 					bypass_power_up_sec,
// 					sizeof(bypass_power_up_sec) /
// 					sizeof(bypass_power_up_sec[0]));
// 				break;
// #endif
// 			default:
// 				dev_err("%s:%u: no such channel(%d)\n", __func__,
// 					__LINE__, i);
// 				break;
// 			}
// 			break;
// 		}
// 		pDspruntimeinfo->mProfileId = profile;
// 	}
// }
//
//

uint8_t tx_buf[2048];
int exTas256x_speakeron(const struct device *dev, uint32_t profile){


    const struct tas2563_config *cfg = dev->config;

    LOG_INF("Writing to I2C");
    if(i2c_write_dt(&cfg->i2c, tx_buf, sizeof(tx_buf)) == 0) {

        LOG_ERR("Failed to Write to I2C: %u ", cfg->i2c.addr);
    }
    return 0;






    struct tas2563_data *tasdata = dev->data;
    const struct tas2563_config *config = dev->config;
    struct tas256x_priv* pTAS256X = &tasdata->lib_priv;

    tas256x_dspon(pTAS256X, profile);
    tas256x_poweron(pTAS256X, profile);
    pTAS256X->dev_state = SMARTPA_POWERON;


    struct i2s_config i2s_conf = {
        .word_size = SAMPLE_BIT_WIDTH,
        .channels = NUMBER_OF_CHANNELS,
        .format = I2S_FMT_DATA_FORMAT_I2S,
        .options = I2S_OPT_BIT_CLK_MASTER | I2S_OPT_FRAME_CLK_MASTER,
        .frame_clk_freq = SAMPLE_FREQUENCY,
        .mem_slab = &mem_slab,
        .block_size = BLOCK_SIZE,
        .timeout = TIMEOUT,
    };

    LOG_DBG("Configuring I2S");
    if(i2s_configure(config->i2s, I2S_DIR_TX, &i2s_conf)) {

        LOG_ERR("I2S Config failed");
        return -1;
    }
    return 0;
}

int exTas256x_speakeroff(const struct device *dev){ 

    struct tas2563_data *tasdata = dev->data;
    struct tas256x_priv* pTAS256X = &tasdata->lib_priv;

    for (int chn = 0; chn < pTAS256X->ndev; chn++) {
        smartamp_cfg *tasdev = (smartamp_cfg *)pTAS256X->tasdevice[chn];
        dspfw_runtime_info *pDspruntimeinfo = tasdev->pMnDspfw_runtime_info;
        tas256x_transmit_registers(pTAS256X, chn, shut_down,
            sizeof(shut_down) / sizeof(shut_down[0]));

        if(pDspruntimeinfo->mProfileId == BYPASS) {

            tas256x_dev_update_bits(pTAS256X, chn, TAS2563_REG_MISC_DSP,
                TAS2563_REG_MISC_DSP_ROM_MODE_MASK, TAS2563_REG_MISC_DSP_ROM_MODE_RAM_MODE);
        }
    }
    pTAS256X->dev_state = SMARTPA_POWEROFF;
    return 0;
}

int stream_audio_samples(const struct device *dev, const int16_t *samples, uint32_t num_samples){

    const struct tas2563_config *cfg = dev->config;
    size_t bytes_left = num_samples * sizeof(int16_t);
    const uint8_t *data_ptr = (const uint8_t *)samples;
    int ret;
    bool started = false;
    int queued_buffers = 0;

    LOG_INF("Starting: %u samples, %u bytes", num_samples, bytes_left);

    if(!device_is_ready(cfg->i2s)) {
        LOG_ERR("I2S device not ready!");
        return -ENODEV;
    }

    while(bytes_left > 0) {

        void *mem_block;
        //LOG_INF("Alloc attempt %d, bytes_left=%u", queued_buffers + 1, bytes_left);

        ret = k_mem_slab_alloc(&mem_slab, &mem_block, K_MSEC(1000));
        if(ret < 0) {

            LOG_ERR("Slab alloc failed after %d buffers: %d", queued_buffers, ret);
            goto err_stop;
        }
        size_t chunk_size = MIN(bytes_left, BLOCK_SIZE);
        
        if(chunk_size < BLOCK_SIZE) {

            memset(mem_block, 0, BLOCK_SIZE);
        }
        memcpy(mem_block, data_ptr, chunk_size);

        ret = i2s_write(cfg->i2s, mem_block, BLOCK_SIZE);
        if(ret < 0) {

            k_mem_slab_free(&mem_slab, mem_block);
            LOG_ERR("I2S Write failed: %d", ret);
            goto err_stop;
        }
        //LOG_INF("i2s_write success");

        queued_buffers++;
        data_ptr += chunk_size;
        bytes_left -= chunk_size;

        if(!started && queued_buffers >= 2) {

            LOG_INF("Calling I2S_TRIGGER_START...");
            ret = i2s_trigger(cfg->i2s, I2S_DIR_TX, I2S_TRIGGER_START);
            if(ret < 0) {

                LOG_ERR("I2S Start failed: %d", ret);
                goto err_stop;
            }
            LOG_INF("I2S Started successfully");
            started = true;
             /* === KEY TEST: Wait and check if DMA frees blocks === */
          //   LOG_INF("Waiting 500ms for DMA...");
          //   k_sleep(K_MSEC(500));
          // /* Add after I2S_TRIGGER_START */
          //   LOG_INF("Free blocks after wait: %u", k_mem_slab_num_free_get(&mem_slab));  
        }
    }

    LOG_INF("All %d buffers queued, draining...", queued_buffers);
    ret = i2s_trigger(cfg->i2s, I2S_DIR_TX, I2S_TRIGGER_DRAIN);
    if(ret < 0) {

        LOG_ERR("I2S Drain failed: %d", ret);
        return ret;
    }
    LOG_INF("Stream complete");
    return 0;

err_stop:
    k_msleep(2000);
    if(started) {
        i2s_trigger(cfg->i2s, I2S_DIR_TX, I2S_TRIGGER_DROP);
    }
    return ret;
}

int tas2563_lib_init(const struct device *dev){


    return 0;


    const struct tas2563_config *cfg = dev->config;
    struct tas2563_data *data = dev->data;
    struct tas256x_priv *p = &data->lib_priv;
    unsigned char ndev = 1;

    data->dev = dev;
    p->ndev = ndev; 
    p->tasdevice = (void **)data->tas_ptrs;

    for (int i = 0; i < ndev; i++) {
        data->hw_cfg[i].mnCurrentBook = cfg->initial_book;
        data->hw_cfg[i].mnCurrentPage = cfg->initial_page;
        data->hw_cfg[i].pMnDspfw_runtime_info = cfg->dsp_info;
        data->hw_cfg[i].pSccb_cfg = cfg->sccb_cfg;
        p->tasdevice[i] = &data->hw_cfg[i];
    }

    p->os_intf = tas256x_get_os_intf();

    for (int i = 0; i < ndev; i++) {
        tas256x_transmit_registers(p, i, prm_tunning_mode_registers,
            sizeof(prm_tunning_mode_registers) / sizeof(prm_tunning_mode_registers[0]));
        tas2563_write_calibrated_data(p, i);
    }
    return 0;
}

static int tas2563_init(const struct device *dev){

    const struct tas2563_config *config = dev->config;
    int ret;

    if (!device_is_ready(config->i2c.bus)) {
        LOG_ERR("I2C not ready");
        return -ENODEV;
    }

    if (!device_is_ready(config->i2s)) {
        LOG_ERR("I2S not ready");
        return -ENODEV;
    }

    // struct i2s_config i2s_conf = {
    //     .word_size = SAMPLE_BIT_WIDTH,
    //     .channels = NUMBER_OF_CHANNELS,
    //     .format = I2S_FMT_DATA_FORMAT_I2S,
    //     .options = I2S_OPT_BIT_CLK_MASTER | I2S_OPT_FRAME_CLK_MASTER,
    //     .frame_clk_freq = SAMPLE_FREQUENCY,
    //     .mem_slab = &mem_slab,
    //     .block_size = BLOCK_SIZE,
    //     .timeout = TIMEOUT,
    // };
    //
    // ret = i2s_configure(config->i2s, I2S_DIR_TX, &i2s_conf);
    // if (ret < 0) {
    //     LOG_ERR("I2S Config failed: %d", ret);
    //     return ret;
    // }
    // uint32_t rate = 0;
    // const struct i2s_mcux_config *i2s_cfg = config->i2s->config;
    // clock_control_get_rate(i2s_cfg->clock_dev, i2s_cfg->clock_subsys, &rate);
    // LOG_INF("Flexcomm7 base clock: %u Hz", rate);
    return tas2563_lib_init(dev);
}

/* --- OS Interface Implementation --- */

int libsccb_rd(void* dev_handle, int reg)
{
    smartamp_cfg *tasdev = (smartamp_cfg *)dev_handle;
    struct tas2563_data *data = CONTAINER_OF(tasdev, struct tas2563_data, hw_cfg[0]);
    const struct tas2563_config *cfg = data->dev->config;
    uint8_t reg_addr = (uint8_t)reg;
    uint8_t val;

    if (i2c_write_read_dt(&cfg->i2c, &reg_addr, 1, &val, 1) == 0) {
        return (int)val;
    }
    return -EIO;
}

int libsccb_wr(void* dev_handle, int reg, unsigned int Value)
{
    smartamp_cfg *tasdev = (smartamp_cfg *)dev_handle;
    struct tas2563_data *data = CONTAINER_OF(tasdev, struct tas2563_data, hw_cfg[0]);
    const struct tas2563_config *cfg = data->dev->config;
    uint8_t tx_buf[2] = {(uint8_t)reg, (uint8_t)Value};

    return (i2c_write_dt(&cfg->i2c, tx_buf, sizeof(tx_buf)) == 0) ? 0 : -EIO;
}

int libsccb_seqwrite(void* dev_handle, int reg, int len, unsigned char* pData)
{
    smartamp_cfg *tasdev = (smartamp_cfg *)dev_handle;
    struct tas2563_data *data = CONTAINER_OF(tasdev, struct tas2563_data, hw_cfg[0]);
    const struct tas2563_config *cfg = data->dev->config;

    /* Using a stack buffer for small writes; for very large FW, dynamic allocation might be needed */
    uint8_t tx_buf[len + 1];
    tx_buf[0] = (uint8_t)reg;
    memcpy(&tx_buf[1], pData, len);

    return (i2c_write_dt(&cfg->i2c, tx_buf, len + 1) == 0) ? 0 : -EIO;
}

int libsccb_seqread(void* dev_handle, int reg, int count, unsigned char* buf)
{
    smartamp_cfg *tasdev = (smartamp_cfg *)dev_handle;
    struct tas2563_data *data = CONTAINER_OF(tasdev, struct tas2563_data, hw_cfg[0]);
    const struct tas2563_config *cfg = data->dev->config;
    uint8_t reg_addr = (uint8_t)reg;

    return (i2c_write_read_dt(&cfg->i2c, &reg_addr, 1, buf, count) == 0) ? 0 : -EIO;
}

void ertos_timedelay(unsigned int msecs) { k_msleep(msecs); }
void gpio_config() { /* Handled via DeviceTree GPIO specs */ }
size_t nv_write(const void *ptr, size_t size, size_t nmemb, void *stream) { return nmemb; }
size_t nv_read(void *ptr, size_t size, size_t nmemb, void *stream) { return nmemb; }

const struct os_interface os_intf = {
    .dev_read = libsccb_rd,
    .dev_write = libsccb_wr,
    .dev_bulk_write = libsccb_seqwrite,
    .dev_bulk_read = libsccb_seqread,
    .GPIO_config = gpio_config,
    .msleep = ertos_timedelay,
    .nv_write = nv_write,
    .nv_read = nv_read
};

static const tas2563_api_t tas2563_api = {

	.speaker_on = exTas256x_speakeron,
	.speaker_off = exTas256x_speakeroff,
	//.get_chip_id = exTas256x_getchipid,
	.stream_audio = stream_audio_samples
};
/* --- Driver Instantiation Macro --- */

#define TAS2563_DEFINE(i)                                                \
    static struct tas2563_data tas2563_data_##i;                         \
    static const struct tas2563_config tas2563_config_##i = {            \
        .i2c = I2C_DT_SPEC_INST_GET(i),                                  \
        .i2s = DEVICE_DT_GET(DT_INST_PHANDLE(i, audio_interface)),      \
        .reset_gpio = GPIO_DT_SPEC_INST_GET_OR(i, reset_gpios, {0}),     \
        .int_gpio = GPIO_DT_SPEC_INST_GET_OR(i, int_gpios, {0}),         \
        .initial_book = -1,                                              \
        .initial_page = -1,                                              \
        .dsp_info = &h_dspfw_runtime_info[0],                            \
        .sccb_cfg = &h_tas256x_sccb[0],                                  \
    };                                                                   \
                                                                         \
    DEVICE_DT_INST_DEFINE(i,                                             \
                          tas2563_init,                                  \
                          NULL,                                          \
                          &tas2563_data_##i,                             \
                          &tas2563_config_##i,                           \
                          POST_KERNEL,                                   \
                          CONFIG_SENSOR_INIT_PRIORITY,                   \
                          &tas2563_api);

TAS2563_DEFINE(0)
//DT_INST_FOREACH_STATUS_OKAY(TAS2563_DEFINE)

