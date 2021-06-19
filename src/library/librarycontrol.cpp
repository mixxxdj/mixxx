#include "library/librarycontrol.h"

#include <QApplication>
#include <QItemSelectionModel>
#include <QModelIndex>
#include <QModelIndexList>
#include <QWindow>
#include <QtDebug>

#include "control/controlobject.h"
#include "control/controlpushbutton.h"
#include "library/library.h"
#include "library/libraryview.h"
#include "mixer/playermanager.h"
#include "moc_librarycontrol.cpp"
#include "widget/wlibrary.h"
#include "widget/wlibrarysidebar.h"
#include "widget/wsearchlineedit.h"
#include "widget/wtracktableview.h"

LoadToGroupController::LoadToGroupController(LibraryControl* pParent, const QString& group)
        : QObject(pParent),
          m_group(group) {
    m_pLoadControl = std::make_unique<ControlPushButton>(ConfigKey(group, "LoadSelectedTrack"));
    connect(m_pLoadControl.get(),
            &ControlObject::valueChanged,
            this,
            &LoadToGroupController::slotLoadToGroup);

    m_pLoadAndPlayControl = std::make_unique<ControlPushButton>(ConfigKey(group, "LoadSelectedTrackAndPlay"));
    connect(m_pLoadAndPlayControl.get(),
            &ControlObject::valueChanged,
            this,
            &LoadToGroupController::slotLoadToGroupAndPlay);

    connect(this,
            &LoadToGroupController::loadToGroup,
            pParent,
            &LibraryControl::slotLoadSelectedTrackToGroup);
}

LoadToGroupController::~LoadToGroupController() = default;

void LoadToGroupController::slotLoadToGroup(double v) {
    if (v > 0) {
        emit loadToGroup(m_group, false);
    }
}

void LoadToGroupController::slotLoadToGroupAndPlay(double v) {
    if (v > 0) {
        emit loadToGroup(m_group, true);
    }
}

