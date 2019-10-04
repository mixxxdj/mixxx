#include <gtest/gtest.h>

#include <QAtomicInt>
#include <QSet>
#include <QThread>
#include <QtDebug>

#include "util/spscfifo.h"

namespace {

TEST(SpscFifoTest, CapacityOne) {
    SpscFifo<int> fifo(1);
    int dequeued = -1;

    EXPECT_TRUE(fifo.push(1));
    EXPECT_FALSE(fifo.push(2));
    EXPECT_FALSE(fifo.push(2));
    EXPECT_TRUE(fifo.pop(&dequeued));
    EXPECT_EQ(1, dequeued);
    EXPECT_FALSE(fifo.pop(&dequeued));
    EXPECT_FALSE(fifo.pop(&dequeued));

    EXPECT_TRUE(fifo.push(2));
    EXPECT_FALSE(fifo.push(3));
    EXPECT_FALSE(fifo.push(3));
    EXPECT_TRUE(fifo.pop(&dequeued));
    EXPECT_EQ(2, dequeued);
    EXPECT_FALSE(fifo.pop(&dequeued));
    EXPECT_FALSE(fifo.pop(&dequeued));

    EXPECT_TRUE(fifo.push(3));
    EXPECT_FALSE(fifo.push(4));
    EXPECT_FALSE(fifo.push(4));
    EXPECT_TRUE(fifo.pop(&dequeued));
    EXPECT_EQ(3, dequeued);
    EXPECT_FALSE(fifo.pop(&dequeued));
    EXPECT_FALSE(fifo.pop(&dequeued));
}

TEST(SpscFifoTest, CapacityTwo) {
    SpscFifo<int> fifo(2);
    int dequeued = -1;

    EXPECT_TRUE(fifo.push(1));
    EXPECT_TRUE(fifo.pop(&dequeued));
    EXPECT_EQ(1, dequeued);
    EXPECT_FALSE(fifo.pop(&dequeued));

    EXPECT_TRUE(fifo.push(2));
    EXPECT_TRUE(fifo.pop(&dequeued));
    EXPECT_EQ(2, dequeued);
    EXPECT_FALSE(fifo.pop(&dequeued));

    EXPECT_TRUE(fifo.push(3));
    EXPECT_TRUE(fifo.pop(&dequeued));
    EXPECT_EQ(3, dequeued);
    EXPECT_FALSE(fifo.pop(&dequeued));

    EXPECT_TRUE(fifo.push(4));
    EXPECT_TRUE(fifo.pop(&dequeued));
    EXPECT_EQ(4, dequeued);
    EXPECT_FALSE(fifo.pop(&dequeued));

    EXPECT_TRUE(fifo.push(5));
    EXPECT_TRUE(fifo.push(6));
    EXPECT_FALSE(fifo.push(7));
    EXPECT_TRUE(fifo.pop(&dequeued));
    EXPECT_EQ(5, dequeued);
    EXPECT_TRUE(fifo.pop(&dequeued));
    EXPECT_EQ(6, dequeued);
    EXPECT_FALSE(fifo.pop(&dequeued));

    EXPECT_TRUE(fifo.push(7));
    EXPECT_TRUE(fifo.push(8));
    EXPECT_TRUE(fifo.pop(&dequeued));
    EXPECT_EQ(7, dequeued);
    EXPECT_TRUE(fifo.push(9));
    EXPECT_FALSE(fifo.push(10));
    EXPECT_TRUE(fifo.pop(&dequeued));
    EXPECT_EQ(8, dequeued);
    EXPECT_TRUE(fifo.push(10));
    EXPECT_TRUE(fifo.pop(&dequeued));
    EXPECT_EQ(9, dequeued);
    EXPECT_TRUE(fifo.pop(&dequeued));
    EXPECT_EQ(10, dequeued);
    EXPECT_FALSE(fifo.pop(&dequeued));
}

template<int capacity>
class SpscFifoWriter: public QThread {
  public:
    explicit SpscFifoWriter(SpscFifo<int>* fifo, QAtomicInt* nextValue, int maxValue)
        : m_fifo(fifo),
          m_nextValue(nextValue),
          m_maxValue(maxValue) {
    }
    virtual ~SpscFifoWriter() = default;

    void run() override {
        for (;;) {
            int nextEnqueued = m_nextValue->fetchAndAddOrdered(1);
            ASSERT_GE(nextEnqueued, 0);
            if (nextEnqueued >= m_maxValue) {
                return;
            }
            while (!m_fifo->push(nextEnqueued)) {
                QThread::usleep(10);
                // repeat and try again
            }
        }
    }

  private:
    SpscFifo<int>* const m_fifo;
    QAtomicInt* const m_nextValue;
    const int m_maxValue;
};

// Using 500k values/rounds this test should take no longer
// than 200 ms. If the test doesn't finish at all this will
// also indicate a bug.
TEST(SpscFifoTest, ConcurrentWriter) {
    const int kCapacity = 20;
    const int kMaxValue = 500000;

    // Lock-free SPSC FIFO
    SpscFifo<int> fifo(kCapacity);

    QAtomicInt nextEnqueueValue(0);
    SpscFifoWriter<kCapacity> writer(&fifo, &nextEnqueueValue, kMaxValue);

    writer.start();

    // Values are enqueued slightly out-of-order and must be
    // buffered and reordered by the reader to check their
    // validity.
    QSet<int> dequeuedBuffer;
    int minValue = 0;
    while (minValue < kMaxValue) {
        int dequeued;
        if (fifo.pop(&dequeued)) {
            // Check that we haven't dequeued the same value twice!
            ASSERT_GE(dequeued, minValue);
            ASSERT_FALSE(dequeuedBuffer.contains(dequeued));
            if (dequeued == minValue) {
                ++minValue;
                while (dequeuedBuffer.remove(minValue)) {
                    ++minValue;
                }
            } else {
                dequeuedBuffer.insert(dequeued);
            }
        }
    }
    int dequeued;
    EXPECT_FALSE(fifo.pop(&dequeued));

    writer.wait();

    EXPECT_TRUE(nextEnqueueValue.load() >= kMaxValue);
    EXPECT_FALSE(fifo.pop(&dequeued));
}

}  // namespace
