#pragma once

#include <QByteArray>

inline QByteArray& qByteArrayReplaceWithPositionAndSize(QByteArray* qbytearray,
        const qsizetype& index,
        const qsizetype& len,
        const char* s,
        const qsizetype& alen) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return qbytearray->replace(index, len, s, alen);
#else
    return qbytearray->replace(static_cast<int>(index),
            static_cast<int>(len),
            s,
            static_cast<int>(alen));
#endif
}
