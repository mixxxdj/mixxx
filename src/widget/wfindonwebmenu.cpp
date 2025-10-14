#include "wfindonwebmenu.h"

#include <QMenu>
#include <QtDebug>

#include "moc_wfindonwebmenu.cpp"
#include "track/track.h"
#include "util/desktophelper.h"
#include "widget/findonweblast.h"

WFindOnWebMenu::WFindOnWebMenu(
        const QPointer<QMenu>& pParent, QPointer<FindOnWebLast> pFindOnWebLast)
        : QMenu(pParent),
          m_pFindOnWebLast(std::move(pFindOnWebLast)) {
}

bool WFindOnWebMenu::hasEntriesForTrack(const Track& track) {
    return !(track.getArtist().isEmpty() &&
            track.getAlbum().isEmpty() &&
            track.getTitle().isEmpty());
}

void WFindOnWebMenu::addActionToServiceMenu(
        const QString& actionId,
        const QString& actionText,
        const QUrl& serviceUrl) {
    addAction(actionText,
            this,
            [this, actionId, actionText, serviceUrl] {
                openInBrowser(serviceUrl);
                m_pFindOnWebLast->update(actionId, actionText, serviceUrl);
            });
    m_pFindOnWebLast->init(actionId, actionText, serviceUrl);
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
