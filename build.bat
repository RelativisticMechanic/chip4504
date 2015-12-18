smlrcc -ppg -tiny -c emu.c -S emu.asm
nasm -f bin chip4504.asm -o chip4504.bin
nasm -f bin boot/bootldr.asm -o bootldr.bin
del emu.asm