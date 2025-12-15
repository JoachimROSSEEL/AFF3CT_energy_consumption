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

start=`date +%s`

# Make folder to store energy measurement
mkdir -p $2

# Source node-conso
source /etc/profile

# Load node conso
module load node-conso/g8b77353

# Starting the  board
node-conso -P 1

# Clearing registers
node-conso -m 1

# Lauching node consumption measurement
node-conso -M 1 -t $1 > $3 &

sleep 1

# Launching RX cgain
echo "Running RX chain with energy test."
./RX_energy_test \
--crc-poly $4 \
--crc-size $5 \
--crc-info-bits $6 \
--enc-info-bits $7 \
-N $8 \
--enc-fb-noise $9 \
--mnt-info-bits ${10} \
-n ${11} \
-m ${12} \
-M ${13} \
-s ${14} \
-D ${15} --dec-implem ${16} --dec-polar-nodes ${17}
# --dec-simd "INTRA" --dec-polar-nodes "{R0,R1,REP}"  \
# SC (FAST or NAIVE) SCAN (NAIVE) SCF --dec-flips 4 (NAIVE) SCL (FAST or NAIVE) SCL_MEM (FAST) ASCL (FAST) ASCL_MEM (FAST) CHASE ML
# "{R0,R1,R0L,REP,REPL,SPC}"

sleep 3

wait

end=`date +%s`
runtime= $((end - start))
echo "runtime is $runtime s."

#exit 0




