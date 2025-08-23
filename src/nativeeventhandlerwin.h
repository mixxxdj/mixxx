#if defined(__WINDOWS__)

#include <QAbstractNativeEventFilter>

class WindowsEventHandler : public QAbstractNativeEventFilter {
  public:
    WindowsEventHandler() = default;
    ~WindowsEventHandler() override = default;
    bool nativeEventFilter(const QByteArray& eventType, void* message, qintptr* result) override;
};

#endif
