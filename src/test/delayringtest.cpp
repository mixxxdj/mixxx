#include <gtest/gtest.h>

#include <QtConcurrentRun>
#include <QtDebug>

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

} // namespace
