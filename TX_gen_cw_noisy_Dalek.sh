#!/bin/bash
#SBATCH --job-name=TX_gen_noisy_cw
#SBATCH --export=ALL
# Walltime
#SBATCH --time=336:00:00
# Mail
#SBATCH --mail-type=ALL
#SBATCH --mail-user=joachim.rosseel@bordeaux-inp.fr
# Output and error files
#SBATCH --out=/scratch/rosseelj/out/TX_gen_noisy_cw.out
#SBATCH --error=/scratch/rosseelj/error/TX_gen_noisy_cw.err
# Declare job non-rerunable
#SBATCH --no-requeue

# SNR for FER = 10-2: 
# SC(4096,2048) : 2.15 dB
# SCL-4(4096,2048) : 1.525 dB
# SC(4096,1024) : 2.25 dB
# SCL-4(4096,1024) : 1.56 dB
# SC(4096,3072) : 3.0 dB
# SCL-4(4096,3072) : 3.8 dB

echo "Running TX chain."
./TX_gen_cw_noisy \
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
-n 100000

exit 0