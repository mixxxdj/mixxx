#include "library/librarycontrol.h"

#include <QApplication>
#include <QItemSelectionModel>
#include <QModelIndex>
#include <QModelIndexList>
#include <QtDebug>

#include "control/controlobject.h"
#include "control/controlpushbutton.h"
#include "mixer/playermanager.h"
#include "widget/wlibrary.h"
#include "widget/wlibrarysidebar.h"
#include "widget/wtracktableview.h"
#include "library/library.h"
#include "library/libraryview.h"


LoadToGroupController::LoadToGroupController(QObject* pParent, const QString& group)
        : QObject(pParent),
          m_group(group) {
    m_pLoadControl = std::make_unique<ControlPushButton>(ConfigKey(group, "LoadSelectedTrack"));
    connect(m_pLoadControl.get(), SIGNAL(valueChanged(double)),
            this, SLOT(slotLoadToGroup(double)));

    m_pLoadAndPlayControl = std::make_unique<ControlPushButton>(ConfigKey(group, "LoadSelectedTrackAndPlay"));
    connect(m_pLoadAndPlayControl.get(), SIGNAL(valueChanged(double)),
            this, SLOT(slotLoadToGroupAndPlay(double)));

    connect(this, SIGNAL(loadToGroup(QString, bool)),
            pParent, SLOT(slotLoadSelectedTrackToGroup(QString, bool)));
}

LoadToGroupController::~LoadToGroupController() = default;

void LoadToGroupController::slotLoadToGroup(double v) {
    if (v > 0) {
        emit(loadToGroup(m_group, false));
    }
}

void LoadToGroupController::slotLoadToGroupAndPlay(double v) {
    if (v > 0) {
        emit(loadToGroup(m_group, true));
    }
}

