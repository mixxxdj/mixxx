#include "qml/qmllegacylibraryitem.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QComboBox>
#include <QContextMenuEvent>
#include <QCoreApplication>
#include <QDir>
#include <QDomDocument>
#include <QElapsedTimer>
#include <QFile>
#include <QFocusEvent>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QIcon>
#include <QMetaEnum>
#include <QPainter>
#include <QPalette>
#include <QPushButton>
#include <QQuickWindow>
#include <QScopedValueRollback>
#include <QScrollBar>
#include <QSplitter>
#include <QStyle>
#include <QStyleHints>
#include <QTableView>
#include <QTextDocument>
#include <QTimer>
#include <QToolTip>
#include <QVBoxLayout>
#include <cmath>
#include <utility>

#include "control/controlproxy.h"
#include "control/controlpushbutton.h"
#include "library/library.h"
#include "library/library_prefs.h"
#include "mixer/playermanager.h"
#include "moc_qmllegacylibraryitem.cpp"
#include "preferences/constants.h"
#include "qml/qmlconfigproxy.h"
#include "qml/qmllibraryproxy.h"
#include "skin/legacy/skincontext.h"
#include "waveform/overviewtype.h"
#include "widget/wcolorpicker.h"
#include "widget/wlibrary.h"
#include "widget/wlibrarysidebar.h"
#include "widget/wsearchlineedit.h"
#include "widget/wtracktableview.h"
#include "widget/wtracktableviewheader.h"

