#include <gtest/gtest.h>

#include <QAtomicInt>
#include <QSet>
#include <QThread>
#include <QtDebug>

#include "util/spscfifo.h"

namespace {

typedef int value_type;

TEST(SpscFifoTest, CapacityOne) {
    SpscFifo<value_type> fifo(1);
    ASSERT_EQ(1, fifo.capacity());
    value_type dequeued = -1;

    EXPECT_TRUE(fifo.try_push(1));
    EXPECT_FALSE(fifo.try_push(2));
    EXPECT_FALSE(fifo.try_push(2));
    EXPECT_EQ(1, fifo.pop(&dequeued));
    EXPECT_EQ(1, dequeued);
    EXPECT_EQ(0, fifo.pop(&dequeued));
    EXPECT_EQ(0, fifo.pop(&dequeued));

    EXPECT_TRUE(fifo.try_push(2));
    EXPECT_FALSE(fifo.try_push(3));
    EXPECT_FALSE(fifo.try_push(3));
    EXPECT_EQ(1, fifo.pop(&dequeued));
    EXPECT_EQ(2, dequeued);
    EXPECT_EQ(0, fifo.pop(&dequeued));
    EXPECT_EQ(0, fifo.pop(&dequeued));

    EXPECT_TRUE(fifo.try_push(3));
    EXPECT_FALSE(fifo.try_push(4));
    EXPECT_FALSE(fifo.try_push(4));
    EXPECT_EQ(1, fifo.pop(&dequeued));
    EXPECT_EQ(3, dequeued);
    EXPECT_EQ(0, fifo.pop(&dequeued));
    EXPECT_EQ(0, fifo.pop(&dequeued));
}

TEST(SpscFifoTest, CapacityTwo) {
    SpscFifo<value_type> fifo(2);
    ASSERT_EQ(2, fifo.capacity());
    value_type dequeued = -1;

    EXPECT_TRUE(fifo.try_push(1));
    EXPECT_EQ(1, fifo.pop(&dequeued));
    EXPECT_EQ(1, dequeued);
    EXPECT_EQ(0, fifo.pop(&dequeued));

    EXPECT_TRUE(fifo.try_push(2));
    EXPECT_EQ(1, fifo.pop(&dequeued));
    EXPECT_EQ(2, dequeued);
    EXPECT_EQ(0, fifo.pop(&dequeued));

    EXPECT_TRUE(fifo.try_push(3));
    EXPECT_EQ(1, fifo.pop(&dequeued));
    EXPECT_EQ(3, dequeued);
    EXPECT_EQ(0, fifo.pop(&dequeued));

    EXPECT_TRUE(fifo.try_push(4));
    EXPECT_EQ(1, fifo.pop(&dequeued));
    EXPECT_EQ(4, dequeued);
    EXPECT_EQ(0, fifo.pop(&dequeued));

    EXPECT_TRUE(fifo.try_push(5));
    EXPECT_TRUE(fifo.try_push(6));
    EXPECT_FALSE(fifo.try_push(7));
    EXPECT_EQ(1, fifo.pop(&dequeued));
    EXPECT_EQ(5, dequeued);
    EXPECT_EQ(1, fifo.pop(&dequeued));
    EXPECT_EQ(6, dequeued);
    EXPECT_EQ(0, fifo.pop(&dequeued));

    EXPECT_TRUE(fifo.try_push(7));
    EXPECT_TRUE(fifo.try_push(8));
    EXPECT_EQ(1, fifo.pop(&dequeued));
    EXPECT_EQ(7, dequeued);
    EXPECT_TRUE(fifo.try_push(9));
    EXPECT_FALSE(fifo.try_push(10));
    EXPECT_EQ(1, fifo.pop(&dequeued));
    EXPECT_EQ(8, dequeued);
    EXPECT_TRUE(fifo.try_push(10));
    EXPECT_EQ(1, fifo.pop(&dequeued));
    EXPECT_EQ(9, dequeued);
    EXPECT_EQ(1, fifo.pop(&dequeued));
    EXPECT_EQ(10, dequeued);
    EXPECT_EQ(0, fifo.pop(&dequeued));
}

template<size_t capacity>
class SpscFifoWriter: public QThread {
  public:
    explicit SpscFifoWriter(SpscFifo<value_type>* fifo, QAtomicInt* nextValue, value_type maxValue)
        : m_fifo(fifo),
          m_nextValue(nextValue),
          m_maxValue(maxValue) {
    }
    virtual ~SpscFifoWriter() = default;

    void run() override {
        for (;;) {
            value_type nextEnqueued = m_nextValue->fetchAndAddOrdered(1);
            ASSERT_LE(0, nextEnqueued);
            if (nextEnqueued >= m_maxValue) {
                return;
            }
            m_fifo->push_all(&nextEnqueued);
        }
    }

  private:
    SpscFifo<value_type>* const m_fifo;
    QAtomicInt* const m_nextValue;
    const value_type m_maxValue;
};

// Using 500k values/rounds this test should take no longer
// than 200 ms. If the test doesn't finish at all this will
// also indicate a bug.
TEST(SpscFifoTest, ConcurrentWriter) {
    const size_t kCapacity = 20;
    const value_type kMaxValue = 500000;

    // Lock-free SPSC FIFO
    SpscFifo<value_type> fifo(kCapacity);
    ASSERT_EQ(kCapacity, fifo.capacity());

    QAtomicInt nextEnqueueValue(0);
    SpscFifoWriter<kCapacity> writer(&fifo, &nextEnqueueValue, kMaxValue);

    writer.start();

    // Values are enqueued slightly out-of-order and must be
    // buffered and reordered by the reader to check their
    // validity.
    QSet<value_type> dequeuedBuffer;
    value_type minValue = 0;
    while (minValue < kMaxValue) {
        value_type dequeued;
        fifo.pop_all(&dequeued);
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
    value_type dequeued;
    EXPECT_FALSE(fifo.pop(&dequeued));

    writer.wait();

    EXPECT_TRUE(nextEnqueueValue.load() >= kMaxValue);
    EXPECT_FALSE(fifo.pop(&dequeued));
}

}  // namespace
