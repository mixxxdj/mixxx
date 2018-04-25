#ifndef WCrateList_H
#define WCrateList_H

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMouseEvent>
#include <QListWidget>
#include <QThread>
#include <QPair>

#include "preferences/usersettings.h"
#include "skin/skincontext.h"
#include "track/track.h"
#include "library/features/crates/cratestorage.h"
#include "library/trackcollection.h"
#include "widget/wlabel.h"



class WCrateListWorker : public QObject {
    Q_OBJECT
public:
    WCrateListWorker(QObject* parent, const TrackCollection* collection);
    ~WCrateListWorker() = default;
public slots:
    void query(int query, QList<TrackId>);
signals:
    void result(int query, uint total, QList<CrateSummary>);
private:
    // add your variables here
    const TrackCollection* m_pTrackCollection;
};

class WCrateItem : public QListWidgetItem {
public:
    WCrateItem(const QString &text, QListWidget *parent);
};

class WCrateItemSome : public QListWidgetItem {
public:
    WCrateItemSome(const QString &text, QListWidget *parent);
};

class WCrateList : public QListWidget, public WBaseWidget {
    Q_OBJECT
  public:
    WCrateList(const char* group, UserSettingsPointer pConfig, const TrackCollection* collection, QWidget* pParent);

    void setup(const QDomNode& node, const SkinContext& context);
    int generateId();

  signals:
    void trackDropped(QString filename, QString group);
    void query(int query, QList<TrackId>);

  public slots:
    void slotTrackLoaded(TrackPointer track);
    void slotTrackSelection(QList<TrackPointer> tracks);
    //void slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack);

  private slots:
    //void updateLabel(Track*);
    void receiveResults(int query, uint total, QList<CrateSummary>);

  private:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

    const char* m_pGroup;
    UserSettingsPointer m_pConfig;
    TrackPointer m_pCurrentTrack;
    //QString m_property;
    //QString m_seperator;
    const TrackCollection* m_pCollection;
    //const CrateStorage* m_cratestore;
    int m_id;
    WCrateListWorker* m_pWorker;

    // shared worker thread
    static QThread* s_worker_thread;
    // id generation
    static int s_last_id;
};
#endif /* WCrateList_H */
