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


# Load module cpudev : enables to modify the CPU driver, its governor, its frequency per core, idle states, and so on
# module load cpudev
# echo $(pwd)
# echo $(cpudev apply --help)
# Apply CPU configuration from yaml file
# cpudev cpufreq --config config_cpu_az4.yaml
# cpudev driver acpi
# cpudev cpufreq --governor userspace --policies "0-3" --frequency "2500000"
# cpudev cpufreq --governor rosseelj --policies "0-3"


sleep 1

module load ncm/gdcc873f

touch /scratch/rosseelj/conso/conso_test.txt

node-conso -P 1 # <= turn on I2C chain 1
node-conso -P 2 # <= turn on I2C chain 2
node-conso -m 1
node-conso -m 2
node-conso -M 1 
node-conso -M 2
node-conso -t 30 > /scratch/rosseelj/conso/conso_test.txt