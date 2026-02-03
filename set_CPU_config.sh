#!/bin/bash
#SBATCH --job-name=set_CPU_config
#SBATCH --export=ALL
# Walltime
#SBATCH --time=336:00:00
# Mail
#SBATCH --mail-type=ALL
#SBATCH --mail-user=joachim.rosseel@bordeaux-inp.fr
# Output and error files
#SBATCH --out=/scratch/rosseelj/out/set_CPU_config.out
#SBATCH --error=/scratch/rosseelj/error/set_CPU_config.err
# Declare job non-rerunable
#SBATCH --no-requeue

# Load module cpudev : enables to modify the CPU driver, its governor, its frequency per core, idle states, and so on
module load cpudev

# # Apply CPU configuration from yaml file
cpudev apply config_cpu_az4.yaml


# Load module cpudev : enables to modify the CPU driver, its governor, its frequency per core, idle states, and so on
# module load cpudev
# echo $(pwd)
# echo $(cpudev apply --help)
# Apply CPU configuration from yaml file
# cpudev cpufreq --config config_cpu_az4.yaml
# cpudev driver acpi
# cpudev cpufreq --governor userspace --policies "0-3" --frequency "2500000"
# cpudev cpufreq --governor rosseelj --policies "0-3"