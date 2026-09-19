#pragma once

#include <QColor>
#include <QElapsedTimer>
#include <QList>
#include <QPixmap>
#include <QPointer>
#include <QQmlEngine>
#include <QQuickPaintedItem>
#include <QRectF>
#include <QTimer>
#include <QWidget>
#include <memory>

class ControlProxy;
class ControlPushButton;
class BaseTrackPlayer;
class QAbstractItemView;
class QHeaderView;
class QScrollBar;
class QSplitter;
class QSplitterHandle;
class Library;
class PlayerManager;
class WLibrary;
class WLibrarySidebar;
class WCoverArt;
class WLabel;
class WNumber;
class WOverview;
class WPushButton;
class WSearchLineEdit;
class WSliderComposed;
class WStatusLight;
class WTrackProperty;
class WVuMeterLegacy;

namespace mixxx {
namespace qml {

class QmlLegacyLibraryItem : public QQuickPaintedItem {
    Q_OBJECT
    QML_NAMED_ELEMENT(LegacyLibraryItem)
    Q_PROPERTY(QRectF previewDeckDropRect READ previewDeckDropRect
                    NOTIFY previewDeckGeometryChanged)
    Q_PROPERTY(bool previewDeckDropEnabled READ previewDeckDropEnabled
                    NOTIFY previewDeckGeometryChanged)

  public:
    explicit QmlLegacyLibraryItem(QQuickItem* pParent = nullptr);
    ~QmlLegacyLibraryItem() override;

    void paint(QPainter* pPainter) override;
    Q_INVOKABLE void focusSearch();

    QRectF previewDeckDropRect() const;
    bool previewDeckDropEnabled() const;

  signals:
    void previewDeckGeometryChanged();

  protected:
    void mousePressEvent(QMouseEvent* pEvent) override;
    void mouseReleaseEvent(QMouseEvent* pEvent) override;
    void mouseMoveEvent(QMouseEvent* pEvent) override;
    void mouseDoubleClickEvent(QMouseEvent* pEvent) override;
    void mouseUngrabEvent() override;
    void wheelEvent(QWheelEvent* pEvent) override;

    void hoverEnterEvent(QHoverEvent* pEvent) override;
    void hoverMoveEvent(QHoverEvent* pEvent) override;
    void hoverLeaveEvent(QHoverEvent* pEvent) override;

    void keyPressEvent(QKeyEvent* pEvent) override;
    void keyReleaseEvent(QKeyEvent* pEvent) override;

    void geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry) override;
    void componentComplete() override;
    void updatePolish() override;

