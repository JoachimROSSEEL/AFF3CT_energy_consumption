#!/bin/bash
#SBATCH --job-name=node_conso
#SBATCH --export=ALL
# Walltime
#SBATCH --time=336:00:00
# Mail
#SBATCH --mail-type=ALL
#SBATCH --mail-user=joachim.rosseel@bordeaux-inp.fr
# Output and error files
#SBATCH --out=/scratch/rosseelj/out/node_conso.out
#SBATCH --error=/scratch/rosseelj/error/node_conso.err
# Declare job non-rerunable
#SBATCH --no-requeue

sleep 1

module load ncm/gdcc873f

touch /scratch/rosseelj/conso/conso_TX_RX.txt

node-conso -P 1
node-conso -m 1
node-conso -M 1 -t 60 > /scratch/rosseelj/conso/conso_TX_RX.txt