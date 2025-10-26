#include "widget/wtrackproperty.h"

#include <QApplication>
#include <QDebug>
#include <QMetaProperty>
#include <QStyleOption>

#include "control/controlobject.h"
#include "moc_wtrackproperty.cpp"
#include "skin/legacy/skincontext.h"
#include "track/track.h"
#include "util/dnd.h"
#include "widget/wtrackmenu.h"

namespace {
// Duration (ms) the widget is 'selected' after left click, i.e. the duration
// a second click would open the value editor
constexpr int kSelectedClickTimeoutMs = 2000;
} // namespace

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
          m_isMainDeck(isMainDeck),
          m_isComment(false),
          m_propertyIsWritable(false),
          m_pSelectedClickTimer(nullptr),
          m_bSelected(false),
          m_pEditor(nullptr) {
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
    int propertyIndex = Track::staticMetaObject.indexOfProperty(property.toUtf8().constData());
    if (propertyIndex == -1) {
        qWarning() << "WTrackProperty: Unknown track property:" << property;
        return;
    }
    m_displayProperty = property;
    // Handle 'titleInfo' property: displays the title or, if both title & artist
    // are empty, filename. Though, this property is not writeable, so we map
    // it to 'title' for the editor.
    if (property == "titleInfo") {
        m_editProperty = "title";
    } else {
        if (!Track::staticMetaObject.property(propertyIndex).isWritable()) {
            return;
        }
        m_editProperty = m_displayProperty;
        if (m_editProperty == QStringLiteral("comment")) {
            m_isComment = true;
        }
    }
    m_propertyIsWritable = true;
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
    if (m_pEditor && m_pEditor->hasFocus()) {
        m_pEditor->hide();
    }
    updateLabel();
}

void WTrackProperty::slotTrackChanged(TrackId trackId) {
    Q_UNUSED(trackId);
    updateLabel();
    if (m_pEditor && m_pEditor->isVisible()) {
        // Close and discard new text
        m_pEditor->hide();
    }
}

void WTrackProperty::updateLabel() {
    if (m_pCurrentTrack) {
        setText(getPropertyStringFromTrack(m_displayProperty));
        return;
    }
    setText("");
}

const QString WTrackProperty::getPropertyStringFromTrack(QString& property) const {
    if (property.isEmpty() || !m_pCurrentTrack) {
        return {};
    }
    QVariant propVar = m_pCurrentTrack->property(property.toUtf8().constData());
    if (propVar.isValid() && propVar.canConvert<QString>()) {
        return propVar.toString();
    }
    return {};
}

void WTrackProperty::mousePressEvent(QMouseEvent* pEvent) {
    DragAndDropHelper::mousePressed(pEvent);

    // Check if there's another open editor. If yes, close it
    WTrackPropertyEditor* otherEditor =
            qobject_cast<WTrackPropertyEditor*>(QApplication::focusWidget());
    if (otherEditor) {
        otherEditor->clearFocus();
        // and don't attempt to activate this editor right away
        return;
    }

    if (!pEvent->buttons().testFlag(Qt::LeftButton) || !m_pCurrentTrack) {
        return;
    }

    // Don't create the editor or toggle the 'selected' state for protected
    // properties like duration.
    if (!m_propertyIsWritable) {
        return;
    }

    if (!m_pSelectedClickTimer) {
        // create & start the timer
        m_pSelectedClickTimer = make_parented<QTimer>(this);
        m_pSelectedClickTimer->setSingleShot(true);
        m_pSelectedClickTimer->setInterval(kSelectedClickTimeoutMs);
        m_pSelectedClickTimer->callOnTimeout(
                this, &WTrackProperty::resetSelectedState);
    } else if (m_pSelectedClickTimer->isActive()) {
        resetSelectedState();
        // create the persistent editor, populate & connect
        if (!m_pEditor) {
            m_pEditor = make_parented<WTrackPropertyEditor>(this);
            connect(m_pEditor,
                    // use custom signal. editingFinished() doesn't suit since it's
                    // also emitted weh pressing Esc (which should cancel editing)
                    &WTrackPropertyEditor::commitEditorData,
                    this,
                    &WTrackProperty::slotCommitEditorData);
        }
        // Don't let the editor expand beyond its initial size
        m_pEditor->setFixedSize(size());

        QString editText = getPropertyStringFromTrack(m_editProperty);
        if (m_displayProperty == "titleInfo" && editText.isEmpty()) {
            editText = tr("title");
        } else if (m_isComment) {
            // For comments we only load the first line,
            // ie. truncate track text at first linebreak.
            // On commit we replace the first line with the edited text.
            int firstLB = editText.indexOf('\n');
            if (firstLB >= 0) {
                editText.truncate(firstLB);
            }
        }
        m_pEditor->setText(editText);
        m_pEditor->selectAll();
        m_pEditor->show();
        m_pEditor->setFocus();
        return;
    }
    // start timer
    m_pSelectedClickTimer->start();
    m_bSelected = true;
    restyleAndRepaint();
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
    m_pTrackMenu->setTrackPropertyName(m_displayProperty);
    m_pTrackMenu->slotShowDlgTrackInfo();
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
        m_pTrackMenu->setTrackPropertyName(m_displayProperty);
        m_pTrackMenu->popup(pEvent->globalPos());
        // Unset the hover state manually (stuck state is probably a Qt bug)
        // TODO(ronso0) Test whether this is still required with Qt6
        QEvent lev = QEvent(QEvent::Leave);
        qApp->sendEvent(this, &lev);
        update();
    }
}

