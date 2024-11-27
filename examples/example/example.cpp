///////////////////////////////////////////////////////////
//                                                       //
// Copyright (c) 2016-2021 by Eyedea Recognition, s.r.o. //
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
//             Seats analyzer library example            //
///////////////////////////////////////////////////////////

#include <cstring>
#include <iostream>
#include <chrono>

#include <SeatsAnalyzer.h>
#include <er_explink.h>
#include <er_type.h>

// Path to module(s) directory
#define LIB_FILENAME ER_LIB_PREFIX "seatsanalyzer" SA_SUFFIX "-" ER_LIB_TARGET DEBUG_SUFFIX ER_LIB_EXT
// #define LIB_FILENAME "seatsanalyzer.unsecured-x64.dll"
#define SDK_DIR     "../../sdk/"
#define SA_LIBRARY SDK_DIR "lib/" LIB_FILENAME
#define CONFIG_FILENAME  SDK_DIR "config.ini"
#define IMAGES_DIR          "../../data/images/"

const char TestImageList[][4096] = {
    IMAGES_DIR "img_1.jpg",
    IMAGES_DIR "img_2.jpg",
    IMAGES_DIR "img_3.jpg",
    IMAGES_DIR "img_4.jpg",
    IMAGES_DIR "img_5.jpg",
    IMAGES_DIR "img_6.jpg",
    IMAGES_DIR "img_7.jpg",
    IMAGES_DIR "img_8.jpg",
};
int NUM_IMG = sizeof(TestImageList)/4096;

