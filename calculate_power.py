# Python script to calculate consummed power on the Dalek cluster from files return by node-conso

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

# File for storing the consummed energy and execution time per polar nodes configuaration
name_file_conso_per_nodes = "power/power_" + str(N) + "_" + str(enc_info_bits) + "_CRC_" + crc_poly + "_" + dec + "_" + dec_implem + ".txt" 
# fields_name = ["Nodes_configuration", "Energy(J)", "Time(μs)"]
file_conso_per_nodes = open(name_file_conso_per_nodes, "w+")
file_conso_per_nodes.write(f'{"Nodes_configuration":<30} {"Power(W/bit)":<30} {"Time(μs)":<30} \n')

# Getting energy file of each polar nodes configuration
# energy_folder = "/scratch/rosseelj/energy/energy_polar_" + str(N) + "_" + str(enc_info_bits) + "_CRC_" + crc_poly + "_Decoder_polar_" + dec 
energy_folder = "/scratch/rosseelj/conso"
# energy_folder = "energy_polar_" + str(N) + "_" + str(enc_info_bits) + "_CRC_" + crc_poly + "_Decoder_polar_" + dec 
energy_files  = [f for f in listdir(energy_folder) if isfile(join(energy_folder, f))]

# Skip first lines (often lines from previous simulations)
skip = 0
nb_skips = 100 

# Waiting time for simulation to launch (sleep between node-conso and launchin decoding)
sleep_time = 1000 # 100 μs

# Variable to store the node configuration, the energy and the time for each file
node_config = ""

# Low intensity treshold : if below, no simulation is running
intensity_thresh_low = 1.5

# High intensity treshold : if superior, a simulation is running
intensity_thresh_high = 2.5 

# Boolean : 
#   - 0 if polar decoding has not started (Intensity inferior to intensity_thresh_high)
#   - 1 to start accumulating consummed power (Intensity superior to intensity_thresh_high)
pwr_beg = 0 

# Boolean : 
#   - 0 if polar decoding still runnning (Intensity superior to intensity_thresh_low)
#   - 1 to stop accumulating consummed power (Intensity superior to intensity_thresh_low)
pwr_end = 0 

# Variable to store power consumption (in)
pwr = 0

# Start time at the start of polar decoding simulation
ex_time_beg = 0
# Start time at the start of polar decoding simulation
ex_time_end = 0
# Duration of polar decoding simulation
ex_time = 0

# Count number of times current is inferior to intensity_thresh_high during decoding simulation
count_inf = 0 

# Count number of times current is inferior to intensity_thresh_high after decoding simulation
count_sup = 0 

for fname in energy_files:
    print(fname)
    node_config = fname.split("_")[-1]
    node_config = node_config.split(".")[0]
    print(node_config)
    f = open(energy_folder + "/" + fname, "r")
    for line in f:
        # Splitting line
        line_split = line.split(" ")
        # Removing empty blanks
        while '' in line_split:
            line_split.remove('')
 
        # Avoid empty lines
        if line_split[0] != '\n':
            
            # Skipping first lines (time of sleep between node conso and launching polar decoding)
            if skip < nb_skips:
                skip += 1
            
            else:
                # First condition : skipping sleep time, between node conso and launching polar decoding
                # Second condition : Take intensity and voltage to compute CPU power (line marked by x.4 in the second column of a power file)
                if float(line_split[0]) > sleep_time and line_split[1] == "0.4":
                    # Detect start of decoding simulation: current above 2.5A, starting to accumulate power
                    if float(line_split[-2][0:-2]) > intensity_thresh_high and pwr_beg == 0:
                        pwr = float(line_split[-2][0:-2]) * float(line_split[-3][0:-2])
                        ex_time_beg = str(line_split[0])
                        pwr_beg = 1
                        print(line)

                    # Line with simulation running: accumulating power
                    elif float(line_split[-2][0:-2]) > intensity_thresh_low and pwr_beg != 0 and pwr_end == 0:
                        pwr += float(line_split[-2][0:-2]) * float(line_split[-3][0:-2])
                        if float(line_split[-2][0:-2])< 2.8: 
                            count_inf += 1

                    # End of simulation: current return under 0.2A 
                    elif float(line_split[-2][0:-2]) < intensity_thresh_low and pwr_beg != 0 and pwr_end == 0:
                        pwr += float(line_split[-2][0:-2]) * float(line_split[-3][0:-2])
                        ex_time_end = str(line_split[0])
                        pwr_end = 1
                        # Computing energy per bit =  energy / (enc_info_bits * n * nb_repeats)
                        pwr = pwr / (enc_info_bits * n * nb_repeats)

                        # Computing execution time
                        ex_time = float(ex_time_end) - float(ex_time_beg)
                        file_conso_per_nodes.write(f'{node_config:<30} {str(pwr):<30} {str(ex_time):<30} \n')
                        print(line)
                        print("Number of times current inferior to 2.8A during simulation", count_inf, "\n")

                    elif pwr_end == 1 and float(line_split[-2][0:-2]) > 2.8: 
                        count_sup += 1      
                        
    print("Number of times current superior to 2.8A after simulation", count_sup, "\n")                    
    # Ressetting to zeros variables
    skip = 0
    count_inf = 0
    count_sup = 0
    pwr_beg = 0 
    pwr_end = 0
    pwr = 0
    ex_time_beg = 0
    ex_time_end = 0
    ex_time = 0
    f.close()

file_conso_per_nodes.close()

