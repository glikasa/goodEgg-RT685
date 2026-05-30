
#ifndef _TAS2563_DRIVER_H_
#define _TAS2563_DRIVER_H_

#define DEFAULT_ENV_TEMPERATURE	(20.0)
#define CALIBRATION_COUNT	(1)

#include <zephyr/device.h>

enum spk_vendor_list {
	GOERTEK,
	SW
};

typedef struct {

	int mnPGID;

	int mProfileId;

	bool bPrmOptimized;

} dspfw_runtime_info;

//int exTas256x_init(void);
//void exTas256x_deinit();
//int exTas256x_speakeron(unsigned int profile);
//void exTas256x_speakeroff();
//int exTas256x_getchipid();
//void exTas256x_calib_start(char *spk_vendor);
//void exTas256x_calib_stop(void);
//int exTas256x_get_f0(unsigned int *f0_array);

/* Custom API structure for the TAS2563 Smart Amplifier */
typedef struct {
    /**
     * @brief Power on the amplifier and switch to a specific profile.
     * Maps to exTas256x_speakeron(profile).
     */
    int (*speaker_on)(const struct device *dev, uint32_t profile);

    /**
     * @brief Power off the amplifier.
     * Maps to exTas256x_speakeroff().
     */
    int (*speaker_off)(const struct device *dev);

    /**
     * @brief Retrieve the hardware Chip ID.
     * Maps to exTas256x_getchipid().
     */
    int (*get_chip_id)(const struct device *dev);

    /**
     * @brief stream audio through i2s.
     */
    int (*stream_audio)(const struct device *dev, const int16_t *samples, uint32_t sample_len);

} tas2563_api_t;

/* 2. Define helper wrappers */
static inline int tas_speaker_on(const struct device *dev, uint32_t profile) {
    const  tas2563_api_t *api = (const tas2563_api_t *)dev->api;
    return api->speaker_on(dev, profile);
}

static inline int tas_speaker_off(const struct device *dev) {
    const  tas2563_api_t *api = (const  tas2563_api_t *)dev->api;
    return api->speaker_off(dev);
}

 static inline int tas_stream_audio(const struct device *dev, const int16_t *samples, uint32_t sample_len){

    const  tas2563_api_t *api = (const  tas2563_api_t *)dev->api;
    return api->stream_audio(dev, samples, sample_len);
 }
#endif
