#ifndef CRATEFEATURE_H
#define CRATEFEATURE_H

#include <QSqlTableModel>
#include <QModelIndex>
#include <QAction>

#include "library/libraryfeature.h"

class TrackCollection;

class CrateFeature : public LibraryFeature {
    Q_OBJECT
  public:
    CrateFeature(QObject* parent, TrackCollection* pTrackCollection);
    virtual ~CrateFeature();

    QVariant title();
    QIcon getIcon();
    int numChildren();
    QVariant child(int n);

    bool dropAccept(const QModelIndex& index, QUrl url);
    bool dragMoveAccept(const QModelIndex& index, QUrl url);
    QAbstractItemModel* getChildModel();

public slots:
    void activate();
    void activateChild(int n);
    void onRightClick(const QPoint& globalPos, QModelIndex index);
    void onClick(QModelIndex index);

    void slotCreateCrate();
    void slotDeleteCrate();

  private:
    TrackCollection* m_pTrackCollection;
    QAction *m_pCreateCrateAction;
    QAction *m_pDeleteCrateAction;
    QSqlTableModel m_crateTableModel;
    QModelIndex m_lastRightClickedIndex;
};

#endif /* CRATEFEATURE_H */
