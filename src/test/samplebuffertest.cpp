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

    SINT growAndWrite(mixxx::ReadAheadSampleBuffer* pSampleBuffer, SINT length) {
        const mixxx::SampleBuffer::WritableSlice writableSlice(
                pSampleBuffer->growForWriting(length));
        for (SINT i = 0; i < writableSlice.length(); ++i) {
            writableSlice[i] = m_writeValue;
            m_writeValue += CSAMPLE_ONE;
        }
        return writableSlice.length();
    }

    SINT shrinkForReadingAndVerify(mixxx::ReadAheadSampleBuffer* pSampleBuffer, SINT maxReadLength) {
        const mixxx::SampleBuffer::ReadableSlice readableSlice(
                pSampleBuffer->shrinkForReading(maxReadLength));
        for (SINT i = 0; i < readableSlice.length(); ++i) {
            EXPECT_EQ(readableSlice[i], m_readValue);
            m_readValue += CSAMPLE_ONE;
        }
        return readableSlice.length();
    }

    SINT shrinkAfterWritingAndVerify(mixxx::ReadAheadSampleBuffer* pSampleBuffer, SINT maxDropLength) {
        const SINT droppedLength =
                pSampleBuffer->shrinkAfterWriting(maxDropLength);
        m_writeValue -= droppedLength * CSAMPLE_ONE;
        return droppedLength;
    }

    void clear(mixxx::ReadAheadSampleBuffer* pSampleBuffer) {
        pSampleBuffer->clear();
        m_writeValue = CSAMPLE_ZERO;
        m_readValue = CSAMPLE_ZERO;
    }

private:
    CSAMPLE m_writeValue;
    CSAMPLE m_readValue;
};

const SINT ReadAheadSampleBufferTest::kCapacity = 100;

TEST_F(ReadAheadSampleBufferTest, emptyWithoutCapacity) {
    mixxx::ReadAheadSampleBuffer sampleBuffer;
    EXPECT_TRUE(sampleBuffer.empty());

    sampleBuffer.clear();
    EXPECT_TRUE(sampleBuffer.empty());

    const mixxx::SampleBuffer::WritableSlice writableSlice(
            sampleBuffer.growForWriting(10));
    EXPECT_EQ(writableSlice.data(), static_cast<CSAMPLE*>(NULL));
    EXPECT_EQ(writableSlice.length(), 0);
    EXPECT_TRUE(sampleBuffer.empty());

    const SINT droppedLength(
            sampleBuffer.shrinkAfterWriting(10));
    EXPECT_EQ(droppedLength, 0);
    EXPECT_TRUE(sampleBuffer.empty());
}

TEST_F(ReadAheadSampleBufferTest, emptyWithCapacity) {
    mixxx::ReadAheadSampleBuffer sampleBuffer(kCapacity);
    EXPECT_TRUE(sampleBuffer.empty());

    sampleBuffer.clear();
    EXPECT_TRUE(sampleBuffer.empty());
}

