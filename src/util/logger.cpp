#include <cstring>

#include "util/logger.h"


namespace {

const char* const kLogPreambleSuffix = " -";
const std::size_t kLogPreambleSuffixLen = std::strlen(kLogPreambleSuffix);

inline QByteArray preambleChars(const char* logContext) {
    QByteArray preamble;
    if (logContext != nullptr) {
        std::size_t logContextLen = std::strlen(logContext);
        if (logContextLen > 0) {
            preamble.reserve(logContextLen + kLogPreambleSuffixLen);
            preamble.append(logContext, logContextLen);
            preamble.append(kLogPreambleSuffix, kLogPreambleSuffixLen);
        }
    }
    return preamble;
}

} // anonymous namespace

namespace mixxx {

Logger::Logger(const char* logContext)
    : m_preambleChars(preambleChars(logContext)) {
}

Logger::Logger(const QLatin1String& logContext)
    : m_preambleChars(preambleChars(logContext.latin1())) {
}

}  // namespace mixxx
