#include <gtest/gtest.h>

#include <QtDebug>

#include "defs.h"
#include "configobject.h"
#include "controlobject.h"
#include "sampleutil.h"
#include "soundmanagerutil.h"
#include "engine/enginemicrophone.h"

namespace {

class EngineMicrophoneTest : public testing::Test {
  protected:
    virtual void SetUp() {
        inputLength = MAX_BUFFER_LEN/2;
        outputLength = MAX_BUFFER_LEN;
        input = SampleUtil::alloc(inputLength);
        output = SampleUtil::alloc(outputLength);
        test = SampleUtil::alloc(outputLength);

        m_pMicrophone = new EngineMicrophone("[Microphone]");
        m_pTalkover = ControlObject::getControl(ConfigKey("[Microphone]", "talkover"));
    }

    virtual void TearDown() {
        SampleUtil::free(input);
        SampleUtil::free(output);
        SampleUtil::free(test);
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
    void FillSequentialWithStride(T* pBuffer, T initial, T increment, T max,
                                  unsigned int stride, unsigned int length) {
        ASSERT_EQ(0, length % stride);
        T value = initial;
        for (unsigned int i = 0; i < length/stride; ++i) {
            for (unsigned int j = 0; j < stride; ++j) {
                pBuffer[i*stride + j] = value;
            }
            value = value + increment;
            if (value > max)
                value = initial;
        }
    }

    void AssertWholeBufferEquals(const CSAMPLE* pBuffer, CSAMPLE value, int iBufferLen) {
        for (int i = 0; i < iBufferLen; ++i) {
            ASSERT_FLOAT_EQ(value, pBuffer[i]);
        }
    }

    template <typename T>
    void AssertBuffersEqual(const T* pBuffer, const T* pExpected, int iBufferLen) {
        for (int i = 0; i < iBufferLen; ++i) {
            ASSERT_FLOAT_EQ(pExpected[i], pBuffer[i]);
        }
    }

    unsigned int inputLength;
    unsigned int outputLength;
    CSAMPLE* input;
    CSAMPLE* output;
    CSAMPLE* test;
    EngineMicrophone* m_pMicrophone;
    ControlObject* m_pTalkover;
};

TEST_F(EngineMicrophoneTest, TestInputMatchesOutput) {
    FillBuffer<CSAMPLE>(input, 0.1f, inputLength);
    SampleUtil::clear(output, outputLength);

    AudioInput micInput = AudioInput(AudioPath::MICROPHONE, 0, 1, 0);
    m_pTalkover->set(1.0);

    m_pMicrophone->receiveBuffer(micInput, input, inputLength);
    m_pMicrophone->process(output, output, outputLength);

    // Check that the output matches the input data.
    AssertWholeBufferEquals(output, 0.1f, outputLength);
}

TEST_F(EngineMicrophoneTest, TestTalkoverDisablesOutput) {
    SampleUtil::clear(output, outputLength);
    AudioInput micInput = AudioInput(AudioPath::MICROPHONE, 0, 1, 0);

    m_pTalkover->set(0.0);
    FillBuffer<CSAMPLE>(input, 0.1f, inputLength);
    m_pMicrophone->receiveBuffer(micInput, input, inputLength);
    m_pMicrophone->process(output, output, outputLength);
    // Check that the output matches the input data.
    AssertWholeBufferEquals(output, 0.0f, outputLength);

    // Now fill the buffer with 2's and turn talkover on. Verify that 2's come
    // out.
    m_pTalkover->set(1.0);
    FillBuffer<CSAMPLE>(input, 0.2f, inputLength);
    m_pMicrophone->receiveBuffer(micInput, input, inputLength);
    m_pMicrophone->process(output, output, outputLength);
    // Check that the output matches the input data.
    AssertWholeBufferEquals(output, 0.2f, outputLength);
}

TEST_F(EngineMicrophoneTest, TestRepeatedInputMatchesOutput) {
    SampleUtil::clear(output, outputLength);
    AudioInput micInput = AudioInput(AudioPath::MICROPHONE, 0, 1, 0);
    m_pTalkover->set(1.0);

    for (int i = 0; i < 10; i++) {
        FillSequentialWithStride<CSAMPLE>(input, 0, 0.001f, 1.0f, 1, inputLength);
        FillSequentialWithStride<CSAMPLE>(test, 0, 0.001f, 1.0f, 2, outputLength);

        m_pMicrophone->receiveBuffer(micInput, input, inputLength);
        m_pMicrophone->process(output, output, outputLength);

        // Check that the output matches the expected output
        AssertBuffersEqual<CSAMPLE>(output, test, outputLength);
    }
}

}  // namespace
