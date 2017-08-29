

obj = S32A_Opaque_BlitRow32 Dx_shaderproc_part sk_memset32 ConvolveHorizontally
source = S32A_Opaque_BlitRow32.c DX_shaderproc_part.c sk_memset32.c ConvolveHorizontally.cpp

GCC     = mips-img-linux-gnu-gcc 
CPP     = mips-img-linux-gnu-g++ 
CFLAGS  = -mmsa -mips32r6 -EL -static 
CPPFLAGS = -mmsa -mips32r6 -EL -flax-vector-conversions -static 

EXECFLAGS = -cpu mips32r6-generic -L $(MIPS_LINUXGNU_ROOT)/sysroot/mipsel-r6-soft

QEMU   = /home/ryan/workdir/qemu/qemu-2.9.0



all: $(obj) exec


$(obj): $(source) 
	$(GCC) $(CFLAGS) S32A_Opaque_BlitRow32.c -o S32A_Opaque_BlitRow32
	$(GCC) $(CFLAGS) DX_shaderproc_part.c -o Dx_shaderproc_part
	$(GCC) $(CFLAGS) sk_memset32.c -o sk_memset32
	$(CPP) $(CPPFLAGS) ConvolveHorizontally.cpp -o ConvolveHorizontally


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
	$(QEMU)/mipsel-linux-user/qemu-mipsel $(EXECFLAGS) ConvolveHorizontally -idx 33

clean:
	-rm *~ $(obj) *.o

