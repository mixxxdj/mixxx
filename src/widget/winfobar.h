#pragma once

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMouseEvent>
#include <QPair>
#include <QPushButton>
#include <QScrollArea>
#include <QSize>
#include <QThread>

#include "library/library.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "library/trackset/crate/cratestorage.h"
#include "preferences/usersettings.h"
#include "skin/skincontext.h"
#include "track/track.h"
#include "widget/wlabel.h"

/// Worker thread that fetches results from the library
class WInfoBarWorker : public QObject {
    Q_OBJECT
  public:
    WInfoBarWorker(QObject* parent, TrackCollectionManager* pTrackCollectionManager);
    ~WInfoBarWorker() = default;

  public slots:
    void query(int query, QList<TrackId>);
  signals:
    void crateResult(int query, uint total, QList<CrateSummary>);

  private:
    // add your variables here
    TrackCollectionManager* const m_pTrackCollectionManager;
};

class WInfoBarItem : public QPushButton {
  public:
    WInfoBarItem(const QString& text, QWidget* parent = nullptr);
};

class WInfoBarCrateItem : public WInfoBarItem {
  public:
    WInfoBarCrateItem(const CrateSummary crate, QWidget* parent = nullptr);
};

class WInfoBarCrateMixedItem : public WInfoBarCrateItem {
  public:
    WInfoBarCrateMixedItem(const CrateSummary crate, QWidget* parent = nullptr);
};

class QVBoxLayout;
class QHBoxLayout;
class QFrame;

class WInfoBar : public QScrollArea, public WBaseWidget {
    Q_OBJECT
  public:
    WInfoBar(QString group, UserSettingsPointer pConfig, Library* pLibrary, QWidget* pParent);

    void setup(const QDomNode& node, const SkinContext& context);
    int generateId();
    void clear();
    QSize minimumSize() const {
        return QSize(0, 0);
    }
    QSize minimumSizeHint() const {
        return QSize(0, 0);
    }
    //QSize const minimumSizeHint();

    /*
    Q_PROPERTY(bool multiLine READ getMultiline WRITE setMultiline)
    Q_PROPERTY(bool showCrates READ getShowCrates WRITE setShowCrates)
    Q_PROPERTY(bool showPlaylists READ getShowPlaylists WRITE setShowPlaylists)
    Q_PROPERTY(bool showHistory READ getShowHistory WRITE setShowHistory)
    Q_PROPERTY(double scrollSpeed READ getScrollSpeed WRITE setScrollSpeed)
    */

  signals:
    void trackDropped(QString filename, QString group);
    void query(int query, QList<TrackId>);

  public slots:
    void slotTrackLoaded(TrackPointer track);
    void slotTrackSelection(QList<TrackPointer> tracks);
    void slotClear() {
        clear();
    };
    //void slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack);

  private slots:
    //void updateLabel(Track*);
    void receiveCrateResults(int query, uint total, QList<CrateSummary>);

  private:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void setupUI();

    QFrame* m_pContainerFrame;
    QHBoxLayout* m_pContainer;
    QFrame* m_pCratesFrame;
    QHBoxLayout* m_pCratesContainer;
    QFrame* m_pPlaylistsFrame;
    QHBoxLayout* m_pPlaylistsContainer;
    QFrame* m_pHistoryFrame;
    QHBoxLayout* m_pHistoryContainer;

    QString m_pGroup;
    UserSettingsPointer m_pConfig;
    TrackPointer m_pCurrentTrack;
    //QString m_property;
    //QString m_separator;
    //TrackCollectionManager* const m_pCollectionManager;
    Library* m_pLibrary;
    //const CrateStorage* m_cratestore;
    int m_id;
    WInfoBarWorker* m_pWorker;

    // shared worker thread
    static QThread* s_worker_thread;
    // id generation
    static int s_last_id;
};
