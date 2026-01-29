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

# Load module cpudev : enables to modify the CPU driver, its governor, its frequency per core, idle states, and so on
# module load cpudev

# # Apply CPU configuration from yaml file
# cpudev apply --config config_cpu_az4.yaml

sleep 1

module load ncm/gdcc873f

touch /scratch/rosseelj/conso/conso_test.txt

node-conso -P 1 # <= turn on I2C chain 1
node-conso -P 2 # <= turn on I2C chain 2
node-conso -m 1
node-conso -m 2
node-conso -M 1 -t 30 > /scratch/rosseelj/conso/conso_test.txt
node-conso -M 2
