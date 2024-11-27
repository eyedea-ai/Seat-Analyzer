///////////////////////////////////////////////////////////
//                                                       //
// Copyright (c) 2014-2020 by Eyedea Recognition, s.r.o. //
//                  ALL RIGHTS RESERVED.                 //
//                                                       //
// Author: Eyedea Recognition, s.r.o.                    //
//                                                       //
// Contact:                                              //
//           web: http://www.eyedea.cz                   //
//           email: info@eyedea.cz                       //
//                                                       //
// Consult your license regarding permissions and        //
// restrictions.                                         //
//                                                       //
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
//                         SA SDK                        //
//                 Seats analyzer library                //
///////////////////////////////////////////////////////////


#ifndef _SA_TYPE_H_
#define _SA_TYPE_H_

#include "er_image.h"
#include "er_type.h"

/** @cond */
#define SA_LABEL_STRING_LENGTH 255
#define SA_MAX_PATH 4096
#define NUM_CONF_OUTPUTS 3
/** @endcond */



/** \defgroup SA_TYPES SA Type definitions
 * @{ Structs and type definitions used in the package */

/** Pointer to SeatsAnalyzer module state
 * \see saInit, saFree */
typedef void *SAState;

/** Detection label
 *
 * Fixed length char array that holds information concerning the type of the detection, for example "window"
 * \see saRunScl, SaDetection */
typedef char SaDetectionLabel[SA_LABEL_STRING_LENGTH];

/** External inference callback interface
 *  The input of the callback is ERImage. The data memeber corresponds to input buffer for inferece.
 *  The output buffer is pre-allocated in SeatsAnalyzer SDK.
 */
typedef int  (*fcn_saInferenceCallback) (const ERImage*, unsigned char*);

/** Configuration structures
 *
 * By default, the SeatsAnalyzer SDK is configured by configuration files pointed by sa_config_path parameter of saInit() function.
 * The SaConfig structure serves to override this default setup.
 * \see saInit */
typedef struct
{
    // detection related paths
    const char* det_sdk_directory;    /**< Directory containg the plugins directory (optional, set NULL for default behavior) */
    const char* det_config_directory; /**< Directory containing the detection config file (optional, set NULL for default behavior) */
    const char* det_config_file;      /**< Detection config filename (optional, set NULL for default behavior) */

    // classification related paths
    const char* scl_model_directory;  /**< Directory containing classification models and p-tables (optional, set NULL for default behavior) */
    const char* scl_model_filename;    /**< Classification model filename (optional, set NULL for default behavior) */
    const char* scl_model_p_table_filename;  /**< P-table filename (optional, set NULL for default behavior) */

    // global modifications
    ERComputationMode computation_mode;  /**< Computation mode of the project */
    int gpu_device_id;  /**< GPU device id to use for computation, only used if computation_mode == 1 */
    int num_threads;  /**< Number of threads to use */

    // External inference interface
    fcn_saInferenceCallback det_inference_callback;           /**< Callback funtion for external detection network inference  (if NULL, external inferece is not used) */
    unsigned int            det_inference_output_buffer_size; /**< Byte size of output buffer from detection inference */
    fcn_saInferenceCallback scl_inference_callback;           /**< Callback funtion for external scl net inference (if NULL, external inferece is not used) */
    unsigned int            scl_inference_output_buffer_size; /**< Byte size of output buffer from scl inference */

} SaConfig;

/** Bounding-box coordinates structure
 * The 4-point bounding box area. Use of this structure is overloaded. It is used for a ROI definition,
 * the detection input parameter of saRunDet funtion (the top_left_xxx and bot_right_xxx coordinates are used),
 * and for the rotated bounding box of the detected object.
 * \see saRunDet, SaDetection */
// typedef struct
// {
//     float   top_left_col;  /**< Top left column */
//     float   top_left_row;  /**< Top left row */
//     float   top_right_col; /**< Top right column */
//     float   top_right_row; /**< Top right row */
//     float   bot_left_col;  /**< Bottom left column */
//     float   bot_left_row;  /**< Bottom left row */
//     float   bot_right_col; /**< Bottom right column */
//     float   bot_right_row; /**< Bottom right row */
// } SaBoundingBox;

/** Structure with data related to a single detected object.
 * \see SaDetResult */
typedef struct
{
    double              confidence; /**< Detection confidence factor */
    ERRotatedRect       position;   /**< Object position */
    SaDetectionLabel    label;      /**< Detection type label */
} SaDetection;

/** Array of SaDetection elements.
 * The structure holds an array of all object detections. The array is dynamically
 * allocated in saRunDet function and must be released by the saFreeDetResult function.
 * \see saRunDet */
