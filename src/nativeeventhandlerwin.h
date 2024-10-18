#if defined(__WINDOWS__)

#include <QAbstractNativeEventFilter>

class WindowsEventHandler : public QAbstractNativeEventFilter {
  public:
    WindowsEventHandler() = default;
    ~WindowsEventHandler() override = default;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    bool nativeEventFilter(const QByteArray& eventType, void* message, long* result) override;
#else
    bool nativeEventFilter(const QByteArray& eventType, void* message, qintptr* result) override;
#endif
};

#endif
