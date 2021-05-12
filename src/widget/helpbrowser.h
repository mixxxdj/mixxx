#include <QPoint>
#include <QTextBrowser>
#include <QUrl>
#include <QVariant>

// forward declarations
QT_FORWARD_DECLARE_CLASS(QHelpEngine);
QT_FORWARD_DECLARE_CLASS(QWidget);

namespace mixxx {

/// This is the HTML renderer that shows Qt Help files and supports
/// loading `qthelp://` URLs and has a custom context menu.
class HelpBrowser : public QTextBrowser {
  public:
    HelpBrowser(QHelpEngine* pHelpEngine, QWidget* parent = nullptr);
    QVariant loadResource(int type, const QUrl& name);

  private slots:
    void slotAnchorClicked(const QUrl& link);
    void slotCustomContextMenu(const QPoint& pos);

  private:
    QHelpEngine* m_pHelpEngine;
};

} // namespace mixxx
