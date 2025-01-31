#include <gtest/gtest.h>

#include <algorithm>
#include <random>
#include <thread>

#include "util/fifo.h"

namespace {

struct Param {
    int requestedBufferSize;
    int expectedBufferSize;
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
    ASSERT_EQ(param.expectedBufferSize - 50, fifo.write(data.data(), 1000000));
    ASSERT_EQ(param.expectedBufferSize, fifo.readAvailable());
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

    int expected;
    expected = (param.offset + 50) % param.expectedBufferSize;
    if (expected < 0) {
        expected += param.expectedBufferSize;
    }
    ASSERT_EQ(expected, fifo.flushReadData(50));

    expected = (param.offset + 100) % param.expectedBufferSize;
    if (expected < 0) {
        expected += param.expectedBufferSize;
    }
    ASSERT_EQ(expected, fifo.flushReadData(1000000));
    ASSERT_EQ(param.expectedBufferSize, fifo.write(data.data(), 1000000));
    ASSERT_EQ(param.expectedBufferSize, fifo.readAvailable());
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
            0, param.expectedBufferSize + param.expectedBufferSize / 10);

    while (k < 1000000) {
        int n = dist(mt);
        int m = std::min(n, fifo.writeAvailable());
        for (int i = 0; i < m; i++) {
            wdata[i] = k++;
        }
        ASSERT_EQ(m, fifo.write(wdata.data(), n));
        n = dist(mt);
        m = std::min(n, fifo.readAvailable());
        ASSERT_EQ(m, fifo.read(rdata.data(), n));
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
        ASSERT_EQ(m, fifo.aquireWriteRegions(n, &ptr1, &size1, &ptr2, &size2));
        ASSERT_EQ(m, size1 + size2);
        for (int i = 0; i < size1; i++) {
            ptr1[i] = k++;
        }
        for (int i = 0; i < size2; i++) {
            ptr2[i] = k++;
        }
        fifo.releaseWriteRegions(m);
        n = dist(mt);
        m = std::min(n, fifo.readAvailable());
        ASSERT_EQ(m, fifo.aquireReadRegions(n, &ptr1, &size1, &ptr2, &size2));
        ASSERT_EQ(m, size1 + size2);
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

constexpr int rwtotal = 500000000;

template<class T_FIFO>
class MultiThreadRW {
    T_FIFO m_fifo;
    bool m_ok;
    const int m_bufferSize;
    const int m_total;
    const bool m_wait;

  public:
    MultiThreadRW(int ringBufferSize, int bufferSize, int total, bool wait)
            : m_fifo(ringBufferSize),
              m_ok{},
              m_bufferSize(bufferSize),
              m_total(total),
              m_wait(wait) {
    }

    void write() {
        int k = 0;
        std::vector<int> buffer(m_bufferSize);
        while (k != m_total) {
            int n = std::min(m_bufferSize, m_total - k);
            if (m_wait) {
                while (m_fifo.writeAvailable() < n) {
                }
            }
            n = std::min(n, m_fifo.writeAvailable());
            for (int j = 0; j < n; j++) {
                buffer[j] = k++;
            }
            m_fifo.write(buffer.data(), n);
        }
    }

    void read() {
        m_ok = true;
        int k = 0;
        std::vector<int> buffer(m_bufferSize);
        while (k != m_total) {
            int n = std::min(m_bufferSize, m_total - k);
            if (m_wait) {
                while (m_fifo.readAvailable() < n) {
                }
            }
            n = m_fifo.read(buffer.data(), n);
            for (int j = 0; j < n; j++) {
                m_ok &= (buffer[j] == k++);
            }
        }
    }

    bool run() {
        m_ok = true;
        std::thread th1(&MultiThreadRW<T_FIFO>::write, this);
        std::thread th2(&MultiThreadRW<T_FIFO>::read, this);
        th1.join();
        th2.join();
        return m_ok;
    }
};

TEST(FifoTest, MultiThreadRW) {
    MultiThreadRW<FIFO<int>> io(65536, 1024, rwtotal, false);
    bool ok = io.run();
    ASSERT_TRUE(ok);
}

TEST(FifoTest, MultiThreadRW_PA) {
    MultiThreadRW<PA::FIFO<int>> io(65536, 1024, rwtotal, false);
    bool ok = io.run();
    ASSERT_TRUE(ok);
}

TEST(FifoTest, MultiThreadRW_Wait) {
    MultiThreadRW<FIFO<int>> io(65536, 1024, rwtotal, true);
    bool ok = io.run();
    ASSERT_TRUE(ok);
}

TEST(FifoTest, MultiThreadRW_PA_Wait) {
    MultiThreadRW<PA::FIFO<int>> io(65536, 1024, rwtotal, true);
    bool ok = io.run();
    ASSERT_TRUE(ok);
}

template<class T_FIFO>
class MultiThreadRegionRW {
    T_FIFO m_fifo;
    bool m_ok;
    const int m_bufferSize;
    const int m_total;
    const bool m_wait;

  public:
    MultiThreadRegionRW(
            int ringBufferSize, int bufferSize, int total, bool wait)
            : m_fifo(ringBufferSize),
              m_ok{},
              m_bufferSize(bufferSize),
              m_total(total),
              m_wait(wait) {
    }

    void write() {
        int k = 0;
        std::vector<int> buffer(m_bufferSize);
        while (k != m_total) {
            int n = std::min(m_bufferSize, m_total - k);
            int* ptr1;
            int* ptr2;
            ring_buffer_size_t size1;
            ring_buffer_size_t size2;
            if (m_wait) {
                while (m_fifo.writeAvailable() < n) {
                }
            }
            n = m_fifo.aquireWriteRegions(n, &ptr1, &size1, &ptr2, &size2);
            for (int j = 0; j < size1; j++) {
                ptr1[j] = k++;
            }
            for (int j = 0; j < size2; j++) {
                ptr2[j] = k++;
            }
            m_fifo.releaseWriteRegions(n);
        }
    }

    void read() {
        m_ok = true;
        int k = 0;
        std::vector<int> buffer(m_bufferSize);
        while (k != m_total) {
            int n = std::min(m_bufferSize, m_total - k);
            int* ptr1;
            int* ptr2;
            ring_buffer_size_t size1;
            ring_buffer_size_t size2;
            if (m_wait) {
                while (m_fifo.readAvailable() < n) {
                }
            }
            n = m_fifo.aquireReadRegions(n, &ptr1, &size1, &ptr2, &size2);
            for (int j = 0; j < size1; j++) {
                m_ok &= (ptr1[j] == k++);
            }
            for (int j = 0; j < size2; j++) {
                m_ok &= (ptr2[j] == k++);
            }
            m_fifo.releaseReadRegions(n);
        }
    }

    bool run() {
        m_ok = true;
        std::thread th1(&MultiThreadRegionRW<T_FIFO>::write, this);
        std::thread th2(&MultiThreadRegionRW<T_FIFO>::read, this);
        th1.join();
        th2.join();
        return m_ok;
    }
};

TEST(FifoTest, MultiThreadRegionRW) {
    MultiThreadRegionRW<FIFO<int>> io(65536, 256, rwtotal, false);
    bool ok = io.run();
    ASSERT_TRUE(ok);
}

TEST(FifoTest, MultiThreadRegionRW_PA) {
    MultiThreadRegionRW<PA::FIFO<int>> io(65536, 256, rwtotal, false);
    bool ok = io.run();
    ASSERT_TRUE(ok);
}

TEST(FifoTest, MultiThreadRegionRW_Wait) {
    MultiThreadRegionRW<FIFO<int>> io(65536, 256, rwtotal, true);
    bool ok = io.run();
    ASSERT_TRUE(ok);
}

TEST(FifoTest, MultiThreadRegionRW_PA_Wait) {
    MultiThreadRegionRW<PA::FIFO<int>> io(65536, 256, rwtotal, true);
    bool ok = io.run();
    ASSERT_TRUE(ok);
}
