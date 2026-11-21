#pragma once

#include <QByteArrayView>
#include <QtGlobal>
#include <cstddef>
#include <cstring>

namespace mixxx {

/// The same value as qChecksum(data) (Qt::ChecksumIso3309, CRC-16/X-25), eight
/// bytes per step. qChecksum goes through a 16 entry table one nibble at a
/// time (about 2 lookups per byte), this is about 24x faster. The checksum is
/// stored in the database (analysis.data_checksum), so the algorithm cannot
/// change.
inline quint16 fastChecksum(const char* pData, std::size_t length) {
    struct Tables {
        quint16 t[8][256];
        Tables() {
            for (unsigned i = 0; i < 256; i++) {
                quint16 crc = static_cast<quint16>(i);
                for (int bit = 0; bit < 8; bit++) {
                    crc = (crc & 1) ? static_cast<quint16>((crc >> 1) ^ 0x8408)
                                    : static_cast<quint16>(crc >> 1);
                }
                t[0][i] = crc;
            }
            for (unsigned i = 0; i < 256; i++) {
                for (int k = 1; k < 8; k++) {
                    const quint16 prev = t[k - 1][i];
                    t[k][i] = static_cast<quint16>((prev >> 8) ^ t[0][prev & 0xFF]);
                }
            }
        }
    };
    static const Tables tables;

    const auto* p = reinterpret_cast<const unsigned char*>(pData);
    quint32 crc = 0xFFFF;
    while (length >= 8) {
        const quint32 x = crc ^ (static_cast<quint32>(p[0]) | (static_cast<quint32>(p[1]) << 8));
        crc = tables.t[7][x & 0xFF] ^ tables.t[6][(x >> 8) & 0xFF] ^
                tables.t[5][p[2]] ^ tables.t[4][p[3]] ^ tables.t[3][p[4]] ^
                tables.t[2][p[5]] ^ tables.t[1][p[6]] ^ tables.t[0][p[7]];
        p += 8;
        length -= 8;
    }
    while (length-- > 0) {
        crc = (crc >> 8) ^ tables.t[0][(crc ^ *p++) & 0xFF];
    }
    return static_cast<quint16>(~crc);
}

inline quint16 fastChecksum(QByteArrayView data) {
    return fastChecksum(data.constData(), static_cast<std::size_t>(data.size()));
}

} // namespace mixxx
