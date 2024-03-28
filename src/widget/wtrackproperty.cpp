#include "widget/wtrackproperty.h"

#include <QApplication>
#include <QDebug>
#include <QUrl>

#include "control/controlpushbutton.h"
#include "moc_wtrackproperty.cpp"
#include "skin/legacy/skincontext.h"
#include "track/track.h"
#include "util/dnd.h"
#include "widget/wtrackmenu.h"

WTrackProperty::WTrackProperty(
        QWidget* pParent,
        UserSettingsPointer pConfig,
        Library* pLibrary,
        const QString& group,
        bool isMainDeck)
        : WLabel(pParent),
          m_group(group),
          m_pConfig(pConfig),
          m_pLibrary(pLibrary),
          m_isMainDeck(isMainDeck) {
    setAcceptDrops(true);
}

WTrackProperty::~WTrackProperty() {
    // Required to allow forward declaration of WTrackMenu in header
}

void WTrackProperty::setup(const QDomNode& node, const SkinContext& context) {
    WLabel::setup(node, context);

    QString property = context.selectString(node, "Property");
    if (property.isEmpty()) {
        return;
    }

    // Check if property with that name exists in Track class
    if (Track::staticMetaObject.indexOfProperty(property.toUtf8().constData()) == -1) {
        qWarning() << "WTrackProperty: Unknown track property:" << property;
        return;
    }
    m_property = property;
}

void WTrackProperty::slotTrackLoaded(TrackPointer pTrack) {
    if (!pTrack) {
        return;
    }
    m_pCurrentTrack = pTrack;
    connect(pTrack.get(),
            &Track::changed,
            this,
            &WTrackProperty::slotTrackChanged);
    updateLabel();
}

void WTrackProperty::slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack) {
    Q_UNUSED(pNewTrack);
    Q_UNUSED(pOldTrack);
    if (m_pCurrentTrack) {
        disconnect(m_pCurrentTrack.get(), nullptr, this, nullptr);
    }
    m_pCurrentTrack.reset();
    updateLabel();
}

void WTrackProperty::slotTrackChanged(TrackId trackId) {
    Q_UNUSED(trackId);
    updateLabel();
}

void WTrackProperty::updateLabel() {
    if (m_pCurrentTrack) {
        if (m_property.isEmpty()) {
            return;
        }
        QVariant property =
                m_pCurrentTrack->property(m_property.toUtf8().constData());
        if (property.isValid() && property.canConvert<QString>()) {
            setText(property.toString());
            return;
        }
    }
    setText("");
}

void WTrackProperty::mousePressEvent(QMouseEvent* pEvent) {
    DragAndDropHelper::mousePressed(pEvent);
}

void WTrackProperty::mouseMoveEvent(QMouseEvent* pEvent) {
    if (m_pCurrentTrack && DragAndDropHelper::mouseMoveInitiatesDrag(pEvent)) {
        DragAndDropHelper::dragTrack(m_pCurrentTrack, this, m_group);
    }
}

void WTrackProperty::mouseDoubleClickEvent(QMouseEvent* pEvent) {
    Q_UNUSED(pEvent);
    if (!m_pCurrentTrack) {
        return;
    }
    ensureTrackMenuIsCreated();
    m_pTrackMenu->loadTrack(m_pCurrentTrack, m_group);
    m_pTrackMenu->showDlgTrackInfo(m_property);
}

void WTrackProperty::dragEnterEvent(QDragEnterEvent* pEvent) {
    DragAndDropHelper::handleTrackDragEnterEvent(pEvent, m_group, m_pConfig);
}

void WTrackProperty::dropEvent(QDropEvent* pEvent) {
    DragAndDropHelper::handleTrackDropEvent(pEvent, *this, m_group, m_pConfig);
}

void WTrackProperty::contextMenuEvent(QContextMenuEvent* pEvent) {
    pEvent->accept();
    if (m_pCurrentTrack) {
        ensureTrackMenuIsCreated();
        m_pTrackMenu->loadTrack(m_pCurrentTrack, m_group);
        // Show the right-click menu
        m_pTrackMenu->popup(pEvent->globalPos());
    }
}

void WTrackProperty::ensureTrackMenuIsCreated() {
    if (m_pTrackMenu.get() != nullptr) {
        return;
    }

    m_pTrackMenu = make_parented<WTrackMenu>(
            this, m_pConfig, m_pLibrary, WTrackMenu::kDeckTrackMenuFeatures);

    // The show control exists only for main decks.
    if (!m_isMainDeck) {
        return;
    }
    // When a track menu for this deck is shown/hidden via contextMenuEvent
    // or pushbutton, it emits trackMenuVisible(bool).
    // The pushbutton is created in BaseTrackPlayer which, on value change requests,
    // also emits a signal which is connected to our slotShowTrackMenuChangeRequest().
    connect(m_pTrackMenu,
            &WTrackMenu::trackMenuVisible,
            this,
            [this](bool visible) {
                ControlObject::set(ConfigKey(m_group, kShowTrackMenuKey),
                        visible ? 1.0 : 0.0);
            });
}

/// This slot handles show/hide requests originating from both pushbutton changes
/// and WTrackMenu's show/hide signals.
/// If the request matches the menu state we only set the control value accordingly.
/// Otherwise, we show/hide the menu as requested. This will result in another
/// change request originating from the menu, then it's a match and we setAndConfirm()
void WTrackProperty::slotShowTrackMenuChangeRequest(bool show) {
    // Ignore no-op
    if ((ControlObject::get(ConfigKey(m_group, kShowTrackMenuKey)) > 0) == show) {
        return;
    }

    // Check for any open track menu.
    // If this is a show request, hide all other menus (decks and library).
    // Assumes there can only be one visible menu per deck
    bool confirmShow = false;
    const QWidgetList topLevelWidgets = QApplication::topLevelWidgets();
    for (QWidget* pWidget : topLevelWidgets) {
        // Ignore other popups and hidden track menus
        WTrackMenu* pTrackMenu = qobject_cast<WTrackMenu*>(pWidget);
        if (pTrackMenu && pTrackMenu->isVisible()) {
            if (show) {
                if (pTrackMenu->getDeckGroup() == m_group) {
                    // Don't return, yet, maybe we still need to hide other menus.
                    confirmShow = true;
                } else {
                    // Hide other menus
                    pTrackMenu->close();
                }
            } else if (pTrackMenu->getDeckGroup() == m_group) {
                // Hide this deck's menu, ignore other menus
                pTrackMenu->close();
                return;
            }
        }
    }

    // If we reach this, this is either a hide request but no menu was found for
    // this deck, or this is a show request and we've found an open menu.
    if (!show || confirmShow) {
        emit setAndConfirmTrackMenuControl(show);
        return;
    }

    // This is a show request and there was no open menu found for this deck.
    // Pop up menu as if we right-clicked at the center of this widget
    // Note: this widget may be hidden so the position may be unexpected,
    // though this is okay as long as all variants of deckN are on the same
    // side of the mixer.
    QContextMenuEvent event(QContextMenuEvent::Mouse,
            QPoint(),
            mapToGlobal(rect().center()));
    contextMenuEvent(&event);
}
