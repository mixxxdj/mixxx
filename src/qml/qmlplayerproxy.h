#pragma once
#include <QColor>
#include <QObject>
#include <QPointer>
#include <QQmlEngine>
#include <QString>
#include <QUrl>

#include "controllers/scripting/javascriptplayerproxy.h"
#include "mixer/basetrackplayer.h"
#include "qml/qmlbeatsmodel.h"
#include "qml/qmlcuesmodel.h"
#include "qml/qmlstemsmodel.h"
#include "track/cueinfo.h"
#include "track/track.h"
#include "waveform/waveform.h"

namespace mixxx {
namespace qml {

class QmlPlayerProxy : public JavascriptPlayerProxy {
    Q_OBJECT
    Q_PROPERTY(bool isLoaded READ isLoaded NOTIFY trackChanged)
    Q_PROPERTY(QString artist READ getArtist WRITE setArtist NOTIFY artistChanged)
    Q_PROPERTY(QString title READ getTitle WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QString album READ getAlbum WRITE setAlbum NOTIFY albumChanged)
    Q_PROPERTY(QString albumArtist READ getAlbumArtist WRITE setAlbumArtist
                    NOTIFY albumArtistChanged)
    Q_PROPERTY(QString genre READ getGenre STORED false NOTIFY genreChanged)
    Q_PROPERTY(QString composer READ getComposer WRITE setComposer NOTIFY composerChanged)
    Q_PROPERTY(QString grouping READ getGrouping WRITE setGrouping NOTIFY groupingChanged)
    Q_PROPERTY(QString year READ getYear WRITE setYear NOTIFY yearChanged)
    Q_PROPERTY(QString trackNumber READ getTrackNumber WRITE setTrackNumber
                    NOTIFY trackNumberChanged)
    Q_PROPERTY(QString trackTotal READ getTrackTotal WRITE setTrackTotal NOTIFY trackTotalChanged)
    Q_PROPERTY(QString comment READ getComment WRITE setComment NOTIFY commentChanged)
    Q_PROPERTY(QString keyText READ getKeyText WRITE setKeyText NOTIFY keyTextChanged)
    Q_PROPERTY(QColor color READ getColor WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(QUrl coverArtUrl READ getCoverArtUrl NOTIFY coverArtUrlChanged)
    Q_PROPERTY(QUrl trackLocationUrl READ getTrackLocationUrl NOTIFY trackLocationUrlChanged)
    QML_NAMED_ELEMENT(Player)
    QML_UNCREATABLE("Only accessible via Mixxx.PlayerManager.getPlayer(group)")

    Q_PROPERTY(int waveformLength READ getWaveformLength NOTIFY waveformLengthChanged)
    Q_PROPERTY(QString waveformTexture READ getWaveformTexture NOTIFY waveformTextureChanged)
    Q_PROPERTY(int waveformTextureSize READ getWaveformTextureSize NOTIFY
                    waveformTextureSizeChanged)
    Q_PROPERTY(int waveformTextureStride READ getWaveformTextureStride NOTIFY
                    waveformTextureStrideChanged)

    Q_PROPERTY(mixxx::qml::QmlBeatsModel* beatsModel MEMBER m_pBeatsModel CONSTANT);
    Q_PROPERTY(mixxx::qml::QmlCuesModel* hotcuesModel MEMBER m_pHotcuesModel CONSTANT);
#ifdef __STEM__
    Q_PROPERTY(mixxx::qml::QmlStemsModel* stemsModel READ getStemsModel CONSTANT);
#endif

  public:
    explicit QmlPlayerProxy(BaseTrackPlayer* pTrackPlayer, QObject* parent = nullptr);

    QString getComment() const;
    QString getKeyText() const;
    QColor getColor() const;
    QUrl getCoverArtUrl() const;
    QUrl getTrackLocationUrl() const;

    /// Needed for interacting with the raw track player object.
    BaseTrackPlayer* internalTrackPlayer() const {
        return m_pTrackPlayer;
    }

    Q_INVOKABLE void loadTrackFromLocation(const QString& trackLocation, bool play = false);
    Q_INVOKABLE void loadTrackFromLocationUrl(const QUrl& trackLocationUrl, bool play = false);

#ifdef __STEM__
    QmlStemsModel* getStemsModel() const {
        return m_pStemsModel.get();
    }
#endif

  public slots:
    void slotBeatsChanged();
    void slotHotcuesChanged();
#ifdef __STEM__
    void slotStemsChanged();
#endif

    void setArtist(const QString& artist);
    void setTitle(const QString& title);
    void setAlbum(const QString& album);
    void setAlbumArtist(const QString& albumArtist);
    void setComposer(const QString& composer);
    void setGrouping(const QString& grouping);
    void setYear(const QString& year);
    void setTrackNumber(const QString& trackNumber);
    void setTrackTotal(const QString& trackTotal);
    void setComment(const QString& comment);
    void setKeyText(const QString& keyText);
    void setColor(const QColor& color);

  signals:
    void cloneFromGroup(const QString& group);

    void commentChanged();
    void keyTextChanged();
    void colorChanged();
    void coverArtUrlChanged();
    void trackLocationUrlChanged();
    void cuesChanged();
#ifdef __STEM__
    void stemsChanged();
#endif
    void loadTrackFromLocationRequested(const QString& trackLocation, bool play);

  private:
    void subclassesSlotTrackLoaded() override;
    void subclassesSlotTrackChanged() override;

    QmlBeatsModel* m_pBeatsModel;
    QmlCuesModel* m_pHotcuesModel;
#ifdef __STEM__
    std::unique_ptr<QmlStemsModel> m_pStemsModel;
#endif
};

} // namespace qml
} // namespace mixxx