LibraryControl::LibraryControl(Library* pLibrary)
        : QObject(pLibrary),
          m_pLibrary(pLibrary),
          m_pLibraryWidget(nullptr),
          m_pSidebarWidget(nullptr),
          m_pSearchbox(nullptr),
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
    connect(m_pMoveUp.get(),
            &ControlPushButton::valueChanged,
            this,
            &LibraryControl::slotMoveUp);
    connect(m_pMoveDown.get(),
            &ControlPushButton::valueChanged,
            this,
            &LibraryControl::slotMoveDown);
    connect(m_pMoveVertical.get(),
            &ControlEncoder::valueChanged,
            this,
            &LibraryControl::slotMoveVertical);

    // Controls to navigate vertically within currently focused widget (up/down buttons)
    m_pScrollUp = std::make_unique<ControlPushButton>(ConfigKey("[Library]", "ScrollUp"));
    m_pScrollDown = std::make_unique<ControlPushButton>(ConfigKey("[Library]", "ScrollDown"));
    m_pScrollVertical = std::make_unique<ControlEncoder>(ConfigKey("[Library]", "ScrollVertical"), false);
    connect(m_pScrollUp.get(),
            &ControlPushButton::valueChanged,
            this,
            &LibraryControl::slotScrollUp);
    connect(m_pScrollDown.get(),
            &ControlPushButton::valueChanged,
            this,
            &LibraryControl::slotScrollDown);
    connect(m_pScrollVertical.get(),
            &ControlEncoder::valueChanged,
            this,
            &LibraryControl::slotScrollVertical);

    // Controls to navigate horizontally within currently selected item (left/right buttons)
    m_pMoveLeft = std::make_unique<ControlPushButton>(ConfigKey("[Library]", "MoveLeft"));
    m_pMoveRight = std::make_unique<ControlPushButton>(ConfigKey("[Library]", "MoveRight"));
    m_pMoveHorizontal = std::make_unique<ControlEncoder>(ConfigKey("[Library]", "MoveHorizontal"), false);
    connect(m_pMoveLeft.get(),
            &ControlPushButton::valueChanged,
            this,
            &LibraryControl::slotMoveLeft);
    connect(m_pMoveRight.get(),
            &ControlPushButton::valueChanged,
            this,
            &LibraryControl::slotMoveRight);
    connect(m_pMoveHorizontal.get(),
            &ControlEncoder::valueChanged,
            this,
            &LibraryControl::slotMoveHorizontal);

    // Control to navigate between widgets (tab/shit+tab button)
    m_pMoveFocusForward = std::make_unique<ControlPushButton>(ConfigKey("[Library]", "MoveFocusForward"));
    m_pMoveFocusBackward = std::make_unique<ControlPushButton>(ConfigKey("[Library]", "MoveFocusBackward"));
    m_pMoveFocus = std::make_unique<ControlEncoder>(ConfigKey("[Library]", "MoveFocus"), false);
    connect(m_pMoveFocusForward.get(),
            &ControlPushButton::valueChanged,
            this,
            &LibraryControl::slotMoveFocusForward);
    connect(m_pMoveFocusBackward.get(),
            &ControlPushButton::valueChanged,
            this,
            &LibraryControl::slotMoveFocusBackward);
    connect(m_pMoveFocus.get(),
            &ControlEncoder::valueChanged,
            this,
            &LibraryControl::slotMoveFocus);

    // Control to "goto" the currently selected item in focused widget (context dependent)
    m_pGoToItem = std::make_unique<ControlPushButton>(ConfigKey("[Library]", "GoToItem"));
    connect(m_pGoToItem.get(),
            &ControlPushButton::valueChanged,
            this,
            &LibraryControl::slotGoToItem);

    // Auto DJ controls
    m_pAutoDjAddTop = std::make_unique<ControlPushButton>(ConfigKey("[Library]","AutoDjAddTop"));
    connect(m_pAutoDjAddTop.get(),
            &ControlPushButton::valueChanged,
            this,
            &LibraryControl::slotAutoDjAddTop);

    m_pAutoDjAddBottom = std::make_unique<ControlPushButton>(ConfigKey("[Library]","AutoDjAddBottom"));
    connect(m_pAutoDjAddBottom.get(),
            &ControlPushButton::valueChanged,
            this,
            &LibraryControl::slotAutoDjAddBottom);

    m_pAutoDjAddReplace = std::make_unique<ControlPushButton>(
            ConfigKey("[Library]", "AutoDjAddReplace"));
    connect(m_pAutoDjAddReplace.get(),
            &ControlPushButton::valueChanged,
            this,
            &LibraryControl::slotAutoDjAddReplace);

    // Sort controls
    m_pSortColumn = std::make_unique<ControlEncoder>(ConfigKey("[Library]", "sort_column"));
    m_pSortOrder = std::make_unique<ControlPushButton>(ConfigKey("[Library]", "sort_order"));
    m_pSortOrder->setButtonMode(ControlPushButton::TOGGLE);
    m_pSortColumnToggle = std::make_unique<ControlEncoder>(ConfigKey("[Library]", "sort_column_toggle"), false);
    connect(m_pSortColumn.get(),
            &ControlEncoder::valueChanged,
            this,
            &LibraryControl::slotSortColumn);
    connect(m_pSortColumnToggle.get(),
            &ControlEncoder::valueChanged,
            this,
            &LibraryControl::slotSortColumnToggle);

    // Font sizes
    m_pFontSizeKnob = std::make_unique<ControlObject>(
            ConfigKey("[Library]", "font_size_knob"), false);
    connect(m_pFontSizeKnob.get(),
            &ControlObject::valueChanged,
            this,
            &LibraryControl::slotFontSize);

    m_pFontSizeDecrement = std::make_unique<ControlPushButton>(
            ConfigKey("[Library]", "font_size_decrement"));
    connect(m_pFontSizeDecrement.get(),
            &ControlPushButton::valueChanged,
            this,
            &LibraryControl::slotDecrementFontSize);

    m_pFontSizeIncrement = std::make_unique<ControlPushButton>(
            ConfigKey("[Library]", "font_size_increment"));
    connect(m_pFontSizeIncrement.get(),
            &ControlPushButton::valueChanged,
            this,
            &LibraryControl::slotIncrementFontSize);

    // Track Color controls
    m_pTrackColorPrev = std::make_unique<ControlPushButton>(ConfigKey("[Library]", "track_color_prev"));
    m_pTrackColorNext = std::make_unique<ControlPushButton>(ConfigKey("[Library]", "track_color_next"));
    connect(m_pTrackColorPrev.get(),
            &ControlPushButton::valueChanged,
            this,
            &LibraryControl::slotTrackColorPrev);
    connect(m_pTrackColorNext.get(),
            &ControlPushButton::valueChanged,
            this,
            &LibraryControl::slotTrackColorNext);

    // Control to navigate between widgets (tab/shit+tab button)
    m_pSelectHistoryNext = std::make_unique<ControlPushButton>(
            ConfigKey("[Library]", "search_history_next"));
    m_pSelectHistoryPrev = std::make_unique<ControlPushButton>(
            ConfigKey("[Library]", "search_history_prev"));
    m_pSelectHistorySelect = std::make_unique<ControlEncoder>(
            ConfigKey("[Library]", "search_history_selector"), false);
    connect(m_pSelectHistoryNext.get(),
            &ControlPushButton::valueChanged,
            this,
            [this](double value) {
                VERIFY_OR_DEBUG_ASSERT(m_pSearchbox) {
                    return;
                }
                if (value >= 1.0) {
                    m_pSearchbox->slotMoveSelectedHistory(1);
                }
            });
    connect(m_pSelectHistoryPrev.get(),
            &ControlPushButton::valueChanged,
            this,
            [this](double value) {
                VERIFY_OR_DEBUG_ASSERT(m_pSearchbox) {
                    return;
                }
                if (value >= 1.0) {
                    m_pSearchbox->slotMoveSelectedHistory(-1);
                }
            });
    connect(m_pSelectHistorySelect.get(),
            &ControlEncoder::valueChanged,
            this,
            [this](double steps) {
                VERIFY_OR_DEBUG_ASSERT(m_pSearchbox) {
                    return;
                }
                if (steps >= 1.0 || steps <= -1.0) {
                    m_pSearchbox->slotMoveSelectedHistory(static_cast<int>(steps));
                }
            });

    /// Deprecated controls
    m_pSelectNextTrack = std::make_unique<ControlPushButton>(ConfigKey("[Playlist]", "SelectNextTrack"));
    connect(m_pSelectNextTrack.get(),
            &ControlPushButton::valueChanged,
            this,
            &LibraryControl::slotSelectNextTrack);

    m_pSelectPrevTrack = std::make_unique<ControlPushButton>(ConfigKey("[Playlist]", "SelectPrevTrack"));
    connect(m_pSelectPrevTrack.get(),
            &ControlPushButton::valueChanged,
            this,
            &LibraryControl::slotSelectPrevTrack);

    // Ignoring no-ops is important since this is for +/- tickers.
    m_pSelectTrack = std::make_unique<ControlObject>(ConfigKey("[Playlist]","SelectTrackKnob"), false);
    connect(m_pSelectTrack.get(),
            &ControlObject::valueChanged,
            this,
            &LibraryControl::slotSelectTrack);

    m_pSelectNextSidebarItem = std::make_unique<ControlPushButton>(ConfigKey("[Playlist]", "SelectNextPlaylist"));
    connect(m_pSelectNextSidebarItem.get(),
            &ControlPushButton::valueChanged,
            this,
            &LibraryControl::slotSelectNextSidebarItem);

    m_pSelectPrevSidebarItem = std::make_unique<ControlPushButton>(ConfigKey("[Playlist]", "SelectPrevPlaylist"));
    connect(m_pSelectPrevSidebarItem.get(),
            &ControlPushButton::valueChanged,
            this,
            &LibraryControl::slotSelectPrevSidebarItem);

    // Ignoring no-ops is important since this is for +/- tickers.
    m_pSelectSidebarItem = std::make_unique<ControlObject>(ConfigKey("[Playlist]", "SelectPlaylist"), false);
    connect(m_pSelectSidebarItem.get(),
            &ControlObject::valueChanged,
            this,
            &LibraryControl::slotSelectSidebarItem);

    m_pToggleSidebarItem = std::make_unique<ControlPushButton>(ConfigKey("[Playlist]", "ToggleSelectedSidebarItem"));
    connect(m_pToggleSidebarItem.get(),
            &ControlPushButton::valueChanged,
            this,
            &LibraryControl::slotToggleSelectedSidebarItem);

    m_pLoadSelectedIntoFirstStopped = std::make_unique<ControlPushButton>(ConfigKey("[Playlist]","LoadSelectedIntoFirstStopped"));
    connect(m_pLoadSelectedIntoFirstStopped.get(),
            &ControlPushButton::valueChanged,
            this,
            &LibraryControl::slotLoadSelectedIntoFirstStopped);

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
    int iNumDecks = static_cast<int>(v);

    if (iNumDecks < 0) {
        return;
    }

    for (int i = 0; i < iNumDecks; ++i) {
        maybeCreateGroupController(PlayerManager::groupForDeck(i));
    }
}

