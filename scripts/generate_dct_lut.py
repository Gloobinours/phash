#!/usr/bin/env python3
import math
import struct
import os

def generate_dct_lut():
    os.makedirs("assets", exist_ok=True)
    
    out_path = "assets/dct_lut.bin"
    PI = 3.141592653589793
    
    with open(out_path, "wb") as f:
        for u in range(32):
            cu = 0.70710678 if u == 0 else 1.0
            for x in range(32):
                val = 0.5 * cu * math.cos((2 * x + 1) * u * PI / 64.0)
                
                f.write(struct.pack("<f", val))
                
    print(f"Generated: {out_path} (4096 bytes)")

if __name__ == "__main__":
    generate_dct_lut()

