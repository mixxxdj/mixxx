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

    SINT writeTail(mixxx::ReadAheadSampleBuffer* pSampleBuffer, SINT size) {
        const mixxx::SampleBuffer::WritableSlice writableSlice(
                pSampleBuffer->writeToTail(size));
        for (SINT i = 0; i < writableSlice.size(); ++i) {
            writableSlice[i] = m_writeValue;
            m_writeValue += CSAMPLE_ONE;
        }
        return writableSlice.size();
    }

    SINT readHeadAndVerify(mixxx::ReadAheadSampleBuffer* pSampleBuffer, SINT size) {
        const mixxx::SampleBuffer::ReadableSlice readableSlice(
                pSampleBuffer->readFromHead(size));
        for (SINT i = 0; i < readableSlice.size(); ++i) {
            EXPECT_EQ(readableSlice[i], m_readValue);
            m_readValue += CSAMPLE_ONE;
        }
        return readableSlice.size();
    }

    SINT readTailAndVerify(mixxx::ReadAheadSampleBuffer* pSampleBuffer, SINT size) {
        const mixxx::SampleBuffer::ReadableSlice readableSlice(
                pSampleBuffer->readFromTail(size));
        for (SINT i = readableSlice.size(); i-- > 0; ) {
            m_writeValue -= CSAMPLE_ONE;
            EXPECT_EQ(readableSlice[i], m_writeValue);
        }
        return readableSlice.size();
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
            sampleBuffer.writeToTail(10));
    EXPECT_EQ(writableSlice.data(), static_cast<CSAMPLE*>(NULL));
    EXPECT_EQ(writableSlice.size(), 0);
    EXPECT_TRUE(sampleBuffer.empty());

    const mixxx::SampleBuffer::ReadableSlice readableSlice(
            sampleBuffer.readFromHead(10));
    EXPECT_EQ(readableSlice.data(), static_cast<const CSAMPLE*>(NULL));
    EXPECT_EQ(readableSlice.size(), 0);
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

    SINT writeCount1 = writeTail(&sampleBuffer, kCapacity + 10);
    EXPECT_EQ(writeCount1, kCapacity); // buffer is full
    EXPECT_FALSE(sampleBuffer.empty());
    EXPECT_EQ(sampleBuffer.writableLength(), 0);

    SINT readCount1 = readHeadAndVerify(&sampleBuffer, kCapacity - 10);
    EXPECT_EQ(readCount1, kCapacity - 10);
    EXPECT_FALSE(sampleBuffer.empty());
    EXPECT_EQ(sampleBuffer.writableLength(), 0);

    SINT writeCount2 = writeTail(&sampleBuffer, kCapacity);
    EXPECT_EQ(writeCount2, 0); // no tail capacity left
    EXPECT_FALSE(sampleBuffer.empty());

    // Trim buffer contents by reallocation
    mixxx::ReadAheadSampleBuffer(sampleBuffer).swap(sampleBuffer);

    SINT writeCount3 = writeTail(&sampleBuffer, kCapacity);
    EXPECT_EQ(writeCount3, readCount1); // buffer has been refilled
    EXPECT_FALSE(sampleBuffer.empty());

    SINT readCount2 = readHeadAndVerify(&sampleBuffer, kCapacity - 10);
    EXPECT_EQ(readCount2, kCapacity - 10);
    EXPECT_FALSE(sampleBuffer.empty());
    EXPECT_EQ(sampleBuffer.writableLength(), 0);

    // Trim buffer contents by reallocation
    mixxx::ReadAheadSampleBuffer(sampleBuffer).swap(sampleBuffer);
    
    SINT writeCount4 = writeTail(&sampleBuffer, kCapacity);
    EXPECT_EQ(writeCount4, readCount2); // buffer has been refilled
    EXPECT_FALSE(sampleBuffer.empty());

    SINT readCount3 = readHeadAndVerify(&sampleBuffer, kCapacity + 10);
    EXPECT_EQ(readCount3, kCapacity); // whole buffer has been read
    EXPECT_TRUE(sampleBuffer.empty());
}

TEST_F(ReadAheadSampleBufferTest, shrink) {
    mixxx::ReadAheadSampleBuffer sampleBuffer(kCapacity);

    SINT writeCount1 = writeTail(&sampleBuffer, kCapacity - 10);
    EXPECT_EQ(writeCount1, kCapacity - 10);
    EXPECT_FALSE(sampleBuffer.empty());
    SINT shrinkCount1 = readTailAndVerify(&sampleBuffer, 10);
    EXPECT_EQ(shrinkCount1, 10);
    SINT readCount1 = readHeadAndVerify(&sampleBuffer, 10);
    EXPECT_EQ(readCount1, 10);
    EXPECT_FALSE(sampleBuffer.empty());
    SINT readCount2 = readHeadAndVerify(&sampleBuffer, kCapacity - 40);
    EXPECT_EQ(readCount2, kCapacity - 40);
    EXPECT_FALSE(sampleBuffer.empty());
    SINT readCount3 = readHeadAndVerify(&sampleBuffer, 20);
    EXPECT_EQ(readCount3, 10);
    EXPECT_TRUE(sampleBuffer.empty());

    SINT writeCount2 = writeTail(&sampleBuffer, 20);
    EXPECT_EQ(writeCount2, 20);
    EXPECT_FALSE(sampleBuffer.empty());
    SINT shrinkCount2 = readTailAndVerify(&sampleBuffer, 21);
    EXPECT_EQ(shrinkCount2, 20);
    EXPECT_TRUE(sampleBuffer.empty());
}

TEST_F(ReadAheadSampleBufferTest, clear) {
    mixxx::ReadAheadSampleBuffer sampleBuffer(kCapacity);

    SINT writeCount = writeTail(&sampleBuffer, 10);
    EXPECT_EQ(writeCount, 10);
    EXPECT_FALSE(sampleBuffer.empty());
    clear(&sampleBuffer);
    EXPECT_TRUE(sampleBuffer.empty());
    SINT readCount = readHeadAndVerify(&sampleBuffer, 10);
    EXPECT_EQ(readCount, 0);
    EXPECT_TRUE(sampleBuffer.empty());
}