void LibraryControl::slotNumSamplersChanged(double v) {
    int iNumSamplers = static_cast<int>(v);

    if (iNumSamplers < 0) {
        return;
    }

    for (int i = 0; i < iNumSamplers; ++i) {
        maybeCreateGroupController(PlayerManager::groupForSampler(i));
    }
}


void LibraryControl::slotNumPreviewDecksChanged(double v) {
    int iNumPreviewDecks = static_cast<int>(v);

    if (iNumPreviewDecks < 0) {
        return;
    }

    for (int i = 0; i < iNumPreviewDecks; ++i) {
        maybeCreateGroupController(PlayerManager::groupForPreviewDeck(i));
    }
}

void LibraryControl::bindSidebarWidget(WLibrarySidebar* pSidebarWidget) {
    if (m_pSidebarWidget) {
        disconnect(m_pSidebarWidget, nullptr, this, nullptr);
    }
    m_pSidebarWidget = pSidebarWidget;
    connect(m_pSidebarWidget,
            &WLibrarySidebar::destroyed,
            this,
            &LibraryControl::sidebarWidgetDeleted);
}

void LibraryControl::bindLibraryWidget(WLibrary* pLibraryWidget, KeyboardEventFilter* pKeyboard) {
    Q_UNUSED(pKeyboard);
    if (m_pLibraryWidget) {
        disconnect(m_pLibraryWidget, nullptr, this, nullptr);
    }
    m_pLibraryWidget = pLibraryWidget;
    connect(m_pLibraryWidget,
            &WLibrary::destroyed,
            this,
            &LibraryControl::libraryWidgetDeleted);
}

