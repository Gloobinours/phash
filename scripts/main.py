import os
import cv2
import time
import ctypes
import numpy as np

class C_Image(ctypes.Structure):
    _fields_ = [
        ("width", ctypes.c_int32),
        ("height", ctypes.c_int32),
        ("channels", ctypes.c_int32),
        ("pixels", ctypes.POINTER(ctypes.c_uint8))
    ]

# Files
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
LIB_PATH = os.path.normpath(os.path.join(SCRIPT_DIR, "../bin/libphash.so"))
IMAGE_PATH = os.path.normpath(os.path.join(SCRIPT_DIR, "../static/jason.jpg"))
LUT_PATH = os.path.normpath(os.path.join(SCRIPT_DIR, "../bin/dct_lut.bin"))

# C phash
lib = ctypes.CDLL(LIB_PATH)
lib.load_phash_tables.argtypes = [ctypes.c_char_p]
lib.load_phash_tables.restype = ctypes.c_int
lib.phash.argtypes = [ctypes.POINTER(C_Image)]
lib.phash.restype = ctypes.c_uint64

# Load the dct LUT
if not lib.load_phash_tables(LUT_PATH.encode('utf-8')):
    raise RuntimeError(f"C library failed to parse required LUT asset at: {LUT_PATH}")

def do_phash(image):
    start_time = time.perf_counter()

    height, width = image.shape

    image_contiguous = np.ascontiguousarray(img, dtype=np.uint8)

    pixel_ptr = img_contiguous.ctypes.data_as(ctypes.POINTER(ctypes.c_uint8))

    c_image_instance = C_Image(
        width=width,
        height=height,
        channels=1,
        pixels=pixel_ptr
    )

    print(f"Image size: {width}x{height}")

    hash = lib.phash(ctypes.byref(c_image_instance))

    end_time = time.perf_counter()

    execution_time = end_time - start_time
    print(f"Executed in {execution_time:.6f} seconds")
    print(f"pHash: {hash:016x}")

    return hash

def start():
    # Initialize Picamera2 object for IMX296
    picam = Picamera2()

    # Configure output format
    config = picam.create_video_configuration(
        main={"size": (1456, 1088), "format": "RGB888"} # Use BGR888 or YUV420 if preferred
    )
    picam.configure(config)
    picam.start()

    # Load lens calibration produced by calibrate.py, if available
    frame_w, frame_h = config["main"]["size"]
    try:
        camera_matrix, dist_coeffs = load_calibration()
        new_camera_matrix, _ = cv2.getOptimalNewCameraMatrix(
            camera_matrix, dist_coeffs, (frame_w, frame_h), 1, (frame_w, frame_h)
        )
        print(f"Loaded lens calibration from {CALIBRATION_FILE}.")
    except FileNotFoundError:
        camera_matrix = dist_coeffs = new_camera_matrix = None
        print(f"No {CALIBRATION_FILE} found - run calibrate.py to enable undistortion.")

    print("Live stream started. Press 'q' to quit.")

    try:
        while True:
            frame_start_time = time.time()
            
            # Capture the current frame
            frame = picam.capture_array()

            # Display the image in an OpenCV window
            cv2.imshow("IMX296 Live Feed", frame)

            # Update a constant-memory running average of frame time
            frame_end_time = time.time()
            frame_time = frame_end_time - frame_start_time
            frame_count += 1
            average_frame_time += (frame_time - average_frame_time) / frame_count

            print(
                f"Frame time: {frame_time:.4f} s | "
                f"Average frame time ({frame_count} frames): {average_frame_time:.4f} s",
                end="\r",
                flush=True,
            )

            # Wait 1ms for the 'q' key to be pressed
            if cv2.waitKey(1) & 0xFF == ord('q'):
                break
    finally:
        # Clean up resources
        picam.stop()
        cv2.destroyAllWindows()
