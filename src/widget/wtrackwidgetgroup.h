#pragma once

#include "track/track_decl.h"
#include "track/trackid.h"
#include "util/parented_ptr.h"
#include "widget/trackdroptarget.h"
#include "widget/wwidgetgroup.h"

class Library;
class WTrackMenu;

class WTrackWidgetGroup : public WWidgetGroup, public TrackDropTarget {
    Q_OBJECT
  public:
    WTrackWidgetGroup(QWidget* pParent,
            UserSettingsPointer pConfig,
            Library* pLibrary,
            const QString& group,
            bool isMainDeck);
    ~WTrackWidgetGroup() override;
    void setup(const QDomNode& node, const SkinContext& context) override;
#ifdef __STEM__
    void trackDropped(const QString& filename,
            const QString& group,
            mixxx::StemChannelSelection stemMask) override;
#else
    void trackDropped(const QString& filename, const QString& group) override;
#endif

  signals:
    // void trackDropped(const QString& fileName, const QString& group) override;
#ifdef __STEM__
    void emitTrackDropped(const QString& filename,
            const QString& group,
            mixxx::StemChannelSelection stemMask);
#else
    void emitTrackDropped(const QString& filename, const QString& group);
#endif
    void cloneDeck(const QString& sourceGroup, const QString& targetGroup) override;

  public slots:
    void slotTrackLoaded(TrackPointer pTrack);
    void slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack);

  protected:
    void paintEvent(QPaintEvent* pe) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

  private slots:
    void slotTrackChanged(TrackId);

  private:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

    void updateColor();

    void ensureTrackMenuIsCreated();

    const QString m_group;
    const UserSettingsPointer m_pConfig;
    Library* m_pLibrary;
    TrackPointer m_pCurrentTrack;
    QColor m_trackColor;
    int m_trackColorAlpha;
    const bool m_isMainDeck;

    parented_ptr<WTrackMenu> m_pTrackMenu;
};