void LibraryControl::bindSearchboxWidget(WSearchLineEdit* pSearchbox) {
    if (m_pSearchbox) {
        disconnect(m_pSearchbox, nullptr, this, nullptr);
    }
    m_pSearchbox = pSearchbox;
    connect(this,
            &LibraryControl::clearSearchIfClearButtonHasFocus,
            m_pSearchbox,
            &WSearchLineEdit::slotClearSearchIfClearButtonHasFocus);
    connect(m_pSearchbox,
            &WSearchLineEdit::destroyed,
            this,
            &LibraryControl::searchboxWidgetDeleted);
}



void LibraryControl::libraryWidgetDeleted() {
    m_pLibraryWidget = nullptr;
}

void LibraryControl::sidebarWidgetDeleted() {
    m_pSidebarWidget = nullptr;
}

void LibraryControl::searchboxWidgetDeleted() {
    m_pSearchbox = nullptr;
}

void LibraryControl::slotLoadSelectedTrackToGroup(const QString& group, bool play) {
    if (!m_pLibraryWidget) {
        return;
    }

    LibraryView* pActiveView = m_pLibraryWidget->getActiveView();
    if (!pActiveView) {
        return;
    }
    pActiveView->loadSelectedTrackToGroup(group, play);
}

void LibraryControl::slotLoadSelectedIntoFirstStopped(double v) {
    if (!m_pLibraryWidget) {
        return;
    }

    if (v > 0) {
        LibraryView* pActiveView = m_pLibraryWidget->getActiveView();
        if (!pActiveView) {
            return;
        }
        pActiveView->loadSelectedTrack();
    }
}

void LibraryControl::slotAutoDjAddTop(double v) {
    if (!m_pLibraryWidget) {
        return;
    }

    if (v > 0) {
        LibraryView* pActiveView = m_pLibraryWidget->getActiveView();
        if (!pActiveView) {
            return;
        }
        pActiveView->slotAddToAutoDJTop();
    }
}

void LibraryControl::slotAutoDjAddBottom(double v) {
    if (!m_pLibraryWidget) {
        return;
    }
    if (v > 0) {
        LibraryView* pActiveView = m_pLibraryWidget->getActiveView();
        if (!pActiveView) {
            return;
        }
        pActiveView->slotAddToAutoDJBottom();
    }
}

