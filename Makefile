

obj = S32A_Opaque_BlitRow32 DX_shaderproc_part sk_memset32
source = S32A_Opaque_BlitRow32_speedup20170822.c DX_shaderproc_part_speedup20170812.c sk_memset32_speedup20170823.c

CC     = mips-img-linux-gnu-gcc 
CFLAGS = -mmsa -mips32r6 -EL -static 

EXECFLAGS = -cpu mips32r6-generic -L $(MIPS_LINUXGNU_ROOT)/sysroot/mipsel-r6-soft

QEMU   = /home/ryan/workdir/qemu/qemu-2.9.0



all: $(obj) exec


$(obj): $(source) 
	$(CC) $(CFLAGS) S32A_Opaque_BlitRow32_speedup20170822.c -o S32A_Opaque_BlitRow32
	$(CC) $(CFLAGS) DX_shaderproc_part_speedup20170812.c -o Dx_shaderproc_part
	$(CC) $(CFLAGS) sk_memset32_speedup20170823.c -o sk_memset32


exec:
	$(QEMU)/mipsel-linux-user/qemu-mipsel $(EXECFLAGS) S32A_Opaque_BlitRow32 -idx 1039 -count 348
	$(QEMU)/mipsel-linux-user/qemu-mipsel $(EXECFLAGS) S32A_Opaque_BlitRow32 -idx 1068 -count 4892
	$(QEMU)/mipsel-linux-user/qemu-mipsel $(EXECFLAGS) S32A_Opaque_BlitRow32 -idx 99 -count 28
	$(QEMU)/mipsel-linux-user/qemu-mipsel $(EXECFLAGS) S32A_Opaque_BlitRow32 -idx 796 -count 4876
	$(QEMU)/mipsel-linux-user/qemu-mipsel $(EXECFLAGS) Dx_shaderproc_part -idx 40 -count 112
	$(QEMU)/mipsel-linux-user/qemu-mipsel $(EXECFLAGS) Dx_shaderproc_part -idx 68 -count 1016
	$(QEMU)/mipsel-linux-user/qemu-mipsel $(EXECFLAGS) sk_memset32 -idx 12 -count 4892
	$(QEMU)/mipsel-linux-user/qemu-mipsel $(EXECFLAGS) sk_memset32 -idx 213 -count 28
	$(QEMU)/mipsel-linux-user/qemu-mipsel $(EXECFLAGS) sk_memset32 -idx 3 -count 1016
	$(QEMU)/mipsel-linux-user/qemu-mipsel $(EXECFLAGS) sk_memset32 -idx 56 -count 348

clean:
	-rm *~ Dx_shaderproc_part S32A_Opaque_BlitRow32 sk_memset32 *.o

