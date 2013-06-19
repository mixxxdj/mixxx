#include <gtest/gtest.h>
#include <QtDebug>
#include <QList>
#include <QPair>

#include "sampleutil.h"

namespace {

class SampleUtilTest : public testing::Test {
  protected:
    virtual void SetUp() {
        SampleUtil::setOptimizations(false);
#ifdef __SSE__
        sseAvailable = SampleUtil::m_sOptimizationsOn ? 1 : 0;
#else
        sseAvailable = 0;
#endif
        sizes.append(1024);
        sizes.append(1025);
        sizes.append(1026);
        sizes.append(1027);
        sizes.append(1028);

        for (int i = 0; i < sizes.size(); ++i) {
            int size = sizes[i];
            CSAMPLE* buffer = new CSAMPLE[size];
            ClearBuffer(buffer, size);
            buffers.append(buffer);
            if (size % 2 == 0)
                evenBuffers.append(i);
        }
    }
    virtual void TearDown() {
        for (int i = 0; i < buffers.size(); ++i) {
            delete buffers[i];
        }
        buffers.clear();
        evenBuffers.clear();
        sizes.clear();
    }

    void ClearBuffer(CSAMPLE* pBuffer, int length) {
        memset(pBuffer, 0, sizeof(pBuffer[0])*length);
    }

    void FillBuffer(CSAMPLE* pBuffer, CSAMPLE value, int length) {
        for (int i = 0; i < length; ++i) {
            pBuffer[i] = value;
        }
    }

    void AssertWholeBufferEquals(CSAMPLE* pBuffer, CSAMPLE value, int iBufferLen) {
        for (int i = 0; i < iBufferLen; ++i) {
            EXPECT_FLOAT_EQ(pBuffer[i], value);
        }
    }

    QList<int> sizes;
    QList<CSAMPLE*> buffers;
    QList<int> evenBuffers;

