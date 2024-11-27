"""
Example script that goes through images in IMAGES_PATH:
prints labels of detections found and seat classification for "window" detections
"""

import os
from cffi import FFI
from PIL import Image
import time

from seatsanalyzer import Seatsanalyzer
from er import ER

# PATHS
EXAMPLES_DIR = os.path.dirname(os.path.realpath(__file__))
PACKAGE_DIR = os.path.join(EXAMPLES_DIR, "..", "..")
SDK_DIR = os.path.join(PACKAGE_DIR, "sdk")
LIB_DIR = os.path.join(SDK_DIR, "lib")
IMAGES_PATH = os.path.join(PACKAGE_DIR, "data", "images")

# setup paths lib and config
lib_name = [name for name in os.listdir(LIB_DIR) if name.startswith("libseatsanalyzer")][0]
SEATSANALYZER_LIB_PATH = os.path.join(LIB_DIR, lib_name)
SUPPORT_LIBS = []
CONFIG_PATH = os.path.join(SDK_DIR, "config.ini")

# init library
ffi = FFI()
eyedea_er = ER(ffi, SEATSANALYZER_LIB_PATH)
sa = Seatsanalyzer(ffi, SEATSANALYZER_LIB_PATH)
sa.init(CONFIG_PATH, None)

for file in os.listdir(IMAGES_PATH):
    print("\n Image:" + file)
    img_path = os.path.join(IMAGES_PATH, file)
    img = Image.open(img_path)

    # Seatsanalyzer expects color image, convert to color if gray
    img = img.convert("RGB")
    er_image = eyedea_er.convert_pil_image_to_erimage(img)

    # Compute detections
    t = time.process_time()
    det_result = sa.run_det(er_image)
    labels_found_string = ", ".join([detection.label for detection in det_result.detections])
    print("Detections computed in " + str(time.process_time() - t) + "s.")
    print("Found {} detections".format(det_result.num_detections) + ", with labels: " + labels_found_string)

    # Compute classifications for all detections
    col = (255, 0, 0)
    for detection in det_result.detections:
        if detection.label == "window":
            # Print classification info
            print("Window classification:")
            scl_result = sa.run_scl(er_image, detection.position, detection.label)
            print("  - left: {}".format(scl_result.left))
            print("  - middle: {}".format(scl_result.middle))
            print("  - right: {}".format(scl_result.right))
