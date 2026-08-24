#include "findonwebmenuwikipedia.h"

#include <QMenu>
#include <QUrlQuery>

#include "moc_findonwebmenuwikipedia.cpp"
#include "track/track.h"
#include "util/parented_ptr.h"

namespace {
const QString kServiceTitle = QStringLiteral("Wikipedia");

const QString kSearchUrl = QStringLiteral("https://en.wikipedia.org/w/index.php?");

const QUrl composeWikipediaUrl(const QString& query) {
    QUrlQuery urlQuery;
    urlQuery.addQueryItem("search", query);
    QUrl url(kSearchUrl);
    url.setQuery(urlQuery);
    return url;
}
} // namespace

FindOnWebMenuWikipedia::FindOnWebMenuWikipedia(const QPointer<QMenu>& pFindOnWebMenu,
        QPointer<FindOnWebLast> pFindOnWebLast,
        const Track& track)
        : WFindOnWebMenu(pFindOnWebMenu, std::move(pFindOnWebLast)) {
    const QString artist = track.getArtist();
    setTitle(kServiceTitle);
    pFindOnWebMenu->addMenu(this);
    addSeparator();
    if (!artist.isEmpty()) {
        const QUrl WikipediaUrlArtist = composeWikipediaUrl(artist);
        addActionToServiceMenu(
                kServiceTitle + QStringLiteral(",Artist"),
                tr("Artist"),
                WikipediaUrlArtist);
    }
}
