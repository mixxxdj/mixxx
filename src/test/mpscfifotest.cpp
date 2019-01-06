#include <gtest/gtest.h>

#include <QAtomicInt>
#include <QSet>
#include <QThread>
#include <QtDebug>

#include "util/mpscfifo.h"

namespace {

TEST(MpscFifoTest, CapacityOne) {
    MpscFifo<int, 1> fifo;
    int dequeued = -1;

    EXPECT_TRUE(fifo.enqueue(1));
    EXPECT_FALSE(fifo.enqueue(2));
    EXPECT_FALSE(fifo.enqueue(2));
    EXPECT_TRUE(fifo.dequeue(&dequeued));
    EXPECT_EQ(1, dequeued);
    EXPECT_FALSE(fifo.dequeue(&dequeued));
    EXPECT_FALSE(fifo.dequeue(&dequeued));

    EXPECT_TRUE(fifo.enqueue(2));
    EXPECT_FALSE(fifo.enqueue(3));
    EXPECT_FALSE(fifo.enqueue(3));
    EXPECT_TRUE(fifo.dequeue(&dequeued));
    EXPECT_EQ(2, dequeued);
    EXPECT_FALSE(fifo.dequeue(&dequeued));
    EXPECT_FALSE(fifo.dequeue(&dequeued));

    EXPECT_TRUE(fifo.enqueue(3));
    EXPECT_FALSE(fifo.enqueue(4));
    EXPECT_FALSE(fifo.enqueue(4));
    EXPECT_TRUE(fifo.dequeue(&dequeued));
    EXPECT_EQ(3, dequeued);
    EXPECT_FALSE(fifo.dequeue(&dequeued));
    EXPECT_FALSE(fifo.dequeue(&dequeued));
}

TEST(MpscFifoTest, CapacityTwo) {
    MpscFifo<int, 2> fifo;
    int dequeued = -1;

    EXPECT_TRUE(fifo.enqueue(1));
    EXPECT_TRUE(fifo.dequeue(&dequeued));
    EXPECT_EQ(1, dequeued);
    EXPECT_FALSE(fifo.dequeue(&dequeued));

    EXPECT_TRUE(fifo.enqueue(2));
    EXPECT_TRUE(fifo.dequeue(&dequeued));
    EXPECT_EQ(2, dequeued);
    EXPECT_FALSE(fifo.dequeue(&dequeued));

    EXPECT_TRUE(fifo.enqueue(3));
    EXPECT_TRUE(fifo.dequeue(&dequeued));
    EXPECT_EQ(3, dequeued);
    EXPECT_FALSE(fifo.dequeue(&dequeued));

    EXPECT_TRUE(fifo.enqueue(4));
    EXPECT_TRUE(fifo.dequeue(&dequeued));
    EXPECT_EQ(4, dequeued);
    EXPECT_FALSE(fifo.dequeue(&dequeued));

    EXPECT_TRUE(fifo.enqueue(5));
    EXPECT_TRUE(fifo.enqueue(6));
    EXPECT_FALSE(fifo.enqueue(7));
    EXPECT_TRUE(fifo.dequeue(&dequeued));
    EXPECT_EQ(5, dequeued);
    EXPECT_TRUE(fifo.dequeue(&dequeued));
    EXPECT_EQ(6, dequeued);
    EXPECT_FALSE(fifo.dequeue(&dequeued));

    EXPECT_TRUE(fifo.enqueue(7));
    EXPECT_TRUE(fifo.enqueue(8));
    EXPECT_TRUE(fifo.dequeue(&dequeued));
    EXPECT_EQ(7, dequeued);
    EXPECT_TRUE(fifo.enqueue(9));
    EXPECT_FALSE(fifo.enqueue(10));
    EXPECT_TRUE(fifo.dequeue(&dequeued));
    EXPECT_EQ(8, dequeued);
    EXPECT_TRUE(fifo.enqueue(10));
    EXPECT_TRUE(fifo.dequeue(&dequeued));
    EXPECT_EQ(9, dequeued);
    EXPECT_TRUE(fifo.dequeue(&dequeued));
    EXPECT_EQ(10, dequeued);
    EXPECT_FALSE(fifo.dequeue(&dequeued));
}

template<int capacity>
class FifoWriter: public QThread {
  public:
    explicit FifoWriter(MpscFifo<int, capacity>* fifo, QAtomicInt* nextValue, int maxValue)
        : m_fifo(fifo),
          m_nextValue(nextValue),
          m_maxValue(maxValue) {
    }
    virtual ~FifoWriter() = default;

    void run() override {
        for (;;) {
            int nextEnqueued = m_nextValue->fetchAndAddOrdered(1);
            ASSERT_GE(nextEnqueued, 0);
            if (nextEnqueued >= m_maxValue) {
                return;
            }
            while (!m_fifo->enqueue(nextEnqueued)) {
                QThread::usleep(10);
                // repeat and try again
            }
        }
    }

  private:
    MpscFifo<int, capacity>* const m_fifo;
    QAtomicInt* const m_nextValue;
    const int m_maxValue;
};

// Using 500k values/rounds this test should take no longer
// than 200 ms. If the test doesn't finish at all this will
// also indicate a bug.
TEST(MpscFifoTest, ConcurrentWriters) {
    const int kCapacity = 20;
    const int kMaxValue = 500000;

    MpscFifo<int, kCapacity> fifo;

    QAtomicInt nextEnqueueValue(0);
    FifoWriter<kCapacity> writer1(&fifo, &nextEnqueueValue, kMaxValue);
    FifoWriter<kCapacity> writer2(&fifo, &nextEnqueueValue, kMaxValue);
    FifoWriter<kCapacity> writer3(&fifo, &nextEnqueueValue, kMaxValue);

    writer1.start();
    writer2.start();
    writer3.start();

    // Values are enqueued slightly out-of-order and must be
    // buffered and reordered by the reader to check their
    // validity.
    QSet<int> dequeuedBuffer;
    int minValue = 0;
    while (minValue < kMaxValue) {
        int dequeued;
        if (fifo.dequeue(&dequeued)) {
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
    EXPECT_FALSE(fifo.dequeue(&dequeued));

    writer1.wait();
    writer2.wait();
    writer3.wait();

    EXPECT_TRUE(nextEnqueueValue.load() >= kMaxValue);
    EXPECT_FALSE(fifo.dequeue(&dequeued));
}

}  // namespace
