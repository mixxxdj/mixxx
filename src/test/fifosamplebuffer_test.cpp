#include "test/mixxxtest.h"

#include "fifosamplebuffer.h"

#include <gmock/gmock.h>

#include <QtDebug>

class FifoSampleBufferTest: public MixxxTest {
public:
    FifoSampleBufferTest()
        : m_writeValue(CSAMPLE_ZERO),
          m_readValue(CSAMPLE_ZERO) {
    }

protected:
    static const SINT kCapacity;

    SINT writeTail(FifoSampleBuffer* pSampleBuffer, SINT size) {
        const std::pair<CSAMPLE*, SINT> writeBuffer(
                pSampleBuffer->growTail(size));
        for (SINT i = 0; i < writeBuffer.second; ++i) {
            writeBuffer.first[i] = m_writeValue;
            m_writeValue += CSAMPLE_ONE;
        }
        return writeBuffer.second;
    }

    SINT readHeadAndVerify(FifoSampleBuffer* pSampleBuffer, SINT size) {
        const std::pair<const CSAMPLE*, SINT>readBuffer(
                pSampleBuffer->shrinkHead(size));
        for (SINT i = 0; i < readBuffer.second; ++i) {
            EXPECT_EQ(readBuffer.first[i], m_readValue);
            m_readValue += CSAMPLE_ONE;
        }
        return readBuffer.second;
    }

    SINT readTailAndVerify(FifoSampleBuffer* pSampleBuffer, SINT size) {
        const std::pair<const CSAMPLE*, SINT>readBuffer(
                pSampleBuffer->shrinkTail(size));
        for (SINT i = readBuffer.second; i-- > 0; ) {
            m_writeValue -= CSAMPLE_ONE;
            EXPECT_EQ(readBuffer.first[i], m_writeValue);
        }
        return readBuffer.second;
    }

    void reset(FifoSampleBuffer* pSampleBuffer) {
        pSampleBuffer->reset();
        m_writeValue = CSAMPLE_ZERO;
        m_readValue = CSAMPLE_ZERO;
    }

private:
    CSAMPLE m_writeValue;
    CSAMPLE m_readValue;
};

const SINT FifoSampleBufferTest::kCapacity = 100;

TEST_F(FifoSampleBufferTest, emptyWithoutCapacity) {
    FifoSampleBuffer sampleBuffer;
    EXPECT_TRUE(sampleBuffer.isEmpty());

    sampleBuffer.trim();
    EXPECT_TRUE(sampleBuffer.isEmpty());

    sampleBuffer.reset();
    EXPECT_TRUE(sampleBuffer.isEmpty());

    const std::pair<CSAMPLE*, SINT> writeResult(
            sampleBuffer.growTail(10));
    EXPECT_EQ(writeResult.first, static_cast<CSAMPLE*>(NULL));
    EXPECT_EQ(writeResult.second, 0);
    EXPECT_TRUE(sampleBuffer.isEmpty());

    const std::pair<const CSAMPLE*, SINT> readResult(
            sampleBuffer.shrinkHead(10));
    EXPECT_EQ(readResult.first, static_cast<const CSAMPLE*>(NULL));
    EXPECT_EQ(readResult.second, 0);
    EXPECT_TRUE(sampleBuffer.isEmpty());
}

TEST_F(FifoSampleBufferTest, emptyWithCapacity) {
    FifoSampleBuffer sampleBuffer(kCapacity);
    EXPECT_TRUE(sampleBuffer.isEmpty());

    sampleBuffer.trim();
    EXPECT_TRUE(sampleBuffer.isEmpty());

    sampleBuffer.reset();
    EXPECT_TRUE(sampleBuffer.isEmpty());
}

TEST_F(FifoSampleBufferTest, readWriteTrim) {
    FifoSampleBuffer sampleBuffer(kCapacity);

    SINT writeCount1 = writeTail(&sampleBuffer, kCapacity + 10);
    EXPECT_EQ(writeCount1, kCapacity); // buffer is full
    EXPECT_FALSE(sampleBuffer.isEmpty());

    SINT readCount1 = readHeadAndVerify(&sampleBuffer, kCapacity - 10);
    EXPECT_EQ(readCount1, kCapacity - 10);
    EXPECT_FALSE(sampleBuffer.isEmpty());

    SINT writeCount2 = writeTail(&sampleBuffer, kCapacity);
    EXPECT_EQ(writeCount2, readCount1); // buffer has been refilled
    EXPECT_FALSE(sampleBuffer.isEmpty());

    SINT readCount2 = readHeadAndVerify(&sampleBuffer, kCapacity - 10);
    EXPECT_EQ(readCount2, kCapacity - 10);
    EXPECT_FALSE(sampleBuffer.isEmpty());

    sampleBuffer.trim();

    SINT writeCount3 = writeTail(&sampleBuffer, kCapacity);
    EXPECT_EQ(writeCount3, readCount2); // buffer has been refilled
    EXPECT_FALSE(sampleBuffer.isEmpty());

    SINT readCount3 = readHeadAndVerify(&sampleBuffer, kCapacity + 10);
    EXPECT_EQ(readCount3, kCapacity); // whole buffer has been read
    EXPECT_TRUE(sampleBuffer.isEmpty());
}

TEST_F(FifoSampleBufferTest, shrink) {
    FifoSampleBuffer sampleBuffer(kCapacity);

    SINT writeCount1 = writeTail(&sampleBuffer, kCapacity - 10);
    EXPECT_EQ(writeCount1, kCapacity - 10);
    EXPECT_FALSE(sampleBuffer.isEmpty());
    SINT shrinkCount1 = readTailAndVerify(&sampleBuffer, 10);
    EXPECT_EQ(shrinkCount1, 10);
    SINT readCount1 = readHeadAndVerify(&sampleBuffer, 10);
    EXPECT_EQ(readCount1, 10);
    EXPECT_FALSE(sampleBuffer.isEmpty());
    SINT readCount2 = readHeadAndVerify(&sampleBuffer, kCapacity - 40);
    EXPECT_EQ(readCount2, kCapacity - 40);
    EXPECT_FALSE(sampleBuffer.isEmpty());
    SINT readCount3 = readHeadAndVerify(&sampleBuffer, 20);
    EXPECT_EQ(readCount3, 10);
    EXPECT_TRUE(sampleBuffer.isEmpty());

    SINT writeCount2 = writeTail(&sampleBuffer, 20);
    EXPECT_EQ(writeCount2, 20);
    EXPECT_FALSE(sampleBuffer.isEmpty());
    SINT shrinkCount2 = readTailAndVerify(&sampleBuffer, 21);
    EXPECT_EQ(shrinkCount2, 20);
    EXPECT_TRUE(sampleBuffer.isEmpty());
}

TEST_F(FifoSampleBufferTest, reset) {
    FifoSampleBuffer sampleBuffer(kCapacity);

    SINT writeCount = writeTail(&sampleBuffer, 10);
    EXPECT_EQ(writeCount, 10);
    EXPECT_FALSE(sampleBuffer.isEmpty());
    reset(&sampleBuffer);
    EXPECT_TRUE(sampleBuffer.isEmpty());
    SINT readCount = readHeadAndVerify(&sampleBuffer, 10);
    EXPECT_EQ(readCount, 0);
    EXPECT_TRUE(sampleBuffer.isEmpty());
}
