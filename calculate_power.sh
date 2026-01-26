#!/bin/bash
#SBATCH --job-name=calculate_power
#SBATCH --export=ALL
# Walltime
#SBATCH --time=336:00:00
# Mail
#SBATCH --mail-type=ALL
#SBATCH --mail-user=joachim.rosseel@bordeaux-inp.fr
# Output and error files
#SBATCH --out=/scratch/rosseelj/out/calculate_power.out
#SBATCH --error=/scratch/rosseelj/error/calcuate_power.err
# Declare job non-rerunable
#SBATCH --no-requeue
# mkdir pids -p
python3 calculate_power.py 