TEST_F(ReadAheadSampleBufferTest, readWriteTrim) {
    mixxx::ReadAheadSampleBuffer sampleBuffer(kCapacity);

    SINT writeCount1 = growAndWrite(&sampleBuffer, kCapacity + 10);
    // Buffer is completely filled with samples
    EXPECT_EQ(writeCount1, kCapacity);
    EXPECT_FALSE(sampleBuffer.empty());
    EXPECT_EQ(sampleBuffer.readableLength(), kCapacity);
    EXPECT_EQ(sampleBuffer.writableLength(), 0);

    SINT readCount1 = shrinkForReadingAndVerify(&sampleBuffer, kCapacity - 10);
    // Buffer contains the remaining 10 samples, but is still full
    EXPECT_EQ(readCount1, kCapacity - 10);
    EXPECT_FALSE(sampleBuffer.empty());
    EXPECT_EQ(sampleBuffer.readableLength(), kCapacity - readCount1);
    EXPECT_EQ(sampleBuffer.writableLength(), 0);

    SINT writeCount2 = growAndWrite(&sampleBuffer, kCapacity);
    // Impossible to write more samples
    EXPECT_EQ(writeCount2, 0);
    EXPECT_FALSE(sampleBuffer.empty());
    EXPECT_EQ(sampleBuffer.readableLength(), kCapacity - readCount1);
    EXPECT_EQ(sampleBuffer.writableLength(), 0);

    // Trim buffer contents by reallocation
    mixxx::ReadAheadSampleBuffer(sampleBuffer).swap(sampleBuffer);
    EXPECT_FALSE(sampleBuffer.empty());
    EXPECT_EQ(sampleBuffer.readableLength(), kCapacity - readCount1);
    EXPECT_EQ(sampleBuffer.writableLength(), readCount1);

    // Refill Buffer with samples
    SINT writeCount3 = growAndWrite(&sampleBuffer, kCapacity);
    EXPECT_EQ(writeCount3, readCount1);
    EXPECT_FALSE(sampleBuffer.empty());
    EXPECT_EQ(sampleBuffer.readableLength(), kCapacity);
    EXPECT_EQ(sampleBuffer.writableLength(), 0);

    // Read from the end
    SINT readCount2 = shrinkAfterWritingAndVerify(&sampleBuffer, readCount1);
    EXPECT_EQ(readCount2, readCount1);
    EXPECT_FALSE(sampleBuffer.empty());
    EXPECT_EQ(sampleBuffer.readableLength(), kCapacity - readCount1);
    EXPECT_EQ(sampleBuffer.writableLength(), readCount1);

    // Trim buffer contents by reallocation has no effect
    mixxx::ReadAheadSampleBuffer(sampleBuffer).swap(sampleBuffer);
    EXPECT_FALSE(sampleBuffer.empty());
    EXPECT_EQ(sampleBuffer.readableLength(), kCapacity - readCount1);
    EXPECT_EQ(sampleBuffer.writableLength(), readCount1);
    
    // Refill Buffer with samples
    SINT writeCount4 = growAndWrite(&sampleBuffer, kCapacity);
    EXPECT_EQ(writeCount4, readCount2); // buffer has been refilled
    EXPECT_FALSE(sampleBuffer.empty());
    EXPECT_EQ(sampleBuffer.readableLength(), kCapacity);
    EXPECT_EQ(sampleBuffer.writableLength(), 0);

    // Read whole buffer
    SINT readCount3 = shrinkAfterWritingAndVerify(&sampleBuffer, kCapacity + 10);
    EXPECT_EQ(readCount3, kCapacity);
    EXPECT_TRUE(sampleBuffer.empty());
    EXPECT_EQ(sampleBuffer.readableLength(), 0);
    EXPECT_EQ(sampleBuffer.writableLength(), kCapacity);
}

TEST_F(ReadAheadSampleBufferTest, shrink) {
    mixxx::ReadAheadSampleBuffer sampleBuffer(kCapacity);

    SINT writeCount1 = growAndWrite(&sampleBuffer, kCapacity - 10);
    EXPECT_EQ(writeCount1, kCapacity - 10);
    EXPECT_FALSE(sampleBuffer.empty());
    SINT shrinkCount1 = shrinkForReadingAndVerify(&sampleBuffer, 10);
    EXPECT_EQ(shrinkCount1, 10);
    SINT readCount1 = shrinkAfterWritingAndVerify(&sampleBuffer, 10);
    EXPECT_EQ(readCount1, 10);
    EXPECT_FALSE(sampleBuffer.empty());
    SINT readCount2 = shrinkAfterWritingAndVerify(&sampleBuffer, kCapacity - 40);
    EXPECT_EQ(readCount2, kCapacity - 40);
    EXPECT_FALSE(sampleBuffer.empty());
    SINT readCount3 = shrinkAfterWritingAndVerify(&sampleBuffer, 20);
    EXPECT_EQ(readCount3, 10);
    EXPECT_TRUE(sampleBuffer.empty());

    SINT writeCount2 = growAndWrite(&sampleBuffer, 20);
    EXPECT_EQ(writeCount2, 20);
    EXPECT_FALSE(sampleBuffer.empty());
    SINT shrinkCount2 = shrinkForReadingAndVerify(&sampleBuffer, 21);
    EXPECT_EQ(shrinkCount2, 20);
    EXPECT_TRUE(sampleBuffer.empty());
}

TEST_F(ReadAheadSampleBufferTest, clear) {
    mixxx::ReadAheadSampleBuffer sampleBuffer(kCapacity);

    SINT writeCount = growAndWrite(&sampleBuffer, 10);
    EXPECT_EQ(writeCount, 10);
    EXPECT_FALSE(sampleBuffer.empty());
    clear(&sampleBuffer);
    EXPECT_TRUE(sampleBuffer.empty());
    SINT readCount = shrinkAfterWritingAndVerify(&sampleBuffer, 10);
    EXPECT_EQ(readCount, 0);
    EXPECT_TRUE(sampleBuffer.empty());
}
