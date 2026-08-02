#include "findonwebmenutunebat.h"

#include <QMenu>
#include <QUrl>
#include <QUrlQuery>

#include "moc_findonwebmenutunebat.cpp"
#include "track/track.h"
#include "util/parented_ptr.h"

namespace {
const QString kServiceTitle = QStringLiteral("Tunebat");
const QString kSearchUrl = QStringLiteral("https://tunebat.com/Search");

QUrl composeTunebatUrl(const QString& query) {
    QUrl url(kSearchUrl);
    QUrlQuery urlQuery;
    urlQuery.addQueryItem("q", query);
    url.setQuery(urlQuery);
    return url;
}
} // namespace

FindOnWebMenuTunebat::FindOnWebMenuTunebat(const QPointer<QMenu>& pFindOnWebMenu,
        QPointer<FindOnWebLast> pFindOnWebLast,
        const Track& track)
        : WFindOnWebMenu(pFindOnWebMenu, std::move(pFindOnWebLast)) {
    const QString artist = track.getArtist();
    const QString trackTitle = track.getTitle();
    setTitle(kServiceTitle);
    pFindOnWebMenu->addMenu(this);
    addSeparator();
    if (!artist.isEmpty()) {
        if (!trackTitle.isEmpty()) {
            const QUrl TunebatUrlArtistWithTrackTitle =
                    composeTunebatUrl(artist + " " + trackTitle);
            addActionToServiceMenu(
                    kServiceTitle + QStringLiteral(",Artist,Title"),
                    tr("Artist + Title"),
                    TunebatUrlArtistWithTrackTitle);
        }
    }
}
