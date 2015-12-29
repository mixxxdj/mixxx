#include "circularsamplebuffer.h"
#include "test/mixxxtest.h"

#include <gmock/gmock.h>

#include <QtDebug>

class CircularSampleBufferTest: public MixxxTest {
public:
    CircularSampleBufferTest()
        : m_writeValue(CSAMPLE_ZERO),
          m_readValue(CSAMPLE_ZERO) {
    }

protected:
    static const SINT kCapacity;

    SINT writeTail(CircularSampleBuffer* pSampleBuffer, SINT size) {
        const SampleBuffer::WritableChunk writableChunk(
                pSampleBuffer->writeToTail(size));
        for (SINT i = 0; i < writableChunk.size(); ++i) {
            writableChunk[i] = m_writeValue;
            m_writeValue += CSAMPLE_ONE;
        }
        return writableChunk.size();
    }

    SINT readHeadAndVerify(CircularSampleBuffer* pSampleBuffer, SINT size) {
        const SampleBuffer::ReadableChunk readableChunk(
                pSampleBuffer->readFromHead(size));
        for (SINT i = 0; i < readableChunk.size(); ++i) {
            EXPECT_EQ(readableChunk[i], m_readValue);
            m_readValue += CSAMPLE_ONE;
        }
        return readableChunk.size();
    }

    SINT readTailAndVerify(CircularSampleBuffer* pSampleBuffer, SINT size) {
        const SampleBuffer::ReadableChunk readableChunk(
                pSampleBuffer->readFromTail(size));
        for (SINT i = readableChunk.size(); i-- > 0; ) {
            m_writeValue -= CSAMPLE_ONE;
            EXPECT_EQ(readableChunk[i], m_writeValue);
        }
        return readableChunk.size();
    }

    void reset(CircularSampleBuffer* pSampleBuffer) {
        pSampleBuffer->reset();
        m_writeValue = CSAMPLE_ZERO;
        m_readValue = CSAMPLE_ZERO;
    }

private:
    CSAMPLE m_writeValue;
    CSAMPLE m_readValue;
};

const SINT CircularSampleBufferTest::kCapacity = 100;

TEST_F(CircularSampleBufferTest, emptyWithoutCapacity) {
    CircularSampleBuffer sampleBuffer;
    EXPECT_TRUE(sampleBuffer.isEmpty());

    sampleBuffer.trim();
    EXPECT_TRUE(sampleBuffer.isEmpty());

    sampleBuffer.reset();
    EXPECT_TRUE(sampleBuffer.isEmpty());

    const SampleBuffer::WritableChunk writableChunk(
            sampleBuffer.writeToTail(10));
    EXPECT_EQ(writableChunk.data(), static_cast<CSAMPLE*>(NULL));
    EXPECT_EQ(writableChunk.size(), 0);
    EXPECT_TRUE(sampleBuffer.isEmpty());

    const SampleBuffer::ReadableChunk readableChunk(
            sampleBuffer.readFromHead(10));
    EXPECT_EQ(readableChunk.data(), static_cast<const CSAMPLE*>(NULL));
    EXPECT_EQ(readableChunk.size(), 0);
    EXPECT_TRUE(sampleBuffer.isEmpty());
}

TEST_F(CircularSampleBufferTest, emptyWithCapacity) {
    CircularSampleBuffer sampleBuffer(kCapacity);
    EXPECT_TRUE(sampleBuffer.isEmpty());

    sampleBuffer.trim();
    EXPECT_TRUE(sampleBuffer.isEmpty());

    sampleBuffer.reset();
    EXPECT_TRUE(sampleBuffer.isEmpty());
}

TEST_F(CircularSampleBufferTest, readWriteTrim) {
    CircularSampleBuffer sampleBuffer(kCapacity);

    SINT writeCount1 = writeTail(&sampleBuffer, kCapacity + 10);
    EXPECT_EQ(writeCount1, kCapacity); // buffer is full
    EXPECT_FALSE(sampleBuffer.isEmpty());
    EXPECT_EQ(sampleBuffer.getTailCapacity(), 0);

    SINT readCount1 = readHeadAndVerify(&sampleBuffer, kCapacity - 10);
    EXPECT_EQ(readCount1, kCapacity - 10);
    EXPECT_FALSE(sampleBuffer.isEmpty());
    EXPECT_EQ(sampleBuffer.getTailCapacity(), 0);

    SINT writeCount2 = writeTail(&sampleBuffer, kCapacity);
    EXPECT_EQ(writeCount2, 0); // no tail capacity left
    EXPECT_FALSE(sampleBuffer.isEmpty());

    sampleBuffer.trim();
    EXPECT_EQ(sampleBuffer.getTailCapacity(), readCount1);

    SINT writeCount3 = writeTail(&sampleBuffer, kCapacity);
    EXPECT_EQ(writeCount3, readCount1); // buffer has been refilled
    EXPECT_FALSE(sampleBuffer.isEmpty());

    SINT readCount2 = readHeadAndVerify(&sampleBuffer, kCapacity - 10);
    EXPECT_EQ(readCount2, kCapacity - 10);
    EXPECT_FALSE(sampleBuffer.isEmpty());
    EXPECT_EQ(sampleBuffer.getTailCapacity(), 0);

    sampleBuffer.trim();
    EXPECT_EQ(sampleBuffer.getTailCapacity(), readCount2);

    SINT writeCount4 = writeTail(&sampleBuffer, kCapacity);
    EXPECT_EQ(writeCount4, readCount2); // buffer has been refilled
    EXPECT_FALSE(sampleBuffer.isEmpty());

    SINT readCount3 = readHeadAndVerify(&sampleBuffer, kCapacity + 10);
    EXPECT_EQ(readCount3, kCapacity); // whole buffer has been read
    EXPECT_TRUE(sampleBuffer.isEmpty());
}

TEST_F(CircularSampleBufferTest, shrink) {
    CircularSampleBuffer sampleBuffer(kCapacity);

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

TEST_F(CircularSampleBufferTest, reset) {
    CircularSampleBuffer sampleBuffer(kCapacity);

    SINT writeCount = writeTail(&sampleBuffer, 10);
    EXPECT_EQ(writeCount, 10);
    EXPECT_FALSE(sampleBuffer.isEmpty());
    reset(&sampleBuffer);
    EXPECT_TRUE(sampleBuffer.isEmpty());
    SINT readCount = readHeadAndVerify(&sampleBuffer, 10);
    EXPECT_EQ(readCount, 0);
    EXPECT_TRUE(sampleBuffer.isEmpty());
}