LibraryControl::LibraryControl(Library* pLibrary)
        : QObject(pLibrary),
          m_pLibrary(pLibrary),
          m_pLibraryWidget(nullptr),
          m_pSidebarWidget(nullptr),
          m_numDecks("[Master]", "num_decks", this),
          m_numSamplers("[Master]", "num_samplers", this),
          m_numPreviewDecks("[Master]", "num_preview_decks", this) {

    slotNumDecksChanged(m_numDecks.get());
    slotNumSamplersChanged(m_numSamplers.get());
    slotNumPreviewDecksChanged(m_numPreviewDecks.get());
    m_numDecks.connectValueChanged(this, &LibraryControl::slotNumDecksChanged);
    m_numSamplers.connectValueChanged(this, &LibraryControl::slotNumSamplersChanged);
    m_numPreviewDecks.connectValueChanged(this, &LibraryControl::slotNumPreviewDecksChanged);

    // Controls to navigate vertically within currently focused widget (up/down buttons)
    m_pMoveUp = std::make_unique<ControlPushButton>(ConfigKey("[Library]", "MoveUp"));
    m_pMoveDown = std::make_unique<ControlPushButton>(ConfigKey("[Library]", "MoveDown"));
    m_pMoveVertical = std::make_unique<ControlEncoder>(ConfigKey("[Library]", "MoveVertical"), false);
    connect(m_pMoveUp.get(), SIGNAL(valueChanged(double)),this, SLOT(slotMoveUp(double)));
    connect(m_pMoveDown.get(), SIGNAL(valueChanged(double)),this, SLOT(slotMoveDown(double)));
    connect(m_pMoveVertical.get(), SIGNAL(valueChanged(double)),this, SLOT(slotMoveVertical(double)));

    // Controls to navigate vertically within currently focused widget (up/down buttons)
    m_pScrollUp = std::make_unique<ControlPushButton>(ConfigKey("[Library]", "ScrollUp"));
    m_pScrollDown = std::make_unique<ControlPushButton>(ConfigKey("[Library]", "ScrollDown"));
    m_pScrollVertical = std::make_unique<ControlEncoder>(ConfigKey("[Library]", "ScrollVertical"), false);
    connect(m_pScrollUp.get(), SIGNAL(valueChanged(double)),this, SLOT(slotScrollUp(double)));
    connect(m_pScrollDown.get(), SIGNAL(valueChanged(double)),this, SLOT(slotScrollDown(double)));
    connect(m_pScrollVertical.get(), SIGNAL(valueChanged(double)),this, SLOT(slotScrollVertical(double)));

    // Controls to navigate horizontally within currently selected item (left/right buttons)
    m_pMoveLeft = std::make_unique<ControlPushButton>(ConfigKey("[Library]", "MoveLeft"));
    m_pMoveRight = std::make_unique<ControlPushButton>(ConfigKey("[Library]", "MoveRight"));
    m_pMoveHorizontal = std::make_unique<ControlEncoder>(ConfigKey("[Library]", "MoveHorizontal"), false);
    connect(m_pMoveLeft.get(), SIGNAL(valueChanged(double)),this, SLOT(slotMoveLeft(double)));
    connect(m_pMoveRight.get(), SIGNAL(valueChanged(double)),this, SLOT(slotMoveRight(double)));
    connect(m_pMoveHorizontal.get(), SIGNAL(valueChanged(double)),this, SLOT(slotMoveHorizontal(double)));

    // Control to navigate between widgets (tab/shit+tab button)
    m_pMoveFocusForward = std::make_unique<ControlPushButton>(ConfigKey("[Library]", "MoveFocusForward"));
    m_pMoveFocusBackward = std::make_unique<ControlPushButton>(ConfigKey("[Library]", "MoveFocusBackward"));
    m_pMoveFocus = std::make_unique<ControlEncoder>(ConfigKey("[Library]", "MoveFocus"), false);
    connect(m_pMoveFocusForward.get(), SIGNAL(valueChanged(double)),this, SLOT(slotMoveFocusForward(double)));
    connect(m_pMoveFocusBackward.get(), SIGNAL(valueChanged(double)),this, SLOT(slotMoveFocusBackward(double)));
    connect(m_pMoveFocus.get(), SIGNAL(valueChanged(double)),this, SLOT(slotMoveFocus(double)));

    // Control to "goto" the currently selected item in focused widget (context dependent)
    m_pGoToItem = std::make_unique<ControlPushButton>(ConfigKey("[Library]", "GoToItem"));
    connect(m_pGoToItem.get(), SIGNAL(valueChanged(double)), this, SLOT(slotGoToItem(double)));

    // Auto DJ controls
    m_pAutoDjAddTop = std::make_unique<ControlPushButton>(ConfigKey("[Library]","AutoDjAddTop"));
    connect(m_pAutoDjAddTop.get(), SIGNAL(valueChanged(double)),
            this, SLOT(slotAutoDjAddTop(double)));

    m_pAutoDjAddBottom = std::make_unique<ControlPushButton>(ConfigKey("[Library]","AutoDjAddBottom"));
    connect(m_pAutoDjAddBottom.get(), SIGNAL(valueChanged(double)),
            this, SLOT(slotAutoDjAddBottom(double)));

    // Sort controls
    m_pSortColumn = std::make_unique<ControlEncoder>(ConfigKey("[Library]", "sort_column"));
    m_pSortOrder = std::make_unique<ControlPushButton>(ConfigKey("[Library]", "sort_order"));
    m_pSortOrder->setButtonMode(ControlPushButton::TOGGLE);
    m_pSortColumnToggle = std::make_unique<ControlEncoder>(ConfigKey("[Library]", "sort_column_toggle"), false);
    connect(m_pSortColumn.get(), SIGNAL(valueChanged(double)),
            this, SLOT(slotSortColumn(double)));
    connect(m_pSortColumnToggle.get(), SIGNAL(valueChanged(double)),
            this, SLOT(slotSortColumnToggle(double)));

    // Font sizes
    m_pFontSizeKnob = std::make_unique<ControlObject>(
            ConfigKey("[Library]", "font_size_knob"), false);
    connect(m_pFontSizeKnob.get(), SIGNAL(valueChanged(double)),
            this, SLOT(slotFontSize(double)));

    m_pFontSizeDecrement = std::make_unique<ControlPushButton>(
            ConfigKey("[Library]", "font_size_decrement"));
    connect(m_pFontSizeDecrement.get(), SIGNAL(valueChanged(double)),
            this, SLOT(slotDecrementFontSize(double)));

    m_pFontSizeIncrement = std::make_unique<ControlPushButton>(
            ConfigKey("[Library]", "font_size_increment"));
    connect(m_pFontSizeIncrement.get(), SIGNAL(valueChanged(double)),
            this, SLOT(slotIncrementFontSize(double)));


    /// Deprecated controls
    m_pSelectNextTrack = std::make_unique<ControlPushButton>(ConfigKey("[Playlist]", "SelectNextTrack"));
    connect(m_pSelectNextTrack.get(), SIGNAL(valueChanged(double)),
            this, SLOT(slotSelectNextTrack(double)));


    m_pSelectPrevTrack = std::make_unique<ControlPushButton>(ConfigKey("[Playlist]", "SelectPrevTrack"));
    connect(m_pSelectPrevTrack.get(), SIGNAL(valueChanged(double)),
            this, SLOT(slotSelectPrevTrack(double)));

    // Ignoring no-ops is important since this is for +/- tickers.
    m_pSelectTrack = std::make_unique<ControlObject>(ConfigKey("[Playlist]","SelectTrackKnob"), false);
    connect(m_pSelectTrack.get(), SIGNAL(valueChanged(double)),
            this, SLOT(slotSelectTrack(double)));


    m_pSelectNextSidebarItem = std::make_unique<ControlPushButton>(ConfigKey("[Playlist]", "SelectNextPlaylist"));
    connect(m_pSelectNextSidebarItem.get(), SIGNAL(valueChanged(double)),
            this, SLOT(slotSelectNextSidebarItem(double)));

    m_pSelectPrevSidebarItem = std::make_unique<ControlPushButton>(ConfigKey("[Playlist]", "SelectPrevPlaylist"));
    connect(m_pSelectPrevSidebarItem.get(), SIGNAL(valueChanged(double)),
            this, SLOT(slotSelectPrevSidebarItem(double)));

    // Ignoring no-ops is important since this is for +/- tickers.
    m_pSelectSidebarItem = std::make_unique<ControlObject>(ConfigKey("[Playlist]", "SelectPlaylist"), false);
    connect(m_pSelectSidebarItem.get(), SIGNAL(valueChanged(double)),
            this, SLOT(slotSelectSidebarItem(double)));

    m_pToggleSidebarItem = std::make_unique<ControlPushButton>(ConfigKey("[Playlist]", "ToggleSelectedSidebarItem"));
    connect(m_pToggleSidebarItem.get(), SIGNAL(valueChanged(double)),
            this, SLOT(slotToggleSelectedSidebarItem(double)));

    m_pLoadSelectedIntoFirstStopped = std::make_unique<ControlPushButton>(ConfigKey("[Playlist]","LoadSelectedIntoFirstStopped"));
    connect(m_pLoadSelectedIntoFirstStopped.get(), SIGNAL(valueChanged(double)),
            this, SLOT(slotLoadSelectedIntoFirstStopped(double)));

    ControlDoublePrivate::insertAlias(ConfigKey("[Playlist]", "AutoDjAddTop"), ConfigKey("[Library]", "AutoDjAddTop"));
    ControlDoublePrivate::insertAlias(ConfigKey("[Playlist]", "AutoDjAddBottom"), ConfigKey("[Library]", "AutoDjAddBottom"));
}

