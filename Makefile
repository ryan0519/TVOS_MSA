

GCC      = mips-img-linux-gnu-gcc 
CPP      = mips-img-linux-gnu-g++ 
CFLAGS   = -mmsa -mips32r6 -EL 
CPPFLAGS = -mmsa -mips32r6 -EL -flax-vector-conversions -static 
LDFLAGS  = -EL -static

QEMU   		:= /home/ryan/workdir/qemu/qemu-2.9.0
QEMUFLAGS 	:= -cpu mips32r6-generic -L $(MIPS_LINUXGNU_ROOT)/sysroot/mipsel-r6-soft

#SRC := $(wildcard *.c)  

SRC    := S32A_Opaque_BlitRow32.c DX_shaderproc_part.c sk_memset32.c
OBJ    := $(SRC:%.c=%.o) 
TARGET := $(SRC:%.c=%)


CPPSRC := ConvolveHorizontally.cpp
CPPOBJ := $(CPPSRC:%.cpp=%.o) 
CPPEXE := $(CPPOBJ:%.o=%)

all: $(TARGET)	$(CPPEXE) CK_BlitRow CK_DXpart CK_MMSET CK_CVH


$(TARGET): %:%.o
	$(GCC) $(LDFLAGS) -o $@ $<

$(CPPEXE): %:%.o
	$(CPP) $(LDFLAGS) -o $@ $<

$(OBJ) : %.o:%.c
	$(GCC) $(CFLAGS) -c $< -o $@

$(CPPOBJ): %.o : %.cpp
	$(CPP) $(CPPFLAGS) -c $< -o $@


#	$(GCC) $(CFLAGS) S32A_Opaque_BlitRow32.c -o S32A_Opaque_BlitRow32
#	$(GCC) $(CFLAGS) DX_shaderproc_part.c -o DX_shaderproc_part
#	$(GCC) $(CFLAGS) sk_memset32.c -o sk_memset32
#	$(CPP) $(CPPFLAGS) ConvolveHorizontally.cpp -o ConvolveHorizontally


CK_BlitRow:
	$(QEMU)/mipsel-linux-user/qemu-mipsel $(QEMUFLAGS) S32A_Opaque_BlitRow32 -idx 1039 -count 348
	$(QEMU)/mipsel-linux-user/qemu-mipsel $(QEMUFLAGS) S32A_Opaque_BlitRow32 -idx 1068 -count 4892
	$(QEMU)/mipsel-linux-user/qemu-mipsel $(QEMUFLAGS) S32A_Opaque_BlitRow32 -idx 99 -count 28
	$(QEMU)/mipsel-linux-user/qemu-mipsel $(QEMUFLAGS) S32A_Opaque_BlitRow32 -idx 796 -count 4876

CK_DXpart:
	$(QEMU)/mipsel-linux-user/qemu-mipsel $(QEMUFLAGS) DX_shaderproc_part -idx 40 -count 112
	$(QEMU)/mipsel-linux-user/qemu-mipsel $(QEMUFLAGS) DX_shaderproc_part -idx 68 -count 1016

CK_MMSET:
	$(QEMU)/mipsel-linux-user/qemu-mipsel $(QEMUFLAGS) sk_memset32 -idx 12 -count 4892
	$(QEMU)/mipsel-linux-user/qemu-mipsel $(QEMUFLAGS) sk_memset32 -idx 213 -count 28
	$(QEMU)/mipsel-linux-user/qemu-mipsel $(QEMUFLAGS) sk_memset32 -idx 3 -count 1016
	$(QEMU)/mipsel-linux-user/qemu-mipsel $(QEMUFLAGS) sk_memset32 -idx 56 -count 348

CK_CVH:
	$(QEMU)/mipsel-linux-user/qemu-mipsel $(QEMUFLAGS) ConvolveHorizontally -idx 33
	$(QEMU)/mipsel-linux-user/qemu-mipsel $(QEMUFLAGS) ConvolveHorizontally -idx 34
	$(QEMU)/mipsel-linux-user/qemu-mipsel $(QEMUFLAGS) ConvolveHorizontally -idx 36

clean:
	-rm *~ $(TARGET) $(CPPEXE) *.o

