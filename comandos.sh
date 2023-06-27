#!/bin/bash
./tp2virtual $1 $2 4 128
./tp2virtual $1 $2 4 256
./tp2virtual $1 $2 4 512
./tp2virtual $1 $2 4 1024
./tp2virtual $1 $2 4 2048
./tp2virtual $1 $2 4 4096
./tp2virtual $1 $2 4 8192
./tp2virtual $1 $2 4 16384
echo "---"
./tp2virtual $1 $2 2 4096
./tp2virtual $1 $2 4 4096
./tp2virtual $1 $2 8 4096
./tp2virtual $1 $2 16 4096
./tp2virtual $1 $2 32 4096
./tp2virtual $1 $2 64 4096
