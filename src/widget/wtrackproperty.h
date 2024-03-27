#pragma once

#include "preferences/usersettings.h"
#include "track/track_decl.h"
#include "track/trackid.h"
#include "util/parented_ptr.h"
#include "widget/trackdroptarget.h"
#include "widget/wlabel.h"

class ControlPushButton;
class Library;
class WTrackMenu;

class WTrackProperty : public WLabel, public TrackDropTarget {
    Q_OBJECT
  public:
    WTrackProperty(
            QWidget* pParent,
            UserSettingsPointer pConfig,
            Library* pLibrary,
            const QString& group,
            bool isMainDeck);
    ~WTrackProperty() override;

    void setup(const QDomNode& node, const SkinContext& context) override;

  signals:
    void trackDropped(const QString& filename, const QString& group) override;
    void cloneDeck(const QString& sourceGroup, const QString& targetGroup) override;
    void setAndConfirmTrackMenuControl(bool visible);

  public slots:
    void slotTrackLoaded(TrackPointer pTrack);
    void slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack);
    void slotShowTrackMenuChangeRequest(bool show);

  protected:
    void contextMenuEvent(QContextMenuEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;

  private:
    void updateLabel();

    void ensureTrackMenuIsCreated();

    const QString m_group;
    const UserSettingsPointer m_pConfig;
    Library* m_pLibrary;
    const bool m_isMainDeck;
    TrackPointer m_pCurrentTrack;
    QString m_property;

    parented_ptr<WTrackMenu> m_pTrackMenu;

  private slots:
    void slotTrackChanged(TrackId);
};
