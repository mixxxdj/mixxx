#include "wfindonwebmenu.h"

#include <QMenu>
#include <QtDebug>

#include "moc_wfindonwebmenu.cpp"
#include "track/track.h"
#include "util/desktophelper.h"

WFindOnWebMenu::WFindOnWebMenu(QWidget* parent)
        : QMenu(tr("Find on Web"), parent) {
}

bool WFindOnWebMenu::hasEntriesForTrack(const Track& track) {
    return !(track.getArtist().isEmpty() &&
            track.getAlbum().isEmpty() &&
            track.getTitle().isEmpty());
}

void WFindOnWebMenu::addActionToServiceMenu(
        QMenu* serviceMenu, const QString& actionText, const QUrl& serviceUrl) {
    serviceMenu->addAction(actionText,
            this,
            [this, serviceUrl] {
                openInBrowser(serviceUrl);
            });
}

QString WFindOnWebMenu::composeActionText(const QString& prefix, const QString& trackProperty) {
    return prefix + QStringLiteral(" | ") + trackProperty;
}

QString WFindOnWebMenu::composeSearchQuery(
        const QString& artist, const QString& trackAlbumOrTitle) {
    return artist + QStringLiteral(" ") + trackAlbumOrTitle;
}

void WFindOnWebMenu::openInBrowser(const QUrl& url) {
    if (!mixxx::DesktopHelper::openUrl(url)) {
        qWarning() << "DesktopHelper::openUrl() failed for " << url;
        DEBUG_ASSERT(false);
    }
}
