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


#ifndef _SA_H_
#define _SA_H_

#include "SeatsAnalyzerType.h"

#if defined(CPP) || defined(__cplusplus) || defined(c_plusplus)
extern "C" 
{
#endif


/** \defgroup SA_API SA API functions
 * @{ SA API functions */

/** Explicit linking of all SeatsAnalyzer SDK API functions into SaAPI structure. */
ER_FUNCTION_PREFIX int saLinkAPI(shlib_hnd handle, SaAPI *api);

 /** Returns SeatsAnalyzer version string */
ER_FUNCTION_PREFIX const char* saVersion();

/** Initializes the library and sets up \p sa_state to point to the library instance.
 * Can be initialized using a configuration file or a SaConfig structure.
 * The SaConfig structure can be used to overried values defined in configuration files.
 *
 * \param[in] sa_config_path path to SeatsAnalyzer configuration file
 * \param[in] sa_config SaConfig configuration structure, set NULL for default configuration using configuration file (sa_config_path)
 * \param[out] sa_state Initialized SeatsAnalyzer state
 * \return Returns a non-zero error code if initialization failed.
 * \snippet example.cpp Init */
ER_FUNCTION_PREFIX int saInit(const char *sa_config_path, const SaConfig* sa_config,  SAState *sa_state);

/** Frees SeatsAnalyzer state.
 * \param[in] sa_state Initialized SeatsAnalyzer state
 * \snippet example.cpp Free */
ER_FUNCTION_PREFIX void saFree(SAState sa_state);

/** Runs windshield detections and sets the provided SaDetResult.
 * Has to be freed using saFreeDetResult.
 * \param[in] sa_state Initialized SeatsAnalyzer state
 * \param[in] image Input image
 * \param[in] bounding_box Region of Interest for detection, set NULL if not used
 * \param[out] result Detection result
 * \return Returns zero on success or error code otherwise.
 * \snippet example.cpp Det */
ER_FUNCTION_PREFIX int saRunDet(SAState sa_state, const ERImage image, const ERRoI *bounding_box, SaDetResult *result);

/** Frees SaDetResult.
 * \param[in] sa_state SeatsAnalyzer state which was used for obtaining detection_result.
 * \param[in] detection_result SaDetResult structure to be freed.
 * \snippet example.cpp DetFree */
ER_FUNCTION_PREFIX void saFreeDetResult(SAState sa_state, SaDetResult *detection_result);

/** Runs seats classification for the input SaBoundingBox crop and returns SaSclResult.
 * \warning Only SaDetectionLabel with value "window" is currently supported, other values will return an error.
 *
 * \param[in] sa_state Initialized SeatsAnalyzer state
 * \param[in] image Input image
 * \param[in] position Detection position, result of saRunDet(), \see SaDetResult
 * \param[in] detection_label Detection label, \see SaDetResult
 * \param[out] result SaSclResult structre with setats classifiction corresponding to windshield detection given by bounding_box position
 * \return Returns zero on success or error code otherwise.
 * \snippet example.cpp Scl */
ER_FUNCTION_PREFIX int saRunScl(SAState sa_state, const ERImage image, const ERRotatedRect *position, const SaDetectionLabel detection_label, SaSclResult *result);
/** @} */

#if defined(CPP) || defined(__cplusplus) || defined(c_plusplus)
}
#endif
#endif
