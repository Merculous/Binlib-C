#!/bin/sh

./compile.sh && valgrind -s --leak-check=full --show-leak-kinds=all ./main kernelcache.release.n88.decrypted kernelcache.release.n88.decrypted.patched diff.txt
