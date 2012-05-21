// setlogfeature.h

#ifndef SETLOGFEATURE_H
#define SETLOGFEATURE_H

#include <QSqlTableModel>
#include <QAction>

#include "library/baseplaylistfeature.h"
#include "configobject.h"
#include "controlobjectthreadmain.h"

class TrackCollection;

class SetlogFeature : public BasePlaylistFeature {
    Q_OBJECT
public:
    SetlogFeature(QObject* parent, ConfigObject<ConfigValue>* pConfig,
                  TrackCollection* pTrackCollection);
    virtual ~SetlogFeature();

    QVariant title();
    QIcon getIcon();

    bool dropAcceptChild(const QModelIndex& index, QUrl url);
    bool dragMoveAcceptChild(const QModelIndex& index, QUrl url);

    virtual void bindWidget(WLibrarySidebar* sidebarWidget,
                    WLibrary* libraryWidget,
                    MixxxKeyboard* keyboard);

  public slots:
    void onRightClick(const QPoint& globalPos);
    void onRightClickChild(const QPoint& globalPos, QModelIndex index);
    void slotJoinWithPrevious();

  protected:
    QModelIndex constructChildModel(int selected_id);

  private slots:
    void slotPlayingDeckChanged(int deck);
    void slotPlaylistTableChanged(int playlistId);


  private:
    virtual QString getRootViewHtml() const;

    QAction *m_pJoinWithPreviousAction;
    int m_playlistId;
};

#endif /* SETLOGFEATURE_H */