namespace mixxx {
namespace qml {

namespace {
const QColor kLegacyLibraryBackgroundColor(0x1e, 0x1e, 0x1e);
} // namespace

QmlLegacyLibraryItem::~QmlLegacyLibraryItem() = default;

QmlLegacyLibraryItem::QmlLegacyLibraryItem(QQuickItem* parent)
        : QQuickPaintedItem(parent),
          m_pRootWidget(std::make_unique<QWidget>()) {
    setAntialiasing(false);
    setOpaquePainting(true);

    // Configure for input handling
    setAcceptedMouseButtons(Qt::AllButtons);
    setAcceptHoverEvents(true);
    setFlag(QQuickItem::ItemAcceptsInputMethod, true);
    setActiveFocusOnTab(true);

    m_toolTipTimer.setSingleShot(true);
    connect(&m_toolTipTimer,
            &QTimer::timeout,
            this,
            &QmlLegacyLibraryItem::showPendingToolTip);
    m_headerAutoScrollTimer.setInterval(50);
    connect(&m_headerAutoScrollTimer,
            &QTimer::timeout,
            this,
            &QmlLegacyLibraryItem::doBridgeAutoScroll);

    QPalette rootPalette = m_pRootWidget->palette();
    rootPalette.setColor(QPalette::Window, kLegacyLibraryBackgroundColor);
    m_pRootWidget->setPalette(rootPalette);
    m_pRootWidget->setAutoFillBackground(true);
    m_pRootWidget->setAttribute(Qt::WA_DontShowOnScreen);
    m_pRootWidget->setObjectName(QStringLiteral("LibraryContainer"));
    // 1. Create splitter layout
    // Name must match the LateNight QSS selector "#LibrarySplitter::handle"
    // so the skin image is applied instead of a native/platform handle.
    // See res/skins/LateNight/library.xml and style_classic.qss:2646.
    auto* pSplitter = new QSplitter(m_pRootWidget.get());
    pSplitter->setObjectName(QStringLiteral("LibrarySplitter"));

    // 2. Sidebar page (search + sidebar)
    auto* pSidebarPage = new QWidget(pSplitter);
    pSidebarPage->setObjectName(QStringLiteral("LibSidebarContainer"));
    pSidebarPage->setAttribute(Qt::WA_StyledBackground, true);
    auto* pSidebarLayout = new QVBoxLayout(pSidebarPage);
    pSidebarLayout->setContentsMargins(0, 0, 0, 0);
    pSidebarLayout->setSpacing(0);

    UserSettingsPointer pConfig = QmlConfigProxy::get();
    auto* pSearchLineBox = new QWidget(pSidebarPage);
    pSearchLineBox->setObjectName(QStringLiteral("SearchLineBox"));
    pSearchLineBox->setAttribute(Qt::WA_StyledBackground, true);
    auto* pSearchLineLayout = new QHBoxLayout(pSearchLineBox);
    pSearchLineLayout->setContentsMargins(0, 0, 0, 0);
    pSearchLineLayout->setSpacing(0);
    m_pSearchLineEdit = new WSearchLineEdit(pSearchLineBox, pConfig);
    applyLegacySearchBoxSkinConfiguration();
    pSearchLineLayout->addWidget(m_pSearchLineEdit);

    auto* pSearchTreeSpacer = new QWidget(pSidebarPage);
    pSearchTreeSpacer->setObjectName(QStringLiteral("SearchTreeSpacer"));
    pSearchTreeSpacer->setAttribute(Qt::WA_StyledBackground, true);
    pSearchTreeSpacer->setFixedHeight(3);

    auto* pSidebarBox = new QWidget(pSidebarPage);
    pSidebarBox->setObjectName(QStringLiteral("SidebarBox"));
    pSidebarBox->setAttribute(Qt::WA_StyledBackground, true);
    auto* pSidebarBoxLayout = new QVBoxLayout(pSidebarBox);
    pSidebarBoxLayout->setContentsMargins(0, 0, 0, 0);
    pSidebarBoxLayout->setSpacing(0);
    m_pSidebar = new WLibrarySidebar(pSidebarBox);
    pSidebarBoxLayout->addWidget(m_pSidebar);

    pSidebarLayout->addWidget(pSearchLineBox);
    pSidebarLayout->addWidget(pSearchTreeSpacer);
    pSidebarLayout->addWidget(pSidebarBox, 1);

    // 3. Library (main content area)
    m_pLibraryWidget = new WLibrary(pSplitter);
    m_pLibraryWidget->setObjectName(QStringLiteral("LibraryContainer"));
    applyLegacyLibrarySkinConfiguration();

    // 4. Add to splitter
    // Collapsibility matches library.xml <Collapsible>1,0</Collapsible>:
    // sidebar (index 0) can be collapsed; track table (index 1) cannot.
    pSplitter->addWidget(pSidebarPage);
    pSplitter->addWidget(m_pLibraryWidget);
    pSplitter->setCollapsible(0, true);
    pSplitter->setCollapsible(1, false);
    pSplitter->setSizes({200, 600});

    // 5. Root layout
    auto* pRootLayout = new QVBoxLayout(m_pRootWidget.get());
    pRootLayout->setContentsMargins(0, 0, 0, 0);
    pRootLayout->addWidget(pSplitter);

    // 6. Initialize the WaveformOverviewType ControlPushButton BEFORE binding
    //    the library, because bindLibraryWidget creates OverviewDelegate which
    //    reads this CO in its constructor. In legacy mode DlgPrefWaveform
    //    creates this CO, but that dialog is never constructed in QML mode.
    initializeOverviewTypeControl();

    // 7. Bind to Library singleton
    Library* pLibrary = QmlLibraryProxy::get();
    VERIFY_OR_DEBUG_ASSERT(pLibrary) {
        return;
    }

    pLibrary->bindSearchboxWidget(m_pSearchLineEdit);
    pLibrary->bindSidebarWidget(m_pSidebar);
    pLibrary->bindLibraryWidget(m_pLibraryWidget, QmlLibraryProxy::getKeyboard());

    // The legacy skin parser makes this connection in parseLibrary().
    // Without it the search signal never reaches WLibrary and the
    // track table is never filtered.
    connect(pLibrary,
            &Library::search,
            m_pLibraryWidget,
            &WLibrary::search);

    // 8. Trigger repaints on visual changes and refresh input tracking for
    //    views that are created or swapped after the initial bind.
    connect(pLibrary, &Library::switchToView, this, [this]() {
        enableEmbeddedWidgetInputTracking();
        applyLegacyScrollbarStyles();
        applyLegacyTableViewBridgeOptions();
        applyLegacyColorPickerBridgeOptions();
        connectSortBypass();
        installEmbeddedWidgetEventFilters();
        connectEmbeddedWidgetUpdateSignals();
        repaintEmbeddedViews();
    });
    connect(pLibrary, &Library::showTrackModel, this, [this]() {
        enableEmbeddedWidgetInputTracking();
        applyLegacyScrollbarStyles();
        applyLegacyTableViewBridgeOptions();
        applyLegacyColorPickerBridgeOptions();
        connectSortBypass();
        installEmbeddedWidgetEventFilters();
        connectEmbeddedWidgetUpdateSignals();
        repaintEmbeddedViews();
    });

    // Initialize default view to Tracks collection to avoid black screen
    pLibrary->searchTracksInCollection();

    // 9. Apply the LateNight classic stylesheet so the embedded
    //    QWidget tree renders branch arrows, preview button icons, and other
    //    SVG-based decorations that are normally applied by LegacySkinParser.
    //    TODO(GSoC): Replace with the QQuickAsyncImageProvider "skin:" scheme
    //    and QML palette bindings once the library panel is ported to QML.
    applyLegacyStylesheet();
    repolishEmbeddedWidgets();
    enableEmbeddedWidgetInputTracking();
    applyLegacyScrollbarStyles();
    applyLegacyTableViewBridgeOptions();
    applyLegacyColorPickerBridgeOptions();
    connectSortBypass();
    installEmbeddedWidgetEventFilters();
    connectEmbeddedWidgetUpdateSignals();

    const QString previewDeckGroup = PlayerManager::groupForPreviewDeck(0);
    m_pPreviewDeckPlay = std::make_unique<ControlProxy>(
            previewDeckGroup,
            QStringLiteral("play"),
            this,
            ControlFlag::NoAssertIfMissing);
    m_pPreviewDeckTrackLoaded = std::make_unique<ControlProxy>(
            previewDeckGroup,
            QStringLiteral("track_loaded"),
            this,
            ControlFlag::NoAssertIfMissing);
    m_pPreviewDeckPlay->connectValueChanged(this, [this](double) {
        requestRender();
    });
    m_pPreviewDeckTrackLoaded->connectValueChanged(this, [this](double) {
        requestRender();
    });
}

void QmlLegacyLibraryItem::renderOffscreen() {
    if (!m_pRootWidget) {
        return;
    }
    syncRootWidgetGlobalPosition();
    updateWidgetSize();
    const QSize size(qMax(1, qRound(width())),
            qMax(1, qRound(height())));
    if (m_offscreenPixmap.size() != size) {
        m_offscreenPixmap = QPixmap(size);
    }
    m_offscreenPixmap.fill(kLegacyLibraryBackgroundColor);

    // Process all pending layout, resize, and geometry events for the QWidget tree
    // so that child widgets (persistent editors) are correctly positioned before rendering.
    QCoreApplication::sendPostedEvents(m_pRootWidget.get());

    QPainter painter(&m_offscreenPixmap);
    const QScopedValueRollback<bool> renderingRollback(m_isRendering, true);
    m_pRootWidget->render(&painter);
}

void QmlLegacyLibraryItem::paint(QPainter* pPainter) {
    pPainter->fillRect(QRectF(0, 0, width(), height()),
            kLegacyLibraryBackgroundColor);
    if (m_offscreenPixmap.isNull()) {
        return;
    }
    pPainter->drawPixmap(0, 0, m_offscreenPixmap);
}

void QmlLegacyLibraryItem::geometryChange(
        const QRectF& newGeometry,
        const QRectF& oldGeometry) {
    QQuickPaintedItem::geometryChange(newGeometry, oldGeometry);
    updateWidgetSize();
    requestRender();
}

void QmlLegacyLibraryItem::componentComplete() {
    QQuickPaintedItem::componentComplete();
    m_componentComplete = true;

    // Flush any dirty state that accumulated during construction
    // (geometry changes, model signals, etc.).
    if (m_isDirty) {
        requestRender();
    }
}

namespace {
constexpr int kHeaderResizeCursorMargin = 4;
constexpr const char* kColorDelegateBridgeProperty =
        "mixxxColorDelegateUseRowBackgroundForColorCell";
constexpr const char* kColorPickerButtonBridgeProperty =
        "mixxxQmlLegacyColorPickerButtonBridge";
constexpr const char* kScrollEditorSyncConnectedProperty =
        "mixxxQmlScrollEditorSyncConnected";

QPointF widgetScenePos(QWidget* target, QWidget* root, const QPoint& rootPos) {
    return QPointF(target->mapFrom(root, rootPos));
}

void updateColorPickerButtonIcon(QPushButton* pButton) {
    pButton->setIcon(QIcon(pButton->isChecked()
                    ? QStringLiteral(":/images/ic_checkmark.svg")
                    : QString()));
}

bool isContextMenuOnMouseRelease() {
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    const QStyleHints* styleHints = QGuiApplication::styleHints();
    return styleHints &&
            styleHints->contextMenuTrigger() == Qt::ContextMenuTrigger::Release;
#else
#if defined(Q_OS_WIN) || defined(Q_OS_MACOS)
    return true; // Windows and macOS trigger on release
#else
    return false; // X11/Linux typically triggers on press
#endif
#endif
}
} // namespace

QWidget* QmlLegacyLibraryItem::widgetAtRootPos(const QPoint& rootPos) const {
    if (!m_pRootWidget) {
        return nullptr;
    }

    QWidget* widget = m_pRootWidget->childAt(rootPos);
    if (!widget) {
        return m_pRootWidget.get();
    }

    while (QWidget* child = widget->childAt(widget->mapFrom(m_pRootWidget.get(), rootPos))) {
        if (child == widget) {
            break;
        }
        widget = child;
    }
    return widget;
}

QAbstractItemView* QmlLegacyLibraryItem::parentItemView(QWidget* widget) const {
    for (QWidget* current = widget; current; current = current->parentWidget()) {
        if (auto* view = qobject_cast<QAbstractItemView*>(current)) {
            return view;
        }
    }
    return nullptr;
}

QHeaderView* QmlLegacyLibraryItem::parentHeaderView(QWidget* widget) const {
    for (QWidget* current = widget; current; current = current->parentWidget()) {
        if (auto* header = qobject_cast<QHeaderView*>(current)) {
            return header;
        }
    }
    return nullptr;
}

QWidget* QmlLegacyLibraryItem::eventTargetFor(QWidget* widget) const {
    if (!widget) {
        return m_pRootWidget.get();
    }

    // QHeaderView IS-A QAbstractItemView, but sort-click handling lives on the
    // header itself, not its internal viewport. Do NOT redirect header clicks.
    if (qobject_cast<QHeaderView*>(widget)) {
        return widget;
    }

    if (auto* view = parentItemView(widget)) {
        // For table/list views, redirect to viewport so delegates get events.
        if (widget == view) {
            QWidget* viewport = view->viewport();
            return viewport ? viewport : widget;
        }
    }
    return widget;
}

QWidget* QmlLegacyLibraryItem::contextMenuTargetFor(QWidget* widget) const {
    if (!widget) {
        return m_pRootWidget.get();
    }

    if (auto* header = parentHeaderView(widget)) {
        return header;
    }

    if (auto* view = parentItemView(widget)) {
        QWidget* viewport = view->viewport();
        return viewport ? viewport : view;
    }

    return widget;
}

QPoint QmlLegacyLibraryItem::mapToGlobalScreen(const QPoint& rootPos) const {
    if (!window()) {
        return rootPos;
    }
    const QPointF scenePos = mapToScene(rootPos);
    return window()->mapToGlobal(scenePos.toPoint());
}

void QmlLegacyLibraryItem::syncRootWidgetGlobalPosition() {
    if (!m_pRootWidget || !window()) {
        return;
    }
    const QPoint globalPos = mapToGlobalScreen(QPoint(0, 0));
    // Only call move() if the position changed — move() posts QEvent::Move
    // to the root widget, and since we have an event filter on it, calling
    // this unconditionally during renderOffscreen() creates a feedback loop.
    if (m_pRootWidget->pos() != globalPos) {
        m_pRootWidget->move(globalPos);
    }
}

bool QmlLegacyLibraryItem::sendContextMenuToWidget(QMouseEvent* event, QWidget* target) {
    if (!m_pRootWidget || !target) {
        return false;
    }

    const QPoint rootPos = event->position().toPoint();
    updateHoverTarget(target, rootPos, event->modifiers());

    const QPoint targetPos = target->mapFrom(m_pRootWidget.get(), rootPos);
    const QPoint globalPos = mapToGlobalScreen(rootPos);

    QContextMenuEvent contextEvent(
            QContextMenuEvent::Mouse,
            targetPos,
            globalPos,
            event->modifiers());

    if (auto* pTrackTableHeader = qobject_cast<WTrackTableViewHeader*>(target)) {
        pTrackTableHeader->contextMenuEvent(&contextEvent);
    } else {
        QApplication::sendEvent(target, &contextEvent);
    }
    if (contextEvent.isAccepted()) {
        event->accept();
    }
    syncCursorFromWidget(target, rootPos);
    return contextEvent.isAccepted();
}

bool QmlLegacyLibraryItem::sendMouseToWidget(QMouseEvent* event, QWidget* target) {
    if (!m_pRootWidget || !target) {
        return false;
    }

    const QPointF rootPos = event->position();
    const QPoint rootPoint = rootPos.toPoint();
    updateHoverTarget(target, rootPoint, event->modifiers());

    const QPointF targetPos = widgetScenePos(target, m_pRootWidget.get(), rootPoint);
    const QPointF globalPos = event->globalPosition();
    QMouseEvent mappedEvent(
            event->type(),
            targetPos,
            rootPos,
            globalPos,
            event->button(),
            event->buttons(),
            event->modifiers(),
            event->source());

    QApplication::sendEvent(target, &mappedEvent);
    event->setAccepted(mappedEvent.isAccepted());
    m_pressedButtons = event->buttons();

    syncCursorFromWidget(target, rootPoint);
    return mappedEvent.isAccepted();
}

void QmlLegacyLibraryItem::sendSyntheticMouseMoveToWidget(QWidget* target,
        const QPoint& rootPos,
        const QPointF& globalPos,
        Qt::KeyboardModifiers modifiers,
        Qt::MouseButtons buttons) {
    if (!m_pRootWidget || !target) {
        return;
    }

    const QPointF targetPos = widgetScenePos(target, m_pRootWidget.get(), rootPos);
    const QPointF windowPos = QPointF(rootPos);
    QMouseEvent moveEvent(
            QEvent::MouseMove,
            targetPos,
            windowPos,
            globalPos,
            Qt::NoButton,
            buttons,
            modifiers);
    QApplication::sendEvent(target, &moveEvent);
    syncCursorFromWidget(target, rootPos);
}

bool QmlLegacyLibraryItem::sendWheelToWidget(QWheelEvent* event) {
    if (!m_pRootWidget) {
        return false;
    }

    QWidget* target = eventTargetFor(widgetAtRootPos(event->position().toPoint()));
    if (!target) {
        return false;
    }

    const QPoint rootPos = event->position().toPoint();
    updateHoverTarget(target, rootPos, event->modifiers());

    // Wheel events over child widgets of a table view (e.g. the Preview
    // column's persistent editor QPushButtons) must be redirected to the
    // view's viewport so the table handles scrolling.  Without this, the
    // child widget swallows the event and horizontal scroll appears stuck.
    QAbstractItemView* scrollView = parentItemView(target);
    if (scrollView) {
        if (target != scrollView && target != scrollView->viewport()) {
            QWidget* viewport = scrollView->viewport();
            if (viewport) {
                target = viewport;
            }
        }
    }

    QWheelEvent mappedEvent(
            widgetScenePos(target, m_pRootWidget.get(), rootPos),
            event->globalPosition(),
            event->pixelDelta(),
            event->angleDelta(),
            event->buttons(),
            event->modifiers(),
            event->phase(),
            event->inverted());

    QApplication::sendEvent(target, &mappedEvent);
    event->setAccepted(mappedEvent.isAccepted());

    syncCursorFromWidget(target, rootPos);
    return mappedEvent.isAccepted();
}

bool QmlLegacyLibraryItem::sendHoverToWidget(QHoverEvent* event) {
    if (!m_pRootWidget) {
        return false;
    }

    QWidget* target = eventTargetFor(widgetAtRootPos(event->position().toPoint()));
    if (!target) {
        return false;
    }

    const QPoint rootPos = event->position().toPoint();
    updateHoverTarget(target, rootPos, event->modifiers());

    const QPoint targetPos = target->mapFrom(m_pRootWidget.get(), rootPos);
    const QPoint oldTargetPos = target->mapFrom(m_pRootWidget.get(), event->oldPos());
    QHoverEvent mappedEvent(
            event->type(),
            targetPos,
#if QT_VERSION < QT_VERSION_CHECK(6, 3, 0)
            event->position(),
#else
            event->globalPosition(),
#endif
            oldTargetPos,
            event->modifiers());

    QApplication::sendEvent(target, &mappedEvent);
    event->setAccepted(mappedEvent.isAccepted());
    m_lastHoverRootPos = event->position();

    // QTableView::entered(), which PreviewButtonDelegate uses to open the
    // real QPushButton editor, is driven by mouse tracking rather than
    // QHoverEvent delivery. Mirror QQuick hover as a no-button mouse move so
    // item-view delegates see the same path they get in a native QWidget skin.
#if QT_VERSION < QT_VERSION_CHECK(6, 3, 0)
    sendSyntheticMouseMoveToWidget(target, rootPos, event->position(), event->modifiers());
#else
    sendSyntheticMouseMoveToWidget(target, rootPos, event->globalPosition(), event->modifiers());
#endif

    scheduleToolTip(target, rootPos);
    syncCursorFromWidget(target, rootPos);
    return mappedEvent.isAccepted();
}

void QmlLegacyLibraryItem::scheduleToolTip(QWidget* target, const QPoint& rootPos) {
    if (!target || m_pressedButtons != Qt::NoButton) {
        cancelToolTip();
        return;
    }

    const int restartDistance = QApplication::startDragDistance();
    if (m_pToolTipTarget == target &&
            (m_toolTipRootPos - rootPos).manhattanLength() < restartDistance) {
        return;
    }

    m_pToolTipTarget = target;
    m_toolTipRootPos = rootPos;
    if (!m_toolTipText.isEmpty()) {
        m_toolTipText.clear();
        QToolTip::hideText();
    }

    int wakeUpDelay = QApplication::style()->styleHint(QStyle::SH_ToolTip_WakeUpDelay);
    if (QStyle* style = target->style()) {
        wakeUpDelay = style->styleHint(QStyle::SH_ToolTip_WakeUpDelay, nullptr, target);
    }
    m_toolTipTimer.start(qMax(0, wakeUpDelay));
}

void QmlLegacyLibraryItem::cancelToolTip() {
    m_toolTipTimer.stop();
    m_pToolTipTarget.clear();
    QToolTip::hideText();
    if (!m_toolTipText.isEmpty()) {
        m_toolTipText.clear();
    }
}

void QmlLegacyLibraryItem::showPendingToolTip() {
    if (!m_pRootWidget || m_pressedButtons != Qt::NoButton) {
        return;
    }

    const auto toolTipsMode = QmlConfigProxy::get()->getValue(
            ConfigKey(QStringLiteral("[Controls]"), QStringLiteral("Tooltips")),
            mixxx::preferences::Tooltips::On);
    if (toolTipsMode == mixxx::preferences::Tooltips::Off ||
            toolTipsMode == mixxx::preferences::Tooltips::OnlyKbdShortcuts) {
        return;
    }

    QWidget* target = eventTargetFor(widgetAtRootPos(m_toolTipRootPos));
    if (!target) {
        return;
    }

    const QString toolTipText = toolTipTextForTarget(target, m_toolTipRootPos);
    if (!toolTipText.isEmpty()) {
        if (toolTipText ==
                QCoreApplication::translate(
                        "BaseTrackTableModel", "Fetching image ...")) {
            m_toolTipTimer.start(50);
            return;
        }

        m_toolTipText = toolTipText;
        QToolTip::showText(
                m_pRootWidget->mapToGlobal(m_toolTipRootPos) + QPoint(12, 18),
                toolTipText,
                m_pRootWidget.get());
        return;
    }
    cancelToolTip();
}

QString QmlLegacyLibraryItem::toolTipTextForTarget(
        QWidget* target, const QPoint& rootPos) const {
    if (!target || !m_pRootWidget) {
        return QString();
    }

    if (auto* header = parentHeaderView(target)) {
        const int section = header->logicalIndexAt(
                header->mapFrom(m_pRootWidget.get(), rootPos));
        if (section >= 0 && header->model()) {
            return header->model()
                    ->headerData(section, header->orientation(), Qt::ToolTipRole)
                    .toString();
        }
    }

    if (auto* view = parentItemView(target)) {
        QWidget* viewport = view->viewport();
        if (!viewport) {
            return QString();
        }
        const QModelIndex index = view->indexAt(
                viewport->mapFrom(m_pRootWidget.get(), rootPos));
        if (index.isValid()) {
            return index.data(Qt::ToolTipRole).toString();
        }
    }

    return target->toolTip();
}

void QmlLegacyLibraryItem::updateHoverTarget(
        QWidget* target,
        const QPoint& rootPos,
        [[maybe_unused]] Qt::KeyboardModifiers modifiers) {
    if (!target || target == m_pLastHoverWidget) {
        return;
    }

    if (m_pLastHoverWidget) {
        QEvent leaveEvent(QEvent::Leave);
        QApplication::sendEvent(m_pLastHoverWidget, &leaveEvent);
    }

    const QPointF targetPos = widgetScenePos(target, m_pRootWidget.get(), rootPos);
    QEnterEvent enterEvent(targetPos,
            QPointF(rootPos),
            QPointF(target->mapToGlobal(targetPos.toPoint())));
    QApplication::sendEvent(target, &enterEvent);
    m_pLastHoverWidget = target;
}

bool QmlLegacyLibraryItem::isHeaderResizeHandle(QHeaderView* header, const QPoint& rootPos) const {
    if (!header || !m_pRootWidget) {
        return false;
    }

    const QPoint headerPos = header->mapFrom(m_pRootWidget.get(), rootPos);
    const int logicalIndex = header->logicalIndexAt(headerPos);
    if (logicalIndex < 0) {
        return false;
    }

    const int sectionStart = header->sectionViewportPosition(logicalIndex);
    const int sectionEnd = sectionStart + header->sectionSize(logicalIndex);
    const int cursorPos = header->orientation() == Qt::Horizontal ? headerPos.x() : headerPos.y();
    return std::abs(cursorPos - sectionStart) <= kHeaderResizeCursorMargin ||
            std::abs(cursorPos - sectionEnd) <= kHeaderResizeCursorMargin;
}

void QmlLegacyLibraryItem::maybeApplyHeaderSortFallback(
        QHeaderView* header, const QPoint& rootPos) {
    if (!header || header != m_pPressedHeader || m_pressedHeaderSection < 0) {
        return;
    }
    if (isHeaderResizeHandle(header, rootPos)) {
        return;
    }
    if ((rootPos - m_pressRootPos).manhattanLength() > QApplication::startDragDistance()) {
        return;
    }

    const QPoint headerPos = header->mapFrom(m_pRootWidget.get(), rootPos);
    const int releaseSection = header->logicalIndexAt(headerPos);
    if (releaseSection != m_pressedHeaderSection || !header->sectionsClickable()) {
        return;
    }

    if (header->sortIndicatorSection() != m_pressedHeaderSortSection ||
            header->sortIndicatorOrder() != m_pressedHeaderSortOrder) {
        // sortIndicatorChanged already fired during the press/release cycle,
        // so the native sort path already ran. No fallback needed.
        return;
    }

    const Qt::SortOrder order = header->sortIndicatorSection() == releaseSection
            ? (header->sortIndicatorOrder() == Qt::AscendingOrder
                              ? Qt::DescendingOrder
                              : Qt::AscendingOrder)
            : Qt::AscendingOrder;

    header->setSortIndicator(releaseSection, order);
    header->update();
}

void QmlLegacyLibraryItem::startHeaderInteraction(QWidget* target, const QPoint& rootPos) {
    resetHeaderInteraction(true);

    QHeaderView* header = parentHeaderView(target);
    if (!header) {
        return;
    }

    m_pPressedHeader = header;
    m_lastForwardedHeaderMoveRootPos = rootPos;

    if (isHeaderResizeHandle(header, rootPos)) {
        m_headerInteraction = HeaderInteraction::Resize;
        return;
    }

    const QPoint headerPos = header->mapFrom(m_pRootWidget.get(), rootPos);
    const int section = header->logicalIndexAt(headerPos);
    if (section < 0 || !header->sectionsMovable()) {
        resetHeaderInteraction();
        return;
    }

    m_headerInteraction = HeaderInteraction::MoveCandidate;
    m_pressedHeaderSection = section;
    m_pressedHeaderSortSection = header->sortIndicatorSection();
    m_pressedHeaderSortOrder = header->sortIndicatorOrder();

    // Disable Qt's native autoscroll for the duration of this interaction.
    // It cannot work correctly in the offscreen bridge environment.
    m_pPressedHeader->setAutoScroll(false);
}

bool QmlLegacyLibraryItem::shouldForwardHeaderMove(QWidget* target, const QPoint& rootPos) {
    if (m_headerInteraction == HeaderInteraction::None) {
        return true;
    }

    QHeaderView* header = m_pPressedHeader ? m_pPressedHeader.data() : parentHeaderView(target);
    if (!header || header != parentHeaderView(target)) {
        stopBridgeAutoScroll();
        return true;
    }

    if (m_headerInteraction == HeaderInteraction::Resize) {
        m_lastForwardedHeaderMoveRootPos = rootPos;
        return true;
    }

    const int dragDistance = QApplication::startDragDistance();
    const int axisDelta = header->orientation() == Qt::Horizontal
            ? std::abs(rootPos.x() - m_pressRootPos.x())
            : std::abs(rootPos.y() - m_pressRootPos.y());
    if (m_headerInteraction == HeaderInteraction::MoveCandidate) {
        if (axisDelta < dragDistance) {
            m_lastForwardedHeaderMoveRootPos = rootPos;
            return true;
        }
        m_headerInteraction = HeaderInteraction::MoveActive;
    }
    m_lastForwardedHeaderMoveRootPos = rootPos;
    return true;
}

void QmlLegacyLibraryItem::startBridgeAutoScroll() {
    if (!m_headerAutoScrollTimer.isActive()) {
        m_headerAutoScrollCount = 0;
        m_headerAutoScrollTimer.start();
    }
}

void QmlLegacyLibraryItem::stopBridgeAutoScroll() {
    m_headerAutoScrollTimer.stop();
    m_headerAutoScrollCount = 0;
}

void QmlLegacyLibraryItem::doBridgeAutoScroll() {
    if (m_headerInteraction != HeaderInteraction::MoveActive ||
            !m_pPressedHeader || !m_pGrabbedWidget) {
        stopBridgeAutoScroll();
        return;
    }

    QHeaderView* header = m_pPressedHeader.data();
    auto* parentView = qobject_cast<QAbstractItemView*>(header->parentWidget());
    if (!parentView) {
        stopBridgeAutoScroll();
        return;
    }

    QScrollBar* scrollBar = header->orientation() == Qt::Horizontal
            ? parentView->horizontalScrollBar()
            : parentView->verticalScrollBar();
    if (!scrollBar) {
        stopBridgeAutoScroll();
        return;
    }

    // Determine scroll direction from cursor position relative to the
    // header viewport, mirroring Qt's autoScrollMargin logic.
    QWidget* viewport = header->viewport();
    if (!viewport) {
        stopBridgeAutoScroll();
        return;
    }

    const QPoint viewportPos = viewport->mapFrom(
            m_pRootWidget.get(), m_lastForwardedHeaderMoveRootPos);
    const QRect area = viewport->rect();
    const int margin = header->autoScrollMargin();
    int direction = 0;
    if (header->orientation() == Qt::Horizontal) {
        if (viewportPos.x() < area.left() + margin) {
            direction = -1;
        } else if (viewportPos.x() >= area.right() - margin) {
            direction = 1;
        }
    } else {
        if (viewportPos.y() < area.top() + margin) {
            direction = -1;
        } else if (viewportPos.y() >= area.bottom() - margin) {
            direction = 1;
        }
    }

    if (direction == 0) {
        stopBridgeAutoScroll();
        return;
    }

    // Progressive acceleration, same logic as QAbstractItemView::doAutoScroll.
    const int pageStep = scrollBar->pageStep();
    if (m_headerAutoScrollCount < pageStep) {
        ++m_headerAutoScrollCount;
    }

    const int oldValue = scrollBar->value();
    scrollBar->setValue(oldValue + direction * m_headerAutoScrollCount);

    if (scrollBar->value() == oldValue) {
        // Hit the limit, stop.
        stopBridgeAutoScroll();
        return;
    }

    // Re-send the last cursor position as a synthetic mouse move so the
    // header updates its section indicator and move target with the new
    // scroll offset.  This is the step that Qt's native autoscroll
    // expects continuous OS mouse-drag events to provide.
    QWidget* target = m_pGrabbedWidget.data();
    const QPoint rootPos = m_lastForwardedHeaderMoveRootPos;
    sendSyntheticMouseMoveToWidget(target,
            rootPos,
            QPointF(mapToGlobalScreen(rootPos)),
            Qt::NoModifier,
            m_pressedButtons);
    requestRender();
}

void QmlLegacyLibraryItem::resetHeaderInteraction(bool stopAutoScroll) {
    stopBridgeAutoScroll();
    if (m_pPressedHeader) {
        m_pPressedHeader->setAutoScroll(true);
    }
    m_pPressedHeader.clear();
    m_pressedHeaderSection = -1;
    m_pressedHeaderSortSection = -1;
    m_pressedHeaderSortOrder = Qt::AscendingOrder;
    m_headerInteraction = HeaderInteraction::None;
    m_lastForwardedHeaderMoveRootPos = QPoint();
}

void QmlLegacyLibraryItem::syncCursorFromWidget(QWidget* target, const QPoint& rootPos) {
    if (!target) {
        unsetCursor();
        return;
    }

    if (auto* header = parentHeaderView(target)) {
        if (isHeaderResizeHandle(header, rootPos)) {
            setCursor(header->orientation() == Qt::Horizontal
                            ? Qt::SplitHCursor
                            : Qt::SplitVCursor);
            return;
        }
    }

    for (QWidget* current = target; current; current = current->parentWidget()) {
        if (current->testAttribute(Qt::WA_SetCursor)) {
            setCursor(current->cursor());
            return;
        }
    }
    unsetCursor();
}

void QmlLegacyLibraryItem::repaintEmbeddedViews() {
    if (!m_pRootWidget) {
        return;
    }

    const auto views = m_pRootWidget->findChildren<QAbstractItemView*>();
    for (QAbstractItemView* view : views) {
        if (view->viewport()) {
            view->viewport()->update();
        }
        view->update();
    }
    requestRender();
}

void QmlLegacyLibraryItem::applyLegacyScrollbarStyle(QScrollBar* scrollBar) {
    if (!scrollBar) {
        return;
    }

    const QString scrollBarStyle = QStringLiteral(R"MIXXXQSS(
QScrollBar {
  border: 0px solid #585858;
  background: #000;
  border-radius: 2px;
  padding: 1px;
  color: #999999;
}
QScrollBar:horizontal {
  min-width: 12px;
  height: 15px;
  border-top-left-radius: 0px;
  border-top-right-radius: 0px;
  background-color: #000;
}
QScrollBar:vertical {
  min-height: 12px;
  width: 15px;
  border-top-left-radius: 0px;
  border-bottom-left-radius: 0px;
  color: #b3b3b3;
  background-color: #000;
}
QScrollBar::groove:horizontal {
  height: 15px;
  background-color: #000;
  border: 0px;
}
QScrollBar::groove:vertical {
  width: 15px;
  background-color: #000;
  border: 0px;
}
QScrollBar::handle:horizontal {
  min-width: 25px;
  border-radius: 2px;
  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #725309, stop:1 #412f05);
}
QScrollBar::handle:vertical {
  min-height: 25px;
  border-radius: 2px;
  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #725309, stop:1 #412f05);
}
QScrollBar::add-page, QScrollBar::sub-page {
  min-width: 15px;
  min-height: 15px;
  background-color: #000;
  border-radius: 2px;
}
QScrollBar::add-line, QScrollBar::sub-line {
  width: 0px;
  height: 0px;
  border: 0px;
}
)MIXXXQSS");

    scrollBar->setAttribute(Qt::WA_StyledBackground, true);
    scrollBar->setAutoFillBackground(true);
    scrollBar->setStyleSheet(scrollBarStyle);
    if (QStyle* style = scrollBar->style()) {
        style->unpolish(scrollBar);
        style->polish(scrollBar);
    }
    scrollBar->ensurePolished();
    scrollBar->update();
}

void QmlLegacyLibraryItem::applyLegacyScrollbarStyles() {
    if (!m_pRootWidget) {
        return;
    }

    const auto scrollBars = m_pRootWidget->findChildren<QScrollBar*>();
    for (QScrollBar* scrollBar : scrollBars) {
        applyLegacyScrollbarStyle(scrollBar);
    }
}

void QmlLegacyLibraryItem::repolishEmbeddedWidgets() {
    if (!m_pRootWidget) {
        return;
    }

    QList<QWidget*> widgets = m_pRootWidget->findChildren<QWidget*>();
    widgets.prepend(m_pRootWidget.get());
    for (QWidget* widget : std::as_const(widgets)) {
        if (QStyle* style = widget->style()) {
            style->unpolish(widget);
            style->polish(widget);
        }
        widget->ensurePolished();
        widget->update();
    }
}

void QmlLegacyLibraryItem::enableEmbeddedWidgetInputTracking() {
    if (!m_pRootWidget) {
        return;
    }

    QList<QWidget*> widgets = m_pRootWidget->findChildren<QWidget*>();
    widgets.prepend(m_pRootWidget.get());
    for (QWidget* widget : std::as_const(widgets)) {
        widget->setMouseTracking(true);
        widget->setAttribute(Qt::WA_Hover, true);
        widget->setAttribute(Qt::WA_NoMousePropagation, false);
    }
}

void QmlLegacyLibraryItem::connectSortBypass() {
    if (!m_pRootWidget) {
        return;
    }

    // WTrackTableView::applySortingIfVisible() bails because isVisible()
    // returns false for our WA_DontShowOnScreen widget tree. We bypass this
    // by directly connecting each table header's sortIndicatorChanged signal
    // to QTableView::sortByColumn (public, inherited), which does the actual
    // model sort. Use Qt::UniqueConnection so this is idempotent across
    // repeated calls from view-switch signals.
    const auto tableViews = m_pRootWidget->findChildren<QTableView*>();
    for (QTableView* tableView : tableViews) {
        QHeaderView* header = tableView->horizontalHeader();
        if (!header) {
            continue;
        }
        connect(header,
                &QHeaderView::sortIndicatorChanged,
                tableView,
                &QTableView::sortByColumn,
                Qt::UniqueConnection);
    }
}

void QmlLegacyLibraryItem::applyLegacyTableViewBridgeOptions() {
    if (!m_pRootWidget) {
        return;
    }

    const auto tableViews = m_pRootWidget->findChildren<WTrackTableView*>();
    for (WTrackTableView* tableView : tableViews) {
        tableView->setProperty(kColorDelegateBridgeProperty, true);
    }
}

void QmlLegacyLibraryItem::applyLegacyColorPickerBridgeOptions() {
    if (!m_pRootWidget) {
        return;
    }

    const auto colorPickers = m_pRootWidget->findChildren<WColorPicker*>();
    for (WColorPicker* colorPicker : colorPickers) {
        const auto buttons = colorPicker->findChildren<QPushButton*>();
        for (QPushButton* button : buttons) {
            if (!button->property(kColorPickerButtonBridgeProperty).toBool()) {
                button->setProperty(kColorPickerButtonBridgeProperty, true);
                connect(button,
                        &QPushButton::toggled,
                        this,
                        [button]() {
                            updateColorPickerButtonIcon(button);
                            QTimer::singleShot(0, button, [button]() {
                                updateColorPickerButtonIcon(button);
                            });
                        });
            }
            updateColorPickerButtonIcon(button);
        }
    }
}

void QmlLegacyLibraryItem::mousePressEvent(QMouseEvent* event) {
    syncRootWidgetGlobalPosition();
    cancelToolTip();
    const QPoint rootPos = event->position().toPoint();
    QWidget* target = eventTargetFor(widgetAtRootPos(rootPos));
    sendSyntheticMouseMoveToWidget(target, rootPos, event->globalPosition(), event->modifiers());
    target = eventTargetFor(widgetAtRootPos(rootPos));
    m_pPressedWidget = target;
    m_pGrabbedWidget = target;
    m_pressedButtons = event->buttons();
    m_pressRootPos = rootPos;
    if (event->button() == Qt::LeftButton) {
        startHeaderInteraction(target, rootPos);
    } else {
        resetHeaderInteraction(true);
    }

    if (sendMouseToWidget(event, target)) {
        repaintEmbeddedViews();
    } else {
        m_pPressedWidget.clear();
        m_pGrabbedWidget.clear();
        resetHeaderInteraction(true);
        QQuickPaintedItem::mousePressEvent(event);
    }

    // Grab QML keyboard focus so keyPressEvent/keyReleaseEvent fire on
    // this item, and synthesize FocusIn on the clicked embedded widget.
    forceActiveFocus(Qt::MouseFocusReason);
    updateEmbeddedFocus(target, Qt::MouseFocusReason);

    if (event->button() == Qt::RightButton &&
            !isContextMenuOnMouseRelease()) {
        QWidget* contextTarget = contextMenuTargetFor(widgetAtRootPos(rootPos));
        if (sendContextMenuToWidget(event, contextTarget)) {
            repaintEmbeddedViews();
            m_pPressedWidget.clear();
            m_pGrabbedWidget.clear();
        }
    }
}

void QmlLegacyLibraryItem::mouseReleaseEvent(QMouseEvent* event) {
    syncRootWidgetGlobalPosition();
    cancelToolTip();
    const QPoint rootPos = event->position().toPoint();
    QWidget* target = m_pGrabbedWidget
            ? m_pGrabbedWidget.data()
            : eventTargetFor(widgetAtRootPos(rootPos));
    const bool accepted = sendMouseToWidget(event, target);
    if (event->button() == Qt::LeftButton) {
        maybeApplyHeaderSortFallback(parentHeaderView(target), rootPos);
    }

    bool contextMenuAccepted = false;
    if (event->button() == Qt::RightButton &&
            isContextMenuOnMouseRelease()) {
        QWidget* contextTarget = contextMenuTargetFor(widgetAtRootPos(rootPos));
        contextMenuAccepted = sendContextMenuToWidget(event, contextTarget);
    }

    if (accepted || contextMenuAccepted) {
        repaintEmbeddedViews();
    } else {
        QQuickPaintedItem::mouseReleaseEvent(event);
    }
    m_pPressedWidget.clear();
    m_pGrabbedWidget.clear();
    m_pressedButtons = Qt::NoButton;
    resetHeaderInteraction(true);
}

void QmlLegacyLibraryItem::mouseMoveEvent(QMouseEvent* event) {
    syncRootWidgetGlobalPosition();
    if (event->buttons() != Qt::NoButton) {
        cancelToolTip();
    }
    const QPoint rootPos = event->position().toPoint();
    QWidget* target = m_pGrabbedWidget
            ? m_pGrabbedWidget.data()
            : eventTargetFor(widgetAtRootPos(rootPos));
    if (!shouldForwardHeaderMove(target, rootPos)) {
        event->accept();
        return;
    }
    if (sendMouseToWidget(event, target)) {
        // During active header column moves, kill Qt's native autoscroll
        // (which cannot work correctly in the offscreen bridge) and arm
        // our own bridge-side autoscroll if the cursor is in the margin.
        if (m_headerInteraction == HeaderInteraction::MoveActive &&
                m_pPressedHeader) {
            QHeaderView* header = m_pPressedHeader.data();
            QWidget* viewport = header->viewport();
            if (viewport) {
                const QPoint vp = viewport->mapFrom(
                        m_pRootWidget.get(), rootPos);
                const QRect area = viewport->rect();
                const int margin = header->autoScrollMargin();
                bool inMargin = false;
                if (header->orientation() == Qt::Horizontal) {
                    inMargin = vp.x() < area.left() + margin ||
                            vp.x() >= area.right() - margin;
                } else {
                    inMargin = vp.y() < area.top() + margin ||
                            vp.y() >= area.bottom() - margin;
                }
                if (inMargin) {
                    startBridgeAutoScroll();
                } else {
                    stopBridgeAutoScroll();
                }
            }
        }
        requestRender();
    } else {
        QQuickPaintedItem::mouseMoveEvent(event);
    }
}

void QmlLegacyLibraryItem::mouseDoubleClickEvent(QMouseEvent* event) {
    syncRootWidgetGlobalPosition();
    cancelToolTip();
    const QPoint rootPos = event->position().toPoint();
    QWidget* target = eventTargetFor(widgetAtRootPos(rootPos));
    sendSyntheticMouseMoveToWidget(target, rootPos, event->globalPosition(), event->modifiers());
    target = eventTargetFor(widgetAtRootPos(rootPos));
    m_pPressedWidget = target;
    m_pGrabbedWidget = target;
    if (sendMouseToWidget(event, target)) {
        repaintEmbeddedViews();
    } else {
        QQuickPaintedItem::mouseDoubleClickEvent(event);
    }
}

void QmlLegacyLibraryItem::mouseUngrabEvent() {
    m_pPressedWidget.clear();
    m_pGrabbedWidget.clear();
    m_pressedButtons = Qt::NoButton;
    resetHeaderInteraction(true);
    QQuickPaintedItem::mouseUngrabEvent();
}

void QmlLegacyLibraryItem::wheelEvent(QWheelEvent* event) {
    syncRootWidgetGlobalPosition();
    cancelToolTip();
    if (sendWheelToWidget(event)) {
        requestRender();
    } else {
        QQuickPaintedItem::wheelEvent(event);
    }
}

void QmlLegacyLibraryItem::hoverEnterEvent(QHoverEvent* event) {
    syncRootWidgetGlobalPosition();
    if (sendHoverToWidget(event)) {
        requestRender();
    } else {
        QQuickPaintedItem::hoverEnterEvent(event);
    }
}

void QmlLegacyLibraryItem::hoverMoveEvent(QHoverEvent* event) {
    syncRootWidgetGlobalPosition();
    if (sendHoverToWidget(event)) {
        requestRender();
    } else {
        QQuickPaintedItem::hoverMoveEvent(event);
    }
}

void QmlLegacyLibraryItem::hoverLeaveEvent(QHoverEvent* event) {
    syncRootWidgetGlobalPosition();
    cancelToolTip();
    if (m_pLastHoverWidget) {
        QEvent leaveEvent(QEvent::Leave);
        QApplication::sendEvent(m_pLastHoverWidget, &leaveEvent);
        m_pLastHoverWidget.clear();
    }
    unsetCursor();
    requestRender();
    QQuickPaintedItem::hoverLeaveEvent(event);
}

void QmlLegacyLibraryItem::keyPressEvent(QKeyEvent* event) {
    if (m_pFocusedWidget) {
        QApplication::sendEvent(m_pFocusedWidget, event);
        requestRender();
    } else {
        QQuickPaintedItem::keyPressEvent(event);
    }
}

void QmlLegacyLibraryItem::keyReleaseEvent(QKeyEvent* event) {
    if (m_pFocusedWidget) {
        QApplication::sendEvent(m_pFocusedWidget, event);
        requestRender();
    } else {
        QQuickPaintedItem::keyReleaseEvent(event);
    }
}

void QmlLegacyLibraryItem::updateEmbeddedFocus(
        QWidget* target, Qt::FocusReason reason) {
    // For QComboBox (WSearchLineEdit), the internal QLineEdit is the
    // deepest click target. Walk up to the QComboBox so that
    // QComboBox::focusInEvent fires — it forwards focus to the line
    // edit internally and sets up the completer and clear button.
    QWidget* focusTarget = target;
    while (focusTarget) {
        if (qobject_cast<QComboBox*>(focusTarget)) {
            break;
        }
        if (focusTarget->focusPolicy() != Qt::NoFocus) {
            break;
        }
        focusTarget = focusTarget->parentWidget();
    }

    if (focusTarget == m_pFocusedWidget) {
        return;
    }

    if (m_pFocusedWidget) {
        QFocusEvent focusOut(QEvent::FocusOut, reason);
        QApplication::sendEvent(m_pFocusedWidget, &focusOut);
    }

    m_pFocusedWidget = focusTarget;

    if (m_pFocusedWidget) {
        QFocusEvent focusIn(QEvent::FocusIn, reason);
        QApplication::sendEvent(m_pFocusedWidget, &focusIn);
    }
}

void QmlLegacyLibraryItem::updateWidgetSize() {
    if (!m_pRootWidget) {
        return;
    }

    const QSize widgetSize(
            qMax(1, qRound(width())),
            qMax(1, qRound(height())));
    if (m_pRootWidget->size() == widgetSize) {
        return;
    }

    m_pRootWidget->resize(widgetSize);
    m_pRootWidget->ensurePolished();
}

void QmlLegacyLibraryItem::applyLegacySearchBoxSkinConfiguration() {
    if (!m_pSearchLineEdit) {
        return;
    }

    UserSettingsPointer pConfig = QmlConfigProxy::get();
    const auto searchDebouncingTimeoutMillis =
            pConfig->getValue(
                    mixxx::library::prefs::kSearchDebouncingTimeoutMillisConfigKey,
                    WSearchLineEdit::kDefaultDebouncingTimeoutMillis);
    WSearchLineEdit::setDebouncingTimeoutMillis(searchDebouncingTimeoutMillis);

    const QString resourcePath = pConfig->getResourcePath();
    const QString lateNightSkinPath = resourcePath + QStringLiteral("skins/LateNight");

    SkinContext context(pConfig, lateNightSkinPath + QStringLiteral("/skin.xml"));
    context.setSkinBasePath(lateNightSkinPath);

    QDomDocument document(QStringLiteral("QmlLegacyLibraryItemSearchBoxSetup"));
    const QString searchBoxXml = QStringLiteral("<SearchBox></SearchBox>");
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    const QDomDocument::ParseResult parseResult = document.setContent(searchBoxXml);
    if (!parseResult) {
        qWarning() << "QmlLegacyLibraryItem: failed to parse search box skin setup"
                   << parseResult.errorMessage << parseResult.errorLine
                   << parseResult.errorColumn;
#else
    QString errorMessage;
    int errorLine;
    int errorColumn;
    if (!document.setContent(searchBoxXml, &errorMessage, &errorLine, &errorColumn)) {
        qWarning() << "QmlLegacyLibraryItem: failed to parse search box skin setup"
                   << errorMessage << errorLine << errorColumn;
#endif
        return;
    }

    m_pSearchLineEdit->setup(document.documentElement(), context);
}

void QmlLegacyLibraryItem::applyLegacyLibrarySkinConfiguration() {
    if (!m_pLibraryWidget) {
        return;
    }

    const QString resourcePath = QmlConfigProxy::get()->getResourcePath();
    const QString lateNightSkinPath = resourcePath + QStringLiteral("skins/LateNight");

    SkinContext context(QmlConfigProxy::get(), lateNightSkinPath + QStringLiteral("/skin.xml"));
    context.setSkinBasePath(lateNightSkinPath);

    QDomDocument document(QStringLiteral("QmlLegacyLibraryItemLibrarySetup"));
    const QString libraryXml = QStringLiteral(
            "<Library>"
            "<ShowButtonText>false</ShowButtonText>"
            "<TrackTableBackgroundColorOpacity>0.175</TrackTableBackgroundColorOpacity>"
            "<SignalColor>#e7c413</SignalColor>"
            "</Library>");
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    const QDomDocument::ParseResult parseResult = document.setContent(libraryXml);
    if (!parseResult) {
        qWarning() << "QmlLegacyLibraryItem: failed to parse library skin setup"
                   << parseResult.errorMessage << parseResult.errorLine
                   << parseResult.errorColumn;
#else
    QString errorMessage;
    int errorLine;
    int errorColumn;
    if (!document.setContent(libraryXml, &errorMessage, &errorLine, &errorColumn)) {
        qWarning() << "QmlLegacyLibraryItem: failed to parse library skin setup"
                   << errorMessage << errorLine << errorColumn;
#endif
        return;
    }

    m_pLibraryWidget->setup(document.documentElement(), context);
}

// Loads style_classic.qss from the LateNight skin directory and
// applies it to the root widget so that the embedded QWidget tree picks up
// SVG branch arrows, preview button icons, and colour tokens.
//
// The legacy QSS uses a custom "skins:" URL scheme that only LegacySkinParser
// knows how to resolve.  We emulate it with a simple string replacement that
// expands "skins:" to the absolute skins/ directory path.
//
// TODO(GSoC): This whole method can be deleted once the library panel is
// ported to QML. At that point styling is handled by the "skin:" image
// provider (QQuickAsyncImageProvider subclass) and pure QML property bindings,
// which is the architecture described in the GSoC proposal.
void QmlLegacyLibraryItem::applyLegacyStylesheet() {
    const QString resourcePath =
            QmlConfigProxy::get()->getResourcePath();
    const QString skinsRoot = QDir::fromNativeSeparators(
            resourcePath + QStringLiteral("skins/"));
    const QString lateNightSkinRoot = QDir::fromNativeSeparators(
            resourcePath + QStringLiteral("skins/LateNight"));
    QDir::setSearchPaths(QStringLiteral("skins"), {skinsRoot});
    QDir::setSearchPaths(QStringLiteral("skin"), {lateNightSkinRoot});
    const QString styleFilePath =
            skinsRoot + QStringLiteral("LateNight/style_classic.qss");

    QFile styleFile(styleFilePath);
    if (!styleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "QmlLegacyLibraryItem: could not open" << styleFilePath
                   << "- library will have no custom styling";
        return;
    }

    QString style = QString::fromUtf8(styleFile.readAll());

    // Resolve the "skins:" URL alias used throughout the QSS file.
    // LegacySkinParser does the same replacement in processStyleNodes().
    style.replace(QStringLiteral("url(skins:"),
            QStringLiteral("url(") + skinsRoot);
    style.replace(QStringLiteral("url(\"skins:"),
            QStringLiteral("url(\"") + skinsRoot);
    style.replace(QStringLiteral("url('skins:"),
            QStringLiteral("url('") + skinsRoot);

    // In the offscreen bridge Qt sometimes falls back to the SVG viewBox size
    // for QHeaderView sort subcontrols. Pin the indicator size to match the
    // native LateNight header instead of rendering a tiny dot.
    style.append(QStringLiteral(
            "\nWTrackTableViewHeader::up-arrow,"
            "\nWTrackTableViewHeader::down-arrow {"
            "\n  width: 14px;"
            "\n  height: 14px;"
            "\n}"));
    style.append(QStringLiteral(
            "\n#LibraryBPMButton::item,"
            "\n#LibraryPlayedCheckbox::item {"
            "\n  background-color: transparent;"
            "\n}"));

    // Prepend default.qss so that SearchClearButton, LibraryPreviewButton,
    // BPM lock and other icon rules are available.
    const QString defaultQssPath =
            skinsRoot + QStringLiteral("default.qss");
    QFile defaultQss(defaultQssPath);
    if (defaultQss.open(QIODevice::ReadOnly | QIODevice::Text)) {
        style.prepend(QString::fromUtf8(defaultQss.readAll()) +
                QStringLiteral("\n"));
    }

    m_pRootWidget->setStyleSheet(style);
}

void QmlLegacyLibraryItem::initializeOverviewTypeControl() {
    // In legacy mode, DlgPrefWaveform creates a ControlPushButton for
    // [Waveform],WaveformOverviewType and seeds it from the config file.
    // In QML mode that dialog is never constructed, so the CO does not
    // exist. OverviewDelegate tries to read it and falls back to 0
    // (= Filtered), which explains the yellow single-colour overviews.
    //
    // We create the CO here, before WLibrary delegates are constructed
    // (bindLibraryWidget), so the delegate sees the correct RGB default.
    UserSettingsPointer pConfig = QmlConfigProxy::get();
    const ConfigKey overviewTypeCfgKey(
            QStringLiteral("[Waveform]"),
            QStringLiteral("WaveformOverviewType"));

    m_pOverviewTypeControl = std::make_unique<ControlPushButton>(overviewTypeCfgKey);
    m_pOverviewTypeControl->setStates(
            QMetaEnum::fromType<mixxx::OverviewType>().keyCount());
    m_pOverviewTypeControl->setReadOnly();

    // Seed from config, defaulting to RGB.
    mixxx::OverviewType overviewType = pConfig->getValue<mixxx::OverviewType>(
            overviewTypeCfgKey, mixxx::OverviewType::RGB);
    m_pOverviewTypeControl->forceSet(static_cast<double>(overviewType));
}

void QmlLegacyLibraryItem::requestRender() {
    m_isDirty = true;
    if (!m_componentComplete || m_isRendering) {
        return;
    }
    // Schedules updatePolish() once before the next scene graph frame.
    // All redundant calls within the same frame are coalesced by Qt for free.
    polish();
}

void QmlLegacyLibraryItem::updatePolish() {
    if (!m_isDirty) {
        return;
    }
    m_isDirty = false;
    renderOffscreen();
    update();
}

bool QmlLegacyLibraryItem::eventFilter(QObject* watched, QEvent* event) {
    if (m_isRendering) {
        return QQuickPaintedItem::eventFilter(watched, event);
    }

    switch (event->type()) {
    case QEvent::Resize:
    case QEvent::Move:
    case QEvent::StyleChange:
    case QEvent::PaletteChange:
    case QEvent::FontChange:
    case QEvent::Show:
    case QEvent::Hide:
    case QEvent::EnabledChange:
    case QEvent::DynamicPropertyChange:
        requestRender();
        break;
    default:
        break;
    }
    return QQuickPaintedItem::eventFilter(watched, event);
}

constexpr const char* kEventFilterInstalledProperty =
        "mixxxQmlLegacyEventFilterInstalled";

void QmlLegacyLibraryItem::installEmbeddedWidgetEventFilters() {
    if (!m_pRootWidget) {
        return;
    }

    QList<QWidget*> widgets = m_pRootWidget->findChildren<QWidget*>();
    widgets.prepend(m_pRootWidget.get());
    for (QWidget* widget : std::as_const(widgets)) {
        if (widget->property(kEventFilterInstalledProperty).toBool()) {
            continue;
        }
        widget->setProperty(kEventFilterInstalledProperty, true);
        widget->installEventFilter(this);

        // Also install on item view viewports explicitly.
        if (auto* view = qobject_cast<QAbstractItemView*>(widget)) {
            if (QWidget* vp = view->viewport()) {
                if (!vp->property(kEventFilterInstalledProperty).toBool()) {
                    vp->setProperty(kEventFilterInstalledProperty, true);
                    vp->installEventFilter(this);
                }
            }
        }
    }
}

void QmlLegacyLibraryItem::syncEmbeddedTableGeometry(QAbstractItemView* view) {
    if (!view) {
        return;
    }

    auto* tableView = qobject_cast<QTableView*>(view);
    if (!tableView) {
        return;
    }

    // Force the table to re-evaluate column positions and scroll extent.
    // doItemsLayout() is the lightest call that invalidates the cached
    // section geometry that QTableView uses for scroll calculations.
    tableView->doItemsLayout();

    QMetaObject::invokeMethod(tableView, "updateEditorGeometries");

    if (tableView->viewport()) {
        tableView->viewport()->update();
    }
    tableView->update();

    QHeaderView* hHeader = tableView->horizontalHeader();
    if (hHeader) {
        if (hHeader->viewport()) {
            hHeader->viewport()->update();
        }
        hHeader->update();
    }

    requestRender();
}

void QmlLegacyLibraryItem::connectEmbeddedWidgetUpdateSignals() {
    if (!m_pRootWidget) {
        return;
    }

    const auto views = m_pRootWidget->findChildren<QAbstractItemView*>();
    for (QAbstractItemView* view : views) {
        QAbstractItemModel* model = view->model();
        if (model) {
            connect(model,
                    &QAbstractItemModel::dataChanged,
                    this,
                    &QmlLegacyLibraryItem::requestRender,
                    Qt::UniqueConnection);
            connect(model,
                    &QAbstractItemModel::rowsInserted,
                    this,
                    &QmlLegacyLibraryItem::requestRender,
                    Qt::UniqueConnection);
            connect(model,
                    &QAbstractItemModel::rowsRemoved,
                    this,
                    &QmlLegacyLibraryItem::requestRender,
                    Qt::UniqueConnection);
            connect(model,
                    &QAbstractItemModel::modelReset,
                    this,
                    &QmlLegacyLibraryItem::requestRender,
                    Qt::UniqueConnection);
            connect(model,
                    &QAbstractItemModel::layoutChanged,
                    this,
                    &QmlLegacyLibraryItem::requestRender,
                    Qt::UniqueConnection);
            connect(model,
                    &QAbstractItemModel::headerDataChanged,
                    this,
                    &QmlLegacyLibraryItem::requestRender,
                    Qt::UniqueConnection);
        }

        QItemSelectionModel* selectionModel = view->selectionModel();
        if (selectionModel) {
            connect(selectionModel,
                    &QItemSelectionModel::selectionChanged,
                    this,
                    &QmlLegacyLibraryItem::requestRender,
                    Qt::UniqueConnection);
            connect(selectionModel,
                    &QItemSelectionModel::currentChanged,
                    this,
                    &QmlLegacyLibraryItem::requestRender,
                    Qt::UniqueConnection);
        }

        auto* tableView = qobject_cast<QTableView*>(view);
        if (tableView) {
            // Connect scrollbars to update editor geometries to prevent drift
            // in offscreen rendering.
            if (!tableView->property(kScrollEditorSyncConnectedProperty).toBool()) {
                tableView->setProperty(kScrollEditorSyncConnectedProperty, true);
                QScrollBar* hScrollBar = tableView->horizontalScrollBar();
                if (hScrollBar) {
                    connect(
                            hScrollBar,
                            &QScrollBar::valueChanged,
                            this,
                            [tableView]() {
                                QMetaObject::invokeMethod(tableView, "updateEditorGeometries");
                            });
                }
                QScrollBar* vScrollBar = tableView->verticalScrollBar();
                if (vScrollBar) {
                    connect(
                            vScrollBar,
                            &QScrollBar::valueChanged,
                            this,
                            [tableView]() {
                                QMetaObject::invokeMethod(tableView, "updateEditorGeometries");
                            });
                }
            }

            // Run an initial sync to cover restored header states from
            // startup, not only live column moves.
            syncEmbeddedTableGeometry(view);
        }
    }

    const auto scrollBars = m_pRootWidget->findChildren<QScrollBar*>();
    for (QScrollBar* scrollBar : scrollBars) {
        connect(scrollBar,
                &QScrollBar::valueChanged,
                this,
                &QmlLegacyLibraryItem::requestRender,
                Qt::UniqueConnection);
        connect(scrollBar,
                &QScrollBar::rangeChanged,
                this,
                &QmlLegacyLibraryItem::requestRender,
                Qt::UniqueConnection);
        connect(scrollBar,
                &QScrollBar::sliderMoved,
                this,
                &QmlLegacyLibraryItem::requestRender,
                Qt::UniqueConnection);
    }
}

} // namespace qml
} // namespace mixxx
