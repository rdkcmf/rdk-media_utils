/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2016 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/
/***************************************************************************
*  
*  Rmf Audio Capture ->  RAC -> rac
*/


/**
 * @defgroup MEDIA_UTILS Media Utils
 * - Audio capture manager presents the audio data to registered applications.
 *
 * @defgroup MEDIA_UTILS_TYPES Media Utils Data types
 * @ingroup  MEDIA_UTILS
 *
 * @defgroup MEDIA_UTILS_HAL_APIS  Media Utils hal APIs
 * @ingroup  MEDIA_UTILS
 *
 **/
 

#ifndef __RMF_AUDIO_CAPTURE_H__
#define __RMF_AUDIO_CAPTURE_H__

#include <stddef.h>  /* size_t */

/* tbd, Can we use RMF_errors instead of having private errors? */
#include "rmf_error.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup MEDIA_UTILS_TYPES
 * @{
 */

typedef struct RMF_AudioCapture_Struct *RMF_AudioCaptureHandle;

typedef char *RMF_AudioCaptureType;

/* tbd, Can we use RMF_errors instead of having private errors? */
#define RMF_ERROR               (1)
#define RMF_INVALID_PARM        (2)


#define RMF_AC_TYPE_PRIMARY     "primary"
#define RMF_AC_TYPE_AUXILIARY   "auxiliary"

typedef enum RMF_AudioCapture_Format {
    racFormat_e16BitStereo,    /**< Stereo, 16 bits per sample interleaved into a 32-bit word. */
    racFormat_e24BitStereo,    /**< Stereo, 24 bits per sample.  The data is aligned to 32-bits,
                                             left-justified.  Left and right channels will interleave
                                             one sample per 32-bit word.  */
    racFormat_e16BitMonoLeft,  /**< Mono, 16 bits per sample interleaved into a 32-bit word. Left channel samples only. */
    racFormat_e16BitMonoRight, /**< Mono, 16 bits per sample interleaved into a 32-bit word. Right channel samples only. */
    racFormat_e16BitMono,      /**< Mono, 16 bits per sample interleaved into a 32-bit word. Left and Right channels mixed. */
    racFormat_e24Bit5_1,       /**< 5.1 Multichannel, 24 bits per sample.  The data is aligned to 32-bits,
                                             left-justified.  Channels will interleave
                                             one sample per 32-bit word, ordered L,R,Ls,Rs,C,LFE.  */
    racFormat_eMax
} racFormat;


typedef enum RMF_AudioCapture_Freq {
    racFreq_e16000,         /**< 16KHz    */
    racFreq_e22050,         /**< 22.05KHz    */
    racFreq_e24000,         /**< 24KHz    */
    racFreq_e32000,         /**< 32KHz    */
    racFreq_e44100,         /**< 44.1KHz  */
    racFreq_e48000,         /**< 48KHz    */
    racFreq_eMax
} racFreq;



/*
 * Callbacks
 */
/* Callback which will be called when captured Audio Buffer is ready to be processed */
typedef rmf_Error (*RMF_AudioCaptureBufferReadyCb)(void *cbBufferReadyParm, void *AudioCaptureBuffer, unsigned int AudioCaptureBufferSize);

/* Callback which will be called when there is a change in the AudioCapture modules status, application can call Get Status */
typedef rmf_Error (*RMF_AudioCapture_StatusChangeCb)(void *cbStatusParm);


typedef struct RMF_AudioCapture_Settings {
    RMF_AudioCaptureBufferReadyCb cbBufferReady;    /**< Callback to send audio data to application.  Must be set during Open call, no other way to get audio data to application */
    void *cbBufferReadyParm;                        /**< Application context data passed back in Buffer Ready callback, can be NULL */

    RMF_AudioCapture_StatusChangeCb cbStatusChange;  /**< If cbStatusChange is NOT NULL, then the library should trigger the callback when there is a status change is AudioCapture
                                                        Examples: OnStart of AC, Change from Start to Stop of AC. On underflow/overflow */
    void *cbStatusParm;                             /**< Application context data passed back in Status Change callback */

    size_t fifoSize;                                /**< FIFO size in bytes.  This value is a total FIFO size to hold all channels of data
                                                         If not set, a default size will be used.  Needs to be big enough to avoid overflow (expected service time * byte rate)*/
    size_t threshold;                               /**< FIFO data callback threshold in bytes.  When the amount of data in the FIFO reaches this level, the dataCallback will be invoked */
    racFormat format;                               /**< Captured data format.  Default is 16bit stereo.
                                                        This value is ignored for compressed data, and
                                                        can not be changed while connected to any inputs. */
    racFreq samplingFreq;                           /**< Captured data sampling freq  tbd, not currently supported..*/

    unsigned int    delayCompensation_ms;           /**< Delay compensation in milli seconds */
} RMF_AudioCapture_Settings;


typedef struct RMF_AudioCapture_Status {
    int             started;                /**< is the capture started */ //TODO: C interface, bool not define, typedef later
    racFormat       format;                 /**< Current capture format (Bit depth & Channel) */
    racFreq         samplingFreq;           /**< Current capture sample rate */
    size_t          fifoDepth;              /**< number of bytes in local fifo */
    unsigned int    overflows;              /**< count of overflows */
    unsigned int    underflows;             /**< count of overflows */
    int             muted;                  /**< is the capture muted */ //TODO: C interface, bool not define, typedef later
    float           volume;                 /**< current capture volume */
    /* tbd ... need to determine what is useful or even what is possible.*/
} RMF_AudioCapture_Status;

/** @} */  //END OF GROUP MEDIA_UTILS_TYPES


/**
 * @addtogroup MEDIA_UTILS_HAL_APIS
 * @{
 */

/*
 * Interfaces
 */

/**
 * @brief This API creates audio capture session, create all required resources to provide audio capture handle.
 *
 * @param[in] handle RMF audio capture instance handle.
 *
 * @return  Returns a opaque AudioCaptureHandle, which has to be passed as an argument for all subsequent AudioCapture calls
 * @return  Returns RMF error code.
 */
rmf_Error RMF_AudioCapture_Open (RMF_AudioCaptureHandle* handle);

/**
 * @brief This API creates audio capture session, create all required resources to provide audio capture handle.
 *
 * @param[in] handle    RMF audio capture instance handle.
 * @param[in] rmfAcType RMF audio capture instance handle.
 *
 * @return  Returns a opaque AudioCaptureHandle, which has to be passed as an argument for all subsequent AudioCapture calls
 * @return  Returns RMF error code.
 */
rmf_Error RMF_AudioCapture_Open_Type (RMF_AudioCaptureHandle* handle, RMF_AudioCaptureType rmfAcType);

/**
 * @brief This API is used to enquire the current status of Audio Capture associated with the current Audio Capture context.
 *
 * @param[in] handle Audio capture handle. 
 * @param[in] status Structure which has audio capture status informations.
 *
 * @return  Returns RMF error code.
 */
rmf_Error RMF_AudioCapture_GetStatus (RMF_AudioCaptureHandle handle, RMF_AudioCapture_Status* status);

/**
 * @brief This API Will return default RMF audio capture settings.
 *
 * Once audio capture gets started this API will continue to return the default capture settings.
 *
 * @param[in] settings Structure which holds the audio properties.
 *
 * @return  Returns RMF error code.
 */
rmf_Error RMF_AudioCapture_GetDefaultSettings (RMF_AudioCapture_Settings* settings);

/**
 * @brief This API will return the audio capture settings which has been successfully set for that context.
 *
 * Return the settings currently being used to capture data as part of the specific Audio capture context.
 *
 * @param[in] handle Audio capture handle.
 * @param[in] settings Structure which holds the audio properties.
 *
 * @return  Returns RMF error code.
 */
rmf_Error RMF_AudioCapture_GetCurrentSettings (RMF_AudioCaptureHandle handle, RMF_AudioCapture_Settings* settings);

/**
 * @brief This API will start the Audio capture.
 *
 * If audio capture settings are retained then this API will reconfigure with the provided capture settings to start audio capture. 
 * If audio capture settings are not retained then audio capture will start with the default capture settings.
 *
 * @param[in] handle audio capture handle.
 * @param[in] settings Structure which holds the audio settings.
 *
 * @return  Returns RMF error code.
*/
rmf_Error RMF_AudioCapture_Start (RMF_AudioCaptureHandle handle, RMF_AudioCapture_Settings* settings);

/**
 * @brief This API will stop the audio capture.
 *
 * Start can be called again after a Stop, as long as Close has not been called.
 *
 * @param[in] handle audio capture handle.
 *
 * @return  Returns RMF error code.
 */
rmf_Error RMF_AudioCapture_Stop (RMF_AudioCaptureHandle handle);

/**
 * @brief This API will free all resources associated with this audio capture handle. It will close audio capture session.
 *
 * @param[in] handle audio capture handle.
 *
 * @return  Returns RMF error code.
 */
rmf_Error RMF_AudioCapture_Close (RMF_AudioCaptureHandle handle);


/** @} */  //END OF GROUP MEDIA_UTILS_HAL_APIS


#ifdef __cplusplus
}
#endif
#endif /* __RMF_AUDIO_CAPTURE_H__ */
