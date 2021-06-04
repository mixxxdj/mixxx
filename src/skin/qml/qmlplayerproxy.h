#include <QColor>
#include <QObject>
#include <QString>

#include "track/track.h"

class BaseTrackPlayer;

namespace mixxx {
namespace skin {
namespace qml {

class QmlPlayerProxy : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString artist READ getArtist WRITE setArtist NOTIFY artistChanged)
    Q_PROPERTY(QString title READ getTitle WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QString album READ getAlbum WRITE setAlbum NOTIFY albumChanged)
    Q_PROPERTY(QString albumArtist READ getAlbumArtist WRITE setAlbumArtist
                    NOTIFY albumArtistChanged)
    Q_PROPERTY(QString genre READ getGenre WRITE setGenre NOTIFY genreChanged)
    Q_PROPERTY(QString composer READ getComposer WRITE setComposer NOTIFY composerChanged)
    Q_PROPERTY(QString grouping READ getGrouping WRITE setGrouping NOTIFY groupingChanged)
    Q_PROPERTY(QString year READ getYear WRITE setYear NOTIFY yearChanged)
    Q_PROPERTY(QString trackNumber READ getTrackNumber WRITE setTrackNumber
                    NOTIFY trackNumberChanged)
    Q_PROPERTY(QString trackTotal READ getTrackTotal WRITE setTrackTotal NOTIFY trackTotalChanged)
    Q_PROPERTY(QString comment READ getComment WRITE setComment NOTIFY commentChanged)
    Q_PROPERTY(QString keyText READ getKeyText WRITE setKeyText NOTIFY keyTextChanged)
    Q_PROPERTY(QColor color READ getColor WRITE setColor NOTIFY colorChanged)

  public:
    explicit QmlPlayerProxy(BaseTrackPlayer* pTrackPlayer, QObject* parent);

    QString getTrack() const;
    QString getTitle() const;
    QString getArtist() const;
    QString getAlbum() const;
    QString getAlbumArtist() const;
    QString getGenre() const;
    QString getComposer() const;
    QString getGrouping() const;
    QString getYear() const;
    QString getTrackNumber() const;
    QString getTrackTotal() const;
    QString getComment() const;
    QString getKeyText() const;
    QColor getColor() const;

  public slots:
    void slotTrackLoaded(TrackPointer pTrack);
    void slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack);
    void slotTrackChanged();

    void setArtist(const QString& artist);
    void setTitle(const QString& title);
    void setAlbum(const QString& album);
    void setAlbumArtist(const QString& albumArtist);
    void setGenre(const QString& genre);
    void setComposer(const QString& composer);
    void setGrouping(const QString& grouping);
    void setYear(const QString& year);
    void setTrackNumber(const QString& trackNumber);
    void setTrackTotal(const QString& trackTotal);
    void setComment(const QString& comment);
    void setKeyText(const QString& keyText);
    void setColor(const QColor& color);

  signals:
    void trackLoading();
    void trackLoaded();
    void trackUnloaded();
    void trackChanged();

    void albumChanged();
    void titleChanged();
    void artistChanged();
    void albumArtistChanged();
    void genreChanged();
    void composerChanged();
    void groupingChanged();
    void yearChanged();
    void trackNumberChanged();
    void trackTotalChanged();
    void commentChanged();
    void keyTextChanged();
    void colorChanged();

  private:
    BaseTrackPlayer* m_pTrackPlayer;
    TrackPointer m_pCurrentTrack;
};

} // namespace qml
} // namespace skin
} // namespace mixxx
