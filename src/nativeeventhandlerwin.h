#include <QAbstractNativeEventFilter>

#include "mixxxapplication.h"

class WindowsEventHandler : public QAbstractNativeEventFilter {
  public:
    WindowsEventHandler(MixxxApplication* pApp);

    bool nativeEventFilter(const QByteArray& eventType, void* message, long* result);

  private:
    MixxxApplication* m_pApp;
};
