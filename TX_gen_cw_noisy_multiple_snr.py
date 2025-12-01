import os
import numpy as np

# Noise parameters
Eb_N0_min = 3.0 # 3.8 # 1.56 # 2.25 # 1.14 # 
step = 0.25
Eb_N0_max = Eb_N0_min + 2*step

Eb_N0 = np.arange(Eb_N0_min, Eb_N0_max, step)

# Number of noisy codewords
n = 100000 

# CRC
crc_poly = "0x8005" # "0x04C11DB7" # "0x8005" 
crc_size = 16 # 32 # 16 
crc_info_bits = 3056 # 16352

# Polar encoder
enc_info_bits = 3072 # 16384 # 2048 
N = 4096 # 32768 # 4096
enc_fb_noise = 0.85  

if __name__ == "__main__":
  for i in range(0, Eb_N0.shape[0] - 1):
    os.system(f"sudo ./TX_gen_cw_noisy --crc-poly {crc_poly} --crc-info-bits {crc_info_bits} --crc-size {crc_size} --enc-info-bits {enc_info_bits} -N {N} --enc-fb-noise {enc_fb_noise} --mnt-info-bits {crc_info_bits} -n {n} -m {Eb_N0[i]} -M {Eb_N0[i+1]} -s {step}")