  private:
    void updateWidgetSize();
    void renderOffscreen();
    void applyLegacyStylesheet();
    void initializeOverviewTypeControl();
    void applyLegacySearchBoxSkinConfiguration();
    void applyLegacyLibrarySkinConfiguration();
    void createLegacyPreviewDeck();
    void applyLegacyPreviewDeckSkinConfiguration();
    void recreateLegacyPreviewOverview();
    void updatePreviewDeckTrackLoaded(double value);
    void applyLegacyCoverArtSkinConfiguration();
    void syncLibraryCoverArtFromSelection();
    void enableEmbeddedWidgetInputTracking();
    QWidget* widgetAtRootPos(const QPoint& rootPos) const;
    QAbstractItemView* parentItemView(QWidget* pWidget) const;
    QHeaderView* parentHeaderView(QWidget* pWidget) const;
    QSplitterHandle* parentSplitterHandle(QWidget* pWidget) const;
    QSplitterHandle* splitterHandleAtRootPos(const QPoint& rootPos) const;
    QWidget* eventTargetFor(QWidget* pWidget) const;
    QWidget* contextMenuTargetFor(QWidget* pWidget) const;
    bool isHeaderResizeHandle(QHeaderView* pHeader, const QPoint& rootPos) const;
    void maybeApplyHeaderSortFallback(QHeaderView* pHeader, const QPoint& rootPos);
    bool sendMouseToWidget(QMouseEvent* pEvent, QWidget* pTarget);
    void sendSyntheticMouseMoveToWidget(QWidget* pTarget,
            const QPoint& rootPos,
            const QPointF& globalPos,
            Qt::KeyboardModifiers modifiers,
            Qt::MouseButtons buttons = Qt::NoButton);
    bool sendWheelToWidget(QWheelEvent* pEvent);
    bool sendHoverToWidget(QHoverEvent* pEvent);
    void scheduleToolTip(QWidget* pTarget, const QPoint& rootPos);
    void cancelToolTip();
    void showPendingToolTip();
    QString toolTipTextForTarget(QWidget* pTarget, const QPoint& rootPos) const;
    QPoint mapToGlobalScreen(const QPoint& rootPos) const;
    void syncRootWidgetGlobalPosition();
    bool sendContextMenuToWidget(QMouseEvent* pEvent, QWidget* pTarget);
    void updateHoverTarget(QWidget* pTarget,
            const QPoint& rootPos,
            Qt::KeyboardModifiers modifiers);
    void syncCursorFromWidget(QWidget* pTarget, const QPoint& rootPos);
    void repaintEmbeddedViews();
    void repolishEmbeddedWidgets();
    void applyLegacyScrollbarStyles();
    void applyLegacyScrollbarStyle(QScrollBar* pScrollBar);
    void applyLegacyTableViewBridgeOptions();
    void applyLegacyColorPickerBridgeOptions();
    void applyInitialSplitterSizes();
    void requestRender();
    void requestRenderForCurrentInteraction();
    void installEmbeddedWidgetEventFilters();
    void connectEmbeddedWidgetUpdateSignals();
    void syncEmbeddedTableGeometry(QAbstractItemView* pView);
    void updateEmbeddedFocus(QWidget* pTarget, Qt::FocusReason reason);

    bool eventFilter(QObject* pWatched, QEvent* pEvent) override;

    enum class HeaderInteraction {
        None,
        Resize,
        MoveCandidate,
        MoveActive,
    };

    void startHeaderInteraction(QWidget* pTarget, const QPoint& rootPos);
    bool shouldForwardHeaderMove(QWidget* pTarget, const QPoint& rootPos);
    void resetHeaderInteraction(bool stopAutoScroll = false);
    bool startSplitterInteraction(QWidget* pTarget, const QPoint& rootPos);
    void resizeSplitterFromRootPos(const QPoint& rootPos);
    void resetSplitterInteraction();
    void setSplitterHandlePressed(QSplitterHandle* pHandle, bool pressed);
    void persistSplitterSizes(QSplitter* pSplitter);
    void startBridgeAutoScroll();
    void stopBridgeAutoScroll();
    void doBridgeAutoScroll();

    std::unique_ptr<QWidget> m_pRootWidget;
    QColor m_legacyLibraryBackgroundColor;

    // Non-owning pointers (owned by m_pRootWidget's widget tree)
    Library* m_pLibrary = nullptr;
    PlayerManager* m_pPlayerManager = nullptr;
    BaseTrackPlayer* m_pPreviewPlayer = nullptr;
    WLibrary* m_pLibraryWidget = nullptr;
    WLibrarySidebar* m_pSidebar = nullptr;
    WSearchLineEdit* m_pSearchLineEdit = nullptr;
    QWidget* m_pLibraryExpandBox = nullptr;
    WPushButton* m_pLibraryExpandButton = nullptr;
    QWidget* m_pPreviewDeckBox = nullptr;
    QWidget* m_pPreviewDeckRightSpacer = nullptr;
    QWidget* m_pPreviewEjectBox = nullptr;
    QWidget* m_pOverviewBox = nullptr;
    WTrackProperty* m_pPreviewTitle = nullptr;
    WNumber* m_pPreviewBpm = nullptr;
    WLabel* m_pPreviewLabel = nullptr;
    WPushButton* m_pPreviewPlayButton = nullptr;
    WPushButton* m_pPreviewEjectButton = nullptr;
    WOverview* m_pPreviewOverview = nullptr;
    WStatusLight* m_pPreviewPeakIndicator = nullptr;
    WVuMeterLegacy* m_pPreviewVuMeter = nullptr;
    WSliderComposed* m_pPreviewSlider = nullptr;
    QWidget* m_pCoverArtBox = nullptr;
    WCoverArt* m_pCoverArt = nullptr;
    QPointer<QSplitter> m_pLibrarySplitter;
    QPointer<QSplitter> m_pCoverArtSplitter;
    QList<int> m_initialLibrarySplitterSizes;
    QList<int> m_initialCoverArtSplitterSizes;
    bool m_initialSplitterSizesApplied = false;

