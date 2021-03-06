
#include <string.h>
#include <stdio.h>
#include <sys/time.h>
#include <stdint.h>

#include <msa.h>

#define SK_RESTRICT __restrict__
#define SkASSERT(a)

static void sk_memset32_portable(int32_t * __restrict__ dst, const int32_t value, int cnt)
{

	int tail          = (cnt & 0x3) ;   // cnt %  4
	int tail_line     = cnt - tail;	
	int v4[4]		  = {value, value, value, value};
	int i;
	for ( i = cnt-1; i >= tail_line ; i--)
    { 
		dst[i] = value;
	}

	cnt -= tail;
	cnt <<= 2;

	if(tail_line <= 0) return;

	__asm__ __volatile__(
		"	lw     $t0, %[cnt] ;"
		"   la     $t3, %[dst] ;"     // t3 = & dst
		"   add    $t5, $t3, $t0 ;"   // t5 = t3 + cnt;
		"   ld.w   $w0, %[v4]  ;"

		"	andi   $t6, $t0, 0x7f ;"  // t6 = cnt & 127 
		"   subu   $t7, $t0, $t6 ;"
		"   beqz   $t7, single ;"
        //  128byte block copy
		 
		"   li     $t1, 128;"
		"   add    $t3, $t3, $t6 ;"   // t2 += t6
		"2:"
		"	sub    $t5, $t5, $t1 ;"   // t5 = t5 - 128
	    "	st.w   $w0, ($t5) ;"
	    "	st.w   $w0, 16($t5) ;"
	    "	st.w   $w0, 32($t5) ;"
	    "	st.w   $w0, 48($t5) ;"
	    "	st.w   $w0, 64($t5) ;"
	    "	st.w   $w0, 80($t5) ;"
	    "	st.w   $w0, 96($t5) ;"
	    "	st.w   $w0, 112($t5) ;"
		"	bne    $t5, $t3, 2b  ;"
		"   sub    $t3, $t3, $t6 ;"
		"   blez   $t6, end ;"
		
		// less 128 byte copy
		"single: "
		"   srl    $t0, $t6, 4 ;"
		"   xori   $t1, $t0, 7 ;"
		"   beqz   $t1, 7f ;"         // Goto 7*16

		"   xori   $t1, $t0, 6 ;"
		"   beqz   $t1, 6f ;"         // Goto 6*16
		
		"   li     $t1, 16;"
		"1:"                          // single copy
		"	sub    $t5, $t5, $t1 ;"   // t5 = t5 - 16;
	    "	st.w   $w0, ($t5) ; "
		"	bne    $t5,  $t3, 1b ;"
		"   b      end ;"

        // 7*16 byte copy
		"7: "
	    "	st.w   $w0, ($t3) ;"
	    "	st.w   $w0, 16($t3) ;"
	    "	st.w   $w0, 32($t3) ;"
	    "	st.w   $w0, 48($t3) ;"
	    "	st.w   $w0, 64($t3) ;"
	    "	st.w   $w0, 80($t3) ;"
	    "	st.w   $w0, 96($t3) ;"
		"   b      end ;"

		// 6*16 byte copy
		"6:"
	    "	st.w   $w0, ($t3) ;"
	    "	st.w   $w0, 16($t3) ;"
	    "	st.w   $w0, 32($t3) ;"
	    "	st.w   $w0, 48($t3) ;"
	    "	st.w   $w0, 64($t3) ;"
	    "	st.w   $w0, 80($t3) ;"
        "end:  "
	   : [dst] "+m" (*dst) 
	   : [cnt] "g" (cnt), [v4] "m" (v4)
	);
	return ;
}

#define assign_16_longs(dst, value)             \
	do {                                        \
		*(dst)++ = value;   *(dst)++ = value;   \
		*(dst)++ = value;   *(dst)++ = value;   \
		*(dst)++ = value;   *(dst)++ = value;   \
		*(dst)++ = value;   *(dst)++ = value;   \
		*(dst)++ = value;   *(dst)++ = value;   \
		*(dst)++ = value;   *(dst)++ = value;   \
		*(dst)++ = value;   *(dst)++ = value;   \
		*(dst)++ = value;   *(dst)++ = value;   \
    } while (0)

static void sk_memset32_portable_c(uint32_t dst[], uint32_t value, int count) {
    int sixteenlongs = count >> 4;
    if (sixteenlongs) {
	    do {
	 	   assign_16_longs(dst, value);
		} while (--sixteenlongs != 0);
	    count &= 15;
	}

	if (count) {
		do {
			*dst++ = value;
	    } while (--count != 0);
	}
}

