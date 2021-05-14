#include "util/logger.h"


// NOTE(uklotzde): The initialization of Logger must not depend on any
// static data, because Logger instances are usually instantiated as
// static instances within other compilation units. The order of static
// initializations across compilation units is undefined!!

namespace {

inline QByteArray preambleChars(const QLatin1String& logContext) {
    QByteArray preamble;
    const int logContextLen = logContext.size();
    if (logContextLen > 0) {
        const QLatin1String preambleSuffix(" -");
        const int preambleSuffixLen = preambleSuffix.size();
        preamble.reserve(logContextLen + preambleSuffixLen);
        preamble.append(logContext.latin1(), logContextLen);
        preamble.append(preambleSuffix.latin1(), preambleSuffixLen);
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