int main(int argc, char *argv[])
{
#ifdef EXPLICIT_LINKING
    /** [Explink] */
    /* load shared library and link functions */
    SaAPI api;
    shlib_hnd hdll = nullptr;
    ER_OPEN_SHLIB(hdll, SA_LIBRARY);
    if (hdll==nullptr) {
        std::cout << "Library '" << SA_LIBRARY << "' not loaded!\n" << ER_SHLIB_LASTERROR << "\n";
        return -1;
    }
    fcn_saLinkAPI pfLinkAPI=nullptr;     /* The function which will link all other api functions */
    ER_LOAD_SHFCN(pfLinkAPI, fcn_saLinkAPI, hdll, "saLinkAPI");
    if (pfLinkAPI==nullptr) {
        std::cout << "Loading function 'saLinkAPI' from " << SA_LIBRARY << " failed!\n";
        return -1;
    }
    if ( pfLinkAPI(hdll, &api) != 0 ){
        std::cout << "Function saLinkAPI() returned with error!\n";
        return -1;
    }
    /** [Explink] */
#else
    /** [Implink] */
    // libseatsanalyzer is linked implicitly during build process all functions 
    // like saInit are visible and directly accessible. 
    // The saLinkAPI(nullptr,...) call just maps all functions to 
    // SeatsAnalyzer API's functions pointers
    SaAPI api;
    saLinkAPI(nullptr, &api);
    /** [Implink] */
#endif
    /** [Init] */
    // Initialize the library
    SaConfig config={};
    std::memset(&config,0,sizeof(SaConfig));
    /* optional values (NULL default) */
    /* detection related paths */
    // config.det_sdk_directory = "../../sdk/";
    // config.det_config_directory = "../../sdk/";
    // config.det_config_file = "config-det-plugin.ini";

    /* classification related paths */
    // config.scl_model_directory = "../../sdk/models/";
    // config.scl_model_filename = "CNN_TF2LITE_SCL_2022Q1.dat";
    // config.scl_model_p_table_filename = "CNN_TF2LITE_SCL_DATA_2022Q1.dat";

    /* mandatory values if config is used */
#ifdef SA_USE_GPU
    /**< Computation mode of the project, 0 - CPU computation, 1 - GPU */
    config.computation_mode = ERComputationMode::ER_COMPUTATION_MODE_GPU;
#else
    config.computation_mode = ERComputationMode::ER_COMPUTATION_MODE_CPU;
#endif
    config.gpu_device_id = 0;  /**< GPU device id to use for computation, only used if computation_mode == 1 */
    config.num_threads = 1;  /**< Number of threads to use */


    SAState sa_state;
    if (api.saInit(CONFIG_FILENAME, &config, &sa_state) != 0)
    {
        getchar();
        return 1;
    }
    /** [Init] */

    // Process images
    // Structures for measuring time
    long long duration_det = 0;
    unsigned int num_detector_runs = 0;
    long long duration_scl = 0;
    unsigned int num_scl_runs = 0;
    // A variable holding the bounding box of an area of detector's interest

    for (int i = 0; i < NUM_IMG; i++)
    {
        // Read the input image from a file
        ERImage er_image;

        std::cout << "Processing image " << TestImageList[i] << "..." << std::endl;
        if (api.erImageRead(&er_image, TestImageList[i]) != 0)
        {
            std::cerr << "test_module(): Can't load the file: " << TestImageList[i] << std::endl;
            getchar();

            return 1;
        }

        // Specify area where to run detection, nullptr means whole image will be used
        ERRoI* roi = nullptr;

        /** [Det] */
        // Run detection
        SaDetResult det_result;
        std::chrono::high_resolution_clock::time_point t1;
        t1 = std::chrono::high_resolution_clock::now();
        if ((api.saRunDet(sa_state, er_image, roi, &det_result)) != 0)
        {
            // Wait for ENTER
            std::cout << "Press [ENTER] to exit." << std::endl;
            getchar();
            continue;
        }
        /** [Det] */

        duration_det += std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - t1).count();
        num_detector_runs += 1;

        // Run SCL on each LP detection
        printf(" - found %d detections\n", det_result.num_detections);
        for (int j = 0; j < det_result.num_detections; j++)
        {
            SaDetection& det = det_result.detections[j];

            // Print information about the detection
            printf(" %d. detection: [%.1fx%.1f at (%.1f,%.1f)], label %s (%.2f)\n",
                j,
                det.position.width, det.position.height, det.position.x, det.position.y,
                det.label, det.confidence  );

            // Classify only a windshield detections
            if (std::strncmp((char *)det.label, "window", sizeof("window") - 1) != 0)
            {
                continue;
            }

            t1 = std::chrono::high_resolution_clock::now();

            /** [Scl] */
            // Run Seat Classification
            SaSclResult scl_result;
            if ( (api.saRunScl(sa_state, er_image, &det.position, det.label, &scl_result)) != 0)
            {
                continue;
            }
            /** [Scl] */

            duration_scl += std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - t1).count();
            num_scl_runs += 1;

            // Print the classification results
            printf("  - left:   %s (%.2f) quality %.2f driver %s (%.2f) belt %s (%.2f) phone %s (%.2f)\n",
                scl_result.left.occupied.result, scl_result.left.occupied.confidence,
                scl_result.left.quality,
                scl_result.left.driver.result, scl_result.left.driver.confidence,
                scl_result.left.belt.result, scl_result.left.belt.confidence,
                scl_result.right.phone.result, scl_result.right.phone.confidence
                );
            printf("  - middle: %s (%.2f) quality %.2f\n",
                scl_result.middle.occupied.result, scl_result.middle.occupied.confidence,
                scl_result.middle.quality
                );
            printf("  - right:  %s (%.2f) quality %.2f driver %s (%.2f) belt %s (%.2f) phone %s (%.2f)\n",
                scl_result.right.occupied.result, scl_result.right.occupied.confidence,
                scl_result.right.quality,
                scl_result.right.driver.result, scl_result.right.driver.confidence,
                scl_result.right.belt.result, scl_result.right.belt.confidence,
                scl_result.right.phone.result, scl_result.right.phone.confidence
                );

        }

        /** [DetFree] */
        // Free the detection results
        api.saFreeDetResult(sa_state, &det_result);
        /** [DetFree] */

        // Free the image data structure
        api.erImageFree(&er_image);
    }


    if (num_detector_runs > 0) {
        printf("Detector speed:\n");
        printf("%d evals, %lld ms\n", num_detector_runs, duration_det);
        printf("Speed: %f ms/eval\n", duration_det / (double)num_detector_runs);
        printf("Speed: %f Hz\n", (double)num_detector_runs / duration_det * 1000.);
    }

    if (num_scl_runs > 0) {
        printf("Classifier speed:\n");
        printf("%d evals, %lld ms\n", num_scl_runs, duration_scl);
        printf("Speed: %f ms/eval\n", duration_scl / (double)num_scl_runs);
        printf("Speed: %f Hz\n", (double)num_scl_runs / duration_scl * 1000.);
    }


    /** [Free] */
    // Free the SDK state
    api.saFree(sa_state);
    //     getchar();
    return 0;
    /** [Free] */
}
