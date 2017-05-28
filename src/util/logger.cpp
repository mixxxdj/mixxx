#include "util/logger.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
#include <cstring>
#endif


namespace {

const QLatin1String kLogPreambleSuffix(" -");
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
const std::size_t kLogPreambleSuffixLen = std::strlen(kLogPreambleSuffix.latin1());
#endif

inline QByteArray preambleChars(const QLatin1String& logContext) {
    QByteArray preamble;
    std::size_t logContextLen = std::strlen(logContext.latin1());
    if (logContextLen > 0) {
        preamble.reserve(logContextLen + kLogPreambleSuffixLen);
        preamble.append(logContext.latin1(), logContextLen);
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
        preamble.append(kLogPreambleSuffix.latin1(), kLogPreambleSuffixLen);
#else
        preamble.append(kLogPreambleSuffix.latin1(), kLogPreambleSuffix.size());
#endif
    }
    return preamble;
}

} // anonymous namespace

namespace mixxx {

Logger::Logger(const char* logContext)
    : m_preambleChars(preambleChars(QLatin1String(logContext))) {
}

Logger::Logger(const QLatin1String& logContext)
    : m_preambleChars(preambleChars(logContext)) {
}

}  // namespace mixxx
