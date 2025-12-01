#!/bin/sh

echo "Running RX chain with energy test."
sudo ./RX_energy_test \
--crc-poly "0x3" \
--crc-size 4 \
--crc-info-bits 28 \
--enc-info-bits 32 \
-N 64 \
--enc-fb-noise 0.85 \
-m 6.0 \
-M 6.5 \
-s 0.5 \
--mnt-info-bits 28 \
-n 10 \
-D SC --dec-implem FAST --dec-simd "INTRA" --dec-polar-nodes "{R0,R1,REP}"  \
# SC (FAST or NAIVE) SCAN (NAIVE) SCF --dec-flips 4 (NAIVE) SCL (FAST or NAIVE) SCL_MEM (FAST) ASCL (FAST) ASCL_MEM (FAST) CHASE ML
# "{R0,R1,R0L,REP,REPL,SPC}"

exit 0



