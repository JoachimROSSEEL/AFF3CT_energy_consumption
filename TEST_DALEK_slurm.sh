#!/bin/bash
#SBATCH --job-name=TEST_DALEK
#SBATCH --export=ALL
# Walltime
#SBATCH --time=336:00:00
# Mail
#SBATCH --mail-type=ALL
#SBATCH --mail-user=joachim.rosseel@bordeaux-inp.fr
# Output and error files
#SBATCH --error=/scratch/rosseelj/error/TEST_dalek.err
# Declare job non-rerunable
#SBATCH --no-requeue

cd "/AFF3CT_energy_consumption" || exit

/bin/sh -c './TEST_DALEK'

sleep 1