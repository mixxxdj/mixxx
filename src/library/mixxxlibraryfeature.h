// mixxxlibraryfeature.h
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)

#ifndef MIXXXLIBRARYFEATURE_H
#define MIXXXLIBRARYFEATURE_H

#include <QStringListModel>
#include <QUrl>
#include <QVariant>
#include <QIcon>
#include <QModelIndex>
#include <QList>
#include <QString>
#include <QSharedPointer>
#include <QObject>

#include "library/libraryfeature.h"
#include "library/dao/trackdao.h"
#include "library/treeitemmodel.h"
#include "preferences/usersettings.h"

class DlgHidden;
class DlgMissing;
class BaseTrackCache;
class LibraryTableModel;
class TrackCollection;

class MixxxLibraryFeature final : public LibraryFeature {
    Q_OBJECT
  public:
    MixxxLibraryFeature(Library* pLibrary,
                        UserSettingsPointer pConfig);
    ~MixxxLibraryFeature() override = default;

    QVariant title() override;
    QIcon getIcon() override;
    bool dropAccept(QList<QUrl> urls, QObject* pSource) override;
    bool dragMoveAccept(QUrl url) override;
    TreeItemModel* getChildModel() override;
    void bindLibraryWidget(WLibrary* pLibrary,
                    KeyboardEventFilter* pKeyboard) override;

    bool hasTrackTable() override {
        return true;
    }

    void searchAndActivate(const QString& query);

  public slots:
    void activate() override;
    void activateChild(const QModelIndex& index) override;
    void refreshLibraryModels();

  private:
    const QString kMissingTitle;
    const QString kHiddenTitle;
    const QIcon m_icon;
    TrackCollection* const m_pTrackCollection;

    QSharedPointer<BaseTrackCache> m_pBaseTrackCache;
    LibraryTableModel* m_pLibraryTableModel;

    TreeItemModel m_childModel;

    DlgMissing* m_pMissingView;
    DlgHidden* m_pHiddenView;
};

#endif /* MIXXXLIBRARYFEATURE_H */