void WTrackProperty::ensureTrackMenuIsCreated() {
    if (m_pTrackMenu.get() != nullptr) {
        return;
    }

    m_pTrackMenu = make_parented<WTrackMenu>(
            this, m_pConfig, m_pLibrary, WTrackMenu::kDeckTrackMenuFeatures);

    // The show control exists only for main decks.
    if (m_isMainDeck) {
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
    // Before and after the loaded tracks file has been removed from disk,
    // instruct the library to save and restore the current index for
    // keyboard/controller navigation.
    connect(m_pTrackMenu,
            &WTrackMenu::saveCurrentViewState,
            this,
            &WTrackProperty::saveCurrentViewState);
    connect(m_pTrackMenu,
            &WTrackMenu::restoreCurrentViewStateOrIndex,
            this,
            &WTrackProperty::restoreCurrentViewState);
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
    QContextMenuEvent* pEvent = new QContextMenuEvent(QContextMenuEvent::Mouse,
            QPoint(),
            mapToGlobal(rect().center()));
    contextMenuEvent(pEvent);
}

void WTrackProperty::slotCommitEditorData(const QString& text) {
    if (!m_pCurrentTrack) {
        return;
    }

    // use real track data instead of text() to be independent from display text
    const QString trackText = getPropertyStringFromTrack(m_editProperty);
    QString editorText = text;
    if (m_isComment) {
        // Transform ALL occurrences of \n into linebreaks.
        // Existing linebreaks are not affected.
        QString cr(QChar::CarriageReturn);
        cr.append(QChar::LineFeed);
        editorText.replace("\\n", cr);
        // For multi-line comments, the editor received only the first line.
        // In order to keep the other lines, we need to replace
        // the first line of the original text with the editor text.
        // (which may add new linebreaks)
        // Note: assumes the comment didn't change while we were editing it.
        int firstLB = trackText.indexOf('\n');
        if (firstLB >= 0) { // has linebreak
            QString trackTSliced = trackText;
            trackTSliced = trackTSliced.sliced(firstLB);
            editorText.append(trackTSliced);
        }
    }
    if (editorText == trackText) {
        return;
    }
    const QVariant var(QVariant::fromValue(editorText));
    m_pCurrentTrack->setProperty(
            m_editProperty.toUtf8().constData(),
            var);
    // Track::changed() will update label
}

void WTrackProperty::resetSelectedState() {
    if (m_pSelectedClickTimer) {
        m_pSelectedClickTimer->stop();
        // explicitly disconnect() queued signals? not crucial
        // here since timeOut() just calls resetSelectedState()
    }
    m_bSelected = false;
    restyleAndRepaint();
}

void WTrackProperty::restyleAndRepaint() {
    emit selectedStateChanged(isSelected());

    style()->unpolish(this);
    style()->polish(this);
    // These calls don't always trigger the repaint, so call it explicitly.
    repaint();
}

WTrackPropertyEditor::WTrackPropertyEditor(QWidget* pParent)
        : QLineEdit(pParent) {
    installEventFilter(this);
}

bool WTrackPropertyEditor::eventFilter(QObject* pObj, QEvent* pEvent) {
    if (pEvent->type() == QEvent::KeyPress) {
        // The widget only receives keystrokes when in edit mode.
        // Esc will close & reset.
        // Enter/Return confirms.
        // Any other keypress is forwarded.
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(pEvent);
        const int key = keyEvent->key();
        switch (key) {
        case Qt::Key_Escape:
            hide();
            return true;
        case Qt::Key_Return:
        case Qt::Key_Enter:
            hide();
            emit commitEditorData(text());
            ControlObject::set(ConfigKey("[Library]", "refocus_prev_widget"), 1);
            return true;
        default:
            break;
        }
    } else if (pEvent->type() == QEvent::FocusOut) {
        // Close and commit if any other widget gets focus
        if (isVisible()) {
            QFocusEvent* fe = static_cast<QFocusEvent*>(pEvent);
            // For any FocusOut, except when showing the QLineEdit's menu,
            // we hide() and commit the current text.
            if (fe->reason() != Qt::PopupFocusReason) {
                hide();
                emit commitEditorData(text());
            }
        }
    }
    return QLineEdit::eventFilter(pObj, pEvent);
}