    // Track the offscreen QWidget mouse state explicitly. Since the visible
    // native window is QQuickWindow, QWidget's implicit grab/cursor machinery
    // cannot escape the hidden widget tree on its own.
    QPointer<QWidget> m_pPressedWidget;
    QPointer<QWidget> m_pGrabbedWidget;
    QPointer<QWidget> m_pLastHoverWidget;
    QPointer<QHeaderView> m_pPressedHeader;
    QPointer<QSplitter> m_pPressedSplitter;
    QPointer<QSplitterHandle> m_pPressedSplitterHandle;
    QPointF m_lastHoverRootPos;
    QPoint m_pressRootPos;
    int m_pressedHeaderSection = -1;
    int m_pressedHeaderSortSection = -1;
    Qt::SortOrder m_pressedHeaderSortOrder = Qt::AscendingOrder;
    Qt::MouseButtons m_pressedButtons = Qt::NoButton;
    HeaderInteraction m_headerInteraction = HeaderInteraction::None;
    QPoint m_lastForwardedHeaderMoveRootPos;
    // Bridge-owned autoscroll for header column moves. Qt's native
    // QHeaderView autoscroll cannot work in an offscreen widget because
    // doAutoScroll() never updates draggedPosition for MoveSection state
    // and depends on continuous OS mouse-drag events the bridge does not
    // receive.  We disable native autoscroll and run our own timer that
    // scrolls the pParent table's scrollbar and sends a synthetic mouse
    // move to keep the header's section-move state in sync.
    QTimer m_headerAutoScrollTimer;
    QTimer m_splitterInteractionWatchdogTimer;
    int m_headerAutoScrollCount = 0;
    int m_splitterHandleIndex = -1;
    QList<int> m_splitterStartSizes;
    QPoint m_splitterStartRootPos;
    QElapsedTimer m_lastResizeInteractionRender;
    bool m_resizeInteractionRenderPending = false;
    bool m_splitterSizesDirty = false;
    bool m_handlingMouseUngrab = false;

    // Track which embedded widget has "keyboard focus". Since the offscreen
    // widget tree can never gain real focus (WA_DontShowOnScreen), we
    // synthesize FocusIn/FocusOut events manually.
    QPointer<QWidget> m_pFocusedWidget;

    // Qt normally creates delayed QHelpEvents for visible QWidget trees. The
    // embedded library is hidden and painted into QML, so the bridge owns that
    // tooltip wake-up path explicitly.
    QTimer m_toolTipTimer;
    QPointer<QWidget> m_pToolTipTarget;
    QPoint m_toolTipRootPos;
    QString m_toolTipText;

    std::unique_ptr<ControlProxy> m_pPreviewDeckPlay;
    std::unique_ptr<ControlProxy> m_pPreviewDeckTrackLoaded;
    std::unique_ptr<ControlProxy> m_pShowPreviewDecks;
    std::unique_ptr<ControlProxy> m_pShowLibraryCoverArt;

    // Owns the [Waveform],WaveformOverviewType ControlPushButton that is
    // normally created by DlgPrefWaveform. In QML mode that dialog does not
    // exist, so we create and own the CO here so that OverviewDelegate can
    // read the correct overview type (RGB by default).
    std::unique_ptr<ControlPushButton> m_pOverviewTypeControl;
    QPixmap m_offscreenPixmap;
    bool m_isRendering = false;
    bool m_componentComplete = false; // gates rendering until QML component is constructed
    bool m_isDirty = false;           // true only when content has actually changed
};

} // namespace qml
} // namespace mixxx
