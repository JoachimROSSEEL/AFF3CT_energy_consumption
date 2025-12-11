#!/bin/bash
#SBATCH --job-name=RX_energy_test_polar_nodes
#SBATCH --export=ALL
# Walltime
#SBATCH --time=336:00:00
# Mail
#SBATCH --mail-type=ALL
#SBATCH --mail-user=joachim.rosseel@bordeaux-inp.fr
# Output and error files
#SBATCH --out=/scratch/rosseelj/out/RX_energy_test_polar_nodes.out
#SBATCH --error=/scratch/rosseelj/error/RX_energy_test_polar_nodes.err
# Declare job non-rerunable
#SBATCH --no-requeue
# mkdir pids -p
python3 RX_energy_test_polar_nodes.py 
# > pids/RX_energy_test_polar_nodes_pid

# Trick to avoid being kicked by the PBS server due to detached process group of Python script
# while [ -e /proc/$(cat "pids/RX_energy_test_polar_nodes_pid") ]
# do
#   sleep 0.1
# done

