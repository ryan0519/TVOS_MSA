

#include <stdio.h>
#include <sys/time.h>
#include <stdint.h>
#include <string.h>

void shaderproc_part(uint32_t* __restrict__ dst, const uint32_t * __restrict__ src, int cnt)
{

	uint32_t tail          = (cnt & 0x3) ;   // cnt %  4
	uint32_t tail_line     = cnt - tail;	
	uint32_t i;

	for ( i = cnt-1; i >= tail_line ; i--)
    { 
		dst[i] = src[i];
	}

	cnt -= tail;
	cnt <<= 2;

	if(tail_line <= 0) return;

	__asm__ (
		"	lw     $t0, %[cnt] ;"
		"	la     $t2, %[src] ;"	  // t2 <= ar1
		"   la     $t3, %[dst] ;"     // t3 <= ar2
		"	add	   $t4, $t2, $t0 ;"   // t4 = t2 + cnt;
		"   add    $t5, $t3, $t0 ;"   // t5 = t3 + cnt;

		"	andi   $t6, $t0, 0x7f ;"  // t6 = cnt & 127 
		"   subu   $t7, $t0, $t6 ;"
		"   beqz   $t7, single ;"
        //  128byte block copy
		 
		"   li     $t1, 128;"
		"   add    $t2, $t2, $t6 ;"   // t2 += t6
		"2:"
		"	sub    $t4, $t4, $t1 ;"   // t4 = t4 - 16;
		"	sub    $t5, $t5, $t1 ;"   // t5 = t5 - 16;
		"   pref   0,   ($t4) ;"
		"   pref   0,   64($t4) ;"
	    "	ld.w   $w0, ($t4) ;"
	    "	ld.w   $w1, 16($t4) ;"
	    "	ld.w   $w2, 32($t4) ;"
	    "	ld.w   $w3, 48($t4) ;"
	    "	ld.w   $w4, 64($t4) ;"
	    "	ld.w   $w5, 80($t4) ;"
	    "	ld.w   $w6, 96($t4) ;"
	    "	ld.w   $w7, 112($t4) ;"
	    "	st.w   $w0, ($t5) ;"
	    "	st.w   $w1, 16($t5) ;"
	    "	st.w   $w2, 32($t5) ;"
	    "	st.w   $w3, 48($t5) ;"
	    "	st.w   $w4, 64($t5) ;"
	    "	st.w   $w5, 80($t5) ;"
	    "	st.w   $w6, 96($t5) ;"
	    "	st.w   $w7, 112($t5) ;"
		"	bne    $t4,  $t2, 2b ;"
		"   sub    $t2, $t2, $t6 ;"
		"   blez   $t6, end ;"
		
		// less 128 byte copy
		"single: "
		"   pref   0,   ($t2) ;"
		"   srl    $t0, $t6, 4 ;"
		"   xori   $t1, $t0, 7 ;"
		"   beqz   $t1, 7f ;"         // Goto 7*16

		"   xori   $t1, $t0, 6 ;"
		"   beqz   $t1, 6f ;"         // Goto 6*16
		
		"   li     $t1, 16;"
		"1:"                          // single copy
		"	sub    $t4, $t4, $t1 ;"   // t4 = t4 - 16;
		"	sub    $t5, $t5, $t1 ;"   // t5 = t5 - 16;
	    "	ld.w   $w0, ($t4) ; "
	    "	st.w   $w0, ($t5) ; "
		"	bne    $t4,  $t2, 1b ;"
		"   b      end ;"

        // 7*16 byte copy
		"7: "
	    "	ld.w   $w0, ($t2) ;"
	    "	ld.w   $w1, 16($t2) ;"
	    "	ld.w   $w2, 32($t2) ;"
	    "	ld.w   $w3, 48($t2) ;"
	    "	ld.w   $w4, 64($t2) ;"
	    "	ld.w   $w5, 80($t2) ;"
	    "	ld.w   $w6, 96($t2) ;"
	    "	st.w   $w0, ($t3) ;"
	    "	st.w   $w1, 16($t3) ;"
	    "	st.w   $w2, 32($t3) ;"
	    "	st.w   $w3, 48($t3) ;"
	    "	st.w   $w4, 64($t3) ;"
	    "	st.w   $w5, 80($t3) ;"
	    "	st.w   $w6, 96($t3) ;"
		"   b      end ;"

		// 6*16 byte copy
		"6:"
	    "	ld.w   $w0, ($t2) ;"
	    "	ld.w   $w1, 16($t2) ;"
	    "	ld.w   $w2, 32($t2) ;"
	    "	ld.w   $w3, 48($t2) ;"
	    "	ld.w   $w4, 64($t2) ;"
	    "	ld.w   $w5, 80($t2) ;"
	    "	st.w   $w0, ($t3) ;"
	    "	st.w   $w1, 16($t3) ;"
	    "	st.w   $w2, 32($t3) ;"
	    "	st.w   $w3, 48($t3) ;"
	    "	st.w   $w4, 64($t3) ;"
	    "	st.w   $w5, 80($t3) ;"

        "end:  "
	   : [dst] "=m" (*dst)
	   : [src] "m"  (*src), [cnt] "g" (cnt)
	   : "memory"
	);

	return;
}

