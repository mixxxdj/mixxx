
#ifndef DLGTRACKINFO_H
#define DLGTRACKINFO_H

#include <QDialog>
#include <QMutex>
#include <QHash>
#include <QList>

#include "ui_dlgtrackinfo.h"
#include "trackinfoobject.h"
#include "dlgtagfetcher.h"
#include "util/types.h"
#include "widget/wcoverartmenu.h"

const int kFilterLength = 5;

class Cue;

class DlgTrackInfo : public QDialog, public Ui::DlgTrackInfo {
    Q_OBJECT
  public:
    DlgTrackInfo(QWidget* parent, DlgTagFetcher& DlgTagFetcher);
    virtual ~DlgTrackInfo();

  public slots:
    // Not thread safe. Only invoke via AutoConnection or QueuedConnection, not
    // directly!
    void loadTrack(TrackPointer pTrack, QString coverLocation, QString md5);
    void slotCoverMenu(const QPoint& pos);

  signals:
    void next();
    void previous();

  private slots:
    void slotNext();
    void slotPrev();
    void OK();
    void apply();
    void cancel();
    void trackUpdated();
    void fetchTag();

    void cueActivate();
    void cueDelete();

    void slotBpmDouble();
    void slotBpmHalve();
    void slotBpmTwoThirds();
    void slotBpmThreeFourth();
    void slotBpmTap();

    void reloadTrackMetadata();
    void slotOpenInFileBrowser();

    void slotPixmapFound(int trackId, QPixmap pixmap);

  private:
    void populateFields(TrackPointer pTrack);
    void populateCues(TrackPointer pTrack);
    void saveTrack();
    void unloadTrack(bool save);
    void clear();
    void init();
    QPixmap scaledCoverArt(QPixmap original);

    QHash<int, Cue*> m_cueMap;
    TrackPointer m_pLoadedTrack;

    CSAMPLE m_bpmTapFilter[kFilterLength];
    QTime m_bpmTapTimer;

    QMutex m_mutex;
    DlgTagFetcher& m_DlgTagFetcher;
    WCoverArtMenu* m_pCoverMenu;
    QPair<QString, QString> m_loadedCover;
};

#endif /* DLGTRACKINFO_H */
