#!/bin/bash
#SBATCH --job-name=RX_energy_test
#SBATCH --export=ALL
# Walltime
#SBATCH --time=336:00:00
# Mail
#SBATCH --mail-type=ALL
#SBATCH --mail-user=joachim.rosseel@bordeaux-inp.fr
# Output and error files
#SBATCH --out=/scratch/rosseelj/out/RX_energy_test.out
#SBATCH --error=/scratch/rosseelj/error/RX_energy_test.err
# Declare job non-rerunable
#SBATCH --no-requeue

# SNR for FER = 10-2: 
# SC(4096,2048) : 2.15 dB
# SCL-4(4096,2048) : 1.525 dB
# SC(4096,1024) : 2.25 dB
# SCL-4(4096,1024) : 1.56 dB
# SC(4096,3072) : 3.0 dB
# SCL-4(4096,3072) : 3.8 dB

echo "Running RX chain with energy test."
./RX_energy_test \
--crc-poly "0x8005" \
--crc-size 16 \
--crc-info-bits 2032 \
--enc-info-bits 2048 \
-N 4096 \
--enc-fb-noise 0.85 \
-m 2.15 \
-M 2.65 \
-s 0.5 \
--mnt-info-bits 2032 \
-n 100000 \
-D SC --dec-implem FAST 
# --dec-simd "INTRA" --dec-polar-nodes "{R0,R1,REP}"  \
# SC (FAST or NAIVE) SCAN (NAIVE) SCF --dec-flips 4 (NAIVE) SCL (FAST or NAIVE) SCL_MEM (FAST) ASCL (FAST) ASCL_MEM (FAST) CHASE ML
# "{R0,R1,R0L,REP,REPL,SPC}"

exit 0



