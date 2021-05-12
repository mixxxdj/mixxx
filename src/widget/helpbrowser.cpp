#include "widget/helpbrowser.h"

#include <QDesktopServices>
#include <QHelpEngine>
#include <QMenu>

#include "defs_urls.h"

namespace {

const QString kHelpUrlScheme = QStringLiteral("qthelp");

} // anonymous namespace

namespace mixxx {

HelpBrowser::HelpBrowser(QHelpEngine* pHelpEngine, QWidget* parent)
        : QTextBrowser(parent),
          m_pHelpEngine(pHelpEngine) {
    // Add custom link handling
    setOpenLinks(false);
    connect(this,
            &QTextBrowser::anchorClicked,
            this,
            &HelpBrowser::slotAnchorClicked);

    // Add custom context menu
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this,
            &QTextBrowser::customContextMenuRequested,
            this,
            &HelpBrowser::slotCustomContextMenu);
}

QVariant HelpBrowser::loadResource(int type, const QUrl& name) {
    if (name.scheme() == kHelpUrlScheme) {
        return QVariant(m_pHelpEngine->fileData(name));
    }

    return QTextBrowser::loadResource(type, name);
}

void HelpBrowser::slotAnchorClicked(const QUrl& link) {
    if (link.scheme() == kHelpUrlScheme) {
        setSource(link);
        return;
    }

    // Load external links in Webbrowser
    QDesktopServices::openUrl(link);
}

void HelpBrowser::slotCustomContextMenu(const QPoint& pos) {
    QMenu* menu = createStandardContextMenu();

    // Add "Open in Browser" action
    QUrl helpUrl = source();
    if (helpUrl.scheme() == kHelpUrlScheme && helpUrl.path().startsWith("/doc/")) {
        QUrl webUrl(MIXXX_MANUAL_URL + helpUrl.path(QUrl::FullyEncoded).mid(4));
        if (helpUrl.hasQuery()) {
            webUrl.setQuery(helpUrl.query());
        }
        if (helpUrl.hasFragment()) {
            webUrl.setFragment(helpUrl.fragment());
        }
        menu->addSeparator();
        menu->addAction(tr("Open in Browser"), [webUrl]() {
            QDesktopServices::openUrl(webUrl);
        });
    }
    menu->exec(mapToGlobal(pos));
}

} // namespace mixxx
