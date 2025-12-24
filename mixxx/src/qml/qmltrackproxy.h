#pragma once
#include <QColor>
#include <QObject>
#include <QPointer>
#include <QQmlEngine>
#include <QString>
#include <QUrl>
#include <QtGlobal>

#include "mixer/basetrackplayer.h"
#include "qml/qmlbeatsmodel.h"
#include "qml/qmlcuesmodel.h"
#include "qml/qmlstemsmodel.h"
#include "track/track_decl.h"
#include "util/parented_ptr.h"

namespace mixxx {
namespace qml {

class QmlTrackProxy : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString artist READ getArtist WRITE setArtist NOTIFY artistChanged)
    Q_PROPERTY(QString title READ getTitle WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QString album READ getAlbum WRITE setAlbum NOTIFY albumChanged)
    Q_PROPERTY(QString albumArtist READ getAlbumArtist WRITE setAlbumArtist
                    NOTIFY albumArtistChanged)
    Q_PROPERTY(QString genre READ getGenre STORED false NOTIFY genreChanged)
    Q_PROPERTY(QString composer READ getComposer WRITE setComposer NOTIFY composerChanged)
    Q_PROPERTY(QString grouping READ getGrouping WRITE setGrouping NOTIFY groupingChanged)
    Q_PROPERTY(int stars READ getStars WRITE setStars NOTIFY starsChanged)
    Q_PROPERTY(QString year READ getYear WRITE setYear NOTIFY yearChanged)
    Q_PROPERTY(QString trackNumber READ getTrackNumber WRITE setTrackNumber
                    NOTIFY trackNumberChanged)
    Q_PROPERTY(QString trackTotal READ getTrackTotal WRITE setTrackTotal NOTIFY trackTotalChanged)
    Q_PROPERTY(QString comment READ getComment WRITE setComment NOTIFY commentChanged)
    Q_PROPERTY(QString keyText READ getKeyText WRITE setKeyText NOTIFY keyTextChanged)
    Q_PROPERTY(QColor color READ getColor WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(double duration READ getDuration NOTIFY durationChanged)
    Q_PROPERTY(int sampleRate READ getSampleRate NOTIFY sampleRateChanged)
    Q_PROPERTY(QUrl coverArtUrl READ getCoverArtUrl NOTIFY coverArtUrlChanged)
    Q_PROPERTY(QUrl trackLocationUrl READ getTrackLocationUrl NOTIFY trackLocationUrlChanged)

    Q_PROPERTY(mixxx::qml::QmlBeatsModel* beatsModel READ getBeatsModel CONSTANT);
    Q_PROPERTY(mixxx::qml::QmlCuesModel* hotcuesModel READ getCuesModel CONSTANT);
#ifdef __STEM__
    Q_PROPERTY(mixxx::qml::QmlStemsModel* stemsModel READ getStemsModel CONSTANT);
#endif

    QML_NAMED_ELEMENT(Track)
    QML_UNCREATABLE("Only accessible via Mixxx.PlayerManager and Mixxx.Library")
  public:
    explicit QmlTrackProxy(TrackPointer track, QObject* parent = nullptr);

    QString getTrack() const;
    QString getTitle() const;
    QString getArtist() const;
    QString getAlbum() const;
    QString getAlbumArtist() const;
    QString getGenre() const;
    QString getComposer() const;
    QString getGrouping() const;
    QString getYear() const;
    int getStars() const;
    QString getTrackNumber() const;
    QString getTrackTotal() const;
    QString getComment() const;
    QString getKeyText() const;
    QColor getColor() const;
    double getDuration() const;
    int getSampleRate() const;
    QUrl getCoverArtUrl() const;
    QUrl getTrackLocationUrl() const;

    QmlBeatsModel* getBeatsModel() const {
        return m_pBeatsModel.get();
    }

    QmlCuesModel* getCuesModel() const {
        return m_pHotcuesModel.get();
    }

#ifdef __STEM__
    QmlStemsModel* getStemsModel() const {
        return m_pStemsModel.get();
    }
#endif

    TrackPointer internal() const {
        return m_pTrack;
    }

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
    void setStars(int stars);
    void setYear(const QString& year);
    void setTrackNumber(const QString& trackNumber);
    void setTrackTotal(const QString& trackTotal);
    void setComment(const QString& comment);
    void setKeyText(const QString& keyText);
    void setColor(const QColor& color);

  signals:
    void albumChanged();
    void titleChanged();
    void artistChanged();
    void albumArtistChanged();
    void genreChanged();
    void composerChanged();
    void groupingChanged();
    void starsChanged();
    void yearChanged();
    void trackNumberChanged();
    void trackTotalChanged();
    void commentChanged();
    void keyTextChanged();
    void colorChanged();
    void durationChanged();
    void sampleRateChanged();
    void coverArtUrlChanged();
    void trackLocationUrlChanged();
    void cuesChanged();
#ifdef __STEM__
    void stemsChanged();
#endif

  private:
    TrackPointer m_pTrack;
    parented_ptr<QmlBeatsModel> m_pBeatsModel;
    parented_ptr<QmlCuesModel> m_pHotcuesModel;
#ifdef __STEM__
    parented_ptr<QmlStemsModel> m_pStemsModel;
#endif
};

} // namespace qml
} // namespace mixxx
