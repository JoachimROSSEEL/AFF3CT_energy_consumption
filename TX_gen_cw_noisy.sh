#!/bin/sh

echo "Running TX chain."
sudo ./TX_gen_cw_noisy \
--crc-poly "0x3" \
--crc-size 4 \
--crc-info-bits 28 \
--enc-info-bits 32 \
-N 64 \
--enc-fb-noise 0.85 \
-m 4.0 \
-M 5.0 \
-s 1 \
--mnt-info-bits 28 \
-n 100000

exit 0