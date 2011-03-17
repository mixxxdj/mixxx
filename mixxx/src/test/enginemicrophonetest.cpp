#include <gtest/gtest.h>

#include <QtDebug>

#include "defs.h"
#include "configobject.h"
#include "sampleutil.h"
#include "soundmanagerutil.h"
#include "engine/enginemicrophone.h"

namespace {

class EngineMicrophoneTest : public testing::Test {
  protected:
    virtual void SetUp() {
        m_pMicrophone = new EngineMicrophone("[Microphone]");
    }

    virtual void TearDown() {
        delete m_pMicrophone;
    }

    void ClearBuffer(CSAMPLE* pBuffer, int length) {
        memset(pBuffer, 0, sizeof(pBuffer[0])*length);
    }

    template <typename T>
    void FillBuffer(T* pBuffer, T value, int length) {
        for (int i = 0; i < length; ++i) {
            pBuffer[i] = value;
        }
    }

    template <typename T>
    void FillSequentialWithStride(T* pBuffer, T initial, T increment, T max, unsigned int stride, int length) {
        Q_ASSERT(length % stride == 0);
        T value = initial;
        for (int i = 0; i < length/stride; ++i) {
            for (int j = 0; j < stride; ++j) {
                pBuffer[i*stride + j] = value;
            }
            value = value + increment;
            if (value > max)
                value = initial;
        }
    }

    void AssertWholeBufferEquals(const CSAMPLE* pBuffer, CSAMPLE value, int iBufferLen) {
        for (int i = 0; i < iBufferLen; ++i) {
            EXPECT_FLOAT_EQ(value, pBuffer[i]);
        }
    }

    template <typename T>
    void AssertBuffersEqual(const T* pBuffer, const T* pExpected, int iBufferLen) {
        for (int i = 0; i < iBufferLen; ++i) {
            EXPECT_FLOAT_EQ(pExpected[i], pBuffer[i]);
        }
    }

    EngineMicrophone* m_pMicrophone;
};

TEST_F(EngineMicrophoneTest, TestInputMatchesOutput) {
    short* input = new short[MAX_BUFFER_LEN/2];
    FillBuffer<short>(input, 1, MAX_BUFFER_LEN/2);

    CSAMPLE* output = SampleUtil::alloc(MAX_BUFFER_LEN);
    SampleUtil::applyGain(output, 0.0f, MAX_BUFFER_LEN);

    AudioInput micInput = AudioInput(AudioPath::MICROPHONE, 0, 0); // What should channelbase be?
    m_pMicrophone->receiveBuffer(micInput, input, MAX_BUFFER_LEN/2);

    m_pMicrophone->process(output, output, MAX_BUFFER_LEN);

    // Check that the output matches the input data.
    AssertWholeBufferEquals(output, 1.0f, MAX_BUFFER_LEN);

    SampleUtil::free(output);
}

TEST_F(EngineMicrophoneTest, TestRepeatedInputMatchesOutput) {
    short* input = new short[MAX_BUFFER_LEN/2];
    CSAMPLE* output = SampleUtil::alloc(MAX_BUFFER_LEN);
    CSAMPLE* test = SampleUtil::alloc(MAX_BUFFER_LEN);
    SampleUtil::applyGain(output, 0.0f, MAX_BUFFER_LEN);
    AudioInput micInput = AudioInput(AudioPath::MICROPHONE, 0, 0); // What should channelbase be?

    for (int i = 0; i < 10; i++) {
        // We have to limit it since short wraps around at 32768 and float
        // doesn't.
        FillSequentialWithStride<short>(input, 0, 1, 30000, 1, MAX_BUFFER_LEN/2);
        FillSequentialWithStride<CSAMPLE>(test, 0, 1.0f, 30000.0f, 2, MAX_BUFFER_LEN);

        m_pMicrophone->receiveBuffer(micInput, input, MAX_BUFFER_LEN/2);
        m_pMicrophone->process(output, output, MAX_BUFFER_LEN);

        // Check that the output matches the expected output
        AssertBuffersEqual<CSAMPLE>(output, test, MAX_BUFFER_LEN);
    }

    SampleUtil::free(output);
    SampleUtil::free(test);
}

}  // namespace
