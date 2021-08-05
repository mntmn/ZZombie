export PATH=$PATH:/opt/amiga/bin

m68k-amigaos-gcc -O2 -o Assign Assign.c -lamiga -Wall -Wno-pointer-sign

cp Assign ../../disk/C/