    int sseAvailable;
};

TEST_F(SampleUtilTest, applyGain1DoesNothing) {
    while (sseAvailable-- >= 0) {
        for (int i = 0; i < buffers.size(); ++i) {
            CSAMPLE* buffer = buffers[i];
            int size = sizes[i];
            FillBuffer(buffer, 1.0f, size);
            SampleUtil::applyGain(buffer, 1.0, size);
            AssertWholeBufferEquals(buffer, 1.0, size);
        }

        SampleUtil::setOptimizations(true);
    }

}

TEST_F(SampleUtilTest, applyGain0ClearsBuffer) {
    while (sseAvailable-- >= 0) {
        for (int i = 0; i < buffers.size(); ++i) {
            CSAMPLE* buffer = buffers[i];
            int size = sizes[i];
            FillBuffer(buffer, 1.0f, size);
            SampleUtil::applyGain(buffer, 0.0, size);
            AssertWholeBufferEquals(buffer, 0.0, size);
        }
        SampleUtil::setOptimizations(true);
    }
}

TEST_F(SampleUtilTest, applyGain) {
    while (sseAvailable-- >= 0) {
        for (int i = 0; i < buffers.size(); ++i) {
            CSAMPLE* buffer = buffers[i];
            int size = sizes[i];
            FillBuffer(buffer, 1.0f, size);
            SampleUtil::applyGain(buffer, 0.5, size);
            AssertWholeBufferEquals(buffer, 0.5, size);
        }
        SampleUtil::setOptimizations(true);
    }
}

TEST_F(SampleUtilTest, applyAlternatingGain) {
    while (sseAvailable-- >= 0) {
        for (int i = 0; i < evenBuffers.size(); ++i) {
            int j = evenBuffers[i];
            CSAMPLE* buffer = buffers[j];
            int size = sizes[j];
            FillBuffer(buffer, 1.0f, size);
            SampleUtil::applyAlternatingGain(buffer, 0.5, -0.5, size);
            for (int s = 0; s < size; s += 2) {
                EXPECT_FLOAT_EQ(buffer[s], 0.5);
                EXPECT_FLOAT_EQ(buffer[s+1], -0.5);
            }
        }
        SampleUtil::setOptimizations(true);
    }
}

TEST_F(SampleUtilTest, addWithGain) {
    while (sseAvailable-- >= 0) {
        for (int i = 0; i < buffers.size(); ++i) {
            CSAMPLE* buffer = buffers[i];
            int size = sizes[i];
            FillBuffer(buffer, 1.0f, size);
            CSAMPLE* buffer2 = new CSAMPLE[size];
            FillBuffer(buffer2, 1.0f, size);
            SampleUtil::addWithGain(buffer, buffer2, 1.0, size);
            AssertWholeBufferEquals(buffer, 2.0f, size);
            SampleUtil::addWithGain(buffer, buffer2, 2.0, size);
            AssertWholeBufferEquals(buffer, 4.0f, size);
            delete buffer2;
        }
        SampleUtil::setOptimizations(true);
    }
}


TEST_F(SampleUtilTest, add2WithGain) {
    while (sseAvailable-- >= 0) {
        for (int i = 0; i < buffers.size(); ++i) {
            CSAMPLE* buffer = buffers[i];
            int size = sizes[i];
            FillBuffer(buffer, 1.0f, size);
            CSAMPLE* buffer2 = new CSAMPLE[size];
            FillBuffer(buffer2, 1.0f, size);
            CSAMPLE* buffer3 = new CSAMPLE[size];
            FillBuffer(buffer3, 1.0f, size);
            SampleUtil::add2WithGain(buffer,
                                     buffer2, 1.0,
                                     buffer3, 1.0,
                                     size);
            AssertWholeBufferEquals(buffer, 3.0f, size);
            SampleUtil::add2WithGain(buffer,
                                     buffer2, 2.0,
                                     buffer3, 3.0,
                                     size);
            AssertWholeBufferEquals(buffer, 8.0f, size);
            delete buffer2;
            delete buffer3;
        }
        SampleUtil::setOptimizations(true);
    }
}

TEST_F(SampleUtilTest, add3WithGain) {
    while (sseAvailable-- >= 0) {
        for (int i = 0; i < buffers.size(); ++i) {
            CSAMPLE* buffer = buffers[i];
            int size = sizes[i];
            FillBuffer(buffer, 1.0f, size);
            CSAMPLE* buffer2 = new CSAMPLE[size];
            FillBuffer(buffer2, 1.0f, size);
            CSAMPLE* buffer3 = new CSAMPLE[size];
            FillBuffer(buffer3, 1.0f, size);
            CSAMPLE* buffer4 = new CSAMPLE[size];
            FillBuffer(buffer4, 1.0f, size);
            SampleUtil::add3WithGain(buffer,
                                     buffer2, 1.0,
                                     buffer3, 1.0,
                                     buffer4, 1.0,
                                     size);
            AssertWholeBufferEquals(buffer, 4.0f, size);
            SampleUtil::add3WithGain(buffer,
                                     buffer2, 2.0,
                                     buffer3, 3.0,
                                     buffer4, 4.0,
                                     size);
            AssertWholeBufferEquals(buffer, 13.0f, size);
            delete buffer2;
            delete buffer3;
            delete buffer4;
        }
        SampleUtil::setOptimizations(true);
    }
}

TEST_F(SampleUtilTest, copyWithGain) {
    while (sseAvailable-- >= 0) {
        for (int i = 0; i < buffers.size(); ++i) {
            CSAMPLE* buffer = buffers[i];
            int size = sizes[i];
            FillBuffer(buffer, 1.0f, size);
            CSAMPLE* buffer2 = new CSAMPLE[size];
            FillBuffer(buffer2, 1.0f, size);
            SampleUtil::copyWithGain(buffer, buffer2, 1.0, size);
            AssertWholeBufferEquals(buffer, 1.0f, size);
            SampleUtil::copyWithGain(buffer, buffer2, 2.0, size);
            AssertWholeBufferEquals(buffer, 2.0f, size);
            delete buffer2;
        }
        SampleUtil::setOptimizations(true);
    }
}

TEST_F(SampleUtilTest, copyWithGainAliased) {
    while (sseAvailable-- >= 0) {
        for (int i = 0; i < buffers.size(); ++i) {
            CSAMPLE* buffer = buffers[i];
            int size = sizes[i];
            FillBuffer(buffer, 1.0f, size);
            SampleUtil::copyWithGain(buffer, buffer, 2.0, size);
            AssertWholeBufferEquals(buffer, 2.0f, size);
        }
        SampleUtil::setOptimizations(true);
    }
}

TEST_F(SampleUtilTest, copy2WithGain) {
    while (sseAvailable-- >= 0) {
        for (int i = 0; i < buffers.size(); ++i) {
            CSAMPLE* buffer = buffers[i];
            int size = sizes[i];
            FillBuffer(buffer, 1.0f, size);
            CSAMPLE* buffer2 = new CSAMPLE[size];
            FillBuffer(buffer2, 1.0f, size);
            CSAMPLE* buffer3 = new CSAMPLE[size];
            FillBuffer(buffer3, 1.0f, size);
            SampleUtil::copy2WithGain(buffer,
                                      buffer2, 1.0,
                                      buffer3, 1.0,
                                      size);
            AssertWholeBufferEquals(buffer, 2.0f, size);
            SampleUtil::copy2WithGain(buffer,
                                      buffer2, 2.0,
                                      buffer3, 3.0,
                                      size);
            AssertWholeBufferEquals(buffer, 5.0f, size);
            delete buffer2;
            delete buffer3;
        }
        SampleUtil::setOptimizations(true);
    }
}

TEST_F(SampleUtilTest, copy2WithGainAliased) {
    while (sseAvailable-- >= 0) {
        for (int i = 0; i < buffers.size(); ++i) {
            CSAMPLE* buffer = buffers[i];
            int size = sizes[i];
            FillBuffer(buffer, 1.0f, size);
            SampleUtil::copy2WithGain(buffer,
                                      buffer, 1.0,
                                      buffer, 1.0,
                                      size);
            AssertWholeBufferEquals(buffer, 2.0f, size);
            SampleUtil::copy2WithGain(buffer,
                                      buffer, 2.0,
                                      buffer, 3.0,
                                      size);
            AssertWholeBufferEquals(buffer, 10.0f, size);
        }
        SampleUtil::setOptimizations(true);
    }
}


TEST_F(SampleUtilTest, copy3WithGain) {
    while (sseAvailable-- >= 0) {
        for (int i = 0; i < buffers.size(); ++i) {
            CSAMPLE* buffer = buffers[i];
            int size = sizes[i];
            FillBuffer(buffer, 1.0f, size);
            CSAMPLE* buffer2 = new CSAMPLE[size];
            FillBuffer(buffer2, 1.0f, size);
            CSAMPLE* buffer3 = new CSAMPLE[size];
            FillBuffer(buffer3, 1.0f, size);
            CSAMPLE* buffer4 = new CSAMPLE[size];
            FillBuffer(buffer4, 1.0f, size);
            SampleUtil::copy3WithGain(buffer,
                                      buffer2, 1.0,
                                      buffer3, 1.0,
                                      buffer4, 1.0,
                                      size);
            AssertWholeBufferEquals(buffer, 3.0f, size);
            SampleUtil::copy3WithGain(buffer,
                                      buffer2, 2.0,
                                      buffer3, 3.0,
                                      buffer4, 4.0,
                                      size);
            AssertWholeBufferEquals(buffer, 9.0f, size);
            delete buffer2;
            delete buffer3;
            delete buffer4;
        }
        SampleUtil::setOptimizations(true);
    }
}

TEST_F(SampleUtilTest, copy3WithGainAliased) {
    while (sseAvailable-- >= 0) {
        for (int i = 0; i < buffers.size(); ++i) {
            CSAMPLE* buffer = buffers[i];
            int size = sizes[i];
            FillBuffer(buffer, 1.0f, size);
            SampleUtil::copy3WithGain(buffer,
                                      buffer, 1.0,
                                      buffer, 1.0,
                                      buffer, 1.0,
                                      size);
            AssertWholeBufferEquals(buffer, 3.0f, size);
            SampleUtil::copy3WithGain(buffer,
                                      buffer, 2.0,
                                      buffer, 3.0,
                                      buffer, 4.0,
                                      size);
            AssertWholeBufferEquals(buffer, 27.0f, size);
        }
        SampleUtil::setOptimizations(true);
    }
}

TEST_F(SampleUtilTest, convert) {
    while (sseAvailable-- >= 0) {
        for (int i = 0; i < buffers.size(); ++i) {
            CSAMPLE* buffer = buffers[i];
            int size = sizes[i];
            FillBuffer(buffer, 1.0f, size);
            SAMPLE* s16 = new SAMPLE[size];
            for (int j = 0; j < size; ++j) {
                s16[j] = j;
            }
            SampleUtil::convert(buffer, s16, size);
            for (int j = 0; j < size; ++j) {
                EXPECT_FLOAT_EQ(j, buffer[j]);
            }
            delete [] s16;
        }
        SampleUtil::setOptimizations(true);
    }
}

TEST_F(SampleUtilTest, sumAbsPerChannel) {
    while (sseAvailable-- >= 0) {
        for (int i = 0; i < evenBuffers.size(); ++i) {
            int j = evenBuffers[i];
            CSAMPLE* buffer = buffers[j];
            int size = sizes[j];
            FillBuffer(buffer, 1.0f, size);
            CSAMPLE fSumL = 0, fSumR = 0;
            SampleUtil::applyAlternatingGain(buffer, 1.0, 2.0, size);
            SampleUtil::sumAbsPerChannel(&fSumL, &fSumR, buffer, size);
            EXPECT_FLOAT_EQ(fSumL, size/2);
            EXPECT_FLOAT_EQ(fSumR, size);
        }
        SampleUtil::setOptimizations(true);
    }
}

TEST_F(SampleUtilTest, interleaveBuffer) {
    while (sseAvailable-- >= 0) {
        for (int i = 0; i < buffers.size(); ++i) {
            CSAMPLE* buffer = buffers[i];
            int size = sizes[i];
            FillBuffer(buffer, 0.0f, size);
            CSAMPLE* buffer2 = new CSAMPLE[size];
            FillBuffer(buffer2, 0.0f, size);
            for (int j = 0; j < size; j++) {
                buffer[j] = j;
                buffer2[j] = -j;
            }
            CSAMPLE* buffer3 = new CSAMPLE[size*2];
            FillBuffer(buffer3, 0.0f, size*2);
            SampleUtil::interleaveBuffer(buffer3, buffer, buffer2, size);

            for (int j = 0; j < size; j++) {
                EXPECT_FLOAT_EQ(buffer3[j*2], j);
                EXPECT_FLOAT_EQ(buffer3[j*2+1], -j);
            }
        }
        SampleUtil::setOptimizations(true);
    }
}

TEST_F(SampleUtilTest, deinterleaveBuffer) {
    while (sseAvailable-- >= 0) {
        for (int i = 0; i < buffers.size(); ++i) {
            CSAMPLE* buffer = buffers[i];
            int size = sizes[i];
            FillBuffer(buffer, 0.0f, size);
            CSAMPLE* buffer2 = new CSAMPLE[size];
            FillBuffer(buffer2, 0.0f, size);
            CSAMPLE* buffer3 = new CSAMPLE[size*2];
            FillBuffer(buffer3, 1.0f, size*2);
            for (int j = 0; j < size; j++) {
                buffer3[j*2] = j;
                buffer3[j*2+1] = -j;
            }
            SampleUtil::deinterleaveBuffer(buffer, buffer2, buffer3, size);

            for (int j = 0; j < size; j++) {
                EXPECT_FLOAT_EQ(buffer[j], j);
                EXPECT_FLOAT_EQ(buffer2[j], -j);
            }

            delete buffer2;
            delete buffer3;
        }
        SampleUtil::setOptimizations(true);
    }
}

TEST_F(SampleUtilTest, testBitwiseFabs) {
    // Test -10k to 10k
    for (int i = -10000; i < 10000; ++i) {
        float val = i;
        int iv = ((int&)val & 0x7FFFFFFF);
        val = (float&)iv;
        EXPECT_FLOAT_EQ(val, fabs(i));
    }
    // Test -1 to 1
    for (int i = -10000; i < 10000; ++i) {
        float val = float(i)/10000.0;
        int iv = ((int&)val & 0x7FFFFFFF);
        val = (float&)iv;
        EXPECT_FLOAT_EQ(val, fabs(float(i)/10000.0));
    }
}

}