void shaderproc_part_C(uint32_t* __restrict__ dst, const uint32_t * __restrict__ src, int cnt)
{

	int count4 = cnt >> 2;
	int fx = 0;
	int dx = 1;
	int i;
	for (i = 0; i < count4; ++i)
	{
		unsigned int src0 = src[fx]; fx += dx;
		unsigned int src1 = src[fx]; fx += dx;
		unsigned int src2 = src[fx]; fx += dx;
		unsigned int src3 = src[fx]; fx += dx;
	    dst[0] = src0;
	    dst[1] = src1;
	    dst[2] = src2;
	    dst[3] = src3;
	
		dst += 4;
	}

	for (i = (count4 << 2); i < cnt; ++i)
	{
		*dst++ = src[fx];
		fx += dx;
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

int testCase(char* idx , int count)
{

	uint32_t src1[255] = {10,11,12,13,14,15,16,17} ;
	uint32_t src2[255] = {7,6,5,4,3,2,1,0};
	uint32_t outMSA[255] = {7,6,5,4,3,2,1,0};
	uint32_t outC[255]   = {7,6,5,4,3,2,1,0};

	int i;   

	struct timeval st, end;
	long diff = 0;

	FILE * fd;
	char f1name[500] = {0};
	char f2name[500] = {0};
	char oname[500]  = {0};

	sprintf(f1name, "sp/p1data%s.raw", idx);
	sprintf(f2name, "sp/p2data%s.raw", idx);
	sprintf(oname,  "sp/opdata%s.raw", idx);

	fd = fopen(f1name, "rb");
	fread((void*)src1, 1, count*sizeof(uint32_t), fd);
	fclose(fd);
	fd = fopen(f2name, "rb");
	fread((void*)src2, 1, count*sizeof(uint32_t), fd);
	fclose(fd);

	memcpy((char*)outMSA, (char*)src2, count*sizeof(uint32_t));
	memcpy((char*)outC,   (char*)src2, count*sizeof(uint32_t));

    gettimeofday(&st, NULL);
	shaderproc_part(outMSA, src1, count);
	gettimeofday(&end,NULL);
    diff = end.tv_sec*1000000 + end.tv_usec - st.tv_sec*1000000 - st.tv_usec;
	sum_time(diff);
	printf("MSA code: tv_sec:%d tv_us:%d \n", total.tv_sec, total.tv_usec);

    gettimeofday(&st, NULL);
	shaderproc_part_C(outC, src1, count);
	gettimeofday(&end,NULL);
    diff = end.tv_sec*1000000 + end.tv_usec - st.tv_sec*1000000 - st.tv_usec;
	sum_time(diff);
	printf("C code: tv_sec:%d tv_us:%d \n", total.tv_sec, total.tv_usec);

	for(i = 0; i<count; i++)
	{
		if(outMSA[i] != outC[i]) printf(" %d:%x:%x:%x ", i, outMSA[i],outC[i],src1[i]);
	}
	printf("check shaderproc_part %s MSA & C complete. Count: %d\n", idx, i*4);

    memset(outC, 0, sizeof(uint32_t)*count);
    fd = fopen(oname, "rb");
    fread((void*) outC, 1, count*sizeof(uint32_t), fd);
    fclose(fd);

    for(i = 0; i<count; i++)
    {
        if(outMSA[i] != outC[i]) printf("%d:%x:%x:%x ", i,  outMSA[i],outC[i],src1[i]);
    }
    printf("check shaderproc_part %s MSA & TVOS complete. Count:%d \n", idx, i*4)	;

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
	//testCase("40", 28);
	//testCase("68", 254);
	return 0;
}


