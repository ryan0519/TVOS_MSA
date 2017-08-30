
#include <string.h>
#include <stdio.h>
#include <sys/time.h>
#include <stdint.h>
#include "SkConvolver.h"
#include <assert.h>
#include <msa.h>
short mask_array[24] = {-1, 0, 0, 0, 0, 0, 0, 0,
                        -1, -1, 0, 0, 0, 0, 0, 0,
                        -1, -1, -1, 0, 0, 0, 0, 0};

inline unsigned char ClampTo8(int a) {
    if (static_cast<unsigned>(a) < 256) {
	    return a;  // Avoid the extra check in the common case.
    }
    if (a < 0) {
        return 0;
    }
    return 255;
}
void ConvolveHorizontally_MSA(const unsigned char* srcData, const SkConvolutionFilter1D& filter, unsigned char* outRow)
{
	int numValues = filter.numValues();
	int filterOffset, filterLength;
	uint32_t tmp[32]  = {0};
	uint32_t su[4];

	v4i32 zero4      = {0};
	v4i32 mask255 = {255, 255, 255, 255};
	v4i32 mask[4];
	
	mask[1] = __builtin_msa_ld_h(mask_array, 0);
	mask[2] = __builtin_msa_ld_h(mask_array, 16);
	mask[3] = __builtin_msa_ld_h(mask_array, 32);

	v4u32* s = reinterpret_cast< v4u32*>(tmp);
	v4u32* ss = reinterpret_cast< v4u32*>(su);
	
	for (int outX = 0; outX < numValues; outX++)
    {
	    
		const v4i32* rowToFilter = reinterpret_cast<const v4i32*>(filter.GetSrcAddr(outX));
		const SkConvolutionFilter1D::ConvolutionFixed* filterValues = filter.FilterForValue(outX, &filterLength);

		v4i32 accum4 = __builtin_msa_ldi_w(0);
		//const unsigned char* rowToFilter = &srcData[filterOffset * 4];
		int filterX;
        for ( filterX = 0; filterX < filterLength>>2 ; filterX++) {

			v4i32 coeff, sign, coeff_0, coeff_1, coeff_2, coeff_3;

			coeff = __builtin_msa_ld_h((void*)(filterValues), 0);
			sign = __builtin_msa_srai_h(coeff, 15);
			coeff = __builtin_msa_ilvr_h(sign, coeff);

			// [32] c0 c0 c0 c0
			coeff_0 = __builtin_msa_splati_w(coeff, 0);
			// [32] c1 c1 c1 c1
			coeff_1 = __builtin_msa_splati_w(coeff, 1);
			// [32] c2 c2 c2 c2
			coeff_2 = __builtin_msa_splati_w(coeff, 2);
			// [32] c3 c3 c3 c3
			coeff_3 = __builtin_msa_splati_w(coeff, 3);

			v4i32 src8 = __builtin_msa_ld_b((void*)rowToFilter, 0);
			// [16] a1 b1 g1 r1 a0 b0 g0 r0
			v4i32 src16_0 = __builtin_msa_ilvr_b(zero4, src8);
			// [32] a0 b0 g0 r0
			v4i32 src_0   = __builtin_msa_ilvr_h(zero4, src16_0);
			// [32] a1 b1 g1 r1
            v4i32 src_1   = __builtin_msa_ilvl_h(zero4, src16_0);

			v4i32 src16_1 = __builtin_msa_ilvl_b(zero4, src8);
			v4i32 src_2 = __builtin_msa_ilvr_h(zero4, src16_1);
			v4i32 src_3 = __builtin_msa_ilvl_h(zero4, src16_1);

			accum4 = __builtin_msa_maddv_w(accum4, src_0, coeff_0);
			accum4 = __builtin_msa_maddv_w(accum4, src_1, coeff_1);
			accum4 = __builtin_msa_maddv_w(accum4, src_2, coeff_2);	
			accum4 = __builtin_msa_maddv_w(accum4, src_3, coeff_3);
				
			filterValues += 4;
			rowToFilter += 1; 
		}

		int r = filterLength & 3;
		if(r >0)
		{
			v4i32 coeff, sign, coeff_0, coeff_1, coeff_2, coeff_3;
			coeff = __builtin_msa_ld_h((void*)(filterValues), 0);
			coeff = __builtin_msa_and_v(coeff, mask[r]);
			sign  = __builtin_msa_srai_h(coeff, 15);
			coeff = __builtin_msa_ilvr_h(sign, coeff);

			// [32] c0 c0 c0 c0
			coeff_0 = __builtin_msa_splati_w(coeff, 0);
			// [32] c1 c1 c1 c1
			coeff_1 = __builtin_msa_splati_w(coeff, 1);
			// [32] c2 c2 c2 c2
			coeff_2 = __builtin_msa_splati_w(coeff, 2);

			v16i8 src8 = __builtin_msa_ld_b((void*)rowToFilter, 0);
			// [16] a1 b1 g1 r1 a0 b0 g0 r0
			v4i32 src16_0 = __builtin_msa_ilvr_b(zero4, src8);
			// [32] a0 b0 g0 r0
			v4i32 src_0 = __builtin_msa_ilvr_h(zero4, src16_0);
			// [32] a1 b1 g1 r1
            v4i32 src_1 = __builtin_msa_ilvl_h(zero4, src16_0);
			v4i32 src16_1 = __builtin_msa_ilvl_b(zero4, src8);
			v4i32 src_2   = __builtin_msa_ilvr_h(zero4, src16_1);

			accum4 = __builtin_msa_maddv_w(accum4, src_0, coeff_0);
			accum4 = __builtin_msa_maddv_w(accum4, src_1, coeff_1);
			accum4 = __builtin_msa_maddv_w(accum4, src_2, coeff_2);	
		}

		accum4 = __builtin_msa_srai_w(accum4, SkConvolutionFilter1D::kShiftBits);	
		accum4 = __builtin_msa_max_s_w(accum4, zero4);	
		accum4 = __builtin_msa_min_s_w(accum4, mask255);	

		// Packing 32 bits |accum| to 16 bits per channel (signed saturation).
		accum4 = __builtin_msa_pckev_h( zero4, accum4);
		// Packing 16 bits |accum| to 8 bits per channel (unsigned saturation).
		accum4 = __builtin_msa_pckev_b(zero4, accum4);
		// Store the pixel value of 32 bits.
		*(reinterpret_cast<int*>(outRow)) = __builtin_msa_copy_s_w(accum4, 0);
		
		outRow += 4;

	}


	return ;
}
void ConvolveHorizontally_C(const unsigned char* srcData, const SkConvolutionFilter1D& filter, unsigned char* outRow)
{
	int filterLength;
	int num = 0; 
	int outX = 0;

	num = filter.numValues();

	for (outX = 0; outX < num; outX++)
    {
		int accum[4] = {0};
		const unsigned char* rowToFilter = filter.GetSrcAddr(outX);
	    const SkConvolutionFilter1D::ConvolutionFixed* filterValues = filter.FilterForValue(outX, &filterLength);
		//const unsigned char* rowToFilter = &srcData[filterOffset * 4];
		int filterX, curFilter;
        for (filterX = 0; filterX < filterLength; filterX++) {

	        curFilter = filterValues[filterX];

	        accum[0] += curFilter * rowToFilter[filterX * 4 + 0];
	        accum[1] += curFilter * rowToFilter[filterX * 4 + 1];
	        accum[2] += curFilter * rowToFilter[filterX * 4 + 2];
		    accum[3] += curFilter * rowToFilter[filterX * 4 + 3];
		}
		accum[0] >>= SkConvolutionFilter1D::kShiftBits;
        accum[1] >>= SkConvolutionFilter1D::kShiftBits;
        accum[2] >>= SkConvolutionFilter1D::kShiftBits;
        accum[3] >>= SkConvolutionFilter1D::kShiftBits;

		outRow[outX * 4 + 0] = ClampTo8(accum[0]);
        outRow[outX * 4 + 1] = ClampTo8(accum[1]);
		outRow[outX * 4 + 2] = ClampTo8(accum[2]);
		outRow[outX * 4 + 3] = ClampTo8(accum[3]);
	}
	return ;
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


typedef struct fvs
{
   int filterLength;
   int outx;
} fvs;

unsigned char* LoadData(SkConvolutionFilter1D* filter, char* idx, int* count)
{
	
    FILE * fd;
    char f1name[500] = {0};

	unsigned char out[1223] = {7,6,5,4,3,2,1,0};
	unsigned char * srcdata   = NULL;
	short * filterValues = NULL;
	unsigned char * target= NULL;

	fvs *data = NULL;
	
	int length = 0;
	int numv   = 0;
	int outx   = 0;
	int i;

	sprintf(f1name, "ch/cdata%s.raw", idx);

    fd = fopen(f1name, "rb");

    fread((void*)count, 1, 4, fd);

	numv = *count; 
	
	data   = (fvs *)malloc(sizeof(fvs) * numv);
	memset(data, 0, sizeof(fvs) * numv);
	target = (unsigned char *)malloc(sizeof(unsigned char) * numv * 4);
	memset(target, 0, sizeof(unsigned char) * numv * 4);

	for(i=0; i<numv; i++ ) 
	{
		fread(data+i, 1, sizeof(fvs), fd);


		srcdata = (unsigned char*)malloc(  data[i].filterLength*4 );
		fread((void*)srcdata, 1,  data[i].filterLength * 4, fd);
		
		filterValues = (short* )malloc( data[i].filterLength * sizeof(short));
		fread((void*)filterValues, 1, sizeof(short) *  data[i].filterLength, fd);
	
		fread((void*)out, 1, 4, fd);
		target[i * 4 + 0] = out[0];
		target[i * 4 + 1] = out[1];
		target[i * 4 + 2] = out[2];
		target[i * 4 + 3] = out[3];

		filter->AddFilter(i, filterValues, data[i].filterLength, srcdata);
		free(srcdata);
		free(filterValues);
	}
	printf("extract outx:%d i:%d numValue:%d \n", data[i].outx, i, numv);
	
    fclose(fd);
	return target;
}

int testCase(char* idx)
{
	unsigned char* outMSA = NULL;
	unsigned char* outC   = NULL;

	uint32_t i;
    int count;
	
	struct timeval st, end;
	long diff = 0;

    FILE * fd;

	SkConvolutionFilter1D filter;
	unsigned char* target = NULL;

	target = LoadData(&filter, idx, &count);
	assert(count > 0) ;
	assert(target != NULL) ;
	
	outMSA = (unsigned char*)malloc (sizeof(unsigned char) * count*4);
	outC   = (unsigned char*)malloc (sizeof(unsigned char) * count*4);
	memset(outMSA, 0, sizeof(unsigned char) * count*4);
	memset(outC  , 0, sizeof(unsigned char) * count*4);

	gettimeofday(&st, NULL);
	ConvolveHorizontally_MSA(NULL, filter, outMSA);
	gettimeofday(&end,NULL);
    diff = end.tv_sec*1000000 + end.tv_usec - st.tv_sec*1000000 - st.tv_usec;
	sum_time(diff);
	printf("MSA tv_sec:%d tv_us:%d \n", total.tv_sec, total.tv_usec);

    gettimeofday(&st, NULL);
	ConvolveHorizontally_C(NULL, filter, outC);
	gettimeofday(&end,NULL);
    diff = end.tv_sec*1000000 + end.tv_usec - st.tv_sec*1000000 - st.tv_usec;
	sum_time(diff);
	printf("C   tv_sec:%d tv_us:%d \n", total.tv_sec, total.tv_usec);

	for(i = 0; i<count*4; i++)
	{
		if(outMSA[i] != outC[i]) printf("%d:%x:%x ", i, outMSA[i], outC[i]);
	}
    printf("check ConvolveHorizontally %s MSA & C complete. Count: %d\n", idx, i);

    for(i = 0; i<count*4; i++)
    {
        if(outC[i] != target[i]) printf("%d:%x:%x ", i,  outC[i],target[i]);
	}
    printf("check ConvolveHorizontally %s C   & TVOS complete. Count:%d \n", idx, i) ;
	free(target);
	free(outMSA);
	free(outC);
	return 0;
}

void usage(char *fname)
{
	printf("Usage: %s <-idx num> <-count count> \n", fname);
	printf("       -idx   num     the file sufix ;\n");
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
	}

	testCase(idx);

	return 0;
}