LibraryControl::~LibraryControl() = default;

void LibraryControl::maybeCreateGroupController(const QString& group) {
    if (m_loadToGroupControllers.find(group) == m_loadToGroupControllers.end()) {
        m_loadToGroupControllers.emplace(group, std::make_unique<LoadToGroupController>(this, group));
    }
}

void LibraryControl::slotNumDecksChanged(double v) {
    int iNumDecks = v;

    if (iNumDecks < 0) {
        return;
    }

    for (int i = 0; i < iNumDecks; ++i) {
        maybeCreateGroupController(PlayerManager::groupForDeck(i));
    }
}

void LibraryControl::slotNumSamplersChanged(double v) {
    int iNumSamplers = v;

    if (iNumSamplers < 0) {
        return;
    }

    for (int i = 0; i < iNumSamplers; ++i) {
        maybeCreateGroupController(PlayerManager::groupForSampler(i));
    }
}


void LibraryControl::slotNumPreviewDecksChanged(double v) {
    int iNumPreviewDecks = v;

    if (iNumPreviewDecks < 0) {
        return;
    }

    for (int i = 0; i < iNumPreviewDecks; ++i) {
        maybeCreateGroupController(PlayerManager::groupForPreviewDeck(i));
    }
}

void LibraryControl::bindSidebarWidget(WLibrarySidebar* pSidebarWidget) {
    if (m_pSidebarWidget) {
        disconnect(m_pSidebarWidget, 0, this, 0);
    }
    m_pSidebarWidget = pSidebarWidget;
    connect(m_pSidebarWidget, SIGNAL(destroyed()),
            this, SLOT(sidebarWidgetDeleted()));
}

