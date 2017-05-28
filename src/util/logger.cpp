#include "util/logger.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
#include <cstring>
#endif


namespace {

const QLatin1String kLogPreambleSuffix(" -");
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
const int kLogPreambleSuffixLen = std::strlen(kLogPreambleSuffix.latin1());
#else
const int kLogPreambleSuffixLen = kLogPreambleSuffix.size();
#endif

inline QByteArray preambleChars(const QLatin1String& logContext) {
    QByteArray preamble;
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    const int logContextLen = std::strlen(logContext.latin1());
#else
    const int logContextLen = logContext.size();
#endif
    if (logContextLen > 0) {
        preamble.reserve(logContextLen + kLogPreambleSuffixLen);
        preamble.append(logContext.latin1(), logContextLen);
        preamble.append(kLogPreambleSuffix.latin1(), kLogPreambleSuffixLen);
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
