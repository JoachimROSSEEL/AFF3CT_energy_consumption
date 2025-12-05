#!/bin/bash
#SBATCH --job-name=TX_RX
#SBATCH --export=ALL
# Walltime
#SBATCH --time=336:00:00
# Mail
#SBATCH --mail-type=ALL
#SBATCH --mail-user=joachim.rosseel@bordeaux-inp.fr
# Output and error files
#SBATCH --out=/scratch/rosseelj/out/TX_RX.out
#SBATCH --error=/scratch/rosseelj/error/TX_RX.err
# Declare job non-rerunable
#SBATCH --no-requeue
module load node-conso/g8b77353

# Polar Code (4096, Rate=1/2)
echo "Running TX_RX chain with energy test."
./TX_RX_energy_test \
--crc-poly "0x8005" \
--crc-size 16 \
--crc-info-bits 2032 \
--enc-info-bits 2048 \
-N 4096 \
--enc-fb-noise 0.85 \
-m -4.0 \
-M 3.5 \
-s 0.25 \
-D SC --dec-implem FAST 
# --dec-polar-nodes "{R0,R1,R0L,REP}"
# SC (FAST or NAIVE) SCAN (NAIVE) SCF --dec-flips 4 (NAIVE) SCL (FAST or NAIVE) SCL_MEM (FAST) ASCL (FAST) ASCL_MEM (FAST) CHASE ML
# "{R0,R1,R0L,REP,REPL,SPC}"

# # Polar Code (4096, Rate=1/2)
# echo "Running TX_RX chain with energy test."
# sudo ./TX_RX_energy_test \
# --crc-poly "0x8005" \
# --crc-size 16 \
# --crc-info-bits 2032 \
# --enc-info-bits 2048 \
# -N 4096 \
# --enc-fb-noise 0.85 \
# -m -4.0 \
# -M 2.75 \
# -s 0.25 \
# -D SC --dec-implem FAST # --dec-polar-nodes "{R0,R1,R0L,REP}"
# SC (FAST or NAIVE) SCAN (NAIVE) SCF --dec-flips 4 (NAIVE) SCL (FAST or NAIVE) SCL_MEM (FAST) ASCL (FAST) ASCL_MEM (FAST) CHASE ML
# "{R0,R1,R0L,REP,REPL,SPC}"

# Polar Code (32768, Rate=1/2)
# echo "Running TX_RX chain with energy test."
# sudo ./TX_RX_energy_test \
# --crc-poly "0x04C11DB7" \
# --crc-size 32 \
# --crc-info-bits 16352 \
# --enc-info-bits 16384 \
# -N 32768 \
# --enc-fb-noise 0.85 \
# -m -4.0 \
# -M 1.5 \
# -s 0.25 \
# -D SC 4 --dec-implem FAST # --dec-polar-nodes "{R0,R1,R0L,REP}"
# SC (FAST or NAIVE) SCAN (NAIVE) SCF --dec-flips 4 (NAIVE) SCL (FAST or NAIVE) SCL_MEM (FAST) ASCL (FAST) ASCL_MEM (FAST) CHASE ML
# "{R0,R1,R0L,REP,REPL,SPC}"
sleep 1

node-conso -P 1
node-conso -m 1
node-conso -M 1 -t 270 > /scratch/rosseelj/conso/conso_TX_RX.txt

exit 0