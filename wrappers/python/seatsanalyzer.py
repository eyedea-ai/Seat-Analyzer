import weakref
from cffi import FFI
from typing import Optional
from er import ERRoI, ERRotatedRect

global_weakkeydict = weakref.WeakKeyDictionary()


class SaError(Exception):
    """Seatsanalyzer Error class."""

    def __init__(self, internal_function_name, internal_error_code):
        self.internal_function_name = internal_function_name
        self.internal_error_code = internal_error_code


class SaConfig:
    """Mirror of SaConfig structure."""

    def __init__(self):
        # leaving paths at None will use paths provided in the configuration file
        self.det_sdk_directory = None
        self.det_config_directory = None
        self.det_config_file = None

        self.scl_model_directory = None
        self.scl_model_filename = None
        self.scl_model_p_table_filename = None

        self.computation_mode = 0
        self.gpu_device_id = 0
        self.num_threads = 0

    def get_c(self, ffi: FFI):
        """
        Converts this Python structure into a C structure.
        :param ffi: The FFI to use for creation of the C structure.
        :return: The resulting C structure
        """
        c_structure = ffi.new("SaConfig *")

        # for all provided paths, create corresponding C string
        det_sdk_directory = ffi.new("const char []", self.det_sdk_directory.encode("utf-8")) \
            if self.det_sdk_directory is not None else ffi.NULL
        det_config_directory = ffi.new("const char []", self.det_config_directory.encode("utf-8")) \
            if self.det_config_directory is not None else ffi.NULL
        det_config_file = ffi.new("const char []", self.det_config_file.encode("utf-8")) \
            if self.det_config_file is not None else ffi.NULL

        scl_model_directory = ffi.new("const char []", self.scl_model_directory.encode("utf-8")) \
            if self.scl_model_directory is not None else ffi.NULL
        scl_model_filename = ffi.new("const char []", self.scl_model_filename.encode("utf-8")) \
            if self.scl_model_filename is not None else ffi.NULL
        scl_model_p_table_filename = ffi.new("const char []", self.scl_model_p_table_filename.encode("utf-8")) \
            if self.scl_model_p_table_filename is not None else ffi.NULL

        # store the created sub-fields in a dict to avoid GC
        global_weakkeydict[c_structure] = (
            det_sdk_directory,
            det_config_directory,
            det_config_file,

            scl_model_directory,
            scl_model_filename,
            scl_model_p_table_filename,
        )

        # paths
        c_structure.det_sdk_directory = det_sdk_directory
        c_structure.det_config_directory = det_config_directory
        c_structure.det_config_file = det_config_file

        c_structure.scl_model_directory = scl_model_directory
        c_structure.scl_model_filename = scl_model_filename
        c_structure.scl_model_p_table_filename = scl_model_p_table_filename

        # computation mode etc
        c_structure.computation_mode = ffi.cast("ERComputationMode", self.computation_mode)
        c_structure.gpu_device_id = ffi.cast("int", self.gpu_device_id)
        c_structure.num_threads = ffi.cast("int", self.num_threads)

        # external inference - not supported in python wrapper, always set to default
        c_structure.det_inference_callback = ffi.NULL
        c_structure.det_inference_output_buffer_size = 0
        c_structure.scl_inference_callback = ffi.NULL
        c_structure.scl_inference_output_buffer_size = 0

        return c_structure


class SaDetection:
    """Mirror of SaDetection structure."""

    def __init__(self):
        self.confidence = 0
        self.position = ERRotatedRect()
        self.label = ""

    def c_init(self, ffi: FFI, c_structure):
        """
        Fills this mirror structure with given C structure data.
        :param ffi: Instance of the FFI class.
        :param c_structure: C structure data.
        """
        if c_structure == ffi.NULL:
            return

        self.confidence = c_structure.confidence
        self.position.c_init(ffi, c_structure.position)
        self.label = ffi.string(c_structure.label).decode("utf-8")


class SaDetResult:
    """Mirror of SaDetResult structure."""

    def __init__(self):
        self.num_detections = 0
        self.detections = []

    def c_init(self, ffi: FFI, c_structure):
        """
        Fills this mirror structure with given C structure data.
        :param ffi: Instance of the FFI class.
        :param c_structure: C structure data.
        """
        if c_structure == ffi.NULL:
            return

        self.num_detections = c_structure.num_detections
        self.detections = []
        for i in range(c_structure.num_detections):
            c_detection = ffi.new("SaDetection *")
            ffi.memmove(c_detection, c_structure.detections + i, ffi.sizeof("SaDetection"))
            detection = SaDetection()
            detection.c_init(ffi, c_detection)
            self.detections.append(detection)


