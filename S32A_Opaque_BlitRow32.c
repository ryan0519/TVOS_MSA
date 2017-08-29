

#include <stdio.h>
#include <sys/time.h>
#include <stdint.h>
#include <string.h>

uint32_t SkAlphaMulQ(uint32_t c, unsigned scale) {
	uint32_t mask = 0xFF00FF;
	uint32_t rb = (((c & mask) * scale) & 0xffffffff ) >> 8;
	uint32_t ag = ((c >> 8) & mask)  * scale;
	return (rb & mask) | (ag & ~mask);
}
static uint32_t SkPMSrcOver(uint32_t src, uint32_t dst) {
    return src + SkAlphaMulQ(dst, 256 - (uint32_t)(src >> 24)  );
}
void S32A_Opaque_BlitRow32(uint32_t* __restrict__ dst, const uint32_t* __restrict__ src, int cnt, unsigned alpha) 
{

	int tail          = (cnt & 0x3) ;   // cnt %  4
	int tail_line     = cnt - tail;	
	int scaleD4[4];
	uint32_t mask4[4]      = {0xFF00FF, 0xFF00FF, 0xFF00FF, 0xFF00FF};
	uint32_t shift24[4]    = {24, 24, 24, 24};
	uint32_t sub4[4]       = {256, 256,256,256};
	uint32_t shift8[4]     = {8, 8, 8, 8};
	int i;
	for ( i = cnt-1; i >= tail_line ; i--)
    { 
		dst[i] = SkPMSrcOver(src[i], dst[i]);
	}

	if(tail_line <= 0) return;

	cnt -= tail;
	cnt <<= 2;
 
	__asm__ __volatile__(
		"	lw     $t0, %[cnt] ;"
		"   la     $t2, %[src] ;"     // t2 = & src
		"   la     $t3, %[dst] ;"     // t3 = & dst
		"   add    $t4, $t2, $t0 ;"   // t4 = t2 + cnt;
		"   add    $t5, $t3, $t0 ;"   // t5 = t3 + cnt;
		"   ld.w   $w7, %[shift24] ;" // shift24
		"   ld.w   $w6, %[mask4] ;"   // mask4
		"   ld.w   $w5, %[sub4] ;"    // sub4
		"   ld.w   $w4, %[shift8] ;"  // shift8;
		"   li     $t1, 16;"
		"2:"
		"	sub    $t4, $t4, $t1 ;"   // t4 = t4 - 16   
		"	sub    $t5, $t5, $t1 ;"   // t5 = t5 - 16

		"   ld.w   $w0, ($t4)   ;"    // src
		"   srl.w  $w1, $w0, $w7 ;"   // w1 = src >> 24
		"   subv.w $w1, $w5, $w1 ;"   // w1 = 256 - w1   :scale
		"   ld.w   $w2, ($t5) ;"      // dst
		"   and.v  $w3, $w2, $w6 ;"   // w3 = dst & mask
        "   mulv.w $w3, $w3, $w1 ;"   // w3 = w3 * scale
		"   srl.w  $w3, $w3, $w4 ;"   // w3 = w3 >> 8    :rb
		"   and.v  $w3, $w3, $w6 ;"   // w3 = w3 & mask  :rb & mask
		"   srl.w  $w2, $w2, $w4 ;"   // w2 = dst >> 8
		"   and.v  $w2, $w2, $w6 ;"   // w2 = w2 & mask
		"   mulv.w $w2, $w2, $w1 ;"   // w2 = w2 * scale :ag
		"   nori.b $w6, $w6, 0x0 ;"   // mask = ~mask
		"   and.v  $w2, $w2, $w6 ;"   // w2 = ag & ~mask
		"   or.v   $w2, $w2, $w3 ;"   // w2 = (rb & mask) | (ag & ~mask)
		"   addv.w $w0, $w0, $w2 ;"   // w0 = src + (rb & mask) | (ag & ~mask)
		"   nori.b $w6, $w6, 0x0 ;"   // mask = ~mask
	    "	st.w   $w0, ($t5) ;"
		"	bne    $t5, $t3, 2b  ;"
	   : [dst] "+m" (*dst)
	   : [cnt] "g" (cnt), [shift8] "m" (shift8), [sub4] "m" (sub4), 
		 [mask4] "m" (mask4), [src] "m" (*src), [shift24] "m" (shift24)
	);

	return ;
}
static void S32A_Opaque_BlitRow32_c(uint32_t* __restrict__ dst, const uint32_t* __restrict__ src, int count, unsigned alpha)
{
    if (count > 0)
    {
	    if (count & 1) {
		    *dst = SkPMSrcOver(*(src++), *dst);
		    dst += 1;
		    count -= 1;
		}

        const uint32_t* __restrict__ srcEnd = src + count;
        while (src != srcEnd) {
	        *dst = SkPMSrcOver(*(src++), *dst);
	        dst += 1;
	        *dst = SkPMSrcOver(*(src++), *dst);
	        dst += 1;
	    }
	}
	return ;
}

