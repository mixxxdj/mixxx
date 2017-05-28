#include <cstring>

#include "util/logger.h"


namespace {

const char* const kLogPreambleSuffix = " -";
const std::size_t kLogPreambleSuffixLen = std::strlen(kLogPreambleSuffix);

inline QByteArray preambleChars(const QLatin1String& logContext) {
    QByteArray preamble;
    std::size_t logContextLen = std::strlen(logContext.latin1());
    if (logContextLen > 0) {
        preamble.reserve(logContextLen + kLogPreambleSuffixLen);
        preamble.append(logContext.latin1(), logContextLen);
        preamble.append(kLogPreambleSuffix, kLogPreambleSuffixLen);
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