class SaClass:
    """Mirror of SaClass structure."""

    def __init__(self):
        self.result = ""
        self.confidence = 0.0

        self.confidences = []

    def c_init(self, ffi: FFI, c_structure):
        """
        Fills this mirror structure with given C structure data.
        :param ffi: Instance of the FFI class.
        :param c_structure: C structure data.
        """
        if c_structure == ffi.NULL:
            return

        if c_structure.result != ffi.NULL:
            self.result = ffi.string(c_structure.result).decode("utf-8")
        self.confidence = c_structure.confidence

        for conf in c_structure.confidences:
            self.confidences.append(conf)

    def __repr__(self):
        return "\"{}\" ({:.2f})".format(self.result, self.confidence)


class SaPosition:
    """Mirror of SaPosition structure."""

    def __init__(self):
        self.quality = 0.0
        self.occupied = None
        self.driver = None
        self.belt = None
        self.phone = None

    def c_init(self, ffi: FFI, c_structure):
        """
        Fills this mirror structure with given C structure data.
        :param ffi: Instance of the FFI class.
        :param c_structure: C structure data.
        """
        if c_structure == ffi.NULL:
            return

        self.quality = c_structure.quality
        self.occupied = SaClass()
        self.occupied.c_init(ffi, c_structure.occupied)
        self.driver = SaClass()
        self.driver.c_init(ffi, c_structure.driver)
        self.belt = SaClass()
        self.belt.c_init(ffi, c_structure.belt)
        self.phone = SaClass()
        self.phone.c_init(ffi, c_structure.phone)

    def __repr__(self):
        return "quality {:.2f}, occupied {}, driver {}, belt {}, phone {}".format(
            self.quality, self.occupied, self.driver, self.belt, self.phone)


class SaSclResult:
    """Mirror of SaSclResult structure."""

    def __init__(self):
        self.left = None
        self.middle = None
        self.right = None

    def c_init(self, ffi: FFI, c_structure):
        """
        Fills this mirror structure with given C structure data.
        :param ffi: Instance of the FFI class.
        :param c_structure: C structure data.
        """
        if c_structure == ffi.NULL:
            return

        self.left = SaPosition()
        self.left.c_init(ffi, c_structure.left)
        self.middle = SaPosition()
        self.middle.c_init(ffi, c_structure.middle)
        self.right = SaPosition()
        self.right.c_init(ffi, c_structure.right)


