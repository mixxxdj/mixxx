/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */
/*
    QM DSP Library

    Centre for Digital Music, Queen Mary, University of London.
    This file 2005-2006 Christian Landone.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "ConstantQ.h"
#include "FFT.h"

#include <iostream>

#ifdef NOT_DEFINED
// see note in CQprecalc

#include "CQprecalc.cpp"

static bool push_precalculated(int uk, int fftlength,
                               std::vector<unsigned> &is,
                               std::vector<unsigned> &js,
                               std::vector<double> &real,
                               std::vector<double> &imag)
{
    if (uk == 76 && fftlength == 16384) {
        push_76_16384(is, js, real, imag);
        return true;
    }
    if (uk == 144 && fftlength == 4096) {
        push_144_4096(is, js, real, imag);
        return true;
    }
    if (uk == 65 && fftlength == 2048) {
        push_65_2048(is, js, real, imag);
        return true;
    }
    if (uk == 84 && fftlength == 65536) {
        push_84_65536(is, js, real, imag);
        return true;
    }
    return false;
}
#endif

//---------------------------------------------------------------------------
// nextpow2 returns the smallest integer n such that 2^n >= x.
static double nextpow2(double x) {
    double y = ceil(log(x)/log(2.0));
    return(y);
}

static double squaredModule(const double & xx, const double & yy) {
    return xx*xx + yy*yy;
}

//----------------------------------------------------------------------------

ConstantQ::ConstantQ( CQConfig Config ) :
    m_sparseKernel(0)
{
    initialise( Config );
}

ConstantQ::~ConstantQ()
{
    deInitialise();
}

//----------------------------------------------------------------------------
void ConstantQ::sparsekernel()
{
//    std::cerr << "ConstantQ: initialising sparse kernel, uK = " << m_uK << ", FFTLength = " << m_FFTLength << "...";

    SparseKernel *sk = new SparseKernel();

#ifdef NOT_DEFINED
    if (push_precalculated(m_uK, m_FFTLength,
                           sk->is, sk->js, sk->real, sk->imag)) {
//        std::cerr << "using precalculated kernel" << std::endl;
        m_sparseKernel = sk;
        return;
    }
#endif

    //generates spectral kernel matrix (upside down?)
    // initialise temporal kernel with zeros, twice length to deal w. complex numbers

    double* hammingWindowRe = new double [ m_FFTLength ];
    double* hammingWindowIm = new double [ m_FFTLength ];
    double* transfHammingWindowRe = new double [ m_FFTLength ];
    double* transfHammingWindowIm = new double [ m_FFTLength ];

    for (unsigned u=0; u < m_FFTLength; u++) 
    {
	hammingWindowRe[u] = 0;
	hammingWindowIm[u] = 0;
    }

    // Here, fftleng*2 is a guess of the number of sparse cells in the matrix
    // The matrix K x fftlength but the non-zero cells are an antialiased
    // square root function. So mostly is a line, with some grey point.
    sk->is.reserve( m_FFTLength*2 );
    sk->js.reserve( m_FFTLength*2 );
    sk->real.reserve( m_FFTLength*2 );
    sk->imag.reserve( m_FFTLength*2 );
	
    // for each bin value K, calculate temporal kernel, take its fft to
    //calculate the spectral kernel then threshold it to make it sparse and 
    //add it to the sparse kernels matrix
    double squareThreshold = m_CQThresh * m_CQThresh;

    FFT m_FFT(m_FFTLength);
	
    for (unsigned k = m_uK; k--; ) 
    {
        for (unsigned u=0; u < m_FFTLength; u++) 
        {
            hammingWindowRe[u] = 0;
            hammingWindowIm[u] = 0;
        }
        
	// Computing a hamming window
	const unsigned hammingLength = (int) ceil( m_dQ * m_FS / ( m_FMin * pow(2,((double)(k))/(double)m_BPO)));

        unsigned origin = m_FFTLength/2 - hammingLength/2;

	for (unsigned i=0; i<hammingLength; i++) 
	{
	    const double angle = 2*PI*m_dQ*i/hammingLength;
	    const double real = cos(angle);
	    const double imag = sin(angle);
	    const double absol = hamming(hammingLength, i)/hammingLength;
	    hammingWindowRe[ origin + i ] = absol*real;
	    hammingWindowIm[ origin + i ] = absol*imag;
	}

        for (unsigned i = 0; i < m_FFTLength/2; ++i) {
            double temp = hammingWindowRe[i];
            hammingWindowRe[i] = hammingWindowRe[i + m_FFTLength/2];
            hammingWindowRe[i + m_FFTLength/2] = temp;
            temp = hammingWindowIm[i];
            hammingWindowIm[i] = hammingWindowIm[i + m_FFTLength/2];
            hammingWindowIm[i + m_FFTLength/2] = temp;
        }
    
	//do fft of hammingWindow
	m_FFT.process( 0, hammingWindowRe, hammingWindowIm, transfHammingWindowRe, transfHammingWindowIm );

		
	for (unsigned j=0; j<( m_FFTLength ); j++) 
	{
	    // perform thresholding
	    const double squaredBin = squaredModule( transfHammingWindowRe[ j ], transfHammingWindowIm[ j ]);
	    if (squaredBin <= squareThreshold) continue;
		
	    // Insert non-zero position indexes, doubled because they are floats
	    sk->is.push_back(j);
	    sk->js.push_back(k);

	    // take conjugate, normalise and add to array sparkernel
	    sk->real.push_back( transfHammingWindowRe[ j ]/m_FFTLength);
	    sk->imag.push_back(-transfHammingWindowIm[ j ]/m_FFTLength);
	}

    }

    delete [] hammingWindowRe;
    delete [] hammingWindowIm;
    delete [] transfHammingWindowRe;
    delete [] transfHammingWindowIm;

/*
    using std::cout;
    using std::endl;

    cout.precision(28);

    int n = sk->is.size();
    int w = 8;
    cout << "static unsigned int sk_i_" << m_uK << "_" << m_FFTLength << "[" << n << "] = {" << endl;
    for (int i = 0; i < n; ++i) {
        if (i % w == 0) cout << "    ";
        cout << sk->is[i];
        if (i + 1 < n) cout << ", ";
        if (i % w == w-1) cout << endl;
    };
    if (n % w != 0) cout << endl;
    cout << "};" << endl;

    n = sk->js.size();
    cout << "static unsigned int sk_j_" << m_uK << "_" << m_FFTLength << "[" << n << "] = {" << endl;
    for (int i = 0; i < n; ++i) {
        if (i % w == 0) cout << "    ";
        cout << sk->js[i];
        if (i + 1 < n) cout << ", ";
        if (i % w == w-1) cout << endl;
    };
    if (n % w != 0) cout << endl;
    cout << "};" << endl;

    w = 2;
    n = sk->real.size();
    cout << "static double sk_real_" << m_uK << "_" << m_FFTLength << "[" << n << "] = {" << endl;
    for (int i = 0; i < n; ++i) {
        if (i % w == 0) cout << "    ";
        cout << sk->real[i];
        if (i + 1 < n) cout << ", ";
        if (i % w == w-1) cout << endl;
    };
    if (n % w != 0) cout << endl;
    cout << "};" << endl;

    n = sk->imag.size();
    cout << "static double sk_imag_" << m_uK << "_" << m_FFTLength << "[" << n << "] = {" << endl;
    for (int i = 0; i < n; ++i) {
        if (i % w == 0) cout << "    ";
        cout << sk->imag[i];
        if (i + 1 < n) cout << ", ";
        if (i % w == w-1) cout << endl;
    };
    if (n % w != 0) cout << endl;
    cout << "};" << endl;

    cout << "static void push_" << m_uK << "_" << m_FFTLength << "(vector<unsigned int> &is, vector<unsigned int> &js, vector<double> &real, vector<double> &imag)" << endl;
    cout << "{\n    is.reserve(" << n << ");\n";
    cout << "    js.reserve(" << n << ");\n";
    cout << "    real.reserve(" << n << ");\n";
    cout << "    imag.reserve(" << n << ");\n";
    cout << "    for (int i = 0; i < " << n << "; ++i) {" << endl;
    cout << "        is.push_back(sk_i_" << m_uK << "_" << m_FFTLength << "[i]);" << endl;
    cout << "        js.push_back(sk_j_" << m_uK << "_" << m_FFTLength << "[i]);" << endl;
    cout << "        real.push_back(sk_real_" << m_uK << "_" << m_FFTLength << "[i]);" << endl;
    cout << "        imag.push_back(sk_imag_" << m_uK << "_" << m_FFTLength << "[i]);" << endl;
    cout << "    }" << endl;
    cout << "}" << endl;
*/
//    std::cerr << "done\n -> is: " << sk->is.size() << ", js: " << sk->js.size() << ", reals: " << sk->real.size() << ", imags: " << sk->imag.size() << std::endl;
    
    m_sparseKernel = sk;
    return;
}