void sk_memset32_mips_msa(uint32_t *dst, uint32_t value, int count)
{
		    SkASSERT(dst != NULL && count >= 0);

			    // dst must be 4-byte aligned.
			SkASSERT((((size_t) dst) & 0x03) == 0);
				
		 	if (count >= 16) {
		 	while (((size_t)dst) & 0x0F) {
			 	*dst++ = value;
				 --count;
		 	}
		 	//v4i32 *d = reinterpret_cast<v4i32*>(dst);
		 	v4i32 *d =(v4i32*)(dst);
		 	v4i32 value_wide = __builtin_msa_fill_w(value);
		 	while (count >= 16) {
				__builtin_msa_st_w(value_wide, (char*) d     , 0);
				__builtin_msa_st_w(value_wide, (char*)(d + 1), 0);
			 	__builtin_msa_st_w(value_wide, (char*)(d + 2), 0);
				__builtin_msa_st_w(value_wide, (char*)(d + 3), 0);
			 	d += 4;
				count -= 16;
		 	}
		 	//dst = reinterpret_cast<uint32_t*>(d);
		 	dst = (uint32_t*)(d);
	 		}
		 	while (count > 0) {
				 *dst++ = value;
				 --count;
	 		}
}


struct timeval total;

static void sum_time(long incr)
{
   int usec = total.tv_usec + incr;
   if( usec > 1000000) {
       total.tv_usec = usec % 1000000;
       total.tv_sec += usec / 1000000;
   } else {
       total.tv_usec = usec;
   }
   return;
}

int testCase(char* idx, int count)
{
	uint32_t outMSA[1223] = {7,6,5,4,3,2,1,0};
	uint32_t outC[1223]   = {7,6,5,4,3,2,1,0};
	uint32_t outMCW[1223]   = {7,6,5,4,3,2,1,0};

	uint32_t i, value=0;

	struct timeval st, end;
	long diff = 0;

    FILE * fd;
    char f1name[500] = {0};
    char f2name[500] = {0};
    char oname[500]  = {0};

	sprintf(f1name, "mt/m1data%s.raw", idx);
	sprintf(f2name, "mt/m2data%s.raw", idx);
	sprintf(oname,  "mt/omdata%s.raw", idx);

    fd = fopen(f1name, "rb");
    fread((void*)outMSA, 1, count*sizeof(uint32_t), fd);
    fclose(fd);
    fd = fopen(f2name, "rb");
    fread((void*)(&value), 1, sizeof(uint32_t), fd);
    fclose(fd);
    
	gettimeofday(&st, NULL);
	sk_memset32_portable   (outMCW, value, count);
	gettimeofday(&end,NULL);
    diff = end.tv_sec*1000000 + end.tv_usec - st.tv_sec*1000000 - st.tv_usec;
	sum_time(diff);
	printf("MCW tv_sec:%d tv_us:%d \n", total.tv_sec, total.tv_usec);

    gettimeofday(&st, NULL);
	sk_memset32_portable_c (outC, value, count);
	gettimeofday(&end,NULL);
    diff = end.tv_sec*1000000 + end.tv_usec - st.tv_sec*1000000 - st.tv_usec;
	sum_time(diff);
	printf("C   tv_sec:%d tv_us:%d \n", total.tv_sec, total.tv_usec);

    gettimeofday(&st, NULL);
	sk_memset32_mips_msa	(outMSA, value, count);
	gettimeofday(&end,NULL);
    diff = end.tv_sec*1000000 + end.tv_usec - st.tv_sec*1000000 - st.tv_usec;
	sum_time(diff);
	printf("MSA   tv_sec:%d tv_us:%d \n", total.tv_sec, total.tv_usec);
	for(i = 0; i<count; i++)
	{
		if(outMSA[i] != outC[i]) printf("%d:%x:%x:%x ", i, outMSA[i], outC[i], value);
	}
    printf("check sk_memset32_portable %s MSA & C complete. Count: %d\n", idx, i*4);

	memset(outC, 0, sizeof(uint32_t)*count);
	fd = fopen(oname, "rb");
    fread((void*) outC, 1, count*sizeof(uint32_t), fd);
    fclose(fd);
    for(i = 0; i<count; i++)
    {
        if(outMSA[i] != outC[i]) printf("%d:%x:%x:%x ", i,  outMSA[i],outC[i],  value);
	}
    printf("check sk_memset32_portable %s MSA & TVOS complete. Count:%d \n", idx, i*4) ;

	return 0;
}

void usage(char *fname)
{
	printf("Usage: %s <-idx num> <-count count> \n", fname);
	printf("       -idx   num     the file sufix ;\n");
	printf("       -count num     the data size;\n");
}
int main(int argc, char* argv[])
{
	int i = 0;
	char idx[10] ={0};
	int  count = 0;

	if(argc < 2) {
		usage(argv[0]);
		return 0;
	}

	for (i = 1; i < argc; i++)  {
		if ((strcmp(argv[i],"-h")==0) || (strcmp(argv[i],"--help")==0))  
        {  
	        usage(argv[0]);  
	        return 0;  
	    }
		else if ((strcmp(argv[i],"-idx")==0))  
		{
			if((i+1) < argc ){
				sprintf(idx, "%s", argv[i+1]);

			}else{
				usage(argv[0]);
			}
		}
		else if ((strcmp(argv[i],"-count")==0))  
		{
			if((i+1) < argc ){
				count = atoi(argv[i+1]);
			}else{
				usage(argv[0]);
			}
		}
	}

	testCase(idx, count/sizeof(uint32_t));
//	testCase("12", 1223);
//	testCase("213", 7);
//	testCase("3", 254);
//	testCase("56", 87);

	return 0;
}


