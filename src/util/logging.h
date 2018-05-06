#ifndef MIXXX_UTIL_LOGGING_H
#define MIXXX_UTIL_LOGGING_H

namespace mixxx {

static const int kDebugLevelMin = 0;
static const int kDebugLevelDefault = 1;

class Logging {
  public:
    static void initialize();
    static void shutdown();
  private:
    Logging() = delete;
};

void install_message_handler();

}  // namespace mixxx

#endif /* MIXXX_UTIL_LOGGING_H */
