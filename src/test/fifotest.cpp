#include <gtest/gtest.h>

#include <algorithm>
#include <random>

#include "util/fifo.h"

namespace {

struct Param {
    std::size_t requestedBufferSize;
    std::size_t expectedBufferSize;
    int offset;
};

class FifoTest : public testing::TestWithParam<Param> {
};

TEST_P(FifoTest, writeAvailableTest) {
    const auto param = FifoTest::GetParam();
    std::vector<float> data(param.requestedBufferSize);
    FIFO<float> fifo(param.requestedBufferSize);
    fifo.releaseReadRegions(param.offset >= 0
                    ? param.offset
                    : std::numeric_limits<std::size_t>::max() + param.offset +
                            1);
    fifo.releaseWriteRegions(param.offset >= 0
                    ? param.offset
                    : std::numeric_limits<std::size_t>::max() + param.offset +
                            1);

    ASSERT_EQ(param.expectedBufferSize, fifo.writeAvailable());
    ASSERT_EQ(100, fifo.write(data.data(), 100));
    ASSERT_EQ(param.expectedBufferSize - 100, fifo.writeAvailable());
    ASSERT_EQ(50, fifo.read(data.data(), 50));
    ASSERT_EQ(param.expectedBufferSize - 50, fifo.writeAvailable());
    ASSERT_EQ(param.expectedBufferSize - 50, fifo.write(data.data(), 1000000));
    ASSERT_EQ(0, fifo.writeAvailable());
}

TEST_P(FifoTest, readAvailableTest) {
    const auto param = FifoTest::GetParam();
    std::vector<float> data(param.requestedBufferSize);
    FIFO<float> fifo(param.requestedBufferSize);
    fifo.releaseReadRegions(param.offset >= 0
                    ? param.offset
                    : std::numeric_limits<std::size_t>::max() + param.offset +
                            1);
    fifo.releaseWriteRegions(param.offset >= 0
                    ? param.offset
                    : std::numeric_limits<std::size_t>::max() + param.offset +
                            1);

    ASSERT_EQ(0, fifo.readAvailable());
    ASSERT_EQ(100, fifo.write(data.data(), 100));
    ASSERT_EQ(100, fifo.readAvailable());
    ASSERT_EQ(50, fifo.read(data.data(), 50));
    ASSERT_EQ(50, fifo.readAvailable());
    ASSERT_EQ(static_cast<int>(param.expectedBufferSize - 50), fifo.write(data.data(), 1000000));
    ASSERT_EQ(static_cast<int>(param.expectedBufferSize), fifo.readAvailable());
}

TEST_P(FifoTest, flushReadTest) {
    const auto param = FifoTest::GetParam();
    std::vector<float> data(param.requestedBufferSize);
    FIFO<float> fifo(param.requestedBufferSize);
    fifo.releaseReadRegions(param.offset >= 0
                    ? param.offset
                    : std::numeric_limits<std::size_t>::max() + param.offset +
                            1);
    fifo.releaseWriteRegions(param.offset >= 0
                    ? param.offset
                    : std::numeric_limits<std::size_t>::max() + param.offset +
                            1);

    ASSERT_EQ(0, fifo.readAvailable());
    ASSERT_EQ(100, fifo.write(data.data(), 100));
    ASSERT_EQ(static_cast<int>((param.offset + 50) % param.expectedBufferSize),
            fifo.flushReadData(50));
    ASSERT_EQ(static_cast<int>((param.offset + 100) % param.expectedBufferSize),
            fifo.flushReadData(1000000));
    ASSERT_EQ(static_cast<int>(param.expectedBufferSize), fifo.write(data.data(), 1000000));
    ASSERT_EQ(static_cast<int>(param.expectedBufferSize), fifo.readAvailable());
}

TEST_P(FifoTest, readWriteStressTest) {
    const auto param = FifoTest::GetParam();
    std::vector<uint32_t> data(param.expectedBufferSize);
    FIFO<uint32_t> fifo(param.requestedBufferSize);
    fifo.releaseReadRegions(param.offset >= 0
                    ? param.offset
                    : std::numeric_limits<std::size_t>::max() + param.offset +
                            1);
    fifo.releaseWriteRegions(param.offset >= 0
                    ? param.offset
                    : std::numeric_limits<std::size_t>::max() + param.offset +
                            1);

    std::vector<uint32_t> wdata(param.expectedBufferSize + param.expectedBufferSize / 10);
    std::vector<uint32_t> rdata(param.expectedBufferSize + param.expectedBufferSize / 10);
    uint32_t k = 0;
    uint32_t j = 0;

    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> dist(
            0, static_cast<int>(param.expectedBufferSize + param.expectedBufferSize / 10));

    while (k < 1000000) {
        int n = dist(mt);
        int m = std::min(n, fifo.writeAvailable());
        for (int i = 0; i < m; i++) {
            wdata[i] = k++;
        }
        ASSERT_EQ(static_cast<int>(m), fifo.write(wdata.data(), n));
        n = dist(mt);
        m = std::min(n, fifo.readAvailable());
        ASSERT_EQ(static_cast<int>(m), fifo.read(rdata.data(), n));
        for (int i = 0; i < m; i++) {
            ASSERT_EQ(j++, rdata[i]);
        }
    }
}

TEST_P(FifoTest, readWriteStressTestRegions) {
    const auto param = FifoTest::GetParam();
    std::vector<uint32_t> data(param.expectedBufferSize);
    FIFO<uint32_t> fifo(param.requestedBufferSize);
    fifo.releaseReadRegions(param.offset >= 0
                    ? param.offset
                    : std::numeric_limits<std::size_t>::max() + param.offset +
                            1);
    fifo.releaseWriteRegions(param.offset >= 0
                    ? param.offset
                    : std::numeric_limits<std::size_t>::max() + param.offset +
                            1);

    std::vector<uint32_t> wdata(param.expectedBufferSize + param.expectedBufferSize / 10);
    std::vector<uint32_t> rdata(param.expectedBufferSize + param.expectedBufferSize / 10);
    uint32_t k = 0;
    uint32_t j = 0;

    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> dist(
            0, param.expectedBufferSize + param.expectedBufferSize / 10);

    while (k < 1000000) {
        int n = dist(mt);
        int m = std::min(n, fifo.writeAvailable());
        uint32_t* ptr1;
        ring_buffer_size_t size1;
        uint32_t* ptr2;
        ring_buffer_size_t size2;
        ASSERT_EQ(static_cast<int>(m), fifo.aquireWriteRegions(n, &ptr1, &size1, &ptr2, &size2));
        ASSERT_EQ(static_cast<int>(m), size1 + size2);
        for (int i = 0; i < size1; i++) {
            ptr1[i] = k++;
        }
        for (int i = 0; i < size2; i++) {
            ptr2[i] = k++;
        }
        fifo.releaseWriteRegions(m);
        n = dist(mt);
        m = std::min(n, fifo.readAvailable());
        ASSERT_EQ(static_cast<int>(m), fifo.aquireReadRegions(n, &ptr1, &size1, &ptr2, &size2));
        ASSERT_EQ(static_cast<int>(m), size1 + size2);
        for (int i = 0; i < size1; i++) {
            ASSERT_EQ(j++, ptr1[i]);
        }
        for (int i = 0; i < size2; i++) {
            ASSERT_EQ(j++, ptr2[i]);
        }
        fifo.releaseReadRegions(m);
    }
}

INSTANTIATE_TEST_SUITE_P(FifoTestSuite,
        FifoTest,
        testing::Values(
                Param{1024, 1024, 0},
                Param{1024, 1024, 1200},
                Param{1024, 1024, -1200},
                Param{65536, 65536, 0},
                Param{65536, 65536, 1200},
                Param{65536, 65536, -1200},
                Param{1234, 2048, 0},
                Param{1234, 2048, -1200},
                Param{1234, 2048, 1200}));

} // namespace
