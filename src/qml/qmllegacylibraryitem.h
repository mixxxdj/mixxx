#pragma once

#include <QPixmap>
#include <QPointer>
#include <QQmlEngine>
#include <QQuickPaintedItem>
#include <QTimer>
#include <QWidget>
#include <memory>

class ControlProxy;
class ControlPushButton;
class QAbstractItemView;
class QHeaderView;
class QScrollBar;
class WLibrary;
class WLibrarySidebar;
class WSearchLineEdit;

namespace mixxx {
namespace qml {

class QmlLegacyLibraryItem : public QQuickPaintedItem {
    Q_OBJECT
    QML_NAMED_ELEMENT(LegacyLibraryItem)

  public:
    explicit QmlLegacyLibraryItem(QQuickItem* parent = nullptr);
    ~QmlLegacyLibraryItem() override;

    void paint(QPainter* pPainter) override;

  protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void mouseUngrabEvent() override;
    void wheelEvent(QWheelEvent* event) override;

    void hoverEnterEvent(QHoverEvent* event) override;
    void hoverMoveEvent(QHoverEvent* event) override;
    void hoverLeaveEvent(QHoverEvent* event) override;

    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;

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
    void enableEmbeddedWidgetInputTracking();
    QWidget* widgetAtRootPos(const QPoint& rootPos) const;
    QAbstractItemView* parentItemView(QWidget* widget) const;
    QHeaderView* parentHeaderView(QWidget* widget) const;
    QWidget* eventTargetFor(QWidget* widget) const;
    QWidget* contextMenuTargetFor(QWidget* widget) const;
    bool isHeaderResizeHandle(QHeaderView* header, const QPoint& rootPos) const;
    void maybeApplyHeaderSortFallback(QHeaderView* header, const QPoint& rootPos);
    bool sendMouseToWidget(QMouseEvent* event, QWidget* target);
    void sendSyntheticMouseMoveToWidget(QWidget* target,
            const QPoint& rootPos,
            const QPointF& globalPos,
            Qt::KeyboardModifiers modifiers,
            Qt::MouseButtons buttons = Qt::NoButton);
    bool sendWheelToWidget(QWheelEvent* event);
    bool sendHoverToWidget(QHoverEvent* event);
    void scheduleToolTip(QWidget* target, const QPoint& rootPos);
    void cancelToolTip();
    void showPendingToolTip();
    QString toolTipTextForTarget(QWidget* target, const QPoint& rootPos) const;
    QPoint mapToGlobalScreen(const QPoint& rootPos) const;
    void syncRootWidgetGlobalPosition();
    bool sendContextMenuToWidget(QMouseEvent* event, QWidget* target);
    void updateHoverTarget(QWidget* target, const QPoint& rootPos, Qt::KeyboardModifiers modifiers);
    void syncCursorFromWidget(QWidget* target, const QPoint& rootPos);
    void repaintEmbeddedViews();
    void repolishEmbeddedWidgets();
    void applyLegacyScrollbarStyles();
    void applyLegacyScrollbarStyle(QScrollBar* scrollBar);
    void applyLegacyTableViewBridgeOptions();
    void applyLegacyColorPickerBridgeOptions();
    void connectSortBypass();
    void requestRender();
    void installEmbeddedWidgetEventFilters();
    void connectEmbeddedWidgetUpdateSignals();
    void syncEmbeddedTableGeometry(QAbstractItemView* view);
    void updateEmbeddedFocus(QWidget* target, Qt::FocusReason reason);

    bool eventFilter(QObject* watched, QEvent* event) override;

    enum class HeaderInteraction {
        None,
        Resize,
        MoveCandidate,
        MoveActive,
    };

    void startHeaderInteraction(QWidget* target, const QPoint& rootPos);
    bool shouldForwardHeaderMove(QWidget* target, const QPoint& rootPos);
    void resetHeaderInteraction(bool stopAutoScroll = false);
    void startBridgeAutoScroll();
    void stopBridgeAutoScroll();
    void doBridgeAutoScroll();

    std::unique_ptr<QWidget> m_pRootWidget;

    // Non-owning pointers (owned by m_pRootWidget's widget tree)
    WLibrary* m_pLibraryWidget = nullptr;
    WLibrarySidebar* m_pSidebar = nullptr;
    WSearchLineEdit* m_pSearchLineEdit = nullptr;

    // Track the offscreen QWidget mouse state explicitly. Since the visible
    // native window is QQuickWindow, QWidget's implicit grab/cursor machinery
    // cannot escape the hidden widget tree on its own.
    QPointer<QWidget> m_pPressedWidget;
    QPointer<QWidget> m_pGrabbedWidget;
    QPointer<QWidget> m_pLastHoverWidget;
    QPointer<QHeaderView> m_pPressedHeader;
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
    // scrolls the parent table's scrollbar and sends a synthetic mouse
    // move to keep the header's section-move state in sync.
    QTimer m_headerAutoScrollTimer;
    int m_headerAutoScrollCount = 0;

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