//-----------------------------------------------------------------------------
double* ConstantQ::process( const double* fftdata )
{
    if (!m_sparseKernel) {
        std::cerr << "ERROR: ConstantQ::process: Sparse kernel has not been initialised" << std::endl;
        return m_CQdata;
    }

    SparseKernel *sk = m_sparseKernel;

    for (unsigned row=0; row<2*m_uK; row++) 
    {
	m_CQdata[ row ] = 0;
	m_CQdata[ row+1 ] = 0;
    }
    const unsigned *fftbin = &(sk->is[0]);
    const unsigned *cqbin  = &(sk->js[0]);
    const double   *real   = &(sk->real[0]);
    const double   *imag   = &(sk->imag[0]);
    const unsigned int sparseCells = sk->real.size();
	
    for (unsigned i = 0; i<sparseCells; i++)
    {
	const unsigned row = cqbin[i];
	const unsigned col = fftbin[i];
	const double & r1  = real[i];
	const double & i1  = imag[i];
	const double & r2  = fftdata[ (2*m_FFTLength) - 2*col - 2 ];
	const double & i2  = fftdata[ (2*m_FFTLength) - 2*col - 2 + 1 ];
	// add the multiplication
	m_CQdata[ 2*row  ] += (r1*r2 - i1*i2);
	m_CQdata[ 2*row+1] += (r1*i2 + i1*r2);
    }

    return m_CQdata;
}


