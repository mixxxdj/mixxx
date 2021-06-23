#include <gtest/gtest.h>

#include <QtDebug>

#include "test/signalpathtest.h"
#include "preferences/usersettings.h"
#include "control/controlobject.h"
#include "engine/channels/enginemicrophone.h"
#include "soundio/soundmanagerutil.h"
#include "util/defs.h"
#include "util/sample.h"
#include "util/types.h"

namespace {

class EngineMicrophoneTest : public SignalPathTest {
  protected:
    void SetUp() override {
        inputLength = MAX_BUFFER_LEN;
        outputLength = MAX_BUFFER_LEN;
        input = SampleUtil::alloc(inputLength);
        output = SampleUtil::alloc(outputLength);
        test = SampleUtil::alloc(outputLength);

        // No need for a real handle in this test.
        m_pMicrophone = new EngineMicrophone(
                ChannelHandleAndGroup(ChannelHandle(), "[Microphone]"), m_pEffectsManager);
        m_pTalkover = ControlObject::getControl(ConfigKey("[Microphone]", "talkover"));
    }

    void TearDown() override {
        SampleUtil::free(input);
        SampleUtil::free(output);
        SampleUtil::free(test);
        delete m_pMicrophone;
    }

    void ClearBuffer(CSAMPLE* pBuffer, int length) {
        SampleUtil::clear(pBuffer, length);
    }

    void FillBuffer(CSAMPLE* pBuffer, CSAMPLE value, int length) {
        SampleUtil::fill(pBuffer, value, length);
    }

    template <typename T>
    void FillSequentialWithStride(T* pBuffer, T initial, T increment, T max,
                                  unsigned int stride, unsigned int length) {
        ASSERT_EQ(0U, length % stride);
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
    FillBuffer(input, 0.1f, inputLength);
    ClearBuffer(output, outputLength);

    AudioInput micInput = AudioInput(AudioPath::MICROPHONE, 0, 1, 0);
    m_pTalkover->set(1.0);

    m_pMicrophone->receiveBuffer(micInput, input, inputLength);
    m_pMicrophone->process(output, outputLength);

    // Check that the output matches the input data.
    AssertWholeBufferEquals(output, 0.1f, outputLength);
}

TEST_F(EngineMicrophoneTest, TestRepeatedInputMatchesOutput) {
    ClearBuffer(output, outputLength);
    AudioInput micInput = AudioInput(AudioPath::MICROPHONE, 0, 1, 0);
    m_pTalkover->set(1.0);

    for (int i = 0; i < 10; i++) {
        FillSequentialWithStride<CSAMPLE>(input, 0, 0.001f, 1.0f, 2, inputLength);
        FillSequentialWithStride<CSAMPLE>(test, 0, 0.001f, 1.0f, 2, outputLength);

        m_pMicrophone->receiveBuffer(micInput, input, inputLength);
        m_pMicrophone->process(output, outputLength);

        // Check that the output matches the expected output
        AssertBuffersEqual<CSAMPLE>(output, test, outputLength);
    }
}

}  // namespace
