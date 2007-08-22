// -----------------------------------------------------------------------------
//	InverseRIAAFilter.h - 
// -----------------------------------------------------------------------------

#ifndef __INVERSE_RIAA_FILTER_H__
#define __INVERSE_RIAA_FILTER_H__

struct stereo_float {
  static float l;
  static float r;
};

// Create one instance of this class and push new samples to the filter function
class InverseRIAAFilter
{
public:
	InverseRIAAFilter();
	~InverseRIAAFilter();

public:
	static void inv_riaa_filter(short* buffer, int size);

private:
	static stereo_float x[3];  // last three input values
	static stereo_float y[3];  // last three output values
};

#endif /*__INVERSE_RIAA_FILTER_H__*/