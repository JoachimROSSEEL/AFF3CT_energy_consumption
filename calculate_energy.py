# Python script to calculate consummed energy on the Dalek cluster from files return by node-conso

from os import listdir
from os.path import isfile, join

# Polar codes encoder and decoder parameters
# Noise parameters
# SNR for FER = 10-2: 
# SC(4096,2048) : 2.15 dB
# SCL-4(4096,2048) : 1.525 dB
# SC(4096,1024) : 2.25 dB
# SCL-4(4096,1024) : 1.56 dB
# SC(4096,3072) : 3.8 dB
# SCL-4(4096,3072) : 3.0 dB

# Noise for FER = 1e-2
Eb_N0_min = 2.15
# 1.14 # 1.525 (SC(32760, R=1/2))
# 2.15 (SC(4096, R=1/2)) # 6.2
step = 0.5 
Eb_N0_max = Eb_N0_min + step

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

# Number of noisy codewords
n = 100000 - 1

# Number of times the noisy codewords data set is decoded
nb_repeats = 10

# Variable to store power
energy = 0

# Boolean to write in file_conso_per_nodes once decoding runtime is reached
write_bool = False               

# Counter to skip nb_skips first lines (some noisy lines from previous measures can be present)
skip = 0
nb_skips = 10

# Waiting time for simulation to launch (sleep between node-conso and launchin decoding)
sleep_time = 11 # 1000 μs

# Variable to store the node configuration, the energy and the time for each file
node_config = ""

# File for storing the consummed energy and execution time per polar nodes configuration
name_file_conso_per_nodes = "energy_az5/energy_" + str(N) + "_" + str(enc_info_bits) + "_CRC_" + crc_poly + "_" + dec + "_" + dec_implem + ".txt" 
# fields_name = ["Nodes_configuration", "Energy(J)", "Time(μs)"]
file_conso_per_nodes = open(name_file_conso_per_nodes, "w+")
file_conso_per_nodes.write(f'{"Nodes_configuration":<30} {"Power(W/bit)":<30} {"Time(μs)":<30} \n')

# Getting energy file of each polar nodes configuration
# energy_folder = "/scratch/rosseelj/energy/energy_polar_" + str(N) + "_" + str(enc_info_bits) + "_CRC_" + crc_poly + "_Decoder_polar_" + dec 
# energy_folder = "/scratch/rosseelj/conso"
energy_folder = "energy_polar_" + str(N) + "_" + str(enc_info_bits) + "_CRC_" + crc_poly + "_Decoder_polar_" + dec 
energy_files  = [f for f in listdir(energy_folder) if isfile(join(energy_folder, f))]

# Getting running time of decoding for each polar nodes configuration
# runtime_folder = "/scratch/rosseelj/runtime/runtime_RX_" + str(N) + "_" + str(enc_info_bits) + "_CRC_" + crc_poly + "_Decoder_polar_" + dec 
runtime_folder = "runtime"
runtime_files  = [f for f in listdir(runtime_folder) if isfile(join(runtime_folder, f))]

# Storing runtime for each node configuration in list_runtimes, according to its order in runtime_files
# list_runtimes[i] corresponds to node configuration i in list_nodes_runtime
list_nodes_runtime = []
list_runtimes = []
for f in runtime_files:
    # Getting nodes config and adding it to list_nodes_runtime
    node_config = f.split("_")[-1]
    node_config = node_config.split(".")[0]
    list_nodes_runtime.append(node_config)

    # Getting runtime for each node configuration and adding it to list_runtimes 
    file_runtime = open(runtime_folder + "/" + f, "r")
    
    # Files composed of two line : 
    #  - Description of each column: Nodes configuration and runtime in seconds
    #  - Values of node configuration and runtime
    n_line = 0 # line number
    for line in file_runtime:
        if n_line == 1:
            runtime = line.split(" ")[1]
            runtime = runtime.split("\n")[0]
            list_runtimes.append(float(runtime))
        n_line =+ 1 
        # Skip first line (description of each column)
    file_runtime.close()

for fname in energy_files:

    # Getting node configuration
    node_config = fname.split("_")[-1]
    node_config = node_config.split(".")[0]

    # Getting decoding runtime for the node configuration
    ind_node_config = list_nodes_runtime.index(node_config)
    runtime = list_runtimes[ind_node_config] * 1000 # multiply by 1000 to convert to μs

    # Resseting variables to their initial values
    energy = 0
    write_bool = False               
    skip = 0

    # Reading energy file (output of node-conso)
    f = open(energy_folder + "/" + fname, "r")
    for line in f:
        # Splitting line
        line_split = line.split(" ")
        # Removing empty blanks
        while '' in line_split:
            line_split.remove('')
 
        # Avoid empty lines
        if line_split[0] != '\n':

            # Skipping first lines 
            if skip < nb_skips:
                skip += 1
            
            # Otherwise, we compute power during decoding runtime
            # First condition : skipping sleep time, between node conso and launching polar decoding
            # Second condition : stop when decoding runtime is reached
            elif float(line_split[0]) > sleep_time and float(line_split[0]) < runtime:
                # Take only intensity and voltage of CPU to compute power (line marked by x.4 in the second column of an energy file)
                if line_split[1] == "0":
                    energy = line_split[-1].split("\n")[0]
                    energy = energy.split("J")[0]

            # Writing in file_conso_per_nodes 
            elif float(line_split[0]) > runtime and write_bool == False:
                file_conso_per_nodes.write(f'{node_config:<30} {str(energy):<30} {str(runtime/1000):<30} \n')
                write_bool = True

    f.close()

file_conso_per_nodes.close()

