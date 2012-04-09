// selectorfeature.h
// Created 8/23/2012 by Keith Salisbury (keith@globalkeith.com)

#ifndef SELECTORFEATURE_H
#define SELECTORFEATURE_H

#include <QStringListModel>
#include "library/libraryfeature.h"
#include "library/treeitemmodel.h"
#include "configobject.h"

class LibraryTableModel;
class TrackCollection;

class SelectorFeature : public LibraryFeature {
    Q_OBJECT
  public:
    SelectorFeature(QObject* parent,
                   ConfigObject<ConfigValue>* pConfig,
                   TrackCollection* pTrackCollection);
    virtual ~SelectorFeature();

    QVariant title();
    QIcon getIcon();

    bool dropAccept(QUrl url);
    bool dropAcceptChild(const QModelIndex& index, QUrl url);
    bool dragMoveAccept(QUrl url);
    bool dragMoveAcceptChild(const QModelIndex& index, QUrl url);

    void bindWidget(WLibrarySidebar* sidebarWidget,
                    WLibrary* libraryWidget,
                    MixxxKeyboard* keyboard);

    TreeItemModel* getChildModel();

  signals:

  public slots:
    void activate();
    void activateChild(const QModelIndex& index);
    void onRightClick(const QPoint& globalPos);
    void onRightClickChild(const QPoint& globalPos, QModelIndex index);
    void onLazyChildExpandation(const QModelIndex& index);

  private slots:

  private:
    ConfigObject<ConfigValue>* m_pConfig;
    TrackCollection* m_pTrackCollection;
    // Used to temporarily enable BPM detection in the prefs before we analyse
    int m_iOldBpmEnabled;
    TreeItemModel m_childModel;
    const static QString m_sSelectorViewName;
};


#endif /* SELECTORFEATURE_H */
