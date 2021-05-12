#include <QFileInfo>
#include <QString>
#include <QWidget>

#include "preferences/usersettings.h"

// forward declarations
QT_FORWARD_DECLARE_CLASS(QHelpEngine);
QT_FORWARD_DECLARE_CLASS(QTabWidget);

namespace mixxx {

class HelpBrowser;

/// The help viewer class represents the whole help window, consisting of a
/// tabbed sidebar (content/index/search) and the HelpBrowser (i.e. the HTML
/// renderer).
class HelpViewer : public QWidget {
  public:
    explicit HelpViewer(const UserSettingsPointer& helpFile, QWidget* parent = nullptr);

    void open(const QString& documentPath);

    /// Returns true if local help is available. Needed to add a web suffix on
    /// the menubar if local help is not available.
    bool hasLocalHelp() {
        return m_pHelpEngine != nullptr;
    }

  private:
    void openDocument(const QString& documentPath);

    QHelpEngine* m_pHelpEngine;
    HelpBrowser* m_pHelpBrowser;
    QTabWidget* m_pTabWidget;

    QString m_documentUrlPrefix;
    QString m_language;
};

} // namespace mixxx
