#include <cstring>

#include "util/logger.h"


namespace {

inline QString preambleString(const char* logContext) {
    if ((logContext == nullptr) || (std::strlen(logContext) == 0)) {
        return QString();
    } else {
        return QString("%1 -").arg(logContext);
    }
}

inline QString preambleString(const QString& logContext) {
    if (logContext.isEmpty()) {
        return QString();
    } else {
        return QString("%1 -").arg(logContext);
    }
}

} // anonymous namespace

namespace mixxx {

Logger::Logger(const char* logContext)
    : m_preambleChars(preambleString(logContext).toLocal8Bit()) {
}
Logger::Logger(const QString& logContext)
    : m_preambleChars(preambleString(logContext).toLocal8Bit()) {
}

}  // namespace mixxx
