// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SK_CONVOLVER_H
#define SK_CONVOLVER_H

#include "SkTArray.h"


typedef short ConvolutionFixed;


// Represents a filter in one dimension. Each output pixel has one entry in this
// object for the filter values contributing to it. You build up the filter
// list by calling AddFilter for each output pixel (in order).
//
// We do 2-dimensional convolution by first convolving each row by one
// SkConvolutionFilter1D, then convolving each column by another one.
//
// Entries are stored in ConvolutionFixed point, shifted left by kShiftBits.
class SkConvolutionFilter1D {
public:
    typedef short ConvolutionFixed;

    // The number of bits that ConvolutionFixed point values are shifted by.
    enum { kShiftBits = 14 };

    SkConvolutionFilter1D();
    ~SkConvolutionFilter1D();

    // Returns the number of filters in this filter. This is the dimension of the
    // output image.
    int numValues() const { return static_cast<int>(fFilters.count()); }

    // Retrieves a filter for the given |valueOffset|, a position in the output
    // image in the direction we're convolving. The offset and length of the
    // filter values are put into the corresponding out arguments (see AddFilter
    // above for what these mean), and a pointer to the first scaling factor is
    // returned. There will be |filterLength| values in this array.
    inline const ConvolutionFixed* FilterForValue(int valueOffset,
                                       int* filterLength) const {
		const FilterInstance& filter = fFilters[valueOffset];
        //*filterOffsedt = filter.fOffset;
        *filterLength = filter.fTrimmedLength;
        if (filter.fTrimmedLength == 0) {
            return NULL;
        }
        return &fFilterValues[filter.fDataLocation-1];
    }
    inline const unsigned char* GetSrcAddr(int valueOffset) const {
		const FilterInstance& filter = fFilters[valueOffset];
		return  &filter.src[0];
	}
	void AddFilter(int filterOffset, const ConvolutionFixed* filterValues, int filterLength, unsigned char* src);

    // Add another value to the fFilterValues array -- useful for
    // SIMD padding which happens outside of this class.

    void addFilterValue( ConvolutionFixed val ) {
        fFilterValues.push_back( val );
    }
private:
    struct FilterInstance {
        // Offset within filterValues for this instance of the filter.
        int fDataLocation;

        // Distance from the left of the filter to the center. IN PIXELS
        int fOffset;

        // Number of values in this filter instance.
        int fTrimmedLength;

        // Filter length as specified. Note that this may be different from
        // 'trimmed_length' if leading/trailing zeros of the original floating
        // point form were clipped differently on each tail.
        int fLength;

		SkTArray<unsigned char> src;
    };

    // Stores the information for each filter added to this class.
    SkTArray<FilterInstance> fFilters;

    // We store all the filter values in this flat list, indexed by
    // |FilterInstance.data_location| to avoid the mallocs required for storing
    // each one separately.
    SkTArray<ConvolutionFixed> fFilterValues;

    // The maximum size of any filter we've added.
    int fMaxFilter;
};

SkConvolutionFilter1D::SkConvolutionFilter1D(): fMaxFilter(0) {
}   
    
SkConvolutionFilter1D::~SkConvolutionFilter1D() {
}

void SkConvolutionFilter1D::AddFilter(int filterOffset, const ConvolutionFixed* filterValues, int filterLength, unsigned char* src) {


	for (int i = 0; i <= filterLength; i++) {
		fFilterValues.push_back(filterValues[i]);
	}

	FilterInstance instance;

	// We pushed filterLength elements onto fFilterValues
	instance.fDataLocation = (static_cast<int>(fFilterValues.count()) - filterLength);

	instance.fOffset = filterOffset;

	instance.fTrimmedLength = filterLength;

	instance.fLength = filterLength;

	for(int i=0; i<filterLength*4; i++)
	{	
		instance.src.push_back(src[i]);
	}

	fFilters.push_back(instance);
}

#endif  // SK_CONVOLVER_H
