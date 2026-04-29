# AFF3CT energy consumption measure for polar codes decoders

## Name
Energy/power consumption measure of polar decoding for AFF3CT librairy.

## Description
This project aims to measure the energy consumption of polar decoders from the AFF3CT library (you can see the documentation here: https://aff3ct.readthedocs.io/en/latest/), according to the allowed polar node simplifications. To measure the energy consumption on your own machine, the librairy CPP Joules is used. Further works will include measure on the Dalek cluster of LIP6 (Its presentation is avaible here: https://arxiv.org/abs/2508.10481). 


## Installation

# AFF3CT installation
To run this project, you will first need to install the AFF3CT library. The installation guide is avaiable here: https://aff3ct.readthedocs.io/en/latest/user/installation/installation.html.

# CPP Joules installation
The energy measurement is done thanks to the library CPP Joules. Thus, you need to install it to make the project work. Documentation can be found here: https://shiva9361.github.io/CPP_Joules/. 

# CPP Joules 
CPP Joules works like the functions tik and tok for time measurement. It works only on Intel CPUs, as it uses RAPL.

The first step is to create an energy tracker : EnergyTracker tracker;

Then, you can measure the consummed energy of a code as follow : 
  tracker.start()
  ...
  code
  ...
  tracker.stop()

Finally, the function tracker.calculate_energy() computes the consummed energy. The results can be save in a csv file tracker.save_csv(filename_energy). 

# Get the project and compile it

To compile the project, use make. Three executable are created :
- TX_RX_energy_test : polar chain with emitter and receiver, and an energy consumption;
- TX_gen_noisy_cw : generate a set of noisy codewords from the polar emitter, at a given SNR; 
- RX_energy_test : decode the set of noisy codewords and return the consummed energy in a file (see CPP Joules documentation).

## Usage
You can modify the parameters of chains in the corresponding sh files : TX_RX_energy_test.sh and TX_gen_noisy_cw.sh.
For the measured energy at the reception, RX_energy_test_polar_nodes is used to called RX_energy_test for different polar node configuration.
Thus, an energy measure of the decoding process is realized for each polar node configuration. 

## Use on Dalek
Dalek is a cluster of the LIP6 laboratory (Sorbonne University in Paris) for energy measurements. 
To use it, you need to create an account on the LIP6 servers.

The Dalek documentation is accessible from https://dalek.proj.lip6.fr/. It explains in particular the NCM module and also the different cluster machines.

The branch dalek is similar to the main branch. The main files and sh files are changed to be used on the Dalek Cluster. 
For instance, the energy measure is done with the NCM module instead of CPP Joules. 

To run a program, you need to install this git repo and the Dalek front.

Some uses cases are described bellow.

A use case is also given with the file node_conso.sh. It measures the consumption of a Dalek node with no running program. 

The file set_CPU_config allows to fix the CPU frequency from a yaml file and to test it on a Dalek node (see Hardware node setttings section on the Dalek documentation).

You can also test the energy measurement with NCM of a polar chain with the file TX_RX_energy_test_Dalek.sh.

To measure the consummed power or energy for the polar decoding, you need to :
    - create a set of noisy codewords from the polar emitter, at a given SNR, thanks to file TX_gen_cw_noisy_Dalek.sh. The creation is specific to the Dalek node, so if you change the simulation node, you need to create again the set.
    - run RX_energy_test_polar_nodes.py. A file from node conso is created for each tested polar node configuration of the decoder. It contains time, energy, intensity and voltage measurements for differents components of the node (such as CPU and GPU).
    - run calcuate_energy.py and calcuate_power.py to compute the consummed energy and power for each decoding configuration from the node-conso files.


## Authors and acknowledgment
I would like to thanks the LIP6 team for their help and frequent debuggings with my simulations on the Dalek cluster. I would also like to thanks the AFF3CT team for helping me in this project. 