void LibraryControl::bindWidget(WLibrary* pLibraryWidget, KeyboardEventFilter* pKeyboard) {
    Q_UNUSED(pKeyboard);
    if (m_pLibraryWidget) {
        disconnect(m_pLibraryWidget, 0, this, 0);
    }
    m_pLibraryWidget = pLibraryWidget;
    connect(m_pLibraryWidget, SIGNAL(destroyed()),
            this, SLOT(libraryWidgetDeleted()));
}

void LibraryControl::libraryWidgetDeleted() {
    m_pLibraryWidget = nullptr;
}

void LibraryControl::sidebarWidgetDeleted() {
    m_pSidebarWidget = nullptr;
}

void LibraryControl::slotLoadSelectedTrackToGroup(QString group, bool play) {
    if (!m_pLibraryWidget) {
        return;
    }

    LibraryView* activeView = m_pLibraryWidget->getActiveView();
    if (!activeView) {
        return;
    }
    activeView->loadSelectedTrackToGroup(group, play);
}

void LibraryControl::slotLoadSelectedIntoFirstStopped(double v) {
    if (!m_pLibraryWidget) {
        return;
    }

    if (v > 0) {
        LibraryView* activeView = m_pLibraryWidget->getActiveView();
        if (!activeView) {
            return;
        }
        activeView->loadSelectedTrack();
    }
}

void LibraryControl::slotAutoDjAddTop(double v) {
    if (!m_pLibraryWidget) {
        return;
    }

    if (v > 0) {
        auto activeView = m_pLibraryWidget->getActiveView();
        if (!activeView) {
            return;
        }
        activeView->slotSendToAutoDJTop();
    }
}

void LibraryControl::slotAutoDjAddBottom(double v) {
    if (!m_pLibraryWidget) {
        return;
    }
    if (v > 0) {
        auto activeView = m_pLibraryWidget->getActiveView();
        if (!activeView) {
            return;
        }
        activeView->slotSendToAutoDJBottom();
    }
}

void LibraryControl::slotSelectNextTrack(double v) {
    if (v > 0) {
        slotSelectTrack(1);
    }
}

void LibraryControl::slotSelectPrevTrack(double v) {
    if (v > 0) {
        slotSelectTrack(-1);
    }
}

