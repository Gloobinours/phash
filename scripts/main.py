import os
import cv2
import time
import ctypes
import numpy as np
from picamera2 import Picamera2


class C_Image(ctypes.Structure):
    _fields_ = [
        ("width", ctypes.c_int32),
        ("height", ctypes.c_int32),
        ("channels", ctypes.c_int32),
        ("pixels", ctypes.POINTER(ctypes.c_uint8)),
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
if not lib.load_phash_tables(LUT_PATH.encode("utf-8")):
    raise RuntimeError(f"C library failed to parse required LUT asset at: {LUT_PATH}")


def do_phash(image):
    start_time = time.perf_counter()

    height, width, _ = image.shape

    image_contiguous = np.ascontiguousarray(image, dtype=np.uint8)

    pixel_ptr = image_contiguous.ctypes.data_as(ctypes.POINTER(ctypes.c_uint8))

    c_image_instance = C_Image(width=width, height=height, channels=1, pixels=pixel_ptr)

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
        main={
            "size": (1456, 1088),
            "format": "RGB888",
        }  # Use BGR888 or YUV420 if preferred
    )
    picam.configure(config)

    picam.set_controls(
        {
            "AnalogueGain": 1.0,
            "ExposureTime": 25000,
            # Disable auto-exposure and auto-white balance for consistent captures
            "AeEnable": False,
            "AwbEnable": False,
            "ColourGains": (
                2.5,
                2.5,
            ),  # (red, blue) gain relative to green (2x green sensitivity)
            "Saturation": 1.0,
            "Contrast": 0.5,
            "Brightness": 0.0,
        }
    )

    picam.start()

    frame_w, frame_h = config["main"]["size"]

    print("Live stream started. Press 'q' to quit.")

    # App time debugging init
    app_start_time = time.perf_counter()
    frame_count = 0
    average_frame_time = 0.0

    try:
        while True:
            frame_start_time = time.perf_counter()

            # Capture the current frame
            frame = picam.capture_array()

            # height, width, _ = frame.shape

            grayscale_img = frame[0 : int(frame_h * 2 / 3), :]
            # grayscale_img = frame[:frame_h, :frame_w]

            # grayscale_img = (
            #     frame[0 : int(height * 2 / 3), :, 0] if len(frame.shape) == 3 else frame
            # )

            do_phash(grayscale_img)

            # Display the image in an OpenCV window
            cv2.imshow("IMX296 Live Feed", frame)

            # Update a constant-memory running average of frame time
            frame_end_time = time.perf_counter()
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
            if cv2.waitKey(1) & 0xFF == ord("q"):
                break
    finally:
        # Clean up resources
        picam.stop()
        cv2.destroyAllWindows()

    app_end_time = time.perf_counter()
    app_execution_time = app_end_time - app_start_time
    print(f"\nTotal execution time: {app_execution_time} seconds")

    if frame_count:
        print(f"Avg frame time: {average_frame_time} seconds")


if __name__ == "__main__":
    start()
