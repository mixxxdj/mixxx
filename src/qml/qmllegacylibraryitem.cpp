#include "qml/qmllegacylibraryitem.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QComboBox>
#include <QContextMenuEvent>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QDomDocument>
#include <QElapsedTimer>
#include <QFile>
#include <QFocusEvent>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QIcon>
#include <QItemSelectionModel>
#include <QMetaEnum>
#include <QMouseEvent>
#include <QPainter>
#include <QPalette>
#include <QPushButton>
#include <QQuickWindow>
#include <QScopedValueRollback>
#include <QScrollBar>
#include <QSplitter>
#include <QStackedLayout>
#include <QStyle>
#include <QStyleHints>
#include <QTableView>
#include <QTextDocument>
#include <QTimer>
#include <QToolTip>
#include <QVBoxLayout>
#include <cmath>
#include <utility>

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "control/controlpushbutton.h"
#include "controllers/keyboard/keyboardeventfilter.h"
#include "library/library.h"
#include "library/library_prefs.h"
#include "library/trackmodel.h"
#include "mixer/basetrackplayer.h"
#include "mixer/playermanager.h"
#include "moc_qmllegacylibraryitem.cpp"
#include "preferences/constants.h"
#include "qml/qmlconfigproxy.h"
#include "qml/qmllibraryproxy.h"
#include "qml/qmlplayermanagerproxy.h"
#include "skin/legacy/skincontext.h"
#include "skin/legacy/tooltips.h"
#include "util/valuetransformer.h"
#include "waveform/overviewtype.h"
#include "waveform/waveformwidgetfactory.h"
#include "widget/controlwidgetconnection.h"
#include "widget/wcolorpicker.h"
#include "widget/wcoverart.h"
#include "widget/wlabel.h"
#include "widget/wlibrary.h"
#include "widget/wlibrarysidebar.h"
#include "widget/wnumber.h"
#include "widget/woverview.h"
#include "widget/wpushbutton.h"
#include "widget/wsearchlineedit.h"
#include "widget/wslidercomposed.h"
#include "widget/wstatuslight.h"
#include "widget/wtrackproperty.h"
#include "widget/wtracktableview.h"
#include "widget/wtracktableviewheader.h"
#include "widget/wvumeterlegacy.h"
#include "widget/wwidgetgroup.h"

namespace mixxx {
namespace qml {

namespace {
constexpr int kInteractionResizeRenderThrottleMillis = 16;

struct SchemeStyle {
    QString qssName;
    QString signalColor;
    QString scrollbarHandleStyle;
    QString scrollbarVerticalStyle;
    QString schemeName;
};

class QmlLibrarySplitterHandle final : public QSplitterHandle {
  public:
    QmlLibrarySplitterHandle(Qt::Orientation orientation, QSplitter* pSplitter)
            : QSplitterHandle(orientation, pSplitter) {
    }

  protected:
    void mousePressEvent(QMouseEvent* pEvent) override {
        pEvent->accept();
    }

    void mouseMoveEvent(QMouseEvent* pEvent) override {
        pEvent->accept();
    }

    void mouseReleaseEvent(QMouseEvent* pEvent) override {
        pEvent->accept();
    }
};

class QmlLibrarySplitter final : public QSplitter {
  public:
    using QSplitter::QSplitter;

