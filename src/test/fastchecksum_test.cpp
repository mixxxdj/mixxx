#include "util/fastchecksum.h"

#include <gtest/gtest.h>

#include <QByteArray>
#include <QByteArrayView>
#include <random>

namespace {

TEST(FastChecksumTest, MatchesQChecksum) {
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> byteDist(0, 255);
    // Cover the tail loop (lengths not a multiple of 8) and longer buffers
    for (int length = 0; length < 300; ++length) {
        QByteArray data(length, Qt::Uninitialized);
        for (int i = 0; i < length; ++i) {
            data[i] = static_cast<char>(byteDist(rng));
        }
        EXPECT_EQ(qChecksum(QByteArrayView(data)),
                mixxx::fastChecksum(QByteArrayView(data)))
                << "length " << length;
    }
    QByteArray large(1 << 20, Qt::Uninitialized);
    for (int i = 0; i < large.size(); ++i) {
        large[i] = static_cast<char>(byteDist(rng));
    }
    EXPECT_EQ(qChecksum(QByteArrayView(large)),
            mixxx::fastChecksum(QByteArrayView(large)));
}

TEST(FastChecksumTest, UnalignedStart) {
    QByteArray data(64, Qt::Uninitialized);
    for (int i = 0; i < data.size(); ++i) {
        data[i] = static_cast<char>(i * 37 + 11);
    }
    for (int offset = 0; offset < 8; ++offset) {
        const QByteArrayView view(data.constData() + offset, data.size() - offset);
        EXPECT_EQ(qChecksum(view), mixxx::fastChecksum(view));
    }
}

} // namespace