void ConstantQ::initialise( CQConfig Config )
{
    m_FS = Config.FS;
    m_FMin = Config.min;		// min freq
    m_FMax = Config.max;		// max freq
    m_BPO = Config.BPO;		// bins per octave
    m_CQThresh = Config.CQThresh;// ConstantQ threshold for kernel generation

    m_dQ = 1/(pow(2,(1/(double)m_BPO))-1);	// Work out Q value for Filter bank
    m_uK = (unsigned int) ceil(m_BPO * log(m_FMax/m_FMin)/log(2.0));	// No. of constant Q bins

//    std::cerr << "ConstantQ::initialise: rate = " << m_FS << ", fmin = " << m_FMin << ", fmax = " << m_FMax << ", bpo = " << m_BPO << ", K = " << m_uK << ", Q = " << m_dQ << std::endl;

    // work out length of fft required for this constant Q Filter bank
    m_FFTLength = (int) pow(2, nextpow2(ceil( m_dQ*m_FS/m_FMin )));

    m_hop = m_FFTLength/8; // <------ hop size is window length divided by 32

//    std::cerr << "ConstantQ::initialise: -> fft length = " << m_FFTLength << ", hop = " << m_hop << std::endl;

    // allocate memory for cqdata
    m_CQdata = new double [2*m_uK];
}

void ConstantQ::deInitialise()
{
    delete [] m_CQdata;
    delete m_sparseKernel;
}

void ConstantQ::process(const double *FFTRe, const double* FFTIm,
                        double *CQRe, double *CQIm)
{
    if (!m_sparseKernel) {
        std::cerr << "ERROR: ConstantQ::process: Sparse kernel has not been initialised" << std::endl;
        return;
    }

    SparseKernel *sk = m_sparseKernel;

    for (unsigned row=0; row<m_uK; row++) 
    {
	CQRe[ row ] = 0;
	CQIm[ row ] = 0;
    }

    const unsigned *fftbin = &(sk->is[0]);
    const unsigned *cqbin  = &(sk->js[0]);
    const double   *real   = &(sk->real[0]);
    const double   *imag   = &(sk->imag[0]);
    const unsigned int sparseCells = sk->real.size();
	
    for (unsigned i = 0; i<sparseCells; i++)
    {
	const unsigned row = cqbin[i];
	const unsigned col = fftbin[i];
	const double & r1  = real[i];
	const double & i1  = imag[i];
	const double & r2  = FFTRe[ m_FFTLength - col - 1 ];
	const double & i2  = FFTIm[ m_FFTLength - col - 1 ];
	// add the multiplication
	CQRe[ row ] += (r1*r2 - i1*i2);
	CQIm[ row ] += (r1*i2 + i1*r2);
    }
}