struct timeval total;

void Sum_Time(long incr)
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

int testCase(char* idx, int count )
{

	uint32_t ar1[1223] = {10,11,12,13,14,15,16,17} ;
	uint32_t ar2[1223] = {7,6,5,4,3,2,1,0};
	uint32_t outMSA[1223] ;
	uint32_t outC  [1223] ;

	int i;  

	int *src1 = ar1;
	int *src2 = ar2;

	struct timeval st, end;
	long diff = 0;

	FILE * fd;
	char f1name[500] = {0};
	char f2name[500] = {0};
	char oname[500]  = {0};

	sprintf(f1name, "br/s1data%s.raw", idx);
	sprintf(f2name, "br/s2data%s.raw", idx);
	sprintf(oname,  "br/odata%s.raw", idx);

	fd = fopen(f1name, "rb");
	fread((void*)src1, 1, count*sizeof(uint32_t), fd);
	fclose(fd);
	fd = fopen(f2name, "rb");
	fread((void*)src2, 1, count*sizeof(uint32_t), fd);
	fclose(fd);
	
	memcpy((char*)outMSA, (char*)src2, count*sizeof(uint32_t));
	memcpy((char*)outC,   (char*)src2, count*sizeof(uint32_t));

    gettimeofday(&st, NULL);
	S32A_Opaque_BlitRow32(outMSA, src1, count, 0xff);
	gettimeofday(&end,NULL);
    diff = end.tv_sec*1000000 + end.tv_usec - st.tv_sec*1000000 - st.tv_usec;
	Sum_Time(diff);
	printf("MSA tv_sec:%d tv_us:%d \n", total.tv_sec, total.tv_usec);
    
	gettimeofday(&st, NULL);
	S32A_Opaque_BlitRow32_c ( outC, src1, count, 0xff);
	gettimeofday(&end,NULL);
    diff = end.tv_sec*1000000 + end.tv_usec - st.tv_sec*1000000 - st.tv_usec;
	Sum_Time(diff);
	printf("C   tv_sec:%d tv_us:%d \n", total.tv_sec, total.tv_usec);
	
	for(i = 0; i<count; i++)
	{
		if(outMSA[i] != outC[i]) printf(" %d:%x:%x:%x ", i, outMSA[i],outC[i],ar2[i]);
	}
	printf("check S32A_Opaque_BlitRow32 %s MSA & C complete. Count: %d\n", idx, i*4);

	memset(outC, 0, sizeof(char)*4*count);
	fd = fopen(oname, "rb");
	fread((void*) outC, 1, count*4*sizeof(char), fd);
	fclose(fd);
	
	for(i = 0; i<count; i++)
	{
		if(outMSA[i] != outC[i]) printf("%d:%x:%x:%x ", i,  outMSA[i],outC[i],ar2[i]);
	}
	printf("check S32A_Opaque_BlitRow32 %s MSA & TVOS complete. Count:%d \n", idx, i*4);

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
//	testCase("1039", 87);
//	testCase("1068", 1223);
//	testCase("99", 7);
//	testCase("796", 1219);

	return 0;
}