void LibraryControl::slotSelectTrack(double v) {
    if (!m_pLibraryWidget) {
        return;
    }

    int i = (int)v;

    auto activeView = m_pLibraryWidget->getActiveView();
    if (!activeView) {
        return;
    }
    activeView->moveSelection(i);
}

void LibraryControl::slotMoveUp(double v) {
    if (v > 0) {
        emitKeyEvent(QKeyEvent{QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier});
    }
}

void LibraryControl::slotMoveDown(double v) {
    if (v > 0) {
        emitKeyEvent(QKeyEvent{QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier});
    }
}

void LibraryControl::slotMoveVertical(double v) {
    const auto key = (v < 0) ? Qt::Key_Up: Qt::Key_Down;
    const auto times = static_cast<unsigned short>(std::abs(v));
    emitKeyEvent(QKeyEvent{QEvent::KeyPress, key, Qt::NoModifier, QString(), false, times});
}

void LibraryControl::slotScrollUp(double v) {
    if (v > 0) {
        emitKeyEvent(QKeyEvent{QEvent::KeyPress, Qt::Key_PageUp, Qt::NoModifier});
    }
}

void LibraryControl::slotScrollDown(double v) {
    if (v > 0) {
        emitKeyEvent(QKeyEvent{QEvent::KeyPress, Qt::Key_PageDown, Qt::NoModifier});
    }
}

void LibraryControl::slotScrollVertical(double v) {
    const auto key = (v < 0) ? Qt::Key_PageUp: Qt::Key_PageDown;
    const auto times = static_cast<unsigned short>(std::abs(v));
    emitKeyEvent(QKeyEvent{QEvent::KeyPress, key, Qt::NoModifier, QString(), false, times});
}

void LibraryControl::slotMoveLeft(double v) {
    if (v > 0) {
        emitKeyEvent(QKeyEvent{QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier});
    }
}

void LibraryControl::slotMoveRight(double v) {
    if (v > 0) {
        emitKeyEvent(QKeyEvent{QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier});
    }
}

void LibraryControl::slotMoveHorizontal(double v) {
    const auto key = (v < 0) ? Qt::Key_Left: Qt::Key_Right;
    const auto times = static_cast<unsigned short>(std::abs(v));
    emitKeyEvent(QKeyEvent{QEvent::KeyPress, key, Qt::NoModifier, QString(), false, times});
}

void LibraryControl::slotMoveFocusForward(double v) {
    if (v > 0) {
        emitKeyEvent(QKeyEvent{QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier});
    }
}

void LibraryControl::slotMoveFocusBackward(double v) {
    if (v > 0) {
        emitKeyEvent(QKeyEvent{QEvent::KeyPress, Qt::Key_Tab, Qt::ShiftModifier});
    }
}

void LibraryControl::slotMoveFocus(double v) {
    const auto shift = (v < 0) ? Qt::ShiftModifier: Qt::NoModifier;
    const auto times = static_cast<unsigned short>(std::abs(v));
    emitKeyEvent(QKeyEvent{QEvent::KeyPress, Qt::Key_Tab, shift, QString(), false, times});
}

void LibraryControl::emitKeyEvent(QKeyEvent&& event) {
    // Ensure a valid library widget has the keyboard focus.
    // QApplication::focusWidget() is not sufficient here because it
    // would return any focused widget like WOverview, WWaveform, QSpinBox
    VERIFY_OR_DEBUG_ASSERT(m_pSidebarWidget) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(m_pLibraryWidget) {
        return;
    }
    if (!m_pLibraryWidget->hasFocus() && !m_pSidebarWidget->hasFocus()) {
        setLibraryFocus();
    }
    auto focusWidget = QApplication::focusWidget();
    VERIFY_OR_DEBUG_ASSERT(focusWidget) {
        return;
    }
    // Send the event pointer to the currently focused widget
    for (auto i = 0; i < event.count(); ++i) {
        QApplication::sendEvent(focusWidget, &event);
    }
}

