/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */
/*
    QM DSP Library

    Centre for Digital Music, Queen Mary, University of London.
    This file 2005-2006 Christian Landone and Katy Noland.

    Fixes to correct chroma offsets and for thread safety contributed
    by Daniel Schürmann.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "GetKeyMode.h"

#include "dsp/rateconversion/Decimator.h"
#include "dsp/chromagram/Chromagram.h"

#include "maths/MathUtilities.h"
#include "base/Pitch.h"

#include <iostream>

#include <cstring>
#include <cstdlib>

static const int kBinsPerOctave = 36;

// Theory: A major third chord consists of a 1st, 3rd, and 5th degrees of a major scale
// while a minor chord is made from the 1st, flatted 3rd, and 5th degrees of a
// major scale.
// The values below are multiplied with the power value of each note in the chroma.
// The values are set in a way to also detect power chords and single notes.
//          c   c#    D   D#    E   F   F#   G    G#   A   A#   B
// Third:  2.4 -1.0 -0.5 -0.3  0.2  1 -1.5  0.7    0  0.2 -0,5 -1
// Power:  1.7 -1.0  0.7 -1.0  0.7  1 -1.0  0.7 -1.0  0.7 -0,5 -1
static const double m_MajorChord[12] =
{ 1.0, -0.5,  0.0, -0.5,  0.7,  0.0, -0.5,  0.7, -0.5,  0.0, -0.5,  0.0};

static const double m_MinorChord[12] =
{ 1.0, -0.5,  0.0,  0.7, -0.5,  0.0, -0.5,  0.7,  0.0, -0.5,  0.0, -0.5};


// The chords of C major progressions
// I   C  Tonic
// ii  Dm
// iii Em
// IV  F  Subdominant
// V   G  Dominant
// vi  A
// vii°Bdim (not detected)
static const double m_ChordToMajorKey[24] =
{ 1.8, -1.0, -1.0, -1.0, -1.0,  1.4, -1.0,  1.4, -1.0, -1.0, -1.0,  0.0,  // Major Chords
 -1.0, -1.0,  0.5, -1.0,  0.5, -1.0, -1.0, -1.0, -1.0,  0.5, -1.0, -1.0}; // minor Chords


// The chords of C minor progressions
// i   Cm  Tonic
// ii° Ddim (not detected)
// III Eb
// iv  Fm  Subdominant
// v   Gm  Dominant
// VI  Ab
// VII Bb/B
// Node: Major chords are used often even in a Minor track, so we weight a minor chord higher
static const double m_ChordToMinorKey[24] =
{-1.0, -1.0, -1.0,  0.5, -1.0, -1.0, -1.0, -1.0,  0.5, -1.0,  0.5,  0.5,  // Major Chords
  2.0, -1.0,  0.5, -1.0, -1.0,  1.5, -1.0,  1.5, -1.0, -1.0, -1.0, -1.0}; // minor Chords

// For power chords and single notes
static const double m_NoteToKey[12] =
{ 0.1, -1.0,  0.0, -1.0,  0.0,  0.0, -1.0,  0.0, -1.0,  0.0, -1.0,  0.0};




//static const double m_MajorScale[12] =
//{ 0.0, -1.0,  0.0, -1.0,  0.0,  0.0, -1.0,  0.0, -1.0,  0.0, -1.0,  0.0};

//static const double m_MajorGypsyScale[12] =
//{ 0.0,  0.0, -1.0, -1.0,  0.0,  0.0, -1.0,  0.0,  0.0, -1.0, -1.0,  0.0};

static const double m_MinorScale[12] =
{ 0.0, -1.0,  0.0,  0.0, -1.0,  0.0, -1.0,  0.0,  0.0, -1.0,  0.0, -1.0};

static const double m_MinorHarmonicScale[12] =
{ 0.0, -1.0,  0.0,  0.0, -1.0,  0.0, -1.0,  0.0,  0.0, -1.0, -1.0,  0.0};

static const double m_MinorMelodicScale[12] =
{ 0.0, -1.0,  0.0,  0.0, -1.0,  0.0, -1.0,  0.0, -1.0,  0.0, -1.0,  0.0};

static const double m_MinorGypsyScale[12] =
{ 0.0, -1.0,  0.0,  0.0, -1.0, -1.0,  0.0,  0.0,  0.0, -1.0, -1.0,  0.0};





//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

GetKeyMode::GetKeyMode(Config config) :
    m_hpcpAverage(config.hpcpAverage),
    m_medianAverage(config.medianAverage),
    m_decimationFactor(config.decimationFactor),
    m_chrPointer(0),
    m_decimatedBuffer(0),
    m_chromaBuffer(0),
    m_meanHPCP(0),
    m_inTuneChroma(0),
    m_maxTuneSumme(0),
    m_majCorr(0),
    m_minCorr(0),
    m_medianFilterBuffer(0),
    m_sortedBuffer(0),
    m_keyStrengths(0),
    m_processCall(0),
    m_secondsPerProcessCall(0.0)
{
    ChromaConfig chromaConfig;
    
    // Chromagram configuration parameters
    chromaConfig.normalise = MathUtilities::NormaliseNone;
    chromaConfig.FS = config.sampleRate / (double)m_decimationFactor;
    if (chromaConfig.FS < 1) {
        chromaConfig.FS = 1;
    }

    // Set C3 (= MIDI #48) as our base:
    // This implies that key = 1 => Cmaj, key = 12 => Bmaj, key = 13 => Cmin, etc.
    const float centsOffset = -12.0f / kBinsPerOctave * 100; // 3 bins per note, start with the first
    chromaConfig.min =
        Pitch::getFrequencyForPitch( 48, centsOffset, config.tuningFrequency );
    chromaConfig.max =
        Pitch::getFrequencyForPitch( 96, centsOffset, config.tuningFrequency );

    chromaConfig.BPO = kBinsPerOctave;
    chromaConfig.CQThresh = 0.0054;

    // Chromagram inst.
    m_chroma = new Chromagram(chromaConfig);

    // Get calculated parameters from chroma object
    m_chromaFrameSize = m_chroma->getFrameSize();
    m_chromaHopSize = m_chroma->getHopSize();
    m_secondsPerProcessCall = m_chromaHopSize / chromaConfig.FS;

//    std::cerr << "chroma frame size = " << m_ChromaFrameSize << ", decimation factor = " << m_DecimationFactor << " therefore block size = " << getBlockSize() << std::endl;

    // Chromagram average and estimated key median filter lengths
    m_chromaBufferSize = 1; // (int)ceil( m_hpcpAverage * m_ChromaConfig.FS/m_ChromaFrameSize );
    m_medianWinSize = (int)ceil(m_medianAverage * chromaConfig.FS / m_chromaFrameSize);
    
    // Reset counters
    m_bufferIndex = 0;
    m_chromaBufferFilling = 0;
    m_medianBufferFilling = 0;

    // Spawn objectc/arrays
    m_decimatedBuffer = new double[m_chromaFrameSize];
    
    m_chromaBuffer = new double[kBinsPerOctave * m_chromaBufferSize];
    memset(m_chromaBuffer, 0, sizeof(double) * kBinsPerOctave * m_chromaBufferSize);
    
    m_meanHPCP = new double[kBinsPerOctave];
    m_inTuneChroma = new double[kBinsPerOctave / 3];
    
    m_majCorr = new double[kBinsPerOctave];
    m_minCorr = new double[kBinsPerOctave];
    
    m_medianFilterBuffer = new int[m_medianWinSize];
    memset( m_medianFilterBuffer, 0, sizeof(int)*m_medianWinSize);
    
    m_sortedBuffer = new int[ m_medianWinSize ];
    memset( m_sortedBuffer, 0, sizeof(int)*m_medianWinSize);
    
    m_decimator = new Decimator( m_chromaFrameSize*m_decimationFactor, m_decimationFactor );

    m_keyStrengths = new double[24];

    for (unsigned int k = 0; k < 25; k++) {
        m_ProgressionProbability[k] = 0;
    }

    for (unsigned int k = 0; k < 48; k++) {
        m_ScaleProbability[k] = 0;
    }
}

GetKeyMode::~GetKeyMode()
{
    delete m_chroma;
    delete m_decimator;
    
    delete [] m_decimatedBuffer;
    delete [] m_chromaBuffer;
    delete [] m_meanHPCP;
    delete [] m_inTuneChroma;
    delete [] m_majCorr;
    delete [] m_minCorr;
    delete [] m_medianFilterBuffer;
    delete [] m_sortedBuffer;
    delete [] m_keyStrengths;
}

int GetKeyMode::process(double *pcmData)
{
    m_processCall++;

    /*
    // skip a part
    if (m_processCall < 1581) {
        return 0;
    }
    */

    //////////////////////////////////////////////
    m_decimator->process(pcmData, m_decimatedBuffer);

    m_chrPointer = m_chroma->process(m_decimatedBuffer);

    double maxNoteValue;
    MathUtilities::getMax(m_chrPointer, kBinsPerOctave, &maxNoteValue);

    /*
    std::cout << "raw chroma: ";
    for (int ii = 0; ii < kBinsPerOctave; ++ii) {
      if (ii % (kBinsPerOctave/12) == 0) std::cout << "\n";
        std::cout << m_ChrPointer[ii] << " ";
    }
    std::cout << std::endl;
    */

    // populate hpcp values;
    int cbidx;
    for (int j = 0; j < kBinsPerOctave; j++) {
        cbidx = (m_bufferIndex * kBinsPerOctave) + j;
        m_chromaBuffer[ cbidx ] = m_chrPointer[j];
    }

    // keep track of input buffers;
    if (m_bufferIndex++ >= m_chromaBufferSize - 1) {
        m_bufferIndex = 0;
    }

    // track filling of chroma matrix
    if (m_chromaBufferFilling++ >= m_chromaBufferSize) {
        m_chromaBufferFilling = m_chromaBufferSize;
    }

    // calculate mean
    for (int k = 0; k < kBinsPerOctave; k++) {
        double mnVal = 0.0;
        for (int j = 0; j < m_chromaBufferFilling; j++) {
            mnVal += m_chromaBuffer[ k + (j * kBinsPerOctave) ];
        }

        m_meanHPCP[k] = mnVal / (double)m_chromaBufferFilling;
    }

    // Normalize for zero average
    double mHPCP = MathUtilities::mean(m_meanHPCP, kBinsPerOctave );
    for (int k = 0; k < kBinsPerOctave; k++ ) {
        m_meanHPCP[k] -= mHPCP;
        m_meanHPCP[k] /= (maxNoteValue - mHPCP);
    }

    std::cout << " ";

    // Original Chroma
    for (unsigned int ii = 0; ii < kBinsPerOctave; ++ii) {
      double value = m_meanHPCP[ii];
      if (value > 0 && maxNoteValue > 0.01) {
          if (value > 0.99) {
              std::cout << "Î";
          } else if (value > 0.66) {
              std::cout << "I";
          } else if (value > 0.33) {
              std::cout << "i";
          } else {
              std::cout << ";";
          }
      }
      else
      {
          if (ii == 3 || ii == 9  || ii == 18 ||  ii == 24 || ii == 30 ||
              ii == 4 || ii == 10 || ii == 19 ||  ii == 25 || ii == 31 ||
              ii == 5 || ii == 11 || ii == 20 ||  ii == 26 || ii == 32) {
              // Mark black keys
              std::cout << "-";
          }
          else {
              std::cout << "_";
          }
      }
      if (ii % 3 == 2) {
          std::cout << " ";
      }
    }

    double maxTunedValue = 0;

    // Use onley notes in tune
    for (unsigned int ii = 0; ii < kBinsPerOctave / 3; ++ii) {
      unsigned int center = ii * 3 + 1;
      double value = m_meanHPCP[center];
      double flat = m_meanHPCP[center - 1];
      double sharp = m_meanHPCP[center + 1];
      if (value > 0.25 && maxNoteValue > 0.01
              && value > flat && value > sharp) {
          if (value > 0.99) {
              std::cout << "Î";
          } else if (value > 0.75) {
              std::cout << "I";
          } else if (value > 0.5) {
              std::cout << "i";
          } else {
              std::cout << ";";
          }
          m_inTuneChroma[ii] = value;
          if  (maxTunedValue < value * maxNoteValue) {
              maxTunedValue = value * maxNoteValue;
          }
      }
      else
      {
          if (ii == 1 || ii == 3  || ii == 6 ||  ii == 8 || ii == 10) {
              // Mark black keys
              std::cout << "-";
          }
          else {
              std::cout << "_";
          }
          m_inTuneChroma[ii] = 0;
      }
      std::cout << " ";
    }

    std::cout << maxTunedValue << " ";

    const double smooth = 1.0-1.0/172.0; // For T of 16 s -> two frames at 120 BPM
    for (int k = 0; k < 12; k++) {
        double sumMinor = 0;
        double sumMinorMelodic = 0;
        double sumMinorHarmonic = 0;
        double sumMinorGypsy = 0;


        for (unsigned int i = 0; i <12; i++) {
            int j = (i - k + 12 + 3) % 12;

            sumMinor += m_inTuneChroma[i] * m_MinorScale[j];
            sumMinorMelodic += m_inTuneChroma[i] * m_MinorMelodicScale[j];
            sumMinorHarmonic += m_inTuneChroma[i] * m_MinorHarmonicScale[j];
            sumMinorGypsy += m_inTuneChroma[i] * m_MinorGypsyScale[j];
        }

        m_ScaleProbability[k] = m_ScaleProbability[k] * smooth + sumMinor * maxTunedValue;
        m_ScaleProbability[k+12] = m_ScaleProbability[k+12] * smooth + sumMinorMelodic * maxTunedValue;
        m_ScaleProbability[k+24] = m_ScaleProbability[k+24] * smooth + sumMinorHarmonic * maxTunedValue;
        m_ScaleProbability[k+36] = m_ScaleProbability[k+36] * smooth + sumMinorGypsy * maxTunedValue;

        m_maxTuneSumme = m_maxTuneSumme * smooth - maxTunedValue;
    }

    // Limit maximum unlikeness to be likely again within 16 s
    for (int k = 0; k < 48; k++) {
        if (m_ScaleProbability[k] < m_maxTuneSumme / 2) {
            m_ScaleProbability[k] = m_maxTuneSumme / 2;
        }
    }

    double maxScaleValue;
    int scale = MathUtilities::getMax( m_ScaleProbability, 48, &maxScaleValue ) + 1;

    if (maxScaleValue < m_maxTuneSumme / 6) {
        // 6 was adjusted from the undetectable outro of "Green Day" - "Boulevard of Broken Dreams"
        scale = 0;
    }
    // std::cout  << " " << scale << " " << maxScaleValue;
    //std::cout << " " << m_maxTuneSumme;
    //std::cout << "  ###";
    //std::cout << " " << m_ScaleProbability[1-1];
    //std::cout << " " << m_ScaleProbability[2-1];
    //std::cout << " " << m_ScaleProbability[3-1];
    //std::cout << " " << m_ScaleProbability[4-1];
    //std::cout << " " << m_ScaleProbability[5-1];
    //std::cout << " " << m_ScaleProbability[6-1];
    //std::cout << " " << m_ScaleProbability[7-1];
    //std::cout << " " << m_ScaleProbability[8-1];
    //std::cout << " " << m_ScaleProbability[9-1];
    //std::cout << " " << m_ScaleProbability[10-1];
    //std::cout << " " << m_ScaleProbability[11-1];
    //std::cout << " " << m_ScaleProbability[12-1];
    //std::cout << " " << m_ScaleProbability[16-1];
    //std::cout << " " << m_ScaleProbability[18-1];
    //std::cout << "  ###";

    /*
    std::cout << std::endl;
    for( k = 0; k < 12; k++ )
    {
        std::cout << " " << m_ScaleProbability[k] << " " << m_ScaleProbability[k+12] << " " << m_ScaleProbability[k+24] << " " << m_ScaleProbability[k+36] << std::endl;
    }

    std::cout << " " << m_maxTuneSumme / 4 << std::endl;
    */

    /*
    std::cout << std::endl;
    for( k = 0; k < 12; k++ )
    {
        std::cout << " " << m_ScaleProbability[k] << " " << m_ScaleProbability[k+12] << " " << m_ScaleProbability[k+24] << " " << m_ScaleProbability[k+36] << std::endl;
    }

    std::cout << " " << m_maxTuneSumme / 4 << std::endl;
    */


    double maxChordValue = 0;
    unsigned int maxChord = 0;

    for (int k = 0; k < 12; k++) {
        m_majCorr[k] = 0;
        m_minCorr[k] = 0;

        for (unsigned int i = 0; i <12; i++) {
            int j = (i - k + 12) % 12;

            m_majCorr[k] += m_inTuneChroma[i] * m_MajorChord[j];
            m_minCorr[k] += m_inTuneChroma[i] * m_MinorChord[j];
        }
        if (m_majCorr[k] > m_minCorr[k]) {
            if (maxChordValue < m_majCorr[k]) {
                maxChordValue = m_majCorr[k];
                maxChord = k + 1;
            }
        } else if (m_majCorr[k] < m_minCorr[k]) {
            if (maxChordValue < m_minCorr[k]) {
                maxChordValue = m_minCorr[k];
                maxChord = k + 1 + 12;
            }
        } else {
            // Single Note or no Min/Maj Chord
            if (maxChordValue < m_majCorr[k]) {
                maxChordValue = m_majCorr[k];
                maxChord = k + 1 + 24;
            }
        }
    }

    const double smoothing = 1.0 - 1.0 / 172.0; // For T of 16 s -> two frames at 120 BPM
    if (maxChord) {
        for (unsigned int k = 0; k < 12; k++) {
            if (maxChord <= 12) {
                m_ProgressionProbability[k+1] = m_ProgressionProbability[k+1] * smoothing +
                        maxTunedValue * (m_ChordToMajorKey[(maxChord-1-k+12) % 12]);
            } else if (maxChord <= 24) {
                m_ProgressionProbability[k+1] = m_ProgressionProbability[k+1] * smoothing +
                        maxTunedValue * (m_ChordToMajorKey[(maxChord-1-k+12) % 12 + 12]);
            } else {
                m_ProgressionProbability[k+1] = m_ProgressionProbability[k+1] * smoothing +
                        maxTunedValue * (m_NoteToKey[(maxChord-1-k+12) % 12 ]);
            }
        }
        for (unsigned int k = 12; k < 24; k++) {
            if (maxChord <= 12) {
                m_ProgressionProbability[k+1] = m_ProgressionProbability[k+1] * smoothing +
                        maxTunedValue * (m_ChordToMinorKey[(maxChord-1-k+24) % 12]);
            } else if (maxChord <= 24) {
                m_ProgressionProbability[k+1] = m_ProgressionProbability[k+1] * smoothing +
                        maxTunedValue * (m_ChordToMinorKey[(maxChord-1-k+24) % 12 + 12]);
            } else {
                m_ProgressionProbability[k+1] = m_ProgressionProbability[k+1] * smoothing +
                        maxTunedValue * (m_NoteToKey[(maxChord-1-k+24+9) % 12 ]);
            }

        }
        // unknown
        m_ProgressionProbability[0] = m_ProgressionProbability[0] * smoothing - maxTunedValue;
    } else {
        // unknown
        maxTunedValue = maxNoteValue;
        m_ProgressionProbability[0] = m_ProgressionProbability[0] * smoothing + maxTunedValue;
    }

    /*
    std::cout << "  ###";
    for( unsigned int k = 0; k < 25; k++ ) {
        m_ScaleProbability[1-1]
        std::cout << " " << m_ProgressionProbability[k];
    }
    std::cout << "  ###";
    */



    //for( unsigned int k = 0; k < 25; k++ )
    //{
    //    std::cout << "/" << m_KeyProbability[k];
    //}
    //d1 = (m_KeyProbability[1] - d1) / maxTunedValue;
    //d22 = (m_KeyProbability[22] - d22) / maxTunedValue;
    //std::cout << " " << d1 << " " << d22;
    //std::cout << " " << m_KeyProbability[4] << " " << m_KeyProbability[13];


    double dummy;
    unsigned int progression = MathUtilities::getMax(m_ProgressionProbability, 25, &dummy);

    unsigned int key = 0;
    if (scale > 0 && scale < 12 && progression > 0) {
        // we can only return western keys, sorry.
        double max = -100;
        for( unsigned int k = 0; k < 12; k++ ) {
            double value = m_ScaleProbability[k];
            if (progression <= 12) {
                if (progression - 1 == k) {
                    // Consider the scale of the detected progression as more likely
                    // This means we allow more of scale notes if the progrssion still matches
                    value -= m_maxTuneSumme / 60;  // m_maxTuneSumme is negative
                }
            } else {
                if ((progression + 2) % 12 == k) {
                    // Consider the scale of the detected progression as more likely
                    // This means we allow more of scale notes if the progrssion still matches
                    value -= m_maxTuneSumme / 60;  // m_maxTuneSumme is negative
                }
            }
            if (value > max) {
                key = k + 1;
                max = value;
            }
        }
        unsigned int minorKey = ((key + 8) % 12) + 13;
        if (m_ProgressionProbability[key] < m_ProgressionProbability[minorKey]) {
            key = minorKey;
        }
    }

    std::cout << " " << maxChord << " " << progression << " " << scale << " " << key << " "
            << (m_processCall - 1) * m_secondsPerProcessCall << std::endl;

    // return maxChord < 25 ? maxChord : 0;
    return key;
}

unsigned int getChromaSize() 
{ 
    return kBinsPerOctave; 
}

double* GetKeyMode::getKeyStrengths()
{
    int k;

    for (k = 0; k < 24; ++k) {
        m_keyStrengths[k] = 0;
    }

    for (k = 0; k < kBinsPerOctave; k++) {
        int idx = k / (kBinsPerOctave/12);
        int rem = k % (kBinsPerOctave/12);
        if (rem == 0 || m_majCorr[k] > m_keyStrengths[idx]) {
            m_keyStrengths[idx] = m_majCorr[k];
        }
    }

    for (k = 0; k < kBinsPerOctave; k++) {
        int idx = (k + kBinsPerOctave) / (kBinsPerOctave/12);
        int rem = k % (kBinsPerOctave/12);
        if (rem == 0 || m_minCorr[k] > m_keyStrengths[idx]) {
            m_keyStrengths[idx] = m_minCorr[k];
        }
    }

/*
    std::cout << "key strengths: ";
    for (int ii = 0; ii < 24; ++ii) {
        if (ii % 6 == 0) std::cout << "\n";
        std::cout << m_keyStrengths[ii] << " ";
    }
    std::cout << std::endl;
*/

    return m_keyStrengths;
}
