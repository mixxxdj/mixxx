#ifndef MIXXX_UTIL_LOGGING_H
#define MIXXX_UTIL_LOGGING_H

namespace mixxx {

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