void LibraryControl::setLibraryFocus() {
    // XXX: Set the focus of the library panel directly instead of sending tab from sidebar
    VERIFY_OR_DEBUG_ASSERT(m_pSidebarWidget) {
        return;
    }
    m_pSidebarWidget->setFocus();
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
    QApplication::sendEvent(m_pSidebarWidget, &event);
}

void LibraryControl::slotSelectSidebarItem(double v) {
    if (m_pSidebarWidget == NULL) {
        return;
    }
    if (v > 0) {
        QApplication::postEvent(m_pSidebarWidget, new QKeyEvent(
            QEvent::KeyPress,
            (int)Qt::Key_Down, Qt::NoModifier, QString(), true));
        QApplication::postEvent(m_pSidebarWidget, new QKeyEvent(
            QEvent::KeyRelease,
            (int)Qt::Key_Down, Qt::NoModifier, QString(), true));
    } else if (v < 0) {
        QApplication::postEvent(m_pSidebarWidget, new QKeyEvent(
            QEvent::KeyPress,
            (int)Qt::Key_Up, Qt::NoModifier, QString(), true));
        QApplication::postEvent(m_pSidebarWidget, new QKeyEvent(
            QEvent::KeyRelease,
            (int)Qt::Key_Up, Qt::NoModifier, QString(), true));
    }
}

void LibraryControl::slotSelectNextSidebarItem(double v) {
    if (v > 0) {
        slotSelectSidebarItem(1);
    }
}

void LibraryControl::slotSelectPrevSidebarItem(double v) {
    if (v > 0) {
        slotSelectSidebarItem(-1);
    }
}

void LibraryControl::slotToggleSelectedSidebarItem(double v) {
    if (m_pSidebarWidget && v > 0) {
        m_pSidebarWidget->toggleSelectedItem();
    }
}

void LibraryControl::slotGoToItem(double v) {
    if (!m_pLibraryWidget) {
        return;
    }
    // Load current track if a LibraryView object has focus
    const auto activeView = m_pLibraryWidget->getActiveView();
    if (activeView && activeView->hasFocus()) {
        return slotLoadSelectedIntoFirstStopped(v);
    }

    // Focus the library if this is a leaf node in the tree
    if (m_pSidebarWidget && m_pSidebarWidget->hasFocus()) {
        if (v > 0 && m_pSidebarWidget->isLeafNodeSelected()) {
            setLibraryFocus();
        } else {
            // Otherwise toggle the sidebar item expanded state
            slotToggleSelectedSidebarItem(v);
        }
    }
    // TODO(xxx) instead of remote control the widgets individual, we should 
    // translate this into Alt+Return and handle it at each library widget 
    // individual https://bugs.launchpad.net/mixxx/+bug/1758618
    //emitKeyEvent(QKeyEvent{QEvent::KeyPress, Qt::Key_Return, Qt::AltModifier});
}

void LibraryControl::slotSortColumn(double v) {
    m_pSortColumnToggle->set(v);
}

void LibraryControl::slotSortColumnToggle(double v) {
    int column = static_cast<int>(v);
    if (static_cast<int>(m_pSortColumn->get()) == column) {
        m_pSortOrder->set(!m_pSortOrder->get());
    } else {
        m_pSortColumn->set(v);
        m_pSortOrder->set(0);
    }
}

void LibraryControl::slotFontSize(double v) {
    if (v == 0.0) {
        return;
    }
    QFont font = m_pLibrary->getTrackTableFont();
    font.setPointSizeF(font.pointSizeF() + v);
    m_pLibrary->setFont(font);
}

void LibraryControl::slotIncrementFontSize(double v) {
    if (v > 0.0) {
        slotFontSize(1);
    }
}

void LibraryControl::slotDecrementFontSize(double v) {
    if (v > 0.0) {
        slotFontSize(-1);
    }
}
