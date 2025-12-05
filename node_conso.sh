
sleep 1

module load node-conso/g8b77353

touch /scratch/rosseelj/conso/conso_TX_RX.txt

node-conso -P 1
node-conso -m 1
node-conso -M 1 -t 300 > /scratch/rosseelj/conso/conso_TX_RX.txt