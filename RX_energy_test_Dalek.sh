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

node-conso -P 1
node-conso -m 1
node-conso -M 1 -t $1 > $2 &

sleep 1
echo $1
echo $7
echo $11
echo $14
echo "Running RX chain with energy test."
./RX_energy_test \
--crc-poly $3 \
--crc-size $4 \
--crc-info-bits $5 \
--enc-info-bits $6 \
-N $7 \
--enc-fb-noise $8 \
--mnt-info-bits $9 \
-n $10 \
-m $11 \
-M $12 \
-s $13 \
-D $14 --dec-implem $15 --dec-polar-nodes $16
# --dec-simd "INTRA" --dec-polar-nodes "{R0,R1,REP}"  \
# SC (FAST or NAIVE) SCAN (NAIVE) SCF --dec-flips 4 (NAIVE) SCL (FAST or NAIVE) SCL_MEM (FAST) ASCL (FAST) ASCL_MEM (FAST) CHASE ML
# "{R0,R1,R0L,REP,REPL,SPC}"

exit 0



