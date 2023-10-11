#!/bin/sh

./compile.sh && gdb -q --args ./main kernelcache.release.n88.decrypted kernelcache.release.n88.decrypted.patched diff.txt