void LibraryControl::slotAutoDjAddReplace(double v) {
    if (!m_pLibraryWidget) {
        return;
    }
    if (v > 0) {
        LibraryView* pActiveView = m_pLibraryWidget->getActiveView();
        if (!pActiveView) {
            return;
        }
        pActiveView->slotAddToAutoDJReplace();
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

    LibraryView* pActiveView = m_pLibraryWidget->getActiveView();
    if (!pActiveView) {
        return;
    }
    pActiveView->moveSelection(i);
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
    // Ensure there's a valid library widget that can receive keyboard focus.
    // QApplication::focusWidget() is not sufficient here because it
    // would return any focused widget like WOverview, WWaveform, QSpinBox
    VERIFY_OR_DEBUG_ASSERT(m_pSidebarWidget) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(m_pLibraryWidget) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(m_pSearchbox) {
        return;
    }
    if (!QApplication::focusWindow()) {
        qDebug() << "Mixxx window is not focused, don't send key events";
        return;
    }

    bool keyIsTab = event.key() == Qt::Key_Tab;
    bool keyIsUpDown = event.key() == Qt::Key_Up || event.key() == Qt::Key_Down;

    // If the main window has focus, any widget can receive Tab.
    // Other keys should be sent to library widgets only to not
    // accidentally alter spinboxes etc.
    // If the searchbox has focus allow only Up/Down to select previous queries.
    if (!keyIsTab && !m_pSidebarWidget->hasFocus()
            && !m_pLibraryWidget->getActiveView()->hasFocus()) {
        if (keyIsUpDown && !m_pSearchbox->hasFocus()) {
            setLibraryFocus();
        }
    }
    if (keyIsTab && !QApplication::focusWidget()){
        setLibraryFocus();
    }

    // Send the event pointer to the currently focused widget
    auto* focusWidget = QApplication::focusWidget();
    if (focusWidget) {
        for (auto i = 0; i < event.count(); ++i) {
            QApplication::sendEvent(focusWidget, &event);
        }
    }
}

void LibraryControl::setLibraryFocus() {
    // TODO: Set the focus of the library panel directly instead of sending tab from sidebar
    VERIFY_OR_DEBUG_ASSERT(m_pSidebarWidget) {
        return;
    }
    // Try to focus the sidebar.
    m_pSidebarWidget->setFocus();

    // This may have failed, for example when a Cover window still has focus,
    // so make sure the sidebar is focused or we'll crash.
    if (!m_pSidebarWidget->hasFocus()) {
        return;
    }
    // Send Tab to move focus to the Tracks table.
    // Obviously only works as desired if the skin widgets are arranged
    // accordingly.
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
    QApplication::sendEvent(m_pSidebarWidget, &event);
}

void LibraryControl::slotSelectSidebarItem(double v) {
    VERIFY_OR_DEBUG_ASSERT(m_pSidebarWidget) {
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
    if (v <= 0) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(m_pSidebarWidget) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(m_pLibraryWidget) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(m_pSearchbox) {
        return;
    }

    // Focus the library if this is a leaf node in the tree
    if (m_pSidebarWidget->hasFocus()) {
        // Note that Tracks and AutoDJ always return 'false':
        // expanding those root items via controllers is considered dispensable
        // because the subfeatures' actions can't be accessed by controllers anyway.
        if (m_pSidebarWidget->isLeafNodeSelected()) {
            setLibraryFocus();
            return;
        } else {
            // Otherwise toggle the sidebar item expanded state
            slotToggleSelectedSidebarItem(v);
        }
    }

    // Load current track if a LibraryView object has focus
    LibraryView* pActiveView = m_pLibraryWidget->getActiveView();
    if (pActiveView && pActiveView->hasFocus()) {
        pActiveView->loadSelectedTrack();
        return;
    }

    // If searchbox has focus jump to the tracks table
    if (m_pSearchbox->hasFocus()) {
        return setLibraryFocus();
    }

    // Clear the search if the searchbox has focus
    emit clearSearchIfClearButtonHasFocus();

    // If the focused window is a dialog, press Enter
    auto* focusWindow = QApplication::focusWindow();
    if (focusWindow && (focusWindow->type() & (Qt::Dialog | Qt::Popup))) {
        QKeyEvent event(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier);
        QApplication::sendEvent(focusWindow, &event);
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
    int sortColumnId = static_cast<int>(v);
    if (sortColumnId == static_cast<int>(TrackModel::SortColumnId::CurrentIndex)) {
        if (!m_pLibraryWidget) {
            return;
        }
        // Get the ID of the column with the cursor
        sortColumnId =
                static_cast<int>(m_pLibraryWidget->getActiveView()
                                         ->getColumnIdFromCurrentIndex());
    }

    if (static_cast<int>(m_pSortColumn->get()) == sortColumnId) {
        m_pSortOrder->set((m_pSortOrder->get() == 0) ? 1.0 : 0.0);
    } else {
        m_pSortColumn->set(sortColumnId);
        m_pSortOrder->set(0.0);
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

void LibraryControl::slotTrackColorPrev(double v) {
    if (!m_pLibraryWidget) {
        return;
    }

    if (v > 0) {
        LibraryView* pActiveView = m_pLibraryWidget->getActiveView();
        if (!pActiveView) {
            return;
        }
        pActiveView->assignPreviousTrackColor();
    }
}

void LibraryControl::slotTrackColorNext(double v) {
    if (!m_pLibraryWidget) {
        return;
    }

    if (v > 0) {
        LibraryView* pActiveView = m_pLibraryWidget->getActiveView();
        if (!pActiveView) {
            return;
        }
        pActiveView->assignNextTrackColor();
    }
}
