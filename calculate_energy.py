# Python script to calculate consummed energy on the Dalek cluster from files return by node-conso

# Codes parameters

import itertools
import csv
import time
import os
from os import listdir
from os.path import isfile, join

# Noise parameters
# SNR for FER = 10-2: 
# SC(4096,2048) : 2.15 dB
# SCL-4(4096,2048) : 1.525 dB
# SC(4096,1024) : 2.25 dB
# SCL-4(4096,1024) : 1.56 dB
# SC(4096,3072) : 3.0 dB
# SCL-4(4096,3072) : 3.8 dB

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

nb_repeats = 10

# File for storing the consummed energy and execution time per polar nodes configuaration
name_file_conso_per_nodes = "energy_" + str(N) + "_" + str(enc_info_bits) + "_CRC_" + crc_poly + "_" + dec + "_" + dec_implem + ".txt" 
# fields_name = ["Nodes_configuration", "Energy(J)", "Time(μs)"]
file_conso_per_nodes = open(name_file_conso_per_nodes, "w+")
file_conso_per_nodes.write(f'{"Nodes_configuration":<30} {"Energy(J)":<30}  {"Time(μs)":<30} \n')

# Getting energy file of each polar nodes configuration
energy_folder = "/scratch/rosseelj/energy/energy_polar_" + str(N) + "_" + str(enc_info_bits) + "_CRC_" + crc_poly + "_Decoder_polar_" + dec 
energy_files  = [f for f in listdir(energy_folder) if isfile(join(energy_folder, f))]

# Variable to store the node configuration, the energy and the time for each file
node_config = ""
# Consumed energy at the start of polar decoding simulation
energy_beg = 0 
# Consumed energy at the end of polar decoding simulation
energy_end = 0
# Consumed energy during decoding simulation
energy = energy_end - energy_beg
# Start time at the start of polar decoding simulation
ex_time_beg = 0
# Start time at the start of polar decoding simulation
ex_time_end = 0
# Duration of polar decoding simulation
ex_time = 0

for fname in energy_files:
    print(fname)
    node_config = fname.split("_")[-1]
    node_config = node_config.split(".")[0]
    print(node_config)
    f = open(energy_folder + "/" + fname, "r")
    for line in f:
        line_split = line.split(" ")
        # Remove empty blank
        while '' in line_split:
            line_split.remove('')
        # print(line_split)
        # If not empty line (possible at end of file), compute energy and time
        if line_split[0] != '\n':
            # Detect start of decoding simulation: current above 0.4A
            if float(line_split[-2][0:-2]) > 0.4 and energy_beg == 0:
                # print(line_split[-2][0:-2])
                # print(line_split[-1][0:-2])
                energy_beg = str(line_split[-1][0:-2])
                ex_time_beg = str(line_split[0])
                print(energy_beg)
                print(ex_time_beg)
            
            # End of simulation: current return under 0.4A 
            if float(line_split[-2][0:-2]) < 0.4 and energy_beg != 0 and energy_end == 0:
                # print(line_split[-2][0:-2])
                # print(line_split[-1][0:-2])
                energy_end = str(line_split[-1][0:-2])
                ex_time_end = str(line_split[0])
                print(energy_end)
                print(ex_time_end)

    energy = float(energy_end) - float(energy_beg)
    ex_time = float(ex_time_end) - float(ex_time_beg)
    file_conso_per_nodes.write(f'{node_config:<30} {str(energy):<30} {str(ex_time):<30} \n')
    f.close()

file_conso_per_nodes.close()