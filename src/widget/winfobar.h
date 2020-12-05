#pragma once

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPair>
#include <QPushButton>
#include <QScrollArea>
#include <QSize>
#include <QStatusBar>
#include <QThread>
#include <QToolButton>
#include <QWidget>

#include "library/library.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "library/trackset/crate/cratestorage.h"
#include "library/trackset/playlistsummary.h"
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
    void queryCrates(int query, QList<TrackId> trackIds);
    void queryPlaylists(int query, QList<TrackId> trackIds);
    void queryHistory(int query, QList<TrackId> trackIds);

  signals:
    void crateResult(int query, int total, QList<CrateSummary>);
    void playlistResult(int query, int total, QList<PlaylistSummary>);
    void historyResult(int query, int total, QList<PlaylistSummary>);

  private:
    // add your variables here
    TrackCollectionManager* const m_pTrackCollectionManager;
};

class WInfoBarItem : public QPushButton {
    Q_OBJECT
  public:
    WInfoBarItem(const QString& text, QWidget* parent = nullptr);
    ~WInfoBarItem() = default;
    void setTotals(int totals) {
        m_totals = totals;
    };
    int totals() {
        return m_totals;
    };
    void setMatches(int matches) {
        m_matches = matches;
    };
    int matches() {
        return m_matches;
    };

    bool notAll() {
        return matches() != totals();
    }

    Q_PROPERTY(int totals READ totals WRITE setTotals);
    Q_PROPERTY(int matches READ matches WRITE setMatches);
    Q_PROPERTY(bool notAll READ notAll);

  private:
    int m_totals;
    int m_matches;
};

class WInfoBarCrateItem : public WInfoBarItem {
    Q_OBJECT

  public:
    WInfoBarCrateItem(const CrateSummary crate, QWidget* parent = nullptr);

  signals:
    void activateCrate(CrateSummary crate);

  private:
    const CrateSummary m_crate;
};

class WInfoBarPlaylistItem : public WInfoBarItem {
    Q_OBJECT
  public:
    WInfoBarPlaylistItem(const PlaylistSummary playlist, bool isHistory, QWidget* parent = nullptr);

    Q_PROPERTY(bool isHistory MEMBER m_isHistory CONSTANT);
  signals:
    void activatePlaylist(PlaylistSummary playlist, bool isHistory);

  private:
    const PlaylistSummary m_playlist;
    const bool m_isHistory;
};

//Q_DECLARE_METATYPE(WInfoBarButtonState);

class WInfoBarButton : public QToolButton {
    Q_OBJECT
  public:
    enum State {
        VISIBLE,
        HIDDEN,
        EXPANDED
    };
    Q_ENUM(State);
    WInfoBarButton(QWidget* pParent);
    ~WInfoBarButton() = default;
    void setState(State newState);
    State state() {
        return m_state;
    };
    Q_PROPERTY(State state READ state WRITE setState NOTIFY stateChanged);
    void nextCheckState() override;

  signals:
    void stateChanged(State newState);

  private slots:
    //void slotTriggered(QAction* action);

  private:
    State m_state;
};

//qRegisterMetaType<WInfoBarButton::States>("States");

class WInfoBarContainer : public QFrame {
    Q_OBJECT
  public:
    WInfoBarContainer(QString name, QWidget* pParent);
    ~WInfoBarContainer() = default;

    void addItem(WInfoBarItem* item);
    void addStretch(int factor);
    void clear();
    void setState(WInfoBarButton::State state);
    WInfoBarButton::State state() {
        VERIFY_OR_DEBUG_ASSERT(m_pButton) {
            return WInfoBarButton::State::VISIBLE;
        }
        return m_pButton->state();
    }
  signals:
    void stateChanged(WInfoBarButton::State newState);

  private:
    void slotStateChange(WInfoBarButton::State state);

    WInfoBarButton* m_pButton;
    QHBoxLayout* m_pContainer;
    QHBoxLayout* m_pMainLayout;
    QWidget* m_pMainContainer;
};

class WInfoBar : public QStatusBar, public WBaseWidget {
    Q_OBJECT
  public:
    WInfoBar(QString group, UserSettingsPointer pConfig, Library* pLibrary, QWidget* pParent);
    ~WInfoBar() = default;

    void setup(const QDomNode& node, const SkinContext& context);
    int generateId();
    void clear();
    QSize minimumSize() const {
        return QSize(0, 0);
    }
    QSize minimumSizeHint() const override {
        return QSize(0, 0);
    }
    QSize sizeHint() const override {
        QSize rv = QStatusBar::sizeHint();
        rv.setWidth(0);
        return rv;
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
    void receiveCrateResults(int queryId, int total, QList<CrateSummary> crates);
    void receivePlaylistResults(int queryId, int total, QList<PlaylistSummary> playlists);
    void receiveHistoryResults(int queryId, int total, QList<PlaylistSummary> playlists);
    void slotActivateCrate(CrateSummary crate);
    void slotActivatePlaylist(PlaylistSummary playlist, bool isHistory);
    void slotRebuildWidgets(WInfoBarButton::State state) {
        Q_UNUSED(state);
        rebuildWidgets();
    };
    void slotCrateTracksChanged(CrateId crate,
            const QList<TrackId>& tracksAdded,
            const QList<TrackId>& tracksRemoved);

  private:
    /*
    void clearContainer(QHBoxLayout* layout);
    void clearCrates() { clearContainer(m_pCratesContainer); }
    void clearPlaylists() { clearContainer(m_pPlaylistsContainer); }
    void clearHistory() { clearContainer(m_pHistoryContainer); }
    */
    void addPlaylistResults(bool isHistory,
            int queryId,
            int total,
            QList<PlaylistSummary> playlists);
    //void updateVisibilty();
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;
    void setupUI();
    void prepareFrame(QFrame* frame);
    void rebuildWidgets();
    void queryCurrentTracks();
    void clearCurrentTracks();

    QFrame* m_pContainerFrame;
    QHBoxLayout* m_pContainer;

    /*
    QFrame* m_pCratesFrame;
    QHBoxLayout* m_pCratesContainer;
    QFrame* m_pPlaylistsFrame;
    QHBoxLayout* m_pPlaylistsContainer;
    QFrame* m_pHistoryFrame;
    QHBoxLayout* m_pHistoryContainer;
    */
    WInfoBarContainer* m_pCratesContainer;
    WInfoBarContainer* m_pPlaylistsContainer;
    WInfoBarContainer* m_pHistoryContainer;

    QString m_pGroup;
    UserSettingsPointer m_pConfig;
    QList<TrackPointer> m_currentTracks{};

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
