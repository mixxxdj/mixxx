#pragma once

#include "qml/qmlplayerproxy.h"

namespace mixxx {
namespace qml {
class JavascriptPlayerProxy : private QmlPlayerProxy {
    Q_OBJECT
    Q_PROPERTY(bool isLoaded READ isLoaded NOTIFY trackChanged)
    Q_PROPERTY(QString artist READ getArtist NOTIFY artistChanged)
    Q_PROPERTY(QString title READ getTitle NOTIFY titleChanged)
    Q_PROPERTY(QString album READ getAlbum NOTIFY albumChanged)
    Q_PROPERTY(QString albumArtist READ getAlbumArtist WRITE setAlbumArtist
                    NOTIFY albumArtistChanged)
    Q_PROPERTY(QString genre READ getGenre STORED false NOTIFY genreChanged)
    Q_PROPERTY(QString composer READ getComposer NOTIFY composerChanged)
    Q_PROPERTY(QString grouping READ getGrouping NOTIFY groupingChanged)
    Q_PROPERTY(QString year READ getYear NOTIFY yearChanged)
    Q_PROPERTY(QString trackNumber READ getTrackNumber WRITE setTrackNumber
                    NOTIFY trackNumberChanged)
    Q_PROPERTY(QString trackTotal READ getTrackTotal NOTIFY trackTotalChanged)

  public:
    using QmlPlayerProxy::QmlPlayerProxy;
    JavascriptPlayerProxy(BaseTrackPlayer* pTrackPlayer, QObject* parent = nullptr)
            : QmlPlayerProxy(pTrackPlayer, parent) {
    }
};
} // namespace qml
} // namespace mixxx