class Seatsanalyzer:
    """
    Python wrapper class for Seatsanalyzer
    """

    @staticmethod
    def __define_ffi(ffi):
        # Type definitions from sa_type.h
        ffi.cdef("""
                #define SA_LABEL_STRING_LENGTH 255
                #define SA_MAX_PATH 4096
                #define NUM_CONF_OUTPUTS 3
        """)
        ffi.cdef("""
                typedef void *SAState;
        """)
        ffi.cdef("""
                typedef char SaDetectionLabel[SA_LABEL_STRING_LENGTH];
        """)
        ffi.cdef("""
                typedef int  (*fcn_saInferenceCallback) (const ERImage*, unsigned char*);
        """)
        ffi.cdef("""
                typedef struct
                {
                    // detection related paths
                    const char* det_sdk_directory;    /**< Directory containg the plugins directory */
                    const char* det_config_directory; /**< Directory containing the detection config file */
                    const char* det_config_file;      /**< Detection config filename */

                    // classification related paths
                    const char* scl_model_directory;  /**< Directory containing classification models and p-tables*/
                    const char* scl_model_filename;    /**< Classification model filename */
                    const char* scl_model_p_table_filename;  /**< P-table filename */

                    // optional values
                    ERComputationMode computation_mode;  /**< Computation mode of the project, 0 - CPU computation, 1 - GPU */
                    int gpu_device_id;  /**< GPU device id to use for computation, only used if computation_mode == 1 */
                    int num_threads;  /**< Number of threads to use */
                    
                    // External inference interface
                    fcn_saInferenceCallback det_inference_callback;           /**< Callback funtion for external detection network inference  (if NULL, external inferece is not used) */
                    unsigned int            det_inference_output_buffer_size; /**< Byte size of output buffer from detection inference */
                    fcn_saInferenceCallback scl_inference_callback;           /**< Callback funtion for external scl net inference (if NULL, external inferece is not used) */
                    unsigned int            scl_inference_output_buffer_size; /**< Byte size of output buffer from scl inference */

                } SaConfig;
        """)
        ffi.cdef("""
                typedef struct
                {
                    /*! License plate detection confidence factor */
                    double              confidence;
                    /*! License plate position */
                    ERRotatedRect       position;
                    /*! Detection type label */
                    SaDetectionLabel    label;
                } SaDetection;
        """)
        ffi.cdef("""
                typedef struct
                {
                    /*! Number of detections */
                    int             num_detections; 
                    /*! Array of detections */
                    SaDetection    *detections;
                } SaDetResult;
        """)
        ffi.cdef("""
                typedef struct
                {
                    char        result[SA_LABEL_STRING_LENGTH];
                    double      confidence;
                    
                    double      confidences[NUM_CONF_OUTPUTS];
                } SaClass;
        """)
        ffi.cdef("""
                typedef struct
                {
                    double      quality;
                    SaClass     occupied;
                    SaClass     driver;
                    SaClass     belt;
                    SaClass     phone;
                } SaPosition;
        """)
        ffi.cdef("""
                typedef struct
                {
                    SaPosition  left;
                    SaPosition  middle;
                    SaPosition  right;
                } SaSclResult;
        """)

        # Function definitions from sa.h
        ffi.cdef("""
                int saInit(const char *sa_config_path, const SaConfig* sa_config,  SAState *sa_state);
        """)
        ffi.cdef("""
                void saFree(SAState sa_state);
        """)
        ffi.cdef("""
                int saRunDet(SAState sa_state, const ERImage image, const ERRoI *bounding_box, SaDetResult *result);
        """)
        ffi.cdef("""
                void saFreeDetResult(SAState sa_state, SaDetResult *detection_result);
        """)
        ffi.cdef("""
                int saRunScl(SAState sa_state, const ERImage image, const ERRotatedRect *position, const SaDetectionLabel detection_label, SaSclResult *result);
        """)

    def __init__(self, ffi: FFI, sdk_lib_path: str, support_libs: list = None) -> None:
        self.sdk_lib_path = sdk_lib_path
        if type(self.sdk_lib_path) is not str:
            raise TypeError("Variable sdk_dir_path must be a string.")

        # Add function definitions to ffi
        self.__define_ffi(ffi)
        self.ffi = ffi

        # Load all the support libs
        if support_libs is not None:
            for lib in support_libs:
                self.ffi.dlopen(lib)

        # Load the main library
        self.__sa = self.ffi.dlopen(self.sdk_lib_path)

        self.__sa_state = self.ffi.new("SAState *", self.ffi.NULL)

    def _free_sa(self, _):
        self.__sa.saFree(self.__sa_state[0])

    def init(self, sa_config_path: str, sa_config: Optional[SaConfig]):
        """
        :param sa_config_path: Path to configuration file
        :param sa_config: Optional configuration structure. If not provided only configuration file is used.
        :return:
        """
        c_sa_config_path = self.ffi.new("const char []", sa_config_path.encode("utf-8"))

        if sa_config is None:
            c_sa_config = self.ffi.NULL
        else:
            c_sa_config = sa_config.get_c(self.ffi)

        if self.__sa_state[0] != self.ffi.NULL:
            print("saInit: Already initialized, skipping...")
            return

        ret_code = self.__sa.saInit(c_sa_config_path, c_sa_config, self.__sa_state)

        if ret_code != 0:
            raise SaError("saInit", ret_code)

        self.__sa_state = self.ffi.gc(self.__sa_state, self._free_sa)

    def run_det(self, image, roi: ERRoI = None) -> SaDetResult:
        # Unwrap the input parameters
        c_image = image[0]
        if roi is not None:
            c_bounding_box = roi.get_c(self.ffi)
        else:
            c_bounding_box = self.ffi.NULL

        # Create det result pointer
        c_det_result = self.ffi.new("SaDetResult *")

        # Call the C function
        det_return_value = self.__sa.saRunDet(self.__sa_state[0], c_image, c_bounding_box, c_det_result)

        # Check the output
        if det_return_value != 0:
            raise SaError("SaRunDet", 1)

        # Wrap the result
        detection_result = SaDetResult()
        detection_result.c_init(self.ffi, c_det_result)

        # Free the result
        self.__sa.saFreeDetResult(self.__sa_state[0], c_det_result)

        return detection_result

    def run_scl(self, image, bounding_box: ERRotatedRect = None, detection_label: str = "") -> SaSclResult:
        # Unwrap the input parameters
        c_image = image[0]
        if bounding_box is not None:
            c_bounding_box = bounding_box.get_c(self.ffi)
        else:
            c_bounding_box = self.ffi.NULL
        c_detection_label = self.ffi.new("const char []", detection_label.encode("utf-8"))

        # Create scl result pointer
        c_scl_result = self.ffi.new("SaSclResult *")

        # Call the C function
        scl_return_value = self.__sa.saRunScl(self.__sa_state[0], c_image, c_bounding_box, c_detection_label,
                                              c_scl_result)

        # Check the output
        if scl_return_value != 0:
            raise SaError("SaRunScl", 1)

        # Wrap the result
        classification_result = SaSclResult()
        classification_result.c_init(self.ffi, c_scl_result)

        # Free the scl_result
        self.ffi.release(c_scl_result)

        return classification_result

