#include <gmock/gmock.h>

#include <QtDebug>

#include "test/mixxxtest.h"
#include "util/readaheadsamplebuffer.h"

class ReadAheadSampleBufferTest: public MixxxTest {
public:
    ReadAheadSampleBufferTest()
        : m_writeValue(CSAMPLE_ZERO),
          m_readValue(CSAMPLE_ZERO) {
    }

protected:
    static const SINT kCapacity;

    SINT writeTail(ReadAheadSampleBuffer* pSampleBuffer, SINT size) {
        const SampleBuffer::WritableSlice writableSlice(
                pSampleBuffer->writeToTail(size));
        for (SINT i = 0; i < writableSlice.size(); ++i) {
            writableSlice[i] = m_writeValue;
            m_writeValue += CSAMPLE_ONE;
        }
        return writableSlice.size();
    }

    SINT readHeadAndVerify(ReadAheadSampleBuffer* pSampleBuffer, SINT size) {
        const SampleBuffer::ReadableSlice readableSlice(
                pSampleBuffer->readFromHead(size));
        for (SINT i = 0; i < readableSlice.size(); ++i) {
            EXPECT_EQ(readableSlice[i], m_readValue);
            m_readValue += CSAMPLE_ONE;
        }
        return readableSlice.size();
    }

    SINT readTailAndVerify(ReadAheadSampleBuffer* pSampleBuffer, SINT size) {
        const SampleBuffer::ReadableSlice readableSlice(
                pSampleBuffer->readFromTail(size));
        for (SINT i = readableSlice.size(); i-- > 0; ) {
            m_writeValue -= CSAMPLE_ONE;
            EXPECT_EQ(readableSlice[i], m_writeValue);
        }
        return readableSlice.size();
    }

    void reset(ReadAheadSampleBuffer* pSampleBuffer) {
        pSampleBuffer->reset();
        m_writeValue = CSAMPLE_ZERO;
        m_readValue = CSAMPLE_ZERO;
    }

private:
    CSAMPLE m_writeValue;
    CSAMPLE m_readValue;
};

const SINT ReadAheadSampleBufferTest::kCapacity = 100;

TEST_F(ReadAheadSampleBufferTest, emptyWithoutCapacity) {
    ReadAheadSampleBuffer sampleBuffer;
    EXPECT_TRUE(sampleBuffer.isEmpty());

    sampleBuffer.reset();
    EXPECT_TRUE(sampleBuffer.isEmpty());

    const SampleBuffer::WritableSlice writableSlice(
            sampleBuffer.writeToTail(10));
    EXPECT_EQ(writableSlice.data(), static_cast<CSAMPLE*>(NULL));
    EXPECT_EQ(writableSlice.size(), 0);
    EXPECT_TRUE(sampleBuffer.isEmpty());

    const SampleBuffer::ReadableSlice readableSlice(
            sampleBuffer.readFromHead(10));
    EXPECT_EQ(readableSlice.data(), static_cast<const CSAMPLE*>(NULL));
    EXPECT_EQ(readableSlice.size(), 0);
    EXPECT_TRUE(sampleBuffer.isEmpty());
}

TEST_F(ReadAheadSampleBufferTest, emptyWithCapacity) {
    ReadAheadSampleBuffer sampleBuffer(kCapacity);
    EXPECT_TRUE(sampleBuffer.isEmpty());

    sampleBuffer.reset();
    EXPECT_TRUE(sampleBuffer.isEmpty());
}

TEST_F(ReadAheadSampleBufferTest, readWriteTrim) {
    ReadAheadSampleBuffer sampleBuffer(kCapacity);

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

    // Trim buffer contents by reallocation
    ReadAheadSampleBuffer(sampleBuffer).swap(sampleBuffer);

    SINT writeCount3 = writeTail(&sampleBuffer, kCapacity);
    EXPECT_EQ(writeCount3, readCount1); // buffer has been refilled
    EXPECT_FALSE(sampleBuffer.isEmpty());

    SINT readCount2 = readHeadAndVerify(&sampleBuffer, kCapacity - 10);
    EXPECT_EQ(readCount2, kCapacity - 10);
    EXPECT_FALSE(sampleBuffer.isEmpty());
    EXPECT_EQ(sampleBuffer.getTailCapacity(), 0);

    // Trim buffer contents by reallocation
    ReadAheadSampleBuffer(sampleBuffer).swap(sampleBuffer);
    
    SINT writeCount4 = writeTail(&sampleBuffer, kCapacity);
    EXPECT_EQ(writeCount4, readCount2); // buffer has been refilled
    EXPECT_FALSE(sampleBuffer.isEmpty());

    SINT readCount3 = readHeadAndVerify(&sampleBuffer, kCapacity + 10);
    EXPECT_EQ(readCount3, kCapacity); // whole buffer has been read
    EXPECT_TRUE(sampleBuffer.isEmpty());
}

TEST_F(ReadAheadSampleBufferTest, shrink) {
    ReadAheadSampleBuffer sampleBuffer(kCapacity);

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

TEST_F(ReadAheadSampleBufferTest, reset) {
    ReadAheadSampleBuffer sampleBuffer(kCapacity);

    SINT writeCount = writeTail(&sampleBuffer, 10);
    EXPECT_EQ(writeCount, 10);
    EXPECT_FALSE(sampleBuffer.isEmpty());
    reset(&sampleBuffer);
    EXPECT_TRUE(sampleBuffer.isEmpty());
    SINT readCount = readHeadAndVerify(&sampleBuffer, 10);
    EXPECT_EQ(readCount, 0);
    EXPECT_TRUE(sampleBuffer.isEmpty());
}
