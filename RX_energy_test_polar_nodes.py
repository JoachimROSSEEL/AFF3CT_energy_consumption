import os
import itertools
import time

# Noise parameters
# Noise for FER = 1e-2
Eb_N0_min = 2.15
# 1.14 # 1.525 (SC(32760, R=1/2))
# 2.15 (SC(4096, R=1/2)) # 6.2
step = 0.5 
Eb_N0_max = Eb_N0_min + step

# Number of noisy codewords
n = 100000 - 1

# CRC
# crc_poly = "0x8005" 
# crc_size = 16
# crc_info_bits = 1008
crc_poly = "0x8005" 
crc_size = 16 
crc_info_bits = 2032 
# crc_poly = "0x8005" 
# crc_size = 16
# crc_info_bits = 3056
# crc_poly = "0x04C11DB7" # "0x8005" 
# crc_size = 32 # 16 
# crc_info_bits = 16352

# Polar encoder
# enc_info_bits = 1024 
# N = 4096
# enc_fb_noise = 0.85  
enc_info_bits = 2048 
N = 4096
enc_fb_noise = 0.85  
# enc_info_bits = 3072 
# N = 4096
# enc_fb_noise = 0.85  
# enc_info_bits = 16384 
# N = 32768 
# enc_fb_noise = 0.85  

# Polar decoder
dec = "SC"
# lsize = 4
dec_implem = "FAST"
simd = "INTER"
# --sim-inter-fra, -F

polar_nodes = ["R0", "R1", "R0L", "REP", "REPL", "SPC"]
list_combi = []
for L in range(len(polar_nodes) + 1):
    for subset in itertools.combinations(polar_nodes, L):
        str_sub = ','.join(subset)
        list_combi.append("{" + str_sub + "}")
list_combi.remove('{}')

# Load node conso 
os.system(f"module load node-conso/g8b77353")
# Duration of energy measurement (in seconds)
te = 200

# Setting writing path for energy consumption
write_path = "scratch/rosseelj/energy/" + "energy_polar_2_" + str(N) + "_" + str(enc_info_bits) + "_CRC_" + crc_poly + "_Decoder_polar_" + dec

# Launching RX chain and energy measurement for each polar node configuration
line_com = f""
if __name__ == "__main__":
  for i in range(0, 1):
    # Starting the  board
    os.system("node-conso -P 1")
    # Clearing registers 
    os.system("node-conso -m 1")
     # Lauching node consommation measurement
    write_file = write_path + "/Decoder_polar" + dec + "_nodes_" + list_combi[i] + ".txt"
    os.system(f"node-conso -M 1 -t {te} > {write_file} &")
    time.sleep(30)

    # Launching RX chain 
    line_com = f"sudo ./RX_energy_test --crc-poly {crc_poly} --crc-info-bits {crc_info_bits} --crc-size {crc_size} --enc-info-bits {enc_info_bits} -N {N} --enc-fb-noise {enc_fb_noise} --mnt-info-bits {crc_info_bits} -n {n} -m {Eb_N0_min} -M {Eb_N0_max} -s {step} -D {dec} --dec-implem {dec_implem} --dec-polar-nodes {list_combi[i]}"
    os.system(line_com) 
    time.sleep(30)

  # line_com = f"sudo ./RX_energy_test --crc-poly {crc_poly} --crc-info-bits {crc_info_bits} --crc-size {crc_size} --enc-info-bits {enc_info_bits} -N {N} --enc-fb-noise {enc_fb_noise} --mnt-info-bits {crc_info_bits} -n {n} -m {Eb_N0_min} -M {Eb_N0_max} -s {step} -D {dec} --dec-implem {dec_implem} --dec-simd {simd} --dec-polar-nodes {list_combi[i]}"
  # os.system(line_com) 
  # time.sleep(30)
  
  # simd = "INTRA"
  # line_com = f"sudo ./RX_energy_test --crc-poly {crc_poly} --crc-info-bits {crc_info_bits} --crc-size {crc_size} --enc-info-bits {enc_info_bits} -N {N} --enc-fb-noise {enc_fb_noise} --mnt-info-bits {crc_info_bits} -n {n} -m {Eb_N0_min} -M {Eb_N0_max} -s {step} -D {dec} --dec-implem {dec_implem} --dec-simd {simd} --dec-polar-nodes {list_combi[i]}"
  # os.system(line_com) 
  # time.sleep(30)
  # dec = "SCL"
  # Eb_N0_min = 3.0 # SCL-4(4096,2048) # 6.2
  # step = 0.5 
  # Eb_N0_max = Eb_N0_min + step
  # for i in range(0, len(list_combi)):
  #   line_com = f"sudo ./RX_energy_test --crc-poly {crc_poly} --crc-info-bits {crc_info_bits} --crc-size {crc_size} --enc-info-bits {enc_info_bits} -N {N} --enc-fb-noise {enc_fb_noise} --mnt-info-bits {crc_info_bits} -n {n} -m {Eb_N0_min} -M {Eb_N0_max} -s {step} -D {dec} -L {lsize} --dec-implem {dec_implem} --dec-polar-nodes {list_combi[i]}"
  #   os.system(line_com) 
  #   time.sleep(30)
  # line_com = f"sudo ./RX_energy_test --crc-poly {crc_poly} --crc-info-bits {crc_info_bits} --crc-size {crc_size} --enc-info-bits {enc_info_bits} -N {N} --enc-fb-noise {enc_fb_noise} --mnt-info-bits {crc_info_bits} -n {n} -m {Eb_N0_min} -M {Eb_N0_max} -s {step} -D {dec} --dec-implem {dec_implem} --dec-simd {simd} --dec-polar-nodes {list_combi[i]}"
  # os.system(line_com) 
  # time.sleep(30)
  
  # simd = "INTRA"
  # line_com = f"sudo ./RX_energy_test --crc-poly {crc_poly} --crc-info-bits {crc_info_bits} --crc-size {crc_size} --enc-info-bits {enc_info_bits} -N {N} --enc-fb-noise {enc_fb_noise} --mnt-info-bits {crc_info_bits} -n {n} -m {Eb_N0_min} -M {Eb_N0_max} -s {step} -D {dec} --dec-implem {dec_implem} --dec-simd {simd} --dec-polar-nodes {list_combi[i]}"
  # os.system(line_com) 
  # time.sleep(30)