  protected:
    QSplitterHandle* createHandle() override {
        return new QmlLibrarySplitterHandle(orientation(), this);
    }
};

SchemeStyle getActiveSchemeStyle() {
    UserSettingsPointer pConfig = QmlConfigProxy::get();
    QString configScheme = pConfig->getValue(
            ConfigKey(QStringLiteral("[Config]"), QStringLiteral("Scheme")));
    if (configScheme.compare(QStringLiteral("Classic"), Qt::CaseInsensitive) == 0) {
        return {
                QStringLiteral("style_classic.qss"),
                QStringLiteral("#e7c413"),
                QStringLiteral(
                        "border-radius: 2px;\n"
                        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
                        "stop:0 #725309, stop:1 #412f05);"),
                QString(),
                QStringLiteral("classic")};
    } else {
        return {
                QStringLiteral("style_palemoon.qss"),
                QStringLiteral("#d9b28c"),
                QStringLiteral("background-color: #333338;"),
                QStringLiteral("border-top: 1px solid #212123;"),
                QStringLiteral("palemoon")};
    }
}

QColor legacyLibraryBackgroundColor(const SchemeStyle& scheme) {
    return scheme.schemeName == QStringLiteral("classic")
            ? QColor(0x0f, 0x0f, 0x0f)
            : QColor(0x08, 0x08, 0x08);
}

bool setDomContent(QDomDocument* pDocument, const QString& content) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    const QDomDocument::ParseResult result = pDocument->setContent(content);
    return static_cast<bool>(result);
#else
    return pDocument->setContent(content);
#endif
}

QList<int> configuredSplitterSizes(
        const UserSettingsPointer& pConfig,
        const ConfigKey& configKey,
        const QList<int>& defaultSizes) {
    if (!pConfig || !pConfig->exists(configKey)) {
        return defaultSizes;
    }

    const QStringList sizeStrings =
            pConfig->getValueString(configKey).split(",");
    if (sizeStrings.size() != defaultSizes.size()) {
        return defaultSizes;
    }

    QList<int> sizes;
    sizes.reserve(sizeStrings.size());
    for (const QString& sizeString : sizeStrings) {
        bool ok = false;
        const int size = sizeString.toInt(&ok);
        if (!ok || size < 0) {
            return defaultSizes;
        }
        sizes.append(size);
    }
    return sizes;
}

void connectSplitterConfig(
        QSplitter* pSplitter,
        const UserSettingsPointer& pConfig,
        const ConfigKey& configKey) {
    QObject::connect(pSplitter,
            &QSplitter::splitterMoved,
            pSplitter,
            [pSplitter, pConfig, configKey](int, int) {
                QStringList sizeStrings;
                const QList<int> sizes = pSplitter->sizes();
                for (const int size : sizes) {
                    sizeStrings.append(QString::number(size));
                }
                pConfig->set(configKey, ConfigValue(sizeStrings.join(",")));
            });
}

void setLateNightPreviewVariables(SkinContext* pContext, const SchemeStyle& scheme) {
    const bool classic = scheme.schemeName == QStringLiteral("classic");
    pContext->setVariable(QStringLiteral("BtnScheme"), scheme.schemeName);
    pContext->setVariable(QStringLiteral("SliderScheme"), scheme.schemeName);
    pContext->setVariable(QStringLiteral("StyleScheme"), scheme.schemeName);
    pContext->setVariable(QStringLiteral("OverviewFontSizePreview"), QStringLiteral("9"));
    pContext->setVariable(QStringLiteral("BgColorOverview_12"),
            classic ? QStringLiteral("rgba(15, 15, 15, 20)") : QStringLiteral("#19191a"));
    pContext->setVariable(QStringLiteral("SignalColor_12"), scheme.signalColor);
    pContext->setVariable(QStringLiteral("SignalHighColor"),
            classic ? QStringLiteral("blue") : QStringLiteral("mediumblue"));
    pContext->setVariable(QStringLiteral("SignalMidColor"),
            classic ? QStringLiteral("green") : QStringLiteral("darkgreen"));
    pContext->setVariable(QStringLiteral("SignalLowColor"),
            classic ? QStringLiteral("red") : QStringLiteral("orangered"));
    pContext->setVariable(QStringLiteral("SignalRGBHighColor"), QString());
    pContext->setVariable(QStringLiteral("SignalRGBMidColor"), QString());
    pContext->setVariable(QStringLiteral("SignalRGBLowColor"), QString());
    pContext->setVariable(QStringLiteral("AxesColor"),
            classic ? QStringLiteral("#ffffff") : QStringLiteral("#999"));
    pContext->setVariable(QStringLiteral("BeatColor"),
            classic ? QStringLiteral("#ffffff") : QStringLiteral("#999"));
    pContext->setVariable(QStringLiteral("PlayPosColor"),
            classic ? QStringLiteral("#00c8ff") : QStringLiteral("#00c6ff"));
    pContext->setVariable(QStringLiteral("CueColor"),
            classic ? QStringLiteral("#ff001c") : QStringLiteral("#ff7a01"));
    pContext->setVariable(QStringLiteral("LoopColor"),
            classic ? QStringLiteral("#00ff00") : QStringLiteral("#00b400"));
    pContext->setVariable(QStringLiteral("IntroOutroColor"),
            classic ? QStringLiteral("#0000ff") : QStringLiteral("#2c5c9a"));
    pContext->setVariable(QStringLiteral("PlayedOverlayColor"),
            classic ? QStringLiteral("#bb000000") : QStringLiteral("#dd151515"));
    pContext->setVariable(QStringLiteral("EndOfTrackColor"), QStringLiteral("#f856e7"));
    pContext->setVariable(QStringLiteral("SlipBorderOutlineColor"),
            classic ? QStringLiteral("#1af000") : QStringLiteral("#f08c00"));
    pContext->setVariable(QStringLiteral("PassthroughLabelColor"),
            classic ? QStringLiteral("#d09300") : QStringLiteral("#b24c12"));
    pContext->setVariable(QStringLiteral("DimBrightThresholdOverview"), QStringLiteral("127"));
    pContext->setVariable(QStringLiteral("VuColor"), QString());
}
} // namespace

QmlLegacyLibraryItem::~QmlLegacyLibraryItem() = default;

QRectF QmlLegacyLibraryItem::previewDeckDropRect() const {
    if (!m_pPreviewDeckBox || !m_pRootWidget) {
        return {};
    }

    const QPoint topLeft = m_pPreviewDeckBox->mapTo(m_pRootWidget.get(), QPoint());
    return QRectF(QPointF(topLeft), QSizeF(m_pPreviewDeckBox->size()));
}

bool QmlLegacyLibraryItem::previewDeckDropEnabled() const {
    return m_pPreviewDeckBox && m_pPreviewDeckBox->isVisible();
}

void QmlLegacyLibraryItem::focusSearch() {
    VERIFY_OR_DEBUG_ASSERT(m_pSearchLineEdit) {
        return;
    }
    forceActiveFocus(Qt::ShortcutFocusReason);
    updateEmbeddedFocus(m_pSearchLineEdit, Qt::ShortcutFocusReason);
    requestRender();
}

QmlLegacyLibraryItem::QmlLegacyLibraryItem(QQuickItem* pParent)
        : QQuickPaintedItem(pParent),
          m_pRootWidget(std::make_unique<QWidget>()),
          m_legacyLibraryBackgroundColor(legacyLibraryBackgroundColor(getActiveSchemeStyle())) {
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
    m_splitterInteractionWatchdogTimer.setInterval(50);
    connect(&m_splitterInteractionWatchdogTimer,
            &QTimer::timeout,
            this,
            [this]() {
                if (!m_pPressedSplitter ||
                        QGuiApplication::mouseButtons() != Qt::NoButton) {
                    return;
                }
                resetSplitterInteraction();
                requestRender();
            });

    QPalette rootPalette = m_pRootWidget->palette();
    rootPalette.setColor(QPalette::Window, m_legacyLibraryBackgroundColor);
    m_pRootWidget->setPalette(rootPalette);
    m_pRootWidget->setAutoFillBackground(true);
    m_pRootWidget->setAttribute(Qt::WA_DontShowOnScreen);
    m_pRootWidget->setObjectName(QStringLiteral("LibraryContainer"));
    m_pRootWidget->show();
    UserSettingsPointer pConfig = QmlConfigProxy::get();
    // 1. Create splitter layout
    // Name must match the LateNight QSS selector "#LibrarySplitter::handle"
    // so the skin image is applied instead of a native/platform handle.
    // See res/skins/LateNight/library.xml and style_classic.qss:2646.
    auto* pSplitter = new QmlLibrarySplitter(m_pRootWidget.get());
    pSplitter->setObjectName(QStringLiteral("LibrarySplitter"));
    pSplitter->setSizePolicy(
            QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    // 2. Sidebar page (search + sidebar)
    auto* pSidebarPage = new WWidgetGroup(pSplitter);
    pSidebarPage->setObjectName(QStringLiteral("LibSidebarContainer"));
    pSidebarPage->setAttribute(Qt::WA_StyledBackground, true);
    pSidebarPage->setSizePolicy(
            QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
    pSidebarPage->setMinimumWidth(100);
    auto* pSidebarLayout = new QVBoxLayout(pSidebarPage);
    pSidebarLayout->setContentsMargins(0, 0, 0, 0);
    pSidebarLayout->setSpacing(0);

    m_pPreviewDeckBox = new WWidgetGroup(pSidebarPage);
    m_pPreviewDeckBox->setSizePolicy(
            QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
    auto* pSearchLineBox = new WWidgetGroup(pSidebarPage);
    pSearchLineBox->setObjectName(QStringLiteral("SearchLineBox"));
    pSearchLineBox->setAttribute(Qt::WA_StyledBackground, true);
    pSearchLineBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
    auto* pSearchLineLayout = new QHBoxLayout(pSearchLineBox);
    pSearchLineLayout->setContentsMargins(0, 0, 0, 0);
    pSearchLineLayout->setSpacing(0);
    pSearchLineLayout->setAlignment(Qt::AlignCenter);
    m_pSearchLineEdit = new WSearchLineEdit(pSearchLineBox, pConfig);
    applyLegacySearchBoxSkinConfiguration();
    pSearchLineLayout->addWidget(m_pSearchLineEdit);

    auto* pSearchAndExpandRow = new WWidgetGroup(pSidebarPage);
    pSearchAndExpandRow->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
    auto* pSearchAndExpandLayout = new QHBoxLayout(pSearchAndExpandRow);
    pSearchAndExpandLayout->setContentsMargins(0, 0, 0, 0);
    pSearchAndExpandLayout->setSpacing(0);
    pSearchAndExpandLayout->setAlignment(Qt::AlignCenter);
    pSearchAndExpandLayout->addWidget(pSearchLineBox, 1);

    m_pLibraryExpandBox = new WWidgetGroup(pSearchAndExpandRow);
    m_pLibraryExpandBox->setObjectName(QStringLiteral("LibExpandBox"));
    m_pLibraryExpandBox->setAttribute(Qt::WA_StyledBackground, true);
    auto* pLibraryExpandLayout = new QVBoxLayout(m_pLibraryExpandBox);
    pLibraryExpandLayout->setContentsMargins(0, 0, 0, 0);
    pLibraryExpandLayout->setSpacing(0);
    pLibraryExpandLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    m_pLibraryExpandButton = new WPushButton(m_pLibraryExpandBox);
    m_pLibraryExpandButton->setObjectName(QStringLiteral("LibExpand"));
    m_pLibraryExpandButton->setFixedWidth(18);
    m_pLibraryExpandButton->setMinimumHeight(18);
    m_pLibraryExpandButton->setSizePolicy(
            QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
    m_pLibraryExpandButton->addAndSetDisplayConnection(
            std::make_unique<ControlParameterWidgetConnection>(
                    m_pLibraryExpandButton,
                    ConfigKey(QStringLiteral("[Skin]"),
                            QStringLiteral("show_maximized_library")),
                    nullptr,
                    ControlParameterWidgetConnection::DIR_DEFAULT,
                    ControlParameterWidgetConnection::EMIT_DEFAULT),
            WBaseWidget::ConnectionSide::Left);
    pLibraryExpandLayout->addWidget(m_pLibraryExpandButton);
    pSearchAndExpandLayout->addWidget(m_pLibraryExpandBox);

    auto* pSearchTreeSpacer = new WWidgetGroup(pSidebarPage);
    pSearchTreeSpacer->setObjectName(QStringLiteral("SearchTreeSpacer"));
    pSearchTreeSpacer->setAttribute(Qt::WA_StyledBackground, true);
    pSearchTreeSpacer->setFixedHeight(3);

    auto* pSidebarCoverSplitter =
            new QmlLibrarySplitter(Qt::Vertical, pSidebarPage);
    pSidebarCoverSplitter->setObjectName(QStringLiteral("SidebarCoverSplitter"));
    pSidebarCoverSplitter->setSizePolicy(
            QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    pSidebarCoverSplitter->setChildrenCollapsible(false);

    auto* pSidebarBox = new WWidgetGroup(pSidebarCoverSplitter);
    pSidebarBox->setObjectName(QStringLiteral("SidebarBox"));
    pSidebarBox->setAttribute(Qt::WA_StyledBackground, true);
    auto* pSidebarBoxLayout = new QVBoxLayout(pSidebarBox);
    pSidebarBoxLayout->setContentsMargins(0, 0, 0, 0);
    pSidebarBoxLayout->setSpacing(0);
    m_pSidebar = new WLibrarySidebar(pSidebarBox);
    pSidebarBoxLayout->addWidget(m_pSidebar);

    m_pCoverArtBox = new WWidgetGroup(pSidebarCoverSplitter);
    m_pCoverArtBox->setObjectName(QStringLiteral("AlignCenter"));
    m_pCoverArtBox->setAttribute(Qt::WA_StyledBackground, true);
    m_pCoverArtBox->setMinimumSize(40, 40);
    auto* pCoverArtBoxLayout = new QVBoxLayout(m_pCoverArtBox);
    pCoverArtBoxLayout->setContentsMargins(0, 0, 0, 0);
    pCoverArtBoxLayout->setSpacing(0);
    m_pCoverArt = new WCoverArt(m_pCoverArtBox, pConfig, QString(), nullptr);
    m_pCoverArt->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_pCoverArt->setMinimumSize(40, 40);
    pCoverArtBoxLayout->addWidget(m_pCoverArt);

    pSidebarCoverSplitter->addWidget(pSidebarBox);
    pSidebarCoverSplitter->addWidget(m_pCoverArtBox);
    pSidebarCoverSplitter->setCollapsible(0, false);
    pSidebarCoverSplitter->setCollapsible(1, false);
    m_pCoverArtSplitter = pSidebarCoverSplitter;
    const ConfigKey coverArtSplitterConfigKey(
            QStringLiteral("[Skin]"),
            QStringLiteral("coverArt_splitsize"));
    QList<int> coverArtSplitterSizes = configuredSplitterSizes(
            pConfig,
            coverArtSplitterConfigKey,
            {1, 1});
    m_initialCoverArtSplitterSizes = coverArtSplitterSizes;
    connectSplitterConfig(
            pSidebarCoverSplitter, pConfig, coverArtSplitterConfigKey);

    pSidebarLayout->addWidget(m_pPreviewDeckBox);
    pSidebarLayout->addWidget(pSearchAndExpandRow);
    pSidebarLayout->addWidget(pSearchTreeSpacer);
    pSidebarLayout->addWidget(pSidebarCoverSplitter, 1);

    // 3. Library (main content area)
    m_pLibraryWidget = new WLibrary(pSplitter);
    m_pLibraryWidget->setObjectName(QStringLiteral("LibraryContainer"));
    applyLegacyLibrarySkinConfiguration();
    applyLegacyCoverArtSkinConfiguration();

    // 4. Add to splitter
    pSplitter->addWidget(pSidebarPage);
    pSplitter->addWidget(m_pLibraryWidget);
    pSplitter->setChildrenCollapsible(false);
    pSplitter->setCollapsible(0, true);
    pSplitter->setCollapsible(1, false);
    m_pLibrarySplitter = pSplitter;
    const ConfigKey librarySplitterConfigKey(
            QStringLiteral("[Skin]"),
            QStringLiteral("librarySidebar_splitsize"));
    QList<int> librarySplitterSizes = configuredSplitterSizes(
            pConfig,
            librarySplitterConfigKey,
            {1, 10});
    m_initialLibrarySplitterSizes = librarySplitterSizes;
    connectSplitterConfig(pSplitter, pConfig, librarySplitterConfigKey);

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
    m_pLibrary = pLibrary;
    m_pPlayerManager = QmlPlayerManagerProxy::get();
    if (m_pPlayerManager) {
        m_pPreviewPlayer = m_pPlayerManager->getPlayer(
                PlayerManager::groupForPreviewDeck(0));
    }
    KeyboardEventFilter* pKeyboard = QmlLibraryProxy::getKeyboard();
    VERIFY_OR_DEBUG_ASSERT(pKeyboard) {
        return;
    }

    pKeyboard->registerSearchBar(m_pSearchLineEdit);
    pLibrary->bindSearchboxWidget(m_pSearchLineEdit);
    pLibrary->bindSidebarWidget(m_pSidebar);
    pLibrary->bindLibraryWidget(m_pLibraryWidget, pKeyboard);

    // The legacy skin parser makes this connection in parseLibrary().
    // Without it the search signal never reaches WLibrary and the
    // track table is never filtered.
    connect(pLibrary,
            &Library::search,
            m_pLibraryWidget,
            &WLibrary::search);
    connect(pLibrary, &Library::switchToView, m_pCoverArt, &WCoverArt::slotReset);
    connect(pLibrary, &Library::enableCoverArtDisplay, m_pCoverArt, &WCoverArt::slotEnable);
    connect(pLibrary, &Library::trackSelected, m_pCoverArt, &WCoverArt::slotLoadTrack);

    createLegacyPreviewDeck();

    // 8. Trigger repaints on visual changes and refresh input tracking for
    //    views that are created or swapped after the initial bind.
    connect(pLibrary, &Library::switchToView, this, [this]() {
        enableEmbeddedWidgetInputTracking();
        applyLegacyScrollbarStyles();
        applyLegacyTableViewBridgeOptions();
        applyLegacyColorPickerBridgeOptions();
        installEmbeddedWidgetEventFilters();
        connectEmbeddedWidgetUpdateSignals();
        repaintEmbeddedViews();
        syncLibraryCoverArtFromSelection();
    });
    connect(pLibrary, &Library::showTrackModel, this, [this]() {
        enableEmbeddedWidgetInputTracking();
        applyLegacyScrollbarStyles();
        applyLegacyTableViewBridgeOptions();
        applyLegacyColorPickerBridgeOptions();
        installEmbeddedWidgetEventFilters();
        connectEmbeddedWidgetUpdateSignals();
        repaintEmbeddedViews();
        syncLibraryCoverArtFromSelection();
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
    syncLibraryCoverArtFromSelection();

    if (QmlConfigProxyBase::s_pInstance) {
        connect(QmlConfigProxyBase::s_pInstance,
                &QmlConfigProxyBase::configSchemeChanged,
                this,
                [this]() {
                    applyLegacyLibrarySkinConfiguration();
                    applyLegacyPreviewDeckSkinConfiguration();
                    applyLegacyCoverArtSkinConfiguration();
                    applyLegacyStylesheet();
                    repolishEmbeddedWidgets();
                    applyLegacyScrollbarStyles();
                    repaintEmbeddedViews();
                    syncLibraryCoverArtFromSelection();
                });
    }

    enableEmbeddedWidgetInputTracking();
    applyLegacyScrollbarStyles();
    applyLegacyTableViewBridgeOptions();
    applyLegacyColorPickerBridgeOptions();
    installEmbeddedWidgetEventFilters();
    connectEmbeddedWidgetUpdateSignals();
    syncLibraryCoverArtFromSelection();

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
    m_pPreviewDeckTrackLoaded->connectValueChanged(this, [this](double value) {
        updatePreviewDeckTrackLoaded(value);
        requestRender();
    });
    updatePreviewDeckTrackLoaded(m_pPreviewDeckTrackLoaded->get());

    m_pShowPreviewDecks = std::make_unique<ControlProxy>(
            QStringLiteral("[Skin]"),
            QStringLiteral("show_preview_decks"),
            this,
            ControlFlag::NoAssertIfMissing);
    m_pShowPreviewDecks->connectValueChanged(this, [this](double value) {
        if (m_pPreviewDeckBox) {
            m_pPreviewDeckBox->setVisible(value > 0.0);
            emit previewDeckGeometryChanged();
            requestRender();
        }
    });
    if (m_pPreviewDeckBox) {
        m_pPreviewDeckBox->setVisible(m_pShowPreviewDecks->toBool());
    }

    m_pShowLibraryCoverArt = std::make_unique<ControlProxy>(
            QStringLiteral("[Skin]"),
            QStringLiteral("show_library_coverart"),
            this,
            ControlFlag::NoAssertIfMissing);
    m_pShowLibraryCoverArt->connectValueChanged(this, [this](double value) {
        if (m_pCoverArtBox) {
            m_pCoverArtBox->setVisible(value > 0.0);
            requestRender();
        }
    });
    if (m_pCoverArtBox) {
        m_pCoverArtBox->setVisible(m_pShowLibraryCoverArt->toBool());
    }
}

void QmlLegacyLibraryItem::renderOffscreen() {
    if (!m_pRootWidget) {
        return;
    }
    syncRootWidgetGlobalPosition();
    updateWidgetSize();
    const QSize logicalSize(qMax(1, qRound(width())),
            qMax(1, qRound(height())));
    qreal devicePixelRatio = window() ? window()->devicePixelRatio() : 0.0;
    if (!qIsFinite(devicePixelRatio) || devicePixelRatio <= 0.0) {
        devicePixelRatio = m_pRootWidget->devicePixelRatioF();
    }
    if (!qIsFinite(devicePixelRatio) || devicePixelRatio <= 0.0) {
        devicePixelRatio = 1.0;
    }
    const QSize physicalSize(
            qMax(1, static_cast<int>(std::ceil(logicalSize.width() * devicePixelRatio))),
            qMax(1, static_cast<int>(std::ceil(logicalSize.height() * devicePixelRatio))));
    if (m_offscreenPixmap.size() != physicalSize ||
            !qFuzzyCompare(m_offscreenPixmap.devicePixelRatio(), devicePixelRatio)) {
        m_offscreenPixmap = QPixmap(physicalSize);
        m_offscreenPixmap.setDevicePixelRatio(devicePixelRatio);
    }
    m_offscreenPixmap.fill(m_legacyLibraryBackgroundColor);

    // Process all pending layout, resize, and geometry events for the QWidget tree
    // so that child widgets (persistent editors) are correctly positioned before rendering.
    QCoreApplication::sendPostedEvents(m_pRootWidget.get());

    QPainter painter(&m_offscreenPixmap);
    const QScopedValueRollback<bool> renderingRollback(m_isRendering, true);
    m_pRootWidget->render(&painter);
}

void QmlLegacyLibraryItem::paint(QPainter* pPainter) {
    pPainter->fillRect(QRectF(0, 0, width(), height()),
            m_legacyLibraryBackgroundColor);
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
    emit previewDeckGeometryChanged();
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

QPointF widgetScenePos(QWidget* pTarget, QWidget* pRoot, const QPoint& rootPos) {
    return QPointF(pTarget->mapFrom(pRoot, rootPos));
}

void updateColorPickerButtonIcon(QPushButton* pButton) {
    pButton->setIcon(QIcon(pButton->isChecked()
                    ? QStringLiteral(":/images/ic_checkmark.svg")
                    : QString()));
}

bool isContextMenuOnMouseRelease() {
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    const QStyleHints* pStyleHints = QGuiApplication::styleHints();
    return pStyleHints &&
            pStyleHints->contextMenuTrigger() == Qt::ContextMenuTrigger::Release;
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

    QWidget* pWidget = m_pRootWidget->childAt(rootPos);
    if (!pWidget) {
        return m_pRootWidget.get();
    }

    while (QWidget* pChild =
                    pWidget->childAt(pWidget->mapFrom(m_pRootWidget.get(), rootPos))) {
        if (pChild == pWidget) {
            break;
        }
        pWidget = pChild;
    }
    return pWidget;
}

QAbstractItemView* QmlLegacyLibraryItem::parentItemView(QWidget* pWidget) const {
    for (QWidget* pCurrent = pWidget; pCurrent; pCurrent = pCurrent->parentWidget()) {
        if (auto* pView = qobject_cast<QAbstractItemView*>(pCurrent)) {
            return pView;
        }
    }
    return nullptr;
}

QHeaderView* QmlLegacyLibraryItem::parentHeaderView(QWidget* pWidget) const {
    for (QWidget* pCurrent = pWidget; pCurrent; pCurrent = pCurrent->parentWidget()) {
        if (auto* pHeader = qobject_cast<QHeaderView*>(pCurrent)) {
            return pHeader;
        }
    }
    return nullptr;
}

QSplitterHandle* QmlLegacyLibraryItem::parentSplitterHandle(QWidget* pWidget) const {
    for (QWidget* pCurrent = pWidget; pCurrent; pCurrent = pCurrent->parentWidget()) {
        if (auto* pHandle = qobject_cast<QSplitterHandle*>(pCurrent)) {
            return pHandle;
        }
    }
    return nullptr;
}

QSplitterHandle* QmlLegacyLibraryItem::splitterHandleAtRootPos(
        const QPoint& rootPos) const {
    if (!m_pRootWidget) {
        return nullptr;
    }

    const auto splitters = m_pRootWidget->findChildren<QSplitter*>();
    for (QSplitter* pSplitter : splitters) {
        for (int index = 1; index < pSplitter->count(); ++index) {
            QSplitterHandle* pHandle = pSplitter->handle(index);
            if (!pHandle || !pHandle->isVisible() || !pHandle->isEnabled()) {
                continue;
            }

            QRect handleRect(
                    pHandle->mapTo(m_pRootWidget.get(), QPoint()), pHandle->size());
            const int margin = 2;
            if (pSplitter->orientation() == Qt::Horizontal) {
                handleRect.adjust(-margin, 0, margin, 0);
            } else {
                handleRect.adjust(0, -margin, 0, margin);
            }
            if (handleRect.contains(rootPos)) {
                return pHandle;
            }
        }
    }
    return nullptr;
}

QWidget* QmlLegacyLibraryItem::eventTargetFor(QWidget* pWidget) const {
    if (!pWidget) {
        return m_pRootWidget.get();
    }

    if (auto* pHandle = parentSplitterHandle(pWidget)) {
        return pHandle;
    }

    // QHeaderView IS-A QAbstractItemView, but sort-click handling lives on the
    // header itself, not its internal viewport. Do NOT redirect header clicks.
    if (qobject_cast<QHeaderView*>(pWidget)) {
        return pWidget;
    }

    if (auto* pView = parentItemView(pWidget)) {
        // For table/list views, redirect to viewport so delegates get events.
        if (pWidget == pView) {
            QWidget* pViewport = pView->viewport();
            return pViewport ? pViewport : pWidget;
        }
    }
    return pWidget;
}

QWidget* QmlLegacyLibraryItem::contextMenuTargetFor(QWidget* pWidget) const {
    if (!pWidget) {
        return m_pRootWidget.get();
    }

    if (auto* pHeader = parentHeaderView(pWidget)) {
        return pHeader;
    }

    if (auto* pView = parentItemView(pWidget)) {
        QWidget* pViewport = pView->viewport();
        return pViewport ? pViewport : pView;
    }

    return pWidget;
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

bool QmlLegacyLibraryItem::sendContextMenuToWidget(QMouseEvent* pEvent, QWidget* pTarget) {
    if (!m_pRootWidget || !pTarget) {
        return false;
    }

    const QPoint rootPos = pEvent->position().toPoint();
    updateHoverTarget(pTarget, rootPos, pEvent->modifiers());

    const QPoint targetPos = pTarget->mapFrom(m_pRootWidget.get(), rootPos);
    const QPoint globalPos = mapToGlobalScreen(rootPos);

    QContextMenuEvent contextEvent(
            QContextMenuEvent::Mouse,
            targetPos,
            globalPos,
            pEvent->modifiers());

    if (auto* pTrackTableHeader = qobject_cast<WTrackTableViewHeader*>(pTarget)) {
        pTrackTableHeader->contextMenuEvent(&contextEvent);
    } else {
        QApplication::sendEvent(pTarget, &contextEvent);
    }
    if (contextEvent.isAccepted()) {
        pEvent->accept();
    }
    syncCursorFromWidget(pTarget, rootPos);
    return contextEvent.isAccepted();
}

bool QmlLegacyLibraryItem::sendMouseToWidget(QMouseEvent* pEvent, QWidget* pTarget) {
    if (!m_pRootWidget || !pTarget) {
        return false;
    }

    const QPointF rootPos = pEvent->position();
    const QPoint rootPoint = rootPos.toPoint();
    updateHoverTarget(pTarget, rootPoint, pEvent->modifiers());

    const QPointF targetPos = widgetScenePos(pTarget, m_pRootWidget.get(), rootPoint);
    const QPointF globalPos = pEvent->globalPosition();
    QMouseEvent mappedEvent(
            pEvent->type(),
            targetPos,
            rootPos,
            globalPos,
            pEvent->button(),
            pEvent->buttons(),
            pEvent->modifiers(),
            pEvent->source());

    QApplication::sendEvent(pTarget, &mappedEvent);
    pEvent->setAccepted(mappedEvent.isAccepted());
    m_pressedButtons = pEvent->buttons();

    syncCursorFromWidget(pTarget, rootPoint);
    return mappedEvent.isAccepted();
}

void QmlLegacyLibraryItem::sendSyntheticMouseMoveToWidget(QWidget* pTarget,
        const QPoint& rootPos,
        const QPointF& globalPos,
        Qt::KeyboardModifiers modifiers,
        Qt::MouseButtons buttons) {
    if (!m_pRootWidget || !pTarget) {
        return;
    }

    const QPointF targetPos = widgetScenePos(pTarget, m_pRootWidget.get(), rootPos);
    const QPointF windowPos = QPointF(rootPos);
    QMouseEvent moveEvent(
            QEvent::MouseMove,
            targetPos,
            windowPos,
            globalPos,
            Qt::NoButton,
            buttons,
            modifiers);
    QApplication::sendEvent(pTarget, &moveEvent);
    syncCursorFromWidget(pTarget, rootPos);
}

bool QmlLegacyLibraryItem::sendWheelToWidget(QWheelEvent* pEvent) {
    if (!m_pRootWidget) {
        return false;
    }

    QWidget* pTarget = eventTargetFor(widgetAtRootPos(pEvent->position().toPoint()));
    if (!pTarget) {
        return false;
    }

    const QPoint rootPos = pEvent->position().toPoint();
    updateHoverTarget(pTarget, rootPos, pEvent->modifiers());

    // Wheel events over child widgets of a table view (e.g. the Preview
    // column's persistent editor QPushButtons) must be redirected to the
    // view's viewport so the table handles scrolling.  Without this, the
    // child widget swallows the event and horizontal scroll appears stuck.
    QAbstractItemView* pScrollView = parentItemView(pTarget);
    if (pScrollView) {
        if (pTarget != pScrollView && pTarget != pScrollView->viewport()) {
            QWidget* pViewport = pScrollView->viewport();
            if (pViewport) {
                pTarget = pViewport;
            }
        }
    }

    QWheelEvent mappedEvent(
            widgetScenePos(pTarget, m_pRootWidget.get(), rootPos),
            pEvent->globalPosition(),
            pEvent->pixelDelta(),
            pEvent->angleDelta(),
            pEvent->buttons(),
            pEvent->modifiers(),
            pEvent->phase(),
            pEvent->inverted());

    QApplication::sendEvent(pTarget, &mappedEvent);
    pEvent->setAccepted(mappedEvent.isAccepted());

    syncCursorFromWidget(pTarget, rootPos);
    return mappedEvent.isAccepted();
}

bool QmlLegacyLibraryItem::sendHoverToWidget(QHoverEvent* pEvent) {
    if (!m_pRootWidget) {
        return false;
    }

    QWidget* pTarget = eventTargetFor(widgetAtRootPos(pEvent->position().toPoint()));
    if (!pTarget) {
        return false;
    }

    const QPoint rootPos = pEvent->position().toPoint();
    updateHoverTarget(pTarget, rootPos, pEvent->modifiers());

    const QPoint targetPos = pTarget->mapFrom(m_pRootWidget.get(), rootPos);
    const QPoint oldTargetPos = pTarget->mapFrom(m_pRootWidget.get(), pEvent->oldPos());
    QHoverEvent mappedEvent(
            pEvent->type(),
            targetPos,
#if QT_VERSION >= QT_VERSION_CHECK(6, 3, 0)
            pEvent->globalPosition(),
#endif
            oldTargetPos,
            pEvent->modifiers());

    QApplication::sendEvent(pTarget, &mappedEvent);
    pEvent->setAccepted(mappedEvent.isAccepted());
    m_lastHoverRootPos = pEvent->position();

    // QTableView::entered(), which PreviewButtonDelegate uses to open the
    // real QPushButton editor, is driven by mouse tracking rather than
    // QHoverEvent delivery. Mirror QQuick hover as a no-button mouse move so
    // item-view delegates see the same path they get in a native QWidget skin.
#if QT_VERSION < QT_VERSION_CHECK(6, 3, 0)
    sendSyntheticMouseMoveToWidget(pTarget, rootPos, pEvent->position(), pEvent->modifiers());
#else
    sendSyntheticMouseMoveToWidget(pTarget, rootPos, pEvent->globalPosition(), pEvent->modifiers());
#endif

    scheduleToolTip(pTarget, rootPos);
    syncCursorFromWidget(pTarget, rootPos);
    return mappedEvent.isAccepted();
}

void QmlLegacyLibraryItem::scheduleToolTip(QWidget* pTarget, const QPoint& rootPos) {
    if (!pTarget || m_pressedButtons != Qt::NoButton) {
        cancelToolTip();
        return;
    }

    const int restartDistance = QApplication::startDragDistance();
    if (m_pToolTipTarget == pTarget &&
            (m_toolTipRootPos - rootPos).manhattanLength() < restartDistance) {
        return;
    }

    m_pToolTipTarget = pTarget;
    m_toolTipRootPos = rootPos;
    if (!m_toolTipText.isEmpty()) {
        m_toolTipText.clear();
        QToolTip::hideText();
    }

    int wakeUpDelay = QApplication::style()->styleHint(QStyle::SH_ToolTip_WakeUpDelay);
    if (QStyle* pStyle = pTarget->style()) {
        wakeUpDelay = pStyle->styleHint(QStyle::SH_ToolTip_WakeUpDelay, nullptr, pTarget);
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

    QWidget* pTarget = eventTargetFor(widgetAtRootPos(m_toolTipRootPos));
    if (!pTarget) {
        return;
    }

    const QString toolTipText = toolTipTextForTarget(pTarget, m_toolTipRootPos);
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
        QWidget* pTarget, const QPoint& rootPos) const {
    if (!pTarget || !m_pRootWidget) {
        return QString();
    }

    if (auto* pHeader = parentHeaderView(pTarget)) {
        const int section = pHeader->logicalIndexAt(
                pHeader->mapFrom(m_pRootWidget.get(), rootPos));
        if (section >= 0 && pHeader->model()) {
            return pHeader->model()
                    ->headerData(section, pHeader->orientation(), Qt::ToolTipRole)
                    .toString();
        }
    }

    if (auto* pView = parentItemView(pTarget)) {
        QWidget* pViewport = pView->viewport();
        if (!pViewport) {
            return QString();
        }
        const QModelIndex index = pView->indexAt(
                pViewport->mapFrom(m_pRootWidget.get(), rootPos));
        if (index.isValid()) {
            return index.data(Qt::ToolTipRole).toString();
        }
    }

    return pTarget->toolTip();
}

void QmlLegacyLibraryItem::updateHoverTarget(
        QWidget* pTarget,
        const QPoint& rootPos,
        [[maybe_unused]] Qt::KeyboardModifiers modifiers) {
    if (!pTarget || pTarget == m_pLastHoverWidget) {
        return;
    }

    if (m_pLastHoverWidget) {
        QEvent leaveEvent(QEvent::Leave);
        QApplication::sendEvent(m_pLastHoverWidget, &leaveEvent);
    }

    const QPointF targetPos = widgetScenePos(pTarget, m_pRootWidget.get(), rootPos);
    QEnterEvent enterEvent(targetPos,
            QPointF(rootPos),
            QPointF(pTarget->mapToGlobal(targetPos.toPoint())));
    QApplication::sendEvent(pTarget, &enterEvent);
    m_pLastHoverWidget = pTarget;
}

bool QmlLegacyLibraryItem::isHeaderResizeHandle(QHeaderView* pHeader, const QPoint& rootPos) const {
    if (!pHeader || !m_pRootWidget) {
        return false;
    }

    const QPoint headerPos = pHeader->mapFrom(m_pRootWidget.get(), rootPos);
    const int logicalIndex = pHeader->logicalIndexAt(headerPos);
    if (logicalIndex < 0) {
        return false;
    }

    const int sectionStart = pHeader->sectionViewportPosition(logicalIndex);
    const int sectionEnd = sectionStart + pHeader->sectionSize(logicalIndex);
    const int cursorPos = pHeader->orientation() == Qt::Horizontal ? headerPos.x() : headerPos.y();
    return std::abs(cursorPos - sectionStart) <= kHeaderResizeCursorMargin ||
            std::abs(cursorPos - sectionEnd) <= kHeaderResizeCursorMargin;
}

void QmlLegacyLibraryItem::maybeApplyHeaderSortFallback(
        QHeaderView* pHeader, const QPoint& rootPos) {
    if (!pHeader || pHeader != m_pPressedHeader || m_pressedHeaderSection < 0) {
        return;
    }
    if (isHeaderResizeHandle(pHeader, rootPos)) {
        return;
    }
    if ((rootPos - m_pressRootPos).manhattanLength() > QApplication::startDragDistance()) {
        return;
    }

    const QPoint headerPos = pHeader->mapFrom(m_pRootWidget.get(), rootPos);
    const int releaseSection = pHeader->logicalIndexAt(headerPos);
    if (releaseSection != m_pressedHeaderSection || !pHeader->sectionsClickable()) {
        return;
    }

    if (pHeader->sortIndicatorSection() != m_pressedHeaderSortSection ||
            pHeader->sortIndicatorOrder() != m_pressedHeaderSortOrder) {
        // sortIndicatorChanged already fired during the press/release cycle,
        // so the native sort path already ran. No fallback needed.
        return;
    }

    const Qt::SortOrder order = pHeader->sortIndicatorSection() == releaseSection
            ? (pHeader->sortIndicatorOrder() == Qt::AscendingOrder
                              ? Qt::DescendingOrder
                              : Qt::AscendingOrder)
            : Qt::AscendingOrder;

    pHeader->setSortIndicator(releaseSection, order);
    pHeader->update();
}

void QmlLegacyLibraryItem::startHeaderInteraction(QWidget* pTarget, const QPoint& rootPos) {
    resetHeaderInteraction(true);

    QHeaderView* pHeader = parentHeaderView(pTarget);
    if (!pHeader) {
        return;
    }

    m_pPressedHeader = pHeader;
    m_lastForwardedHeaderMoveRootPos = rootPos;

    if (isHeaderResizeHandle(pHeader, rootPos)) {
        m_headerInteraction = HeaderInteraction::Resize;
        return;
    }

    const QPoint headerPos = pHeader->mapFrom(m_pRootWidget.get(), rootPos);
    const int section = pHeader->logicalIndexAt(headerPos);
    if (section < 0 || !pHeader->sectionsMovable()) {
        resetHeaderInteraction();
        return;
    }

    m_headerInteraction = HeaderInteraction::MoveCandidate;
    m_pressedHeaderSection = section;
    m_pressedHeaderSortSection = pHeader->sortIndicatorSection();
    m_pressedHeaderSortOrder = pHeader->sortIndicatorOrder();

    // Disable Qt's native autoscroll for the duration of this interaction.
    // It cannot work correctly in the offscreen bridge environment.
    m_pPressedHeader->setAutoScroll(false);
}

bool QmlLegacyLibraryItem::shouldForwardHeaderMove(QWidget* pTarget, const QPoint& rootPos) {
    if (m_headerInteraction == HeaderInteraction::None) {
        return true;
    }

    QHeaderView* pHeader = m_pPressedHeader ? m_pPressedHeader.data() : parentHeaderView(pTarget);
    if (!pHeader || pHeader != parentHeaderView(pTarget)) {
        stopBridgeAutoScroll();
        return true;
    }

    if (m_headerInteraction == HeaderInteraction::Resize) {
        m_lastForwardedHeaderMoveRootPos = rootPos;
        return true;
    }

    const int dragDistance = QApplication::startDragDistance();
    const int axisDelta = pHeader->orientation() == Qt::Horizontal
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

    QHeaderView* pHeader = m_pPressedHeader.data();
    auto* pParentView = qobject_cast<QAbstractItemView*>(pHeader->parentWidget());
    if (!pParentView) {
        stopBridgeAutoScroll();
        return;
    }

    QScrollBar* pScrollBar = pHeader->orientation() == Qt::Horizontal
            ? pParentView->horizontalScrollBar()
            : pParentView->verticalScrollBar();
    if (!pScrollBar) {
        stopBridgeAutoScroll();
        return;
    }

    // Determine scroll direction from cursor position relative to the
    // header viewport, mirroring Qt's autoScrollMargin logic.
    QWidget* pViewport = pHeader->viewport();
    if (!pViewport) {
        stopBridgeAutoScroll();
        return;
    }

    const QPoint viewportPos = pViewport->mapFrom(
            m_pRootWidget.get(), m_lastForwardedHeaderMoveRootPos);
    const QRect area = pViewport->rect();
    const int margin = pHeader->autoScrollMargin();
    int direction = 0;
    if (pHeader->orientation() == Qt::Horizontal) {
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
    const int pageStep = pScrollBar->pageStep();
    if (m_headerAutoScrollCount < pageStep) {
        ++m_headerAutoScrollCount;
    }

    const int oldValue = pScrollBar->value();
    pScrollBar->setValue(oldValue + direction * m_headerAutoScrollCount);

    if (pScrollBar->value() == oldValue) {
        // Hit the limit, stop.
        stopBridgeAutoScroll();
        return;
    }

    // Re-send the last cursor position as a synthetic mouse move so the
    // header updates its section indicator and move target with the new
    // scroll offset.  This is the step that Qt's native autoscroll
    // expects continuous OS mouse-drag events to provide.
    QWidget* pTarget = m_pGrabbedWidget.data();
    const QPoint rootPos = m_lastForwardedHeaderMoveRootPos;
    sendSyntheticMouseMoveToWidget(pTarget,
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

bool QmlLegacyLibraryItem::startSplitterInteraction(
        QWidget* pTarget, const QPoint& rootPos) {
    resetHeaderInteraction(true);
    resetSplitterInteraction();

    QSplitterHandle* pHandle = parentSplitterHandle(pTarget);
    if (!pHandle) {
        return false;
    }

    QSplitter* pSplitter = pHandle->splitter();
    if (!pSplitter || !dynamic_cast<QmlLibrarySplitter*>(pSplitter)) {
        return false;
    }

    int handleIndex = -1;
    for (int index = 1; index < pSplitter->count(); ++index) {
        if (pSplitter->handle(index) == pHandle) {
            handleIndex = index;
            break;
        }
    }
    if (handleIndex < 1) {
        return false;
    }

    const QList<int> splitterSizes = pSplitter->sizes();
    if (splitterSizes.size() != pSplitter->count()) {
        return false;
    }

    m_pPressedSplitter = pSplitter;
    m_pPressedSplitterHandle = pHandle;
    m_splitterHandleIndex = handleIndex;
    m_splitterStartSizes = splitterSizes;
    m_splitterStartRootPos = rootPos;
    setSplitterHandlePressed(pHandle, true);
    setKeepMouseGrab(true);
    grabMouse();
    m_splitterInteractionWatchdogTimer.start();
    return true;
}

void QmlLegacyLibraryItem::resizeSplitterFromRootPos(const QPoint& rootPos) {
    QSplitter* pSplitter = m_pPressedSplitter.data();
    if (!pSplitter || m_splitterHandleIndex <= 0 ||
            m_splitterHandleIndex >= pSplitter->count() ||
            m_splitterHandleIndex >= m_splitterStartSizes.size()) {
        return;
    }

    const int beforeIndex = m_splitterHandleIndex - 1;
    const int afterIndex = m_splitterHandleIndex;
    QWidget* pBefore = pSplitter->widget(beforeIndex);
    QWidget* pAfter = pSplitter->widget(afterIndex);
    if (!pBefore || !pAfter) {
        return;
    }

    const bool horizontal = pSplitter->orientation() == Qt::Horizontal;
    const int delta = horizontal
            ? rootPos.x() - m_splitterStartRootPos.x()
            : rootPos.y() - m_splitterStartRootPos.y();

    const int total = m_splitterStartSizes.at(beforeIndex) +
            m_splitterStartSizes.at(afterIndex);
    if (total <= 0) {
        return;
    }

    const QSize beforeMinimumSizeHint = pBefore->minimumSizeHint();
    const QSize afterMinimumSizeHint = pAfter->minimumSizeHint();
    const int beforeMinimum = pSplitter->isCollapsible(beforeIndex)
            ? 0
            : (horizontal
                              ? qMax(pBefore->minimumWidth(), beforeMinimumSizeHint.width())
                              : qMax(pBefore->minimumHeight(), beforeMinimumSizeHint.height()));
    const int afterMinimum = pSplitter->isCollapsible(afterIndex)
            ? 0
            : (horizontal
                              ? qMax(pAfter->minimumWidth(), afterMinimumSizeHint.width())
                              : qMax(pAfter->minimumHeight(), afterMinimumSizeHint.height()));
    const int minimumBeforeSize = qMin(beforeMinimum, total);
    const int maximumBeforeSize = qMax(minimumBeforeSize, total - afterMinimum);
    const int beforeSize = qBound(
            minimumBeforeSize,
            m_splitterStartSizes.at(beforeIndex) + delta,
            maximumBeforeSize);

    QList<int> sizes = m_splitterStartSizes;
    sizes[beforeIndex] = beforeSize;
    sizes[afterIndex] = total - beforeSize;
    if (sizes.at(beforeIndex) > 0) {
        pBefore->show();
    }
    const bool coverArtVisible = pAfter != m_pCoverArtBox ||
            !m_pShowLibraryCoverArt || m_pShowLibraryCoverArt->toBool();
    if (sizes.at(afterIndex) > 0 && coverArtVisible) {
        pAfter->show();
    }
    const QList<int> previousSizes = pSplitter->sizes();
    pSplitter->setSizes(sizes);
    m_splitterSizesDirty = m_splitterSizesDirty || previousSizes != pSplitter->sizes();
}

void QmlLegacyLibraryItem::resetSplitterInteraction() {
    QSplitterHandle* pHandle = m_pPressedSplitterHandle.data();
    QSplitter* pSplitter = m_pPressedSplitter.data();
    const bool hadSplitterInteraction = pSplitter != nullptr;
    if (pSplitter && m_splitterSizesDirty) {
        persistSplitterSizes(pSplitter);
    }
    m_pPressedSplitterHandle.clear();
    m_pPressedSplitter.clear();
    m_splitterHandleIndex = -1;
    m_splitterStartSizes.clear();
    m_splitterStartRootPos = QPoint();
    m_resizeInteractionRenderPending = false;
    m_splitterSizesDirty = false;
    m_splitterInteractionWatchdogTimer.stop();
    setKeepMouseGrab(false);
    if (pHandle) {
        setSplitterHandlePressed(pHandle, false);
    }
    if (hadSplitterInteraction && !m_handlingMouseUngrab) {
        ungrabMouse();
    }
}

void QmlLegacyLibraryItem::persistSplitterSizes(QSplitter* pSplitter) {
    if (!pSplitter) {
        return;
    }

    const ConfigKey configKey = pSplitter == m_pLibrarySplitter
            ? ConfigKey(QStringLiteral("[Skin]"),
                      QStringLiteral("librarySidebar_splitsize"))
            : ConfigKey(QStringLiteral("[Skin]"),
                      QStringLiteral("coverArt_splitsize"));
    QStringList sizeStrings;
    const QList<int> sizes = pSplitter->sizes();
    for (const int size : sizes) {
        sizeStrings.append(QString::number(size));
    }
    QmlConfigProxy::get()->set(configKey, ConfigValue(sizeStrings.join(",")));
}

void QmlLegacyLibraryItem::setSplitterHandlePressed(
        QSplitterHandle* pHandle, bool pressed) {
    if (!pHandle || !pHandle->splitter()) {
        return;
    }

    QSplitter* pSplitter = pHandle->splitter();
    if (pSplitter->property("bridgePressed").toBool() == pressed) {
        return;
    }

    pSplitter->setProperty("bridgePressed", pressed);
    if (QStyle* pStyle = pSplitter->style()) {
        pStyle->unpolish(pSplitter);
        pStyle->polish(pSplitter);
    }
    pHandle->update();
}

void QmlLegacyLibraryItem::syncCursorFromWidget(QWidget* pTarget, const QPoint& rootPos) {
    if (!pTarget) {
        unsetCursor();
        return;
    }

    if (auto* pHeader = parentHeaderView(pTarget)) {
        if (isHeaderResizeHandle(pHeader, rootPos)) {
            setCursor(pHeader->orientation() == Qt::Horizontal
                            ? Qt::SplitHCursor
                            : Qt::SplitVCursor);
            return;
        }
    }

    if (auto* pHandle = parentSplitterHandle(pTarget)) {
        setCursor(pHandle->orientation() == Qt::Horizontal
                        ? Qt::SplitHCursor
                        : Qt::SplitVCursor);
        return;
    }

    for (QWidget* pCurrent = pTarget; pCurrent; pCurrent = pCurrent->parentWidget()) {
        if (pCurrent->testAttribute(Qt::WA_SetCursor)) {
            setCursor(pCurrent->cursor());
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
    for (QAbstractItemView* pView : views) {
        if (pView->viewport()) {
            pView->viewport()->update();
        }
        pView->update();
    }
    requestRender();
}

void QmlLegacyLibraryItem::applyLegacyScrollbarStyle(QScrollBar* pScrollBar) {
    if (!pScrollBar) {
        return;
    }

    const SchemeStyle scheme = getActiveSchemeStyle();
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
  %1
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
  %2
}
QScrollBar::handle:vertical {
  min-height: 25px;
  %2
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
)MIXXXQSS")
                                           .arg(scheme.scrollbarVerticalStyle,
                                                   scheme.scrollbarHandleStyle);

    pScrollBar->setAttribute(Qt::WA_StyledBackground, true);
    pScrollBar->setAutoFillBackground(true);
    pScrollBar->setStyleSheet(scrollBarStyle);
    if (QStyle* pStyle = pScrollBar->style()) {
        pStyle->unpolish(pScrollBar);
        pStyle->polish(pScrollBar);
    }
    pScrollBar->ensurePolished();
    pScrollBar->update();
}

void QmlLegacyLibraryItem::applyLegacyScrollbarStyles() {
    if (!m_pRootWidget) {
        return;
    }

    const auto scrollBars = m_pRootWidget->findChildren<QScrollBar*>();
    for (QScrollBar* pScrollBar : scrollBars) {
        applyLegacyScrollbarStyle(pScrollBar);
    }
}

void QmlLegacyLibraryItem::repolishEmbeddedWidgets() {
    if (!m_pRootWidget) {
        return;
    }

    QList<QWidget*> widgets = m_pRootWidget->findChildren<QWidget*>();
    widgets.prepend(m_pRootWidget.get());
    for (QWidget* pWidget : std::as_const(widgets)) {
        if (QStyle* pStyle = pWidget->style()) {
            pStyle->unpolish(pWidget);
            pStyle->polish(pWidget);
        }
        pWidget->ensurePolished();
        pWidget->update();
    }
}

void QmlLegacyLibraryItem::enableEmbeddedWidgetInputTracking() {
    if (!m_pRootWidget) {
        return;
    }

    QList<QWidget*> widgets = m_pRootWidget->findChildren<QWidget*>();
    widgets.prepend(m_pRootWidget.get());
    for (QWidget* pWidget : std::as_const(widgets)) {
        pWidget->setMouseTracking(true);
        pWidget->setAttribute(Qt::WA_Hover, true);
        pWidget->setAttribute(Qt::WA_NoMousePropagation, false);
    }
}

void QmlLegacyLibraryItem::applyLegacyTableViewBridgeOptions() {
    if (!m_pRootWidget) {
        return;
    }

    const auto tableViews = m_pRootWidget->findChildren<WTrackTableView*>();
    for (WTrackTableView* pTableView : tableViews) {
        pTableView->setProperty(kColorDelegateBridgeProperty, true);
    }
}

void QmlLegacyLibraryItem::applyLegacyColorPickerBridgeOptions() {
    if (!m_pRootWidget) {
        return;
    }

    const auto colorPickers = m_pRootWidget->findChildren<WColorPicker*>();
    for (WColorPicker* pColorPicker : colorPickers) {
        const auto buttons = pColorPicker->findChildren<QPushButton*>();
        for (QPushButton* pButton : buttons) {
            if (!pButton->property(kColorPickerButtonBridgeProperty).toBool()) {
                pButton->setProperty(kColorPickerButtonBridgeProperty, true);
                connect(pButton,
                        &QPushButton::toggled,
                        this,
                        [pButton]() {
                            updateColorPickerButtonIcon(pButton);
                            QTimer::singleShot(0, pButton, [pButton]() {
                                updateColorPickerButtonIcon(pButton);
                            });
                        });
            }
            updateColorPickerButtonIcon(pButton);
        }
    }
}

void QmlLegacyLibraryItem::mousePressEvent(QMouseEvent* pEvent) {
    syncRootWidgetGlobalPosition();
    cancelToolTip();
    const QPoint rootPos = pEvent->position().toPoint();
    QWidget* pTarget = eventTargetFor(widgetAtRootPos(rootPos));
    QSplitterHandle* pSplitterHandle = nullptr;
    if (pEvent->button() == Qt::LeftButton) {
        pSplitterHandle = splitterHandleAtRootPos(rootPos);
        if (pSplitterHandle) {
            pTarget = pSplitterHandle;
        }
    }
    if (!pSplitterHandle) {
        sendSyntheticMouseMoveToWidget(
                pTarget, rootPos, pEvent->globalPosition(), pEvent->modifiers());
        pTarget = eventTargetFor(widgetAtRootPos(rootPos));
        if (pEvent->button() == Qt::LeftButton) {
            pSplitterHandle = splitterHandleAtRootPos(rootPos);
            if (pSplitterHandle) {
                pTarget = pSplitterHandle;
            }
        }
    }
    m_pPressedWidget = pTarget;
    m_pGrabbedWidget = pTarget;
    m_pressedButtons = pEvent->buttons() | pEvent->button();
    m_pressRootPos = rootPos;
    const bool splitterInteractionStarted =
            pEvent->button() == Qt::LeftButton &&
            startSplitterInteraction(pTarget, rootPos);
    if (splitterInteractionStarted) {
        pEvent->accept();
        requestRender();
    } else if (pEvent->button() == Qt::LeftButton) {
        startHeaderInteraction(pTarget, rootPos);
    } else {
        resetHeaderInteraction(true);
    }

    if (splitterInteractionStarted) {
        forceActiveFocus(Qt::MouseFocusReason);
        updateEmbeddedFocus(pTarget, Qt::MouseFocusReason);
        return;
    }

    if (sendMouseToWidget(pEvent, pTarget)) {
        repaintEmbeddedViews();
    } else {
        m_pPressedWidget.clear();
        m_pGrabbedWidget.clear();
        resetHeaderInteraction(true);
        QQuickPaintedItem::mousePressEvent(pEvent);
    }

    // Grab QML keyboard focus so keyPressEvent/keyReleaseEvent fire on
    // this item, and synthesize FocusIn on the clicked embedded widget.
    forceActiveFocus(Qt::MouseFocusReason);
    updateEmbeddedFocus(pTarget, Qt::MouseFocusReason);

    if (pEvent->button() == Qt::RightButton &&
            !isContextMenuOnMouseRelease()) {
        QWidget* pContextTarget = contextMenuTargetFor(widgetAtRootPos(rootPos));
        if (sendContextMenuToWidget(pEvent, pContextTarget)) {
            repaintEmbeddedViews();
            m_pPressedWidget.clear();
            m_pGrabbedWidget.clear();
        }
    }
}

void QmlLegacyLibraryItem::mouseReleaseEvent(QMouseEvent* pEvent) {
    syncRootWidgetGlobalPosition();
    cancelToolTip();
    const QPoint rootPos = pEvent->position().toPoint();
    QWidget* pTarget = m_pGrabbedWidget
            ? m_pGrabbedWidget.data()
            : eventTargetFor(widgetAtRootPos(rootPos));
    const bool wasSplitterResize = m_pPressedSplitter != nullptr;
    bool accepted = false;
    if (wasSplitterResize) {
        resizeSplitterFromRootPos(rootPos);
        pEvent->accept();
        accepted = true;
    } else {
        accepted = sendMouseToWidget(pEvent, pTarget);
        if (pEvent->button() == Qt::LeftButton) {
            maybeApplyHeaderSortFallback(parentHeaderView(pTarget), rootPos);
        }
    }

    bool contextMenuAccepted = false;
    if (pEvent->button() == Qt::RightButton &&
            isContextMenuOnMouseRelease()) {
        QWidget* pContextTarget = contextMenuTargetFor(widgetAtRootPos(rootPos));
        contextMenuAccepted = sendContextMenuToWidget(pEvent, pContextTarget);
    }

    if (accepted || contextMenuAccepted) {
        repaintEmbeddedViews();
    } else {
        QQuickPaintedItem::mouseReleaseEvent(pEvent);
    }
    m_pPressedWidget.clear();
    m_pGrabbedWidget.clear();
    m_pressedButtons = Qt::NoButton;
    resetHeaderInteraction(true);
    resetSplitterInteraction();
    if (wasSplitterResize) {
        requestRender();
    }
}

void QmlLegacyLibraryItem::mouseMoveEvent(QMouseEvent* pEvent) {
    syncRootWidgetGlobalPosition();
    if (pEvent->buttons() != Qt::NoButton) {
        cancelToolTip();
    }
    const QPoint rootPos = pEvent->position().toPoint();
    QWidget* pTarget = m_pGrabbedWidget
            ? m_pGrabbedWidget.data()
            : eventTargetFor(widgetAtRootPos(rootPos));
    if (m_pPressedSplitter) {
        if (!(pEvent->buttons() & Qt::LeftButton) &&
                !(m_pressedButtons & Qt::LeftButton)) {
            resetSplitterInteraction();
            requestRender();
            pEvent->accept();
            return;
        }
        resizeSplitterFromRootPos(rootPos);
        pEvent->accept();
        requestRenderForCurrentInteraction();
        return;
    }
    if (!shouldForwardHeaderMove(pTarget, rootPos)) {
        pEvent->accept();
        return;
    }
    if (sendMouseToWidget(pEvent, pTarget)) {
        // During active header column moves, kill Qt's native autoscroll
        // (which cannot work correctly in the offscreen bridge) and arm
        // our own bridge-side autoscroll if the cursor is in the margin.
        if (m_headerInteraction == HeaderInteraction::MoveActive &&
                m_pPressedHeader) {
            QHeaderView* pHeader = m_pPressedHeader.data();
            QWidget* pViewport = pHeader->viewport();
            if (pViewport) {
                const QPoint vp = pViewport->mapFrom(
                        m_pRootWidget.get(), rootPos);
                const QRect area = pViewport->rect();
                const int margin = pHeader->autoScrollMargin();
                bool inMargin = false;
                if (pHeader->orientation() == Qt::Horizontal) {
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
        requestRenderForCurrentInteraction();
    } else {
        QQuickPaintedItem::mouseMoveEvent(pEvent);
    }
}

void QmlLegacyLibraryItem::mouseDoubleClickEvent(QMouseEvent* pEvent) {
    syncRootWidgetGlobalPosition();
    cancelToolTip();
    const QPoint rootPos = pEvent->position().toPoint();
    QWidget* pTarget = eventTargetFor(widgetAtRootPos(rootPos));
    sendSyntheticMouseMoveToWidget(pTarget, rootPos, pEvent->globalPosition(), pEvent->modifiers());
    pTarget = eventTargetFor(widgetAtRootPos(rootPos));
    m_pPressedWidget = pTarget;
    m_pGrabbedWidget = pTarget;
    if (sendMouseToWidget(pEvent, pTarget)) {
        repaintEmbeddedViews();
    } else {
        QQuickPaintedItem::mouseDoubleClickEvent(pEvent);
    }
}

void QmlLegacyLibraryItem::mouseUngrabEvent() {
    const QScopedValueRollback<bool> ungrabRollback(m_handlingMouseUngrab, true);
    const bool hadSplitterInteraction = m_pPressedSplitter != nullptr;
    m_pPressedWidget.clear();
    m_pGrabbedWidget.clear();
    m_pressedButtons = Qt::NoButton;
    resetHeaderInteraction(true);
    resetSplitterInteraction();
    if (hadSplitterInteraction) {
        requestRender();
    }
    QQuickPaintedItem::mouseUngrabEvent();
}

void QmlLegacyLibraryItem::wheelEvent(QWheelEvent* pEvent) {
    syncRootWidgetGlobalPosition();
    cancelToolTip();
    if (sendWheelToWidget(pEvent)) {
        requestRender();
    } else {
        QQuickPaintedItem::wheelEvent(pEvent);
    }
}

void QmlLegacyLibraryItem::hoverEnterEvent(QHoverEvent* pEvent) {
    syncRootWidgetGlobalPosition();
    if (m_pPressedSplitter &&
            QGuiApplication::mouseButtons() == Qt::NoButton) {
        resetSplitterInteraction();
        requestRender();
    }
    if (sendHoverToWidget(pEvent)) {
        requestRender();
    } else {
        QQuickPaintedItem::hoverEnterEvent(pEvent);
    }
}

void QmlLegacyLibraryItem::hoverMoveEvent(QHoverEvent* pEvent) {
    syncRootWidgetGlobalPosition();
    if (m_pPressedSplitter &&
            QGuiApplication::mouseButtons() == Qt::NoButton) {
        resetSplitterInteraction();
        requestRender();
    }
    if (sendHoverToWidget(pEvent)) {
        requestRender();
    } else {
        QQuickPaintedItem::hoverMoveEvent(pEvent);
    }
}

void QmlLegacyLibraryItem::hoverLeaveEvent(QHoverEvent* pEvent) {
    syncRootWidgetGlobalPosition();
    cancelToolTip();
    if (m_pPressedSplitter &&
            QGuiApplication::mouseButtons() == Qt::NoButton) {
        resetSplitterInteraction();
        requestRender();
    }
    if (m_pLastHoverWidget) {
        QEvent leaveEvent(QEvent::Leave);
        QApplication::sendEvent(m_pLastHoverWidget, &leaveEvent);
        m_pLastHoverWidget.clear();
    }
    unsetCursor();
    requestRender();
    QQuickPaintedItem::hoverLeaveEvent(pEvent);
}

void QmlLegacyLibraryItem::keyPressEvent(QKeyEvent* pEvent) {
    if (m_pFocusedWidget) {
        QApplication::sendEvent(m_pFocusedWidget, pEvent);
        requestRender();
    } else {
        QQuickPaintedItem::keyPressEvent(pEvent);
    }
}

void QmlLegacyLibraryItem::keyReleaseEvent(QKeyEvent* pEvent) {
    if (m_pFocusedWidget) {
        QApplication::sendEvent(m_pFocusedWidget, pEvent);
        requestRender();
    } else {
        QQuickPaintedItem::keyReleaseEvent(pEvent);
    }
}

void QmlLegacyLibraryItem::updateEmbeddedFocus(
        QWidget* pTarget, Qt::FocusReason reason) {
    // For QComboBox (WSearchLineEdit), the internal QLineEdit is the
    // deepest click target. Walk up to the QComboBox so that
    // QComboBox::focusInEvent fires — it forwards focus to the line
    // edit internally and sets up the completer and clear button.
    QWidget* pFocusTarget = pTarget;
    while (pFocusTarget) {
        if (qobject_cast<QComboBox*>(pFocusTarget)) {
            break;
        }
        if (pFocusTarget->focusPolicy() != Qt::NoFocus) {
            break;
        }
        pFocusTarget = pFocusTarget->parentWidget();
    }

    if (m_pSearchLineEdit) {
        const bool searchFocused = pFocusTarget == m_pSearchLineEdit;
        if (m_pSearchLineEdit->property("qmlBridgeFocused").toBool() != searchFocused) {
            m_pSearchLineEdit->setProperty("qmlBridgeFocused", searchFocused);
            m_pSearchLineEdit->style()->unpolish(m_pSearchLineEdit);
            m_pSearchLineEdit->style()->polish(m_pSearchLineEdit);
            m_pSearchLineEdit->update();
        }
    }

    if (pFocusTarget == m_pFocusedWidget) {
        return;
    }

    if (m_pFocusedWidget) {
        QFocusEvent focusOut(QEvent::FocusOut, reason);
        QApplication::sendEvent(m_pFocusedWidget, &focusOut);
    }

    m_pFocusedWidget = pFocusTarget;

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
        applyInitialSplitterSizes();
        return;
    }

    m_pRootWidget->resize(widgetSize);
    m_pRootWidget->ensurePolished();
    applyInitialSplitterSizes();
}

void QmlLegacyLibraryItem::applyInitialSplitterSizes() {
    if (m_initialSplitterSizesApplied || !m_pRootWidget ||
            !m_pLibrarySplitter || !m_pCoverArtSplitter ||
            m_pRootWidget->width() <= 1 || m_pRootWidget->height() <= 1) {
        return;
    }

    const auto applySizes = [this](QSplitter* pSplitter, const QList<int>& sizes) {
        if (!pSplitter || sizes.size() != pSplitter->count()) {
            return false;
        }

        for (int index = 0; index < pSplitter->count(); ++index) {
            QWidget* pWidget = pSplitter->widget(index);
            const bool coverArtVisible = pWidget != m_pCoverArtBox ||
                    !m_pShowLibraryCoverArt || m_pShowLibraryCoverArt->toBool();
            if (pWidget && sizes.at(index) > 0 && coverArtVisible) {
                pWidget->show();
            }
        }
        pSplitter->setSizes(sizes);
        return true;
    };

    if (!applySizes(m_pCoverArtSplitter, m_initialCoverArtSplitterSizes) ||
            !applySizes(m_pLibrarySplitter, m_initialLibrarySplitterSizes)) {
        return;
    }

    m_initialSplitterSizesApplied = true;
}

void QmlLegacyLibraryItem::createLegacyPreviewDeck() {
    if (!m_pPreviewDeckBox || !m_pLibrary) {
        return;
    }

    auto* pPreviewLayout = new QHBoxLayout(m_pPreviewDeckBox);
    pPreviewLayout->setContentsMargins(0, 0, 0, 0);
    pPreviewLayout->setSpacing(0);

    auto* pPreviewDeck = new WWidgetGroup(m_pPreviewDeckBox);
    pPreviewDeck->setObjectName(QStringLiteral("PreviewDeck"));
    pPreviewDeck->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    pPreviewDeck->setMinimumWidth(100);
    auto* pPreviewDeckLayout = new QHBoxLayout(pPreviewDeck);
    pPreviewDeckLayout->setContentsMargins(0, 0, 0, 0);
    pPreviewDeckLayout->setSpacing(0);

    auto* pLeftPart = new WWidgetGroup(pPreviewDeck);
    pLeftPart->setObjectName(QStringLiteral("PreviewDeckLeftPart"));
    pLeftPart->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    auto* pLeftPartLayout = new QVBoxLayout(pLeftPart);
    pLeftPartLayout->setContentsMargins(0, 0, 0, 0);
    pLeftPartLayout->setSpacing(0);

    auto* pTitleEjectRow = new WWidgetGroup(pLeftPart);
    pTitleEjectRow->setObjectName(QStringLiteral("PreviewTitleEjectRow"));
    pTitleEjectRow->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    auto* pTitleEjectLayout = new QHBoxLayout(pTitleEjectRow);
    pTitleEjectLayout->setContentsMargins(0, 0, 0, 0);
    pTitleEjectLayout->setSpacing(0);

    auto* pTextBox = new WWidgetGroup(pTitleEjectRow);
    pTextBox->setObjectName(QStringLiteral("PreviewDeckTextBoxBox"));
    pTextBox->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Minimum);
    pTextBox->setFixedHeight(20);
    auto* pTextBoxLayout = new QStackedLayout(pTextBox);
    pTextBoxLayout->setContentsMargins(0, 0, 0, 0);
    pTextBoxLayout->setStackingMode(QStackedLayout::StackAll);

    auto* pTitleBpmRow = new WWidgetGroup(pTextBox);
    pTitleBpmRow->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    pTitleBpmRow->setFixedHeight(20);
    auto* pTitleBpmLayout = new QHBoxLayout(pTitleBpmRow);
    pTitleBpmLayout->setContentsMargins(0, 0, 0, 0);
    pTitleBpmLayout->setSpacing(0);

    const QString previewDeckGroup = PlayerManager::groupForPreviewDeck(0);
    UserSettingsPointer pConfig = QmlConfigProxy::get();
    m_pPreviewTitle = new WTrackProperty(
            pTitleBpmRow, pConfig, m_pLibrary, previewDeckGroup, false);
    m_pPreviewTitle->setObjectName(QStringLiteral("PreviewTitle"));
    m_pPreviewTitle->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_pPreviewTitle->setFixedHeight(20);
    pTitleBpmLayout->addWidget(m_pPreviewTitle, 1);

    m_pPreviewBpm = new WNumber(pTitleBpmRow);
    m_pPreviewBpm->setObjectName(QStringLiteral("PreviewBPM"));
    m_pPreviewBpm->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_pPreviewBpm->setFixedSize(45, 20);
    m_pPreviewBpm->addAndSetDisplayConnection(
            std::make_unique<ControlParameterWidgetConnection>(
                    m_pPreviewBpm,
                    ConfigKey(previewDeckGroup, QStringLiteral("visual_bpm")),
                    nullptr,
                    ControlParameterWidgetConnection::DIR_TO_WIDGET,
                    ControlParameterWidgetConnection::EMIT_NEVER),
            WBaseWidget::ConnectionSide::None);
    pTitleBpmLayout->addWidget(m_pPreviewBpm);
    pTextBoxLayout->addWidget(pTitleBpmRow);

    m_pPreviewLabel = new WLabel(pTextBox);
    m_pPreviewLabel->setObjectName(QStringLiteral("PreviewLabel"));
    m_pPreviewLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_pPreviewLabel->setFixedHeight(20);
    pTextBoxLayout->addWidget(m_pPreviewLabel);

    m_pPreviewEjectBox = new WWidgetGroup(pTitleEjectRow);
    m_pPreviewEjectBox->setObjectName(QStringLiteral("PreviewEjectBox"));
    m_pPreviewEjectBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    auto* pEjectLayout = new QHBoxLayout(m_pPreviewEjectBox);
    pEjectLayout->setContentsMargins(0, 0, 0, 0);
    pEjectLayout->setSpacing(0);
    m_pPreviewEjectButton = new WPushButton(m_pPreviewEjectBox);
    m_pPreviewEjectButton->setObjectName(QStringLiteral("EjectButton12"));
    m_pPreviewEjectButton->setFixedSize(21, 18);
    m_pPreviewEjectButton->addAndSetDisplayConnection(
            std::make_unique<ControlParameterWidgetConnection>(
                    m_pPreviewEjectButton,
                    ConfigKey(previewDeckGroup, QStringLiteral("eject")),
                    nullptr,
                    static_cast<ControlParameterWidgetConnection::DirectionOption>(
                            ControlParameterWidgetConnection::DIR_FROM_AND_TO_WIDGET |
                            ControlParameterWidgetConnection::DIR_DEFAULT),
                    ControlParameterWidgetConnection::EMIT_DEFAULT),
            WBaseWidget::ConnectionSide::None);
    pEjectLayout->addWidget(m_pPreviewEjectButton);

    pTitleEjectLayout->addWidget(pTextBox, 1);
    pTitleEjectLayout->addWidget(m_pPreviewEjectBox);
    pLeftPartLayout->addWidget(pTitleEjectRow);

    auto* pPlayOverview = new WWidgetGroup(pLeftPart);
    pPlayOverview->setObjectName(QStringLiteral("PreviewPlayOverview"));
    pPlayOverview->setSizePolicy(
            QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    pPlayOverview->setMinimumHeight(34);
    auto* pPlayOverviewLayout = new QHBoxLayout(pPlayOverview);
    pPlayOverviewLayout->setContentsMargins(0, 0, 0, 0);
    pPlayOverviewLayout->setSpacing(0);

    auto* pPlayBox = new WWidgetGroup(pPlayOverview);
    pPlayBox->setObjectName(QStringLiteral("PreviewPlayBox"));
    pPlayBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    auto* pPlayBoxLayout = new QHBoxLayout(pPlayBox);
    pPlayBoxLayout->setContentsMargins(0, 0, 0, 0);
    pPlayBoxLayout->setSpacing(0);

    m_pPreviewPlayButton = new WPushButton(pPlayBox);
    m_pPreviewPlayButton->setObjectName(QStringLiteral("PlayPreview"));
    m_pPreviewPlayButton->setFixedSize(34, 34);
    m_pPreviewPlayButton->addAndSetDisplayConnection(
            std::make_unique<ControlParameterWidgetConnection>(
                    m_pPreviewPlayButton,
                    ConfigKey(previewDeckGroup, QStringLiteral("play")),
                    nullptr,
                    ControlParameterWidgetConnection::DIR_DEFAULT,
                    ControlParameterWidgetConnection::EMIT_DEFAULT),
            WBaseWidget::ConnectionSide::Left);
    m_pPreviewPlayButton->addConnection(
            std::make_unique<ControlParameterWidgetConnection>(
                    m_pPreviewPlayButton,
                    ConfigKey(previewDeckGroup, QStringLiteral("start")),
                    nullptr,
                    ControlParameterWidgetConnection::DIR_DEFAULT,
                    ControlParameterWidgetConnection::EMIT_DEFAULT),
            WBaseWidget::ConnectionSide::Right);
    m_pPreviewPlayButton->addAndSetDisplayConnection(
            std::make_unique<ControlParameterWidgetConnection>(
                    m_pPreviewPlayButton,
                    ConfigKey(previewDeckGroup, QStringLiteral("play_indicator")),
                    nullptr,
                    ControlParameterWidgetConnection::DIR_TO_WIDGET,
                    ControlParameterWidgetConnection::EMIT_NEVER),
            WBaseWidget::ConnectionSide::None);
    pPlayBoxLayout->addWidget(m_pPreviewPlayButton);

    m_pOverviewBox = new WWidgetGroup(pPlayOverview);
    m_pOverviewBox->setObjectName(QStringLiteral("OverviewBox"));
    m_pOverviewBox->setSizePolicy(
            QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    m_pOverviewBox->setFixedHeight(34);
    m_pOverviewBox->setProperty("highlight", 0);
    auto* pOverviewLayout = new QVBoxLayout(m_pOverviewBox);
    pOverviewLayout->setContentsMargins(0, 0, 0, 0);
    pOverviewLayout->setSpacing(0);

    pPlayOverviewLayout->addWidget(pPlayBox);
    pPlayOverviewLayout->addWidget(m_pOverviewBox, 1);
    pLeftPartLayout->addWidget(pPlayOverview);

    auto* pVuBox = new WWidgetGroup(pPreviewDeck);
    pVuBox->setObjectName(QStringLiteral("PreviewVuMeter"));
    pVuBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    auto* pVuLayout = new QVBoxLayout(pVuBox);
    pVuLayout->setContentsMargins(0, 0, 0, 0);
    pVuLayout->setSpacing(0);

    auto* pVuMeterBox = new WWidgetGroup(pVuBox);
    pVuMeterBox->setObjectName(QStringLiteral("VuMeterBox"));
    pVuMeterBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    auto* pVuMeterBoxLayout = new QVBoxLayout(pVuMeterBox);
    pVuMeterBoxLayout->setContentsMargins(0, 0, 0, 0);
    pVuMeterBoxLayout->setSpacing(0);

    m_pPreviewPeakIndicator = new WStatusLight(pVuMeterBox);
    m_pPreviewPeakIndicator->addAndSetDisplayConnection(
            std::make_unique<ControlParameterWidgetConnection>(
                    m_pPreviewPeakIndicator,
                    ConfigKey(previewDeckGroup, QStringLiteral("peak_indicator")),
                    nullptr,
                    ControlParameterWidgetConnection::DIR_TO_WIDGET,
                    ControlParameterWidgetConnection::EMIT_NEVER),
            WBaseWidget::ConnectionSide::None);
    pVuMeterBoxLayout->addWidget(m_pPreviewPeakIndicator);

    m_pPreviewVuMeter = new WVuMeterLegacy(pVuMeterBox);
    m_pPreviewVuMeter->addAndSetDisplayConnection(
            std::make_unique<ControlParameterWidgetConnection>(
                    m_pPreviewVuMeter,
                    ConfigKey(previewDeckGroup, QStringLiteral("vu_meter")),
                    nullptr,
                    ControlParameterWidgetConnection::DIR_TO_WIDGET,
                    ControlParameterWidgetConnection::EMIT_NEVER),
            WBaseWidget::ConnectionSide::None);
    pVuMeterBoxLayout->addWidget(m_pPreviewVuMeter);
    pVuLayout->addWidget(pVuMeterBox);

    m_pPreviewSlider = new WSliderComposed(pPreviewDeck);
    m_pPreviewSlider->setObjectName(QStringLiteral("PreviewPregain"));
    m_pPreviewSlider->setFixedSize(10, 54);
    m_pPreviewSlider->addAndSetDisplayConnection(
            std::make_unique<ControlParameterWidgetConnection>(
                    m_pPreviewSlider,
                    ConfigKey(previewDeckGroup, QStringLiteral("pregain")),
                    nullptr,
                    static_cast<ControlParameterWidgetConnection::DirectionOption>(
                            ControlParameterWidgetConnection::DIR_FROM_AND_TO_WIDGET |
                            ControlParameterWidgetConnection::DIR_DEFAULT),
                    ControlParameterWidgetConnection::EMIT_DEFAULT),
            WBaseWidget::ConnectionSide::None);

    pPreviewDeckLayout->addWidget(pLeftPart);
    pPreviewDeckLayout->addWidget(pVuBox);
    pPreviewDeckLayout->addWidget(m_pPreviewSlider);

    auto* pRightSpacer = new WWidgetGroup(m_pPreviewDeckBox);
    pRightSpacer->setObjectName(QStringLiteral("PreviewDeckRightSpacer"));
    pRightSpacer->setAttribute(Qt::WA_StyledBackground, true);
    pRightSpacer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    pRightSpacer->setFixedWidth(
            getActiveSchemeStyle().schemeName == QStringLiteral("classic") ? 4 : 3);
    m_pPreviewDeckRightSpacer = pRightSpacer;

    pPreviewLayout->addWidget(pPreviewDeck);
    pPreviewLayout->addWidget(pRightSpacer);

    if (m_pPreviewPlayer) {
        connect(m_pPreviewPlayer,
                &BaseTrackPlayer::newTrackLoaded,
                m_pPreviewTitle,
                &WTrackProperty::slotTrackLoaded);
        connect(m_pPreviewPlayer,
                &BaseTrackPlayer::loadingTrack,
                m_pPreviewTitle,
                &WTrackProperty::slotLoadingTrack);
    }
    if (m_pPlayerManager) {
        connect(m_pPreviewTitle,
                &WTrackProperty::trackDropped,
                m_pPlayerManager,
                &PlayerManager::slotLoadLocationToPlayerMaybePlay);
        connect(m_pPreviewTitle,
                &WTrackProperty::cloneDeck,
                m_pPlayerManager,
                &PlayerManager::slotCloneDeck);
        if (auto* pFactory = WaveformWidgetFactory::instance()) {
            pFactory->addVuMeter(m_pPreviewVuMeter);
        }
    }

    applyLegacyPreviewDeckSkinConfiguration();
    if (m_pPreviewPlayer) {
        const TrackPointer pTrack = m_pPreviewPlayer->getLoadedTrack();
        if (pTrack) {
            m_pPreviewTitle->slotTrackLoaded(pTrack);
        }
    }
}

void QmlLegacyLibraryItem::applyLegacyPreviewDeckSkinConfiguration() {
    if (!m_pPreviewDeckBox) {
        return;
    }

    const QString resourcePath = QmlConfigProxy::get()->getResourcePath();
    const QString skinsRoot = QDir::fromNativeSeparators(
            resourcePath + QStringLiteral("skins/"));
    const QString lateNightSkinPath = resourcePath + QStringLiteral("skins/LateNight");
    QDir::setSearchPaths(QStringLiteral("skins"), {skinsRoot});
    QDir::setSearchPaths(QStringLiteral("skin"), {lateNightSkinPath});

    const SchemeStyle scheme = getActiveSchemeStyle();
    SkinContext context(QmlConfigProxy::get(), lateNightSkinPath + QStringLiteral("/skin.xml"));
    context.setSkinBasePath(lateNightSkinPath);
    setLateNightPreviewVariables(&context, scheme);
    context.setVariable(QStringLiteral("Group"), PlayerManager::groupForPreviewDeck(0));
    context.setVariable(QStringLiteral("SignalColor"), scheme.signalColor);
    context.setVariable(QStringLiteral("BgColor"),
            scheme.schemeName == QStringLiteral("classic")
                    ? QStringLiteral("rgba(15, 15, 15, 20)")
                    : QStringLiteral("#19191a"));
    context.setVariable(QStringLiteral("OverviewFontSize"), QStringLiteral("9"));

    if (m_pPreviewTitle) {
        QDomDocument document(QStringLiteral("QmlLegacyLibraryItemPreviewTitleSetup"));
        if (setDomContent(&document,
                    QStringLiteral(
                            "<TrackProperty><Property>info</"
                            "Property><Elide>right</Elide></TrackProperty>"))) {
            m_pPreviewTitle->setup(document.documentElement(), context);
            m_pPreviewTitle->Init();
        }
    }
    if (m_pPreviewBpm) {
        QDomDocument document(QStringLiteral("QmlLegacyLibraryItemPreviewBpmSetup"));
        if (setDomContent(&document,
                    QStringLiteral("<Number><Alignment>right</Alignment></Number>"))) {
            m_pPreviewBpm->setup(document.documentElement(), context);
            m_pPreviewBpm->Init();
        }
    }
    if (m_pPreviewLabel) {
        QDomDocument document(QStringLiteral("QmlLegacyLibraryItemPreviewLabelSetup"));
        if (setDomContent(&document,
                    QStringLiteral(
                            "<Label><Text>Preview</Text><Alignment>left</"
                            "Alignment></Label>"))) {
            m_pPreviewLabel->setup(document.documentElement(), context);
            m_pPreviewLabel->Init();
        }
    }
    if (m_pPreviewEjectButton) {
        QDomDocument document(QStringLiteral("QmlLegacyLibraryItemPreviewEjectSetup"));
        if (setDomContent(&document,
                    QStringLiteral("<PushButton><NumberStates>1</NumberStates></PushButton>"))) {
            m_pPreviewEjectButton->setup(document.documentElement(), context);
            m_pPreviewEjectButton->Init();
        }
    }
    if (m_pPreviewPlayButton) {
        QDomDocument document(QStringLiteral("QmlLegacyLibraryItemPreviewPlaySetup"));
        const QString xml = QStringLiteral(
                "<PushButton><NumberStates>2</NumberStates>"
                "<RightClickIsPushButton>true</RightClickIsPushButton>"
                "<State><Number>0</Number>"
                "<Unpressed "
                "scalemode=\"STRETCH\">skins:LateNight/%1/buttons/"
                "btn_embedded_square_big.svg</Unpressed>"
                "<Pressed "
                "scalemode=\"STRETCH\">skins:LateNight/%1/buttons/"
                "btn_embedded_square_big_active.svg</Pressed>"
                "</State>"
                "<State><Number>1</Number>"
                "<Unpressed "
                "scalemode=\"STRETCH\">skins:LateNight/%1/buttons/"
                "btn_embedded_square_big_active.svg</Unpressed>"
                "<Pressed "
                "scalemode=\"STRETCH\">skins:LateNight/%1/buttons/"
                "btn_embedded_square_big_active.svg</Pressed>"
                "</State></PushButton>")
                                    .arg(scheme.schemeName);
        if (setDomContent(&document, xml)) {
            m_pPreviewPlayButton->setup(document.documentElement(), context);
            m_pPreviewPlayButton->Init();
        }
    }
    if (m_pPreviewPeakIndicator) {
        QDomDocument document(QStringLiteral("QmlLegacyLibraryItemPreviewPeakSetup"));
        const QString xml = QStringLiteral(
                "<StatusLight><PathBack>skins:LateNight/%1/style/"
                "vu_preview_clipping_bg_.png</PathBack>"
                "<PathStatusLight>skins:LateNight/%1/style/"
                "vu_preview_clipping_active.png</PathStatusLight></"
                "StatusLight>")
                                    .arg(scheme.schemeName);
        if (setDomContent(&document, xml)) {
            m_pPreviewPeakIndicator->setup(document.documentElement(), context);
            m_pPreviewPeakIndicator->Init();
        }
    }
    if (m_pPreviewVuMeter) {
        QDomDocument document(QStringLiteral("QmlLegacyLibraryItemPreviewVuSetup"));
        const QString xml = QStringLiteral(
                "<VuMeter><PathBack>skins:LateNight/%1/style/vu_preview_level_bg_.png</PathBack>"
                "<PathVu>skins:LateNight/%1/style/vu_preview_level_active.png</PathVu>"
                "<Horizontal>false</Horizontal><PeakHoldSize>4</PeakHoldSize>"
                "<PeakHoldTime>500</PeakHoldTime><PeakFallTime>10</PeakFallTime>"
                "<PeakFallStep>2</PeakFallStep></VuMeter>")
                                    .arg(scheme.schemeName);
        if (setDomContent(&document, xml)) {
            m_pPreviewVuMeter->setup(document.documentElement(), context);
            m_pPreviewVuMeter->Init();
        }
    }
    if (m_pPreviewSlider) {
        QDomDocument document(QStringLiteral("QmlLegacyLibraryItemPreviewSliderSetup"));
        const QString xml = QStringLiteral(
                "<Slider><Handle "
                "scalemode=\"STRETCH_ASPECT\">skins:LateNight/%1/sliders/"
                "knob_volume_previewdeck.svg</Handle>"
                "<Slider>skins:LateNight/%1/sliders/"
                "slider_volume_previewdeck.svg</Slider>"
                "<Horizontal>false</Horizontal></Slider>")
                                    .arg(scheme.schemeName);
        if (setDomContent(&document, xml)) {
            m_pPreviewSlider->setup(document.documentElement(), context);
            m_pPreviewSlider->Init();
        }
    }
    if (m_pPreviewDeckRightSpacer) {
        m_pPreviewDeckRightSpacer->setAttribute(Qt::WA_StyledBackground, true);
        m_pPreviewDeckRightSpacer->setFixedWidth(
                scheme.schemeName == QStringLiteral("classic") ? 4 : 3);
    }

    recreateLegacyPreviewOverview();

    Tooltips tooltips;
    if (m_pPreviewTitle) {
        m_pPreviewTitle->setBaseTooltip(tooltips.tooltipForId(QStringLiteral("text")));
    }
    if (m_pPreviewBpm) {
        m_pPreviewBpm->setBaseTooltip(tooltips.tooltipForId(QStringLiteral("visual_bpm")));
    }
    if (m_pPreviewEjectButton) {
        m_pPreviewEjectButton->setBaseTooltip(tooltips.tooltipForId(QStringLiteral("eject")));
    }
    if (m_pPreviewPlayButton) {
        m_pPreviewPlayButton->setBaseTooltip(
                tooltips.tooltipForId(QStringLiteral("cue_gotoandplay_cue_default")));
    }
    if (m_pPreviewOverview) {
        m_pPreviewOverview->setBaseTooltip(
                tooltips.tooltipForId(QStringLiteral("waveform_overview")));
    }
    if (m_pPreviewVuMeter) {
        m_pPreviewVuMeter->setBaseTooltip(
                tooltips.tooltipForId(QStringLiteral("preview_VuMeter")));
    }
    if (m_pPreviewPeakIndicator) {
        m_pPreviewPeakIndicator->setBaseTooltip(
                tooltips.tooltipForId(QStringLiteral("preview_peak_indicator")));
    }
    if (m_pPreviewSlider) {
        m_pPreviewSlider->setBaseTooltip(tooltips.tooltipForId(QStringLiteral("pregain")));
    }

    updatePreviewDeckTrackLoaded(m_pPreviewDeckTrackLoaded
                    ? m_pPreviewDeckTrackLoaded->get()
                    : 0.0);
}

void QmlLegacyLibraryItem::recreateLegacyPreviewOverview() {
    if (!m_pOverviewBox || !m_pPlayerManager || !m_pPreviewPlayer || !m_pLibrary) {
        return;
    }

    auto* pLayout = qobject_cast<QVBoxLayout*>(m_pOverviewBox->layout());
    if (!pLayout) {
        return;
    }
    if (m_pPreviewOverview) {
        pLayout->removeWidget(m_pPreviewOverview);
        delete m_pPreviewOverview;
        m_pPreviewOverview = nullptr;
    }

    const QString resourcePath = QmlConfigProxy::get()->getResourcePath();
    const QString overviewPath =
            resourcePath + QStringLiteral("skins/LateNight/decks/overview.xml");
    QFile overviewFile(overviewPath);
    if (!overviewFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }
    QDomDocument document(QStringLiteral("QmlLegacyLibraryItemPreviewOverviewDocument"));
    if (!setDomContent(&document, QString::fromUtf8(overviewFile.readAll()))) {
        return;
    }
    const QDomElement overviewNode = SkinContext::selectElement(
            document.documentElement(), QStringLiteral("Overview"));
    if (overviewNode.isNull()) {
        return;
    }

    const QString previewDeckGroup = PlayerManager::groupForPreviewDeck(0);
    const SchemeStyle scheme = getActiveSchemeStyle();
    SkinContext context(QmlConfigProxy::get(), overviewPath);
    context.setSkinBasePath(resourcePath + QStringLiteral("skins/LateNight"));
    setLateNightPreviewVariables(&context, scheme);
    context.setVariable(QStringLiteral("Group"), previewDeckGroup);
    context.setVariable(QStringLiteral("SignalColor"), scheme.signalColor);
    context.setVariable(QStringLiteral("BgColor"),
            scheme.schemeName == QStringLiteral("classic")
                    ? QStringLiteral("rgba(15, 15, 15, 20)")
                    : QStringLiteral("#19191a"));
    context.setVariable(QStringLiteral("OverviewFontSize"), QStringLiteral("9"));

    auto* pOverview = new WOverview(
            previewDeckGroup, m_pPlayerManager, QmlConfigProxy::get(), m_pOverviewBox);
    pOverview->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    pOverview->setFixedHeight(34);
    pOverview->addAndSetDisplayConnection(
            std::make_unique<ControlParameterWidgetConnection>(
                    pOverview,
                    ConfigKey(previewDeckGroup, QStringLiteral("playposition")),
                    nullptr,
                    static_cast<ControlParameterWidgetConnection::DirectionOption>(
                            ControlParameterWidgetConnection::DIR_FROM_AND_TO_WIDGET |
                            ControlParameterWidgetConnection::DIR_DEFAULT),
                    ControlParameterWidgetConnection::EMIT_DEFAULT),
            WBaseWidget::ConnectionSide::None);
    pOverview->setup(overviewNode, context);
    connect(pOverview,
            &WOverview::trackDropped,
            m_pPlayerManager,
            &PlayerManager::slotLoadLocationToPlayerMaybePlay);
    connect(pOverview,
            &WOverview::cloneDeck,
            m_pPlayerManager,
            &PlayerManager::slotCloneDeck);
    connect(m_pLibrary,
            &Library::onTrackAnalyzerProgress,
            pOverview,
            &WOverview::onTrackAnalyzerProgress);
    connect(m_pPreviewPlayer,
            &BaseTrackPlayer::newTrackLoaded,
            pOverview,
            &WOverview::slotTrackLoaded);
    connect(m_pPreviewPlayer,
            &BaseTrackPlayer::loadingTrack,
            pOverview,
            &WOverview::slotLoadingTrack);
    pLayout->addWidget(pOverview);
    m_pPreviewOverview = pOverview;
    pOverview->initWithTrack(m_pPreviewPlayer->getLoadedTrack());
}

void QmlLegacyLibraryItem::updatePreviewDeckTrackLoaded(double value) {
    const bool loaded = value > 0.0;
    if (m_pPreviewBpm) {
        m_pPreviewBpm->setVisible(loaded);
    }
    if (m_pPreviewLabel) {
        m_pPreviewLabel->setVisible(!loaded);
    }
    if (m_pPreviewEjectBox) {
        m_pPreviewEjectBox->setVisible(loaded);
    }
    if (m_pOverviewBox) {
        m_pOverviewBox->setProperty("highlight", loaded ? 1 : 0);
        if (QStyle* pStyle = m_pOverviewBox->style()) {
            pStyle->unpolish(m_pOverviewBox);
            pStyle->polish(m_pOverviewBox);
        }
        m_pOverviewBox->update();
    }
}

void QmlLegacyLibraryItem::applyLegacyCoverArtSkinConfiguration() {
    if (!m_pCoverArt) {
        return;
    }

    const QString resourcePath = QmlConfigProxy::get()->getResourcePath();
    const QString skinsRoot = QDir::fromNativeSeparators(
            resourcePath + QStringLiteral("skins/"));
    const QString lateNightSkinPath = resourcePath + QStringLiteral("skins/LateNight");
    QDir::setSearchPaths(QStringLiteral("skins"), {skinsRoot});
    QDir::setSearchPaths(QStringLiteral("skin"), {lateNightSkinPath});

    SkinContext context(QmlConfigProxy::get(), lateNightSkinPath + QStringLiteral("/skin.xml"));
    context.setSkinBasePath(lateNightSkinPath);

    const SchemeStyle scheme = getActiveSchemeStyle();
    QDomDocument document(QStringLiteral("QmlLegacyLibraryItemCoverArtSetup"));
    const QString coverArtXml = QStringLiteral(
            "<CoverArt><DefaultCover>skins:LateNight/%1/style/"
            "cover_default.svg</DefaultCover></CoverArt>")
                                        .arg(scheme.schemeName);
    if (!setDomContent(&document, coverArtXml)) {
        return;
    }
    m_pCoverArt->setup(document.documentElement(), context);
}

void QmlLegacyLibraryItem::syncLibraryCoverArtFromSelection() {
    if (!m_pLibraryWidget || !m_pCoverArt) {
        return;
    }

    WTrackTableView* pTracksView = m_pLibraryWidget->getCurrentTrackTableView();
    if (!pTracksView) {
        m_pCoverArt->slotLoadTrack(TrackPointer());
        return;
    }

    QItemSelectionModel* pSelectionModel = pTracksView->selectionModel();
    if (!pSelectionModel) {
        m_pCoverArt->slotLoadTrack(TrackPointer());
        return;
    }

    const QModelIndexList selectedRows = pSelectionModel->selectedRows();
    if (selectedRows.size() != 1 || !selectedRows.first().isValid()) {
        m_pCoverArt->slotLoadTrack(TrackPointer());
        return;
    }

    auto* pTrackModel = dynamic_cast<TrackModel*>(pTracksView->model());
    if (!pTrackModel) {
        m_pCoverArt->slotLoadTrack(TrackPointer());
        return;
    }

    m_pCoverArt->slotLoadTrack(pTrackModel->getTrack(selectedRows.first()));
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
    const QString skinsRoot = QDir::fromNativeSeparators(
            resourcePath + QStringLiteral("skins/"));
    QDir::setSearchPaths(QStringLiteral("skins"), {skinsRoot});
    QDir::setSearchPaths(QStringLiteral("skin"), {lateNightSkinPath});

    SkinContext context(QmlConfigProxy::get(), lateNightSkinPath + QStringLiteral("/skin.xml"));
    context.setSkinBasePath(lateNightSkinPath);

    const SchemeStyle scheme = getActiveSchemeStyle();

    QDomDocument document(QStringLiteral("QmlLegacyLibraryItemLibrarySetup"));
    const QString libraryXml = QStringLiteral(
            "<Library>"
            "<ShowButtonText>false</ShowButtonText>"
            "<TrackTableBackgroundColorOpacity>0.175</TrackTableBackgroundColorOpacity>"
            "<SignalColor>%1</SignalColor>"
            "</Library>")
                                       .arg(scheme.signalColor);
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

    if (m_pLibraryExpandButton) {
        QDomDocument buttonDocument(QStringLiteral("QmlLegacyLibraryItemLibraryExpandSetup"));
        const QString buttonXml = QStringLiteral(
                "<PushButton>"
                "<NumberStates>2</NumberStates>"
                "<RightClickIsPushButton>false</RightClickIsPushButton>"
                "<State><Number>0</Number>"
                "<Unpressed scalemode=\"STRETCH\">skins:LateNight/%1/buttons/btn__.svg</Unpressed>"
                "<Pressed scalemode=\"STRETCH\">skins:LateNight/%1/buttons/btn__.svg</Pressed>"
                "</State>"
                "<State><Number>1</Number>"
                "<Unpressed scalemode=\"STRETCH\">skins:LateNight/%1/buttons/btn__.svg</Unpressed>"
                "<Pressed scalemode=\"STRETCH\">skins:LateNight/%1/buttons/btn__.svg</Pressed>"
                "</State>"
                "</PushButton>")
                                          .arg(scheme.schemeName);
        if (setDomContent(&buttonDocument, buttonXml)) {
            m_pLibraryExpandButton->setup(buttonDocument.documentElement(), context);
            m_pLibraryExpandButton->Init();
            Tooltips tooltips;
            m_pLibraryExpandButton->setBaseTooltip(
                    tooltips.tooltipForId(QStringLiteral("maximize_library")));
        }
    }
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
    const SchemeStyle scheme = getActiveSchemeStyle();
    m_legacyLibraryBackgroundColor = legacyLibraryBackgroundColor(scheme);
    QPalette rootPalette = m_pRootWidget->palette();
    rootPalette.setColor(QPalette::Window, m_legacyLibraryBackgroundColor);
    m_pRootWidget->setPalette(rootPalette);
    const QString commonStyleFilePath =
            lateNightSkinRoot + QStringLiteral("/style.qss");
    const QString schemeStyleFilePath =
            lateNightSkinRoot + QStringLiteral("/") + scheme.qssName;

    QString style;
    QFile commonStyleFile(commonStyleFilePath);
    if (commonStyleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        style = QString::fromUtf8(commonStyleFile.readAll());
    }

    QFile schemeStyleFile(schemeStyleFilePath);
    if (!schemeStyleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "QmlLegacyLibraryItem: could not open" << schemeStyleFilePath
                   << "- library will have no custom styling";
        return;
    }
    style.append(QStringLiteral("\n"));
    style.append(QString::fromUtf8(schemeStyleFile.readAll()));

    // Resolve the "skins:" URL alias used throughout the QSS file.
    // LegacySkinParser does the same replacement in processStyleNodes().
    style.replace(QStringLiteral("url(skins:"),
            QStringLiteral("url(") + skinsRoot);
    style.replace(QStringLiteral("url(\"skins:"),
            QStringLiteral("url(\"") + skinsRoot);
    style.replace(QStringLiteral("url('skins:"),
            QStringLiteral("url('") + skinsRoot);
    style.replace(QStringLiteral("url(skin:"),
            QStringLiteral("url(") + lateNightSkinRoot);
    style.replace(QStringLiteral("url(\"skin:"),
            QStringLiteral("url(\"") + lateNightSkinRoot);
    style.replace(QStringLiteral("url('skin:"),
            QStringLiteral("url('") + lateNightSkinRoot);

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
    const QString splitterHandlePath = lateNightSkinRoot +
            QStringLiteral("/") + scheme.schemeName + QStringLiteral("/style/");
    const QString pressedExtension = scheme.schemeName == QStringLiteral("classic")
            ? QStringLiteral(".png")
            : QStringLiteral(".svg");
    style.append(QStringLiteral(
            "\n#LibrarySplitter[bridgePressed=\"true\"]::handle {"
            "\n  image: url(%1splitter_handle_vertical_pressed%2);"
            "\n}"
            "\n#SidebarCoverSplitter[bridgePressed=\"true\"]::handle {"
            "\n  image: url(%1splitter_handle_horizontal_pressed%2);"
            "\n}")
                    .arg(splitterHandlePath, pressedExtension));

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

    if (ControlObject::exists(overviewTypeCfgKey)) {
        return;
    }

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

void QmlLegacyLibraryItem::requestRenderForCurrentInteraction() {
    if (!m_pPressedSplitter) {
        requestRender();
        return;
    }

    if (!m_lastResizeInteractionRender.isValid() ||
            m_lastResizeInteractionRender.elapsed() >= kInteractionResizeRenderThrottleMillis) {
        m_lastResizeInteractionRender.restart();
        requestRender();
        return;
    }

    if (m_resizeInteractionRenderPending) {
        return;
    }

    m_resizeInteractionRenderPending = true;
    const int remainingMillis = kInteractionResizeRenderThrottleMillis -
            static_cast<int>(m_lastResizeInteractionRender.elapsed());
    QTimer::singleShot(qMax(0, remainingMillis), this, [this]() {
        m_resizeInteractionRenderPending = false;
        if (!m_pPressedSplitter) {
            return;
        }
        m_lastResizeInteractionRender.restart();
        requestRender();
    });
}

void QmlLegacyLibraryItem::updatePolish() {
    if (!m_isDirty) {
        return;
    }
    m_isDirty = false;
    renderOffscreen();
    update();
}

bool QmlLegacyLibraryItem::eventFilter(QObject* pWatched, QEvent* pEvent) {
    if (m_isRendering) {
        return QQuickPaintedItem::eventFilter(pWatched, pEvent);
    }

    if (pEvent->type() == QEvent::Hide && m_pPressedSplitter &&
            (pWatched == m_pRootWidget.get() ||
                    pWatched == m_pPressedSplitter.data() ||
                    pWatched == m_pPressedSplitterHandle.data())) {
        resetSplitterInteraction();
        requestRender();
    }

    switch (pEvent->type()) {
    case QEvent::Resize:
    case QEvent::Move:
    case QEvent::Show:
    case QEvent::Hide:
        emit previewDeckGeometryChanged();
        requestRenderForCurrentInteraction();
        break;
    case QEvent::StyleChange:
    case QEvent::PaletteChange:
    case QEvent::FontChange:
    case QEvent::EnabledChange:
    case QEvent::DynamicPropertyChange:
    case QEvent::UpdateRequest:
        requestRenderForCurrentInteraction();
        break;
    default:
        break;
    }
    return QQuickPaintedItem::eventFilter(pWatched, pEvent);
}

constexpr const char* kEventFilterInstalledProperty =
        "mixxxQmlLegacyEventFilterInstalled";

void QmlLegacyLibraryItem::installEmbeddedWidgetEventFilters() {
    if (!m_pRootWidget) {
        return;
    }

    QList<QWidget*> widgets = m_pRootWidget->findChildren<QWidget*>();
    widgets.prepend(m_pRootWidget.get());
    for (QWidget* pWidget : std::as_const(widgets)) {
        if (pWidget->property(kEventFilterInstalledProperty).toBool()) {
            continue;
        }
        pWidget->setProperty(kEventFilterInstalledProperty, true);
        pWidget->installEventFilter(this);

        // Also install on item view viewports explicitly.
        if (auto* pView = qobject_cast<QAbstractItemView*>(pWidget)) {
            if (QWidget* pViewport = pView->viewport()) {
                if (!pViewport->property(kEventFilterInstalledProperty).toBool()) {
                    pViewport->setProperty(kEventFilterInstalledProperty, true);
                    pViewport->installEventFilter(this);
                }
            }
        }
    }
}

void QmlLegacyLibraryItem::syncEmbeddedTableGeometry(QAbstractItemView* pView) {
    if (!pView) {
        return;
    }

    auto* pTableView = qobject_cast<QTableView*>(pView);
    if (!pTableView) {
        return;
    }

    // Force the table to re-evaluate column positions and scroll extent.
    // doItemsLayout() is the lightest call that invalidates the cached
    // section geometry that QTableView uses for scroll calculations.
    pTableView->doItemsLayout();

    QMetaObject::invokeMethod(pTableView, "updateEditorGeometries");

    if (pTableView->viewport()) {
        pTableView->viewport()->update();
    }
    pTableView->update();

    QHeaderView* pHeader = pTableView->horizontalHeader();
    if (pHeader) {
        if (pHeader->viewport()) {
            pHeader->viewport()->update();
        }
        pHeader->update();
    }

    requestRender();
}

void QmlLegacyLibraryItem::connectEmbeddedWidgetUpdateSignals() {
    if (!m_pRootWidget) {
        return;
    }

    const auto views = m_pRootWidget->findChildren<QAbstractItemView*>();
    for (QAbstractItemView* pView : views) {
        QAbstractItemModel* pModel = pView->model();
        if (pModel) {
            connect(pModel,
                    &QAbstractItemModel::dataChanged,
                    this,
                    &QmlLegacyLibraryItem::requestRender,
                    Qt::UniqueConnection);
            connect(pModel,
                    &QAbstractItemModel::rowsInserted,
                    this,
                    &QmlLegacyLibraryItem::requestRender,
                    Qt::UniqueConnection);
            connect(pModel,
                    &QAbstractItemModel::rowsRemoved,
                    this,
                    &QmlLegacyLibraryItem::requestRender,
                    Qt::UniqueConnection);
            connect(pModel,
                    &QAbstractItemModel::modelReset,
                    this,
                    &QmlLegacyLibraryItem::requestRender,
                    Qt::UniqueConnection);
            connect(pModel,
                    &QAbstractItemModel::layoutChanged,
                    this,
                    &QmlLegacyLibraryItem::requestRender,
                    Qt::UniqueConnection);
            connect(pModel,
                    &QAbstractItemModel::headerDataChanged,
                    this,
                    &QmlLegacyLibraryItem::requestRender,
                    Qt::UniqueConnection);
        }

        QItemSelectionModel* pSelectionModel = pView->selectionModel();
        if (pSelectionModel) {
            connect(pSelectionModel,
                    &QItemSelectionModel::selectionChanged,
                    this,
                    &QmlLegacyLibraryItem::requestRender,
                    Qt::UniqueConnection);
            connect(pSelectionModel,
                    &QItemSelectionModel::currentChanged,
                    this,
                    &QmlLegacyLibraryItem::requestRender,
                    Qt::UniqueConnection);
        }

        auto* pTableView = qobject_cast<QTableView*>(pView);
        if (pTableView) {
            // Connect scrollbars to update editor geometries to prevent drift
            // in offscreen rendering.
            if (!pTableView->property(kScrollEditorSyncConnectedProperty).toBool()) {
                pTableView->setProperty(kScrollEditorSyncConnectedProperty, true);
                QScrollBar* pHScrollBar = pTableView->horizontalScrollBar();
                if (pHScrollBar) {
                    connect(
                            pHScrollBar,
                            &QScrollBar::valueChanged,
                            this,
                            [pTableView]() {
                                QMetaObject::invokeMethod(pTableView, "updateEditorGeometries");
                            });
                }
                QScrollBar* pVScrollBar = pTableView->verticalScrollBar();
                if (pVScrollBar) {
                    connect(
                            pVScrollBar,
                            &QScrollBar::valueChanged,
                            this,
                            [pTableView]() {
                                QMetaObject::invokeMethod(pTableView, "updateEditorGeometries");
                            });
                }
            }

            // Run an initial sync to cover restored header states from
            // startup, not only live column moves.
            syncEmbeddedTableGeometry(pView);
        }
    }

    const auto scrollBars = m_pRootWidget->findChildren<QScrollBar*>();
    for (QScrollBar* pScrollBar : scrollBars) {
        connect(pScrollBar,
                &QScrollBar::valueChanged,
                this,
                &QmlLegacyLibraryItem::requestRender,
                Qt::UniqueConnection);
        connect(pScrollBar,
                &QScrollBar::rangeChanged,
                this,
                &QmlLegacyLibraryItem::requestRender,
                Qt::UniqueConnection);
        connect(pScrollBar,
                &QScrollBar::sliderMoved,
                this,
                &QmlLegacyLibraryItem::requestRender,
                Qt::UniqueConnection);
    }
}

} // namespace qml
} // namespace mixxx
