// mixxxlibraryfeature.h
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)

#ifndef MIXXXLIBRARYFEATURE_H
#define MIXXXLIBRARYFEATURE_H

#include <QStringListModel>

#include "library/libraryfeature.h"
#include "library/dao/trackdao.h"
#include "treeitemmodel.h"
#include "configobject.h"
#include "dlghidden.h"
#include "dlgmissing.h"


class BaseTrackCache;
class LibraryTableModel;
class TrackCollection;

class MixxxLibraryFeature : public LibraryFeature {
    Q_OBJECT
    public:
    MixxxLibraryFeature(QObject* parent,
                        TrackCollection* pTrackCollection,
                        ConfigObject<ConfigValue>* pConfig);
    virtual ~MixxxLibraryFeature();

    QVariant title();
    QIcon getIcon();
    bool dropAccept(QList<QUrl> urls, QWidget *pSource);
    bool dragMoveAccept(QUrl url);
    TreeItemModel* getChildModel();
    void bindWidget(WLibrary* pLibrary,
                    MixxxKeyboard* pKeyboard);

  public slots:
    void activate();
    void activateChild(const QModelIndex& index);
    void refreshLibraryModels();

  private:
    const QString kMissingTitle;
    const QString kHiddenTitle;
    QSharedPointer<BaseTrackCache> m_pBaseTrackCache;
    LibraryTableModel* m_pLibraryTableModel;
    DlgMissing* m_pMissingView;
    DlgHidden* m_pHiddenView;
    TreeItemModel m_childModel;
    TrackDAO& m_trackDao;
    ConfigObject<ConfigValue>* m_pConfig;
    TrackCollection* m_pTrackCollection;
};

#endif /* MIXXXLIBRARYFEATURE_H */
