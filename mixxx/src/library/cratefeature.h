#ifndef CRATEFEATURE_H
#define CRATEFEATURE_H

#include <QSqlTableModel>
#include <QModelIndex>
#include <QAction>

#include "library/libraryfeature.h"
#include "library/cratetablemodel.h"

class TrackCollection;

class CrateFeature : public LibraryFeature {
    Q_OBJECT
  public:
    CrateFeature(QObject* parent, TrackCollection* pTrackCollection);
    virtual ~CrateFeature();

    QVariant title();
    QIcon getIcon();

    bool dropAccept(QUrl url);
    bool dropAcceptChild(const QModelIndex& index, QUrl url);
    bool dragMoveAccept(QUrl url);
    bool dragMoveAcceptChild(const QModelIndex& index, QUrl url);

    void bindWidget(WLibrarySidebar* sidebarWidget,
                    WLibrary* libraryWidget);

    QAbstractItemModel* getChildModel();
  signals:
    void showPage(const QUrl& page);

  public slots:
    void activate();
    void activateChild(const QModelIndex& index);
    void onRightClick(const QPoint& globalPos);
    void onRightClickChild(const QPoint& globalPos, QModelIndex index);

    void slotCreateCrate();
    void slotDeleteCrate();

  private:
    TrackCollection* m_pTrackCollection;
    QAction *m_pCreateCrateAction;
    QAction *m_pDeleteCrateAction;
    QSqlTableModel m_crateListTableModel;
    CrateTableModel m_crateTableModel;
    QModelIndex m_lastRightClickedIndex;
};

#endif /* CRATEFEATURE_H */
