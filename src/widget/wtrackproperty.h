#ifndef WTRACKPROPERTY_H
#define WTRACKPROPERTY_H

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMouseEvent>

#include "preferences/usersettings.h"
#include "skin/skincontext.h"
#include "track/track.h"
#include "widget/trackdroptarget.h"
#include "widget/wlabel.h"

class WTrackProperty : public WLabel, public TrackDropTarget {
    Q_OBJECT
  public:
    WTrackProperty(const char* group, UserSettingsPointer pConfig, QWidget* pParent);
    ~WTrackProperty() override;

    void setup(const QDomNode& node, const SkinContext& context) override;
    void contextMenuEvent(QContextMenuEvent * event) override;

signals:
    void trackDropped(QString filename, QString group) override;
    void cloneDeck(QString source_group, QString target_group) override;

  public slots:
    void slotTrackLoaded(TrackPointer track);
    void slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack);

  private slots:
    void slotTrackChanged(TrackId);
    void slotOpenInFileBrowser();

  private:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

    void updateLabel();
    void createContextMenuActions();

    const char* m_pGroup;
    UserSettingsPointer m_pConfig;
    TrackPointer m_pCurrentTrack;
    QString m_property;

    // Context menu machinery
    QMenu *m_pMenu;

    QAction *m_pFileBrowserAct;
};


#endif /* WTRACKPROPERTY_H */
