// selectorfeature.h
// Created 8/23/2012 by Keith Salisbury (keith@globalkeith.com)
#ifndef SELECTORFEATURE_H
#define SELECTORFEATURE_H

#include <QStringListModel>

#include "library/libraryfeature.h"
#include "library/treeitemmodel.h"
#include "configobject.h"
#include "dlgselector.h"

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

    bool dropAccept(QList<QUrl> urls,QWidget *pSource);
    bool dragMoveAccept(QUrl url);
    void bindWidget(WLibrary* libraryWidget,
                    MixxxKeyboard* keyboard);
    TreeItemModel* getChildModel();

  public slots:
    void activate();
    void setSeedTrack(TrackPointer pTrack);
    void calculateAllSimilarities(const QString& filename);

  private:
    ConfigObject<ConfigValue>* m_pConfig;
    TrackCollection* m_pTrackCollection;
    const static QString m_sSelectorViewName;
    TreeItemModel m_childModel;
    DlgSelector* m_pSelectorView;
};
#endif /* SELECTORFEATURE_H */
