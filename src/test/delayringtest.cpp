#include <gtest/gtest.h>

#include <QtConcurrentRun>
#include <QtDebug>
#include <atomic>

#include "mixxxtest.h"
#include "util/delayring.h"

namespace {

class DelayRingTest : public MixxxTest {
  protected:
    struct TestDataStuct {
        uint64_t sequence;
        double value;
    };

    static constexpr size_t kRingSize = 4;
};

TEST_F(DelayRingTest, ConcurrentPushAndGet) {
    DelayRing<TestDataStuct, kRingSize> ring;

    // Produce fast
    auto producer = QtConcurrent::run([&ring]() {
        for (int i = 0; i < 20; ++i) {
            TestDataStuct data = {static_cast<uint64_t>(i), static_cast<double>(i) * 1.5};
            ring.push(data);
            QThread::usleep(3);
        }
    });

    // Read slow
    auto consumer = QtConcurrent::run([&ring]() {
        for (int i = 0; i < 15; ++i) {
            TestDataStuct data0;
            TestDataStuct data1;
            // Try to read most recent
            if (ring.getAt(0, &data0)) {
                EXPECT_GE(data0.sequence, 0);
                EXPECT_LE(data0.sequence, 25);
                EXPECT_DOUBLE_EQ(data0.value, static_cast<double>(data0.sequence) * 1.5);
            }

            // allow the producer to advance
            QThread::usleep(2);

            // Try to read delayed
            if (ring.getAt(1, &data1)) {
                EXPECT_GE(data1.sequence, 0);
                EXPECT_LE(data1.sequence, 25);
                EXPECT_DOUBLE_EQ(data1.value, static_cast<double>(data1.sequence) * 1.5);

                // Note: It can happen that the producer has updated the ring
                // in between reading data1 and data2. That means we read same
                // value twice or even a more recent value. This is the desired
                // corner case we want to test to verify data integrity
                // qWarning() << data1.sequence << data0.sequence;
            }
            QThread::usleep(10);
        }
    });

    producer.waitForFinished();
    consumer.waitForFinished();
}

TEST_F(DelayRingTest, ConcurrentWraparound) {
    DelayRing<TestDataStuct, kRingSize> ring;

    // Make sure we have old values that the race can start
    for (size_t i = 0; i < kRingSize; ++i) {
        TestDataStuct data = {static_cast<uint64_t>(i), static_cast<double>(i) * 1.5};
        ring.push(data);
    }

    // Number of pushed values, updated after each push. The ring's write
    // index is always pushed or pushed + 1.
    std::atomic<uint64_t> pushed{kRingSize};

    // produce slowly new values.
    auto producer = QtConcurrent::run([&ring, &pushed]() {
        for (size_t i = kRingSize; i < 100; ++i) {
            TestDataStuct data = {static_cast<uint64_t>(i), static_cast<double>(i) * 1.5};
            ring.push(data);
            pushed.store(i + 1, std::memory_order_release);
            QThread::usleep(3);
        }
    });

    // read all values in a tight loop
    auto consumer = QtConcurrent::run([&ring, &pushed]() {
        for (int iteration = 0; iteration < 200; ++iteration) {
            // Read all available indices
            for (size_t at = 0; at < kRingSize; ++at) {
                TestDataStuct data;
                const uint64_t pushedBefore = pushed.load(std::memory_order_acquire);
                if (ring.getAt(at, &data)) {
                    const uint64_t pushedAfter = pushed.load(std::memory_order_acquire);
                    EXPECT_DOUBLE_EQ(data.value, static_cast<double>(data.sequence) * 1.5);
                    // getAt() counts back from the write index at the time of
                    // the call. The producer may push any number of values
                    // between two calls, so an older index can return a newer
                    // value than the previous call did.
                    EXPECT_GE(data.sequence + 1 + at, pushedBefore);
                    EXPECT_LE(data.sequence + at, pushedAfter);
                }
            }

            QThread::usleep(1);
        }
    });

    producer.waitForFinished();
    consumer.waitForFinished();
}

} // namespace