typedef struct
{
    int             num_detections; /**< Number of detections */
    SaDetection    *detections; /**< Array of detections */
} SaDetResult;

/** Contains the result for a particular classification task.
 * Used by SaPosition for each classification task.
 * \warning result is not guaranteed to be defined - see SaSclResult for more info about which tasks are currently implemented.
 *
 * If implemented, result is one of the following: \n
 * '0' - signifies "false" \n
 * '1' - signifies "true" \n
 * '?' - signifies neither "true" or "false" can be determined with large enough confidence
 * \see SaPosition */
typedef struct
{
    char        result[SA_LABEL_STRING_LENGTH];
    double      confidence;  /**< Confidence of the result */

    double      confidences[NUM_CONF_OUTPUTS]; /**< Confidence for each possible output - False, True, Neither */
} SaClass;

/** Classification result structure per seat position in the vehicle.
 * Holds SaClass results for each classification task and the overall visual "quality"/"clarity" of the position.
 * \see SaSclResult */
typedef struct
{
    double      quality; /**< General image "quality" at the given position */
    SaClass     occupied; /**< Is the seat occupied by a passenger? */
    SaClass     driver; /**< Is the driver on this seat on this seat position? */
    SaClass     belt; /**< Does the passenger on this seatbelt have their seatbelt fastened? */
    SaClass     phone; /**< Is the passenger on this position holding a mobile phone? */
} SaPosition;

/** Classification result structure.
 * Holds SaPosition structures for left, middle and right positions.
 * \warning not all tasks are guaranteed to be defined for all positions!
 *
 * The following tasks are currently implemented and will have their SaClass.result fields set to a non-empty value: \n
 * left - occupied, driver, belt \n
 * middle - occupied \n
 * right - occupied, driver, belt, phone
 * \see saRunScl */
typedef struct
{
    SaPosition  left; /**< Left position in the vehicle from the perspective of the camera. */
    SaPosition  middle; /**< Middle position in the vehicle from the perspective of the camera. */
    SaPosition  right; /**< Right position in the vehicle from the perspective of the camera. */
} SaSclResult;
/** @} */


/** \addtogroup ExplicitLinking
 * @{ Function pointer type definitions used when using explicit linking. */
typedef const char*  (*fcn_saVersion)();
typedef int  (*fcn_saInit)(const char *, SaConfig* const, SAState *);
typedef void (*fcn_saFree)(SAState);
typedef int  (*fcn_saRunDet)(SAState, const ERImage, const ERRoI *, SaDetResult *);
typedef void (*fcn_saFreeDetResult)(SAState, SaDetResult *);
typedef int  (*fcn_saRunScl)(SAState, const ERImage, const ERRotatedRect *, const SaDetectionLabel, SaSclResult *);
/** @} */

/** \addtogroup ExplicitLinking
 * @{ Structure with all SeatsAnalyzer SDK functions. */
typedef struct
{
    shlib_hnd handle; /**< explicitly linked SDK library handle */

    /* SeatsAnalyzer SDK functions */
    fcn_saVersion                       saVersion;                        /**< saVersion */
    fcn_saInit                          saInit;                           /**< saInit */
    fcn_saFree                          saFree;                           /**< saFree */
    fcn_saRunDet                        saRunDet;                         /**< saRunDet */
    fcn_saFreeDetResult                 saFreeDetResult;                  /**< saFreeDetResult */
    fcn_saRunScl                        saRunScl;                         /**< saRunScl */
    /* ERImage functions */
    fcn_erImageGetDataTypeSize          erImageGetDataTypeSize;           /**< erImageGetDataTypeSize */
    fcn_erImageGetColorModelNumChannels erImageGetColorModelNumChannels;  /**< erImageGetColorModelNumChannels */
    fcn_erImageGetPixelDepth            erImageGetPixelDepth;             /**< erImageGetPixelDepth */
    fcn_erImageAllocateBlank            erImageAllocateBlank;             /**< erImageAllocateBlank */
    fcn_erImageAllocate                 erImageAllocate;                  /**< erImageAllocate */
    fcn_erImageAllocateAndWrap          erImageAllocateAndWrap;           /**< erImageAllocateAndWrap */
    fcn_erImageCopy                     erImageCopy;                      /**< erImageCopy */
    fcn_erImageRead                     erImageRead;                      /**< erImageRead */
    fcn_erImageWrite                    erImageWrite;                     /**< erImageWrite */
    fcn_erImageFree                     erImageFree;                      /**< erImageFree */
} SaAPI;
/** @} */

/** \addtogroup ExplicitLinking
 * @{ */
typedef int  (*fcn_saLinkAPI)(shlib_hnd, SaAPI *); /**< The only function needed to be loaded from the library. */
/** @} */

#endif
