#!/bin/sh

# Polar Code (4096, Rate=1/4)
echo "Running TX_RX chain with energy test."
sudo ./TX_RX_energy_test \
--crc-poly "0x8005" \
--crc-size 16 \
--crc-info-bits 3056 \
--enc-info-bits 3072 \
-N 4096 \
--enc-fb-noise 0.85 \
-m -4.0 \
-M 3.75 \
-s 0.25 \
-D SCL -L 4 --dec-implem FAST # --dec-polar-nodes "{R0,R1,R0L,REP}"
# SC (FAST or NAIVE) SCAN (NAIVE) SCF --dec-flips 4 (NAIVE) SCL (FAST or NAIVE) SCL_MEM (FAST) ASCL (FAST) ASCL_MEM (FAST) CHASE ML
# "{R0,R1,R0L,REP,REPL,SPC}"

# # Polar Code (4096, Rate=1/2)
# echo "Running TX_RX chain with energy test."
# sudo ./TX_RX_energy_test \
# --crc-poly "0x8005" \
# --crc-size 16 \
# --crc-info-bits 2032 \
# --enc-info-bits 2048 \
# -N 4096 \
# --enc-fb-noise 0.85 \
# -m -4.0 \
# -M 2.75 \
# -s 0.25 \
# -D SC --dec-implem FAST # --dec-polar-nodes "{R0,R1,R0L,REP}"
# SC (FAST or NAIVE) SCAN (NAIVE) SCF --dec-flips 4 (NAIVE) SCL (FAST or NAIVE) SCL_MEM (FAST) ASCL (FAST) ASCL_MEM (FAST) CHASE ML
# "{R0,R1,R0L,REP,REPL,SPC}"

# Polar Code (32768, Rate=1/2)
# echo "Running TX_RX chain with energy test."
# sudo ./TX_RX_energy_test \
# --crc-poly "0x04C11DB7" \
# --crc-size 32 \
# --crc-info-bits 16352 \
# --enc-info-bits 16384 \
# -N 32768 \
# --enc-fb-noise 0.85 \
# -m -4.0 \
# -M 1.5 \
# -s 0.25 \
# -D SC 4 --dec-implem FAST # --dec-polar-nodes "{R0,R1,R0L,REP}"
# SC (FAST or NAIVE) SCAN (NAIVE) SCF --dec-flips 4 (NAIVE) SCL (FAST or NAIVE) SCL_MEM (FAST) ASCL (FAST) ASCL_MEM (FAST) CHASE ML
# "{R0,R1,R0L,REP,REPL,SPC}"

exit 0