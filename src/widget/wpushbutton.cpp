#include "widget/wpushbutton.h"

#include <QApplication>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPixmap>
#include <QStyleOption>
#include <QStylePainter>
#include <QTouchEvent>
#include <QtDebug>

#include "control/controlbehavior.h"
#include "control/controlobject.h"
#include "control/controlpushbutton.h"
#include "moc_wpushbutton.cpp"
#include "util/debug.h"
#include "widget/wpixmapstore.h"

WPushButton::WPushButton(QWidget* pParent)
        : WWidget(pParent),
          m_leftButtonMode(ControlPushButton::PUSH),
          m_rightButtonMode(ControlPushButton::PUSH) {
    setStates(0);
}

WPushButton::WPushButton(QWidget* pParent, ControlPushButton::ButtonMode leftButtonMode,
                         ControlPushButton::ButtonMode rightButtonMode)
        : WWidget(pParent),
          m_leftButtonMode(leftButtonMode),
          m_rightButtonMode(rightButtonMode) {
    setStates(0);
}

void WPushButton::setup(const QDomNode& node, const SkinContext& context) {
    setScaleFactor(context.getScaleFactor());

    // Number of states
    int iNumStates = context.selectInt(node, "NumberStates");
    setStates(iNumStates);

    // Set background pixmap if available

    QDomElement backPathNode = context.selectElement(node, "BackPath");
    if (!backPathNode.isNull()) {
        PixmapSource backgroundSource = context.getPixmapSource(backPathNode);
        if (!backgroundSource.isEmpty()) {
            // The implicit default in <1.12.0 was FIXED so we keep it for
            // backwards compatibility.
            setPixmapBackground(
                    backgroundSource,
                    context.selectScaleMode(backPathNode, Paintable::FIXED),
                    context.getScaleFactor());
        }
    }

    // Adds an ellipsis to truncated text
    QString elide;
    if (context.hasNodeSelectString(node, "Elide", &elide)) {
        elide = elide.toLower();
        if (elide == "right") {
            m_elideMode = Qt::ElideRight;
        } else if (elide == "middle") {
            m_elideMode = Qt::ElideMiddle;
        } else if (elide == "left") {
            m_elideMode = Qt::ElideLeft;
        } else if (elide == "none") {
            m_elideMode = Qt::ElideNone;
        } else {
            qDebug() << "WPushButton::setup(): Elide =" << elide <<
                    "unknown, use right, middle, left or none.";
        }
    }

    // Load pixmaps and set texts for associated states
    QDomNode state = context.selectNode(node, "State");
    while (!state.isNull()) {
        if (state.isElement() && state.nodeName() == "State") {
            // support for variables in State elements

            // Creating a SkinContext is expensive (nearly 20% of our CPU usage
            // creating a typical skin from profiling). As an optimization, we
            // look for "SetVariable" nodes to see if we need to create a
            // context.
            QScopedPointer<SkinContext> createdStateContext;
            if (context.hasVariableUpdates(state)) {
                createdStateContext.reset(new SkinContext(&context));
                createdStateContext->updateVariables(state);
            }

            const SkinContext* stateContext = !createdStateContext.isNull() ?
                    createdStateContext.data() : &context;

            int iState = stateContext->selectInt(state, "Number");
            if (iState < m_iNoStates) {

                QDomElement unpressedNode = stateContext->selectElement(state, "Unpressed");
                PixmapSource pixmapSource = stateContext->getPixmapSource(unpressedNode);
                // The implicit default in <1.12.0 was FIXED so we keep it for
                // backwards compatibility.
                Paintable::DrawMode unpressedMode =
                        stateContext->selectScaleMode(unpressedNode, Paintable::FIXED);
                if (!pixmapSource.isEmpty()) {
                    setPixmap(iState, false, pixmapSource,
                              unpressedMode, context.getScaleFactor());
                }

                QDomElement pressedNode = stateContext->selectElement(state, "Pressed");
                pixmapSource = stateContext->getPixmapSource(pressedNode);
                // The implicit default in <1.12.0 was FIXED so we keep it for
                // backwards compatibility.
                Paintable::DrawMode pressedMode =
                        stateContext->selectScaleMode(pressedNode, Paintable::FIXED);
                if (!pixmapSource.isEmpty()) {
                    setPixmap(iState, true, pixmapSource,
                              pressedMode, context.getScaleFactor());
                }

                m_text.replace(iState, stateContext->selectString(state, "Text"));
                QString alignment = stateContext->selectString(state, "Alignment").toLower();
                if (alignment == "left") {
                    m_align.replace(iState, Qt::AlignLeft);
                } else if (alignment == "right") {
                    m_align.replace(iState, Qt::AlignRight);
                } else {
                    // Default is center.
                    m_align.replace(iState, Qt::AlignCenter);
                }
            }
        }
        state = state.nextSibling();
    }

    ControlParameterWidgetConnection* leftConnection = nullptr;
    if (m_leftConnections.isEmpty()) {
        if (!m_connections.isEmpty()) {
            // If no left connection is set, the this is the left connection
            leftConnection = m_connections.at(0);
        }
    } else {
        leftConnection = m_leftConnections.at(0);
    }

    if (leftConnection) {
        bool leftClickForcePush = context.selectBool(node, "LeftClickIsPushButton", false);
        m_leftButtonMode = ControlPushButton::PUSH;
        if (!leftClickForcePush) {
            const ConfigKey& configKey = leftConnection->getKey();
            ControlPushButton* p = qobject_cast<ControlPushButton*>(
                    ControlObject::getControl(configKey));
            if (p) {
                m_leftButtonMode = p->getButtonMode();
            }
        }
        if (leftConnection->getEmitOption() &
                ControlParameterWidgetConnection::EMIT_DEFAULT) {
            switch (m_leftButtonMode) {
                case ControlPushButton::PUSH:
                case ControlPushButton::POWERWINDOW:
                case ControlPushButton::LONGPRESSLATCHING:
                    leftConnection->setEmitOption(
                            ControlParameterWidgetConnection::EMIT_ON_PRESS_AND_RELEASE);
                    break;
                case ControlPushButton::TOGGLE:
                case ControlPushButton::TRIGGER:
                default:
                    leftConnection->setEmitOption(
                            ControlParameterWidgetConnection::EMIT_ON_PRESS);
                    break;
            }
        }
        if (leftConnection->getDirectionOption() &
                        ControlParameterWidgetConnection::DIR_DEFAULT) {
            if (m_pDisplayConnection == leftConnection) {
                leftConnection->setDirectionOption(ControlParameterWidgetConnection::DIR_FROM_AND_TO_WIDGET);
            } else {
                leftConnection->setDirectionOption(ControlParameterWidgetConnection::DIR_FROM_WIDGET);
                if (m_pDisplayConnection->getDirectionOption() &
                        ControlParameterWidgetConnection::DIR_DEFAULT) {
                    m_pDisplayConnection->setDirectionOption(ControlParameterWidgetConnection::DIR_TO_WIDGET);
                }
            }
        }
    }

    if (!m_rightConnections.isEmpty()) {
        ControlParameterWidgetConnection* rightConnection = m_rightConnections.at(0);
        bool rightClickForcePush = context.selectBool(node, "RightClickIsPushButton", false);
        m_rightButtonMode = ControlPushButton::PUSH;
        if (!rightClickForcePush) {
            const ConfigKey configKey = rightConnection->getKey();
            ControlPushButton* p = qobject_cast<ControlPushButton*>(
                    ControlObject::getControl(configKey));
            if (p) {
                m_rightButtonMode = p->getButtonMode();
                if (m_rightButtonMode != ControlPushButton::PUSH &&
                        m_rightButtonMode != ControlPushButton::TOGGLE &&
                        m_rightButtonMode != ControlPushButton::TRIGGER) {
                    SKIN_WARNING(node, context)
                            << "WPushButton::setup: Connecting a Pushbutton not in PUSH, TRIGGER or TOGGLE mode is not implemented\n"
                            << "Please consider to set <RightClickIsPushButton>true</RightClickIsPushButton>";
                }
            }
        }
        if (rightConnection->getEmitOption() &
                ControlParameterWidgetConnection::EMIT_DEFAULT) {
            switch (m_rightButtonMode) {
                case ControlPushButton::PUSH:
                case ControlPushButton::POWERWINDOW:
                case ControlPushButton::LONGPRESSLATCHING:
                    rightConnection->setEmitOption(
                            ControlParameterWidgetConnection::EMIT_ON_PRESS_AND_RELEASE);
                    break;
                case ControlPushButton::TOGGLE:
                case ControlPushButton::TRIGGER:
                default:
                    rightConnection->setEmitOption(
                            ControlParameterWidgetConnection::EMIT_ON_PRESS);
                    break;
            }
        }
        if (rightConnection->getDirectionOption() &
                        ControlParameterWidgetConnection::DIR_DEFAULT) {
            rightConnection->setDirectionOption(ControlParameterWidgetConnection::DIR_FROM_WIDGET);
        }
    }

    setFocusPolicy(Qt::NoFocus);
}

void WPushButton::setStates(int iStates) {
    m_bHovered = false;
    m_bPressed = false;
    m_iNoStates = iStates;
    m_elideMode = Qt::ElideNone;
    m_activeTouchButton = Qt::NoButton;

    m_pressedPixmaps.resize(iStates);
    m_unpressedPixmaps.resize(iStates);
    m_text.resize(iStates);
    m_align.resize(iStates);
}

void WPushButton::setPixmap(int iState,
        bool bPressed,
        const PixmapSource& source,
        Paintable::DrawMode mode,
        double scaleFactor) {
    QVector<PaintablePointer>& pixmaps = bPressed ?
            m_pressedPixmaps : m_unpressedPixmaps;

    if (iState < 0 || iState >= pixmaps.size()) {
        return;
    }

    PaintablePointer pPixmap = WPixmapStore::getPaintable(source, mode, scaleFactor);
    if (pPixmap.isNull() || pPixmap->isNull()) {
        // Only log if it looks like the user tried to specify a pixmap.
        if (!source.isEmpty()) {
            qDebug() << "WPushButton: Error loading pixmap:" << source.getPath();
        }
    } else if (mode == Paintable::FIXED) {
        // Set size of widget equal to pixmap size
        setFixedSize(pPixmap->size());
    }
    pixmaps.replace(iState, pPixmap);
}

void WPushButton::setPixmapBackground(const PixmapSource& source,
        Paintable::DrawMode mode,
        double scaleFactor) {
    // Load background pixmap
    m_pPixmapBack = WPixmapStore::getPaintable(source, mode, scaleFactor);
    if (!source.isEmpty() &&
            (m_pPixmapBack.isNull() || m_pPixmapBack->isNull())) {
        // Only log if it looks like the user tried to specify a pixmap.
        qDebug() << "WPushButton: Error loading background pixmap:" << source.getPath();
    }
}

void WPushButton::restyleAndRepaint() {
    emit displayValueChanged(readDisplayValue());

    // According to http://stackoverflow.com/a/3822243 this is the least
    // expensive way to restyle just this widget.
    // Since we expect button connections to not change at high frequency we
    // don't try to detect whether things have changed for WPushButton, we just
    // re-render.
    style()->unpolish(this);
    style()->polish(this);

    // These calls don't always trigger the repaint, so call it explicitly.
    repaint();
}

void WPushButton::onConnectedControlChanged(double dParameter, double dValue) {
    Q_UNUSED(dParameter);
    // Enums are not currently represented using parameter space so it doesn't
    // make sense to use the parameter here yet.
    if (m_iNoStates == 1) {
        m_bPressed = (dValue == 1.0);
    }

    restyleAndRepaint();
}

void WPushButton::paintEvent(QPaintEvent* e) {
    Q_UNUSED(e);
    QStyleOption option;
    option.initFrom(this);
    QStylePainter p(this);
    p.drawPrimitive(QStyle::PE_Widget, option);

    if (m_iNoStates == 0) {
        return;
    }

    if (m_pPixmapBack) {
        m_pPixmapBack->draw(rect(), &p);
    }

    const QVector<PaintablePointer>& pixmaps = m_bPressed ?
            m_pressedPixmaps : m_unpressedPixmaps;


    // m_text, m_pressedPixmaps and m_unpressedPixmaps are all the same size (as
    // per setup()) so if one is empty, all are empty.
    if (pixmaps.isEmpty()) {
        return;
    }

    int idx = readDisplayValue();
    // Just in case m_iNoStates is somehow different from pixmaps.size().
    if (idx < 0) {
        idx = 0;
    } else if (idx >= pixmaps.size()) {
        idx = pixmaps.size() - 1;
    }

    PaintablePointer pPixmap = pixmaps.at(idx);
    if (!pPixmap.isNull() && !pPixmap->isNull()) {
        pPixmap->draw(rect(), &p);
    }

    QString text = m_text.at(idx);
    if (!text.isEmpty()) {
        QFontMetrics metrics(font());
        // ToDo(ronso0) Consider css padding for buttons with border images
        // * read QWidget::style()->property("padding-left");
        // * adjust width()
        // * transform rect()
//        int textWidth = width() - lPad - rPad;
//        QRect textRect = rect().adjust(x1, y1, x2, y2);
        QString elidedText = metrics.elidedText(text, m_elideMode, width());
        p.drawText(rect(), m_align.at(idx), elidedText);
    }
}

void WPushButton::mousePressEvent(QMouseEvent * e) {
    const bool leftClick = e->button() == Qt::LeftButton;
    const bool rightClick = e->button() == Qt::RightButton;

    if (m_leftButtonMode == ControlPushButton::POWERWINDOW
            && m_iNoStates == 2) {
        if (leftClick) {
            m_clickTimer.setSingleShot(true);
            m_clickTimer.start(ControlPushButtonBehavior::kPowerWindowTimeMillis);
            m_bPressed = true;

            double emitValue = getControlParameterLeft() == 0.0 ? 1.0 : 0.0;
            setControlParameterLeftDown(emitValue);
            restyleAndRepaint();
        }
        // discharge right clicks here, because is used for latching in POWERWINDOW mode
        return;
    }

    if (rightClick) {
        // This is the secondary button function always a Pushbutton
        // due the lack of visual feedback we do not allow a toggle function
        if (m_rightButtonMode == ControlPushButton::PUSH ||
                m_rightButtonMode == ControlPushButton::TRIGGER ||
                m_iNoStates == 1) {
            m_bPressed = true;
            setControlParameterRightDown(1.0);
            restyleAndRepaint();
        }
        return;
    }

    if (leftClick) {
        m_bPressed = true;
        double emitValue;
        if (m_leftButtonMode == ControlPushButton::PUSH
                || m_iNoStates == 1) {
            // This is either forced to behave like a push button on left-click
            // or this is a push button.
            emitValue = 1.0;
        } else {
            // Toggle through the states
            emitValue = getControlParameterLeft();
            if (!util_isnan(emitValue) && m_iNoStates > 0) {
                emitValue = static_cast<int>(emitValue + 1.0) % m_iNoStates;
            }
            if (m_leftButtonMode == ControlPushButton::LONGPRESSLATCHING) {
                m_clickTimer.setSingleShot(true);
                m_clickTimer.start(ControlPushButtonBehavior::kLongPressLatchingTimeMillis);
            }
        }
        setControlParameterLeftDown(emitValue);
        restyleAndRepaint();
    }
}

bool WPushButton::event(QEvent* e) {
    if (e->type() == QEvent::WindowDeactivate) {
        // if the window is deactivated while in pressed state
        if (m_bPressed) {
            m_bPressed = false;
            restyleAndRepaint();
        }
    } else if (e->type() == QEvent::Enter) {
        m_bHovered = true;
        restyleAndRepaint();
    } else if (e->type() == QEvent::Leave) {
        if (m_bPressed) {
            // A Leave event is sent instead of a mouseReleaseEvent()
            // fake it to not get stuck in pressed state
            QMouseEvent mouseEvent = QMouseEvent(
                    QEvent::MouseButtonRelease,
                    QPointF(),
                    QPointF(),
                    QPointF(),
                    Qt::LeftButton,
                    Qt::NoButton,
                    Qt::NoModifier,
                    Qt::MouseEventSynthesizedByApplication);
            mouseReleaseEvent(&mouseEvent);
        }
        m_bHovered = false;
        restyleAndRepaint();
    }
    return WWidget::event(e);
}

void WPushButton::focusOutEvent(QFocusEvent* e) {
    qDebug() << "focusOutEvent" << e->reason();
    if (m_bPressed && e->reason() != Qt::MouseFocusReason) {
        // Since we support multi touch there is no reason to reset
        // the pressed flag if the Primary touch point is moved to an
        // other widget
        m_bPressed = false;
        restyleAndRepaint();
    }
    QWidget::focusOutEvent(e);
}

void WPushButton::mouseReleaseEvent(QMouseEvent * e) {
    const bool leftClick = e->button() == Qt::LeftButton;
    const bool rightClick = e->button() == Qt::RightButton;

    if (m_leftButtonMode == ControlPushButton::POWERWINDOW
            && m_iNoStates == 2) {
        if (leftClick) {
            const bool rightButtonDown = QApplication::mouseButtons() & Qt::RightButton;
            if (m_bPressed && !m_clickTimer.isActive() && !rightButtonDown) {
                // Release button after timer, but not if right button is clicked
                double emitValue = getControlParameterLeft() == 0.0 ? 1.0 : 0.0;
                setControlParameterLeftUp(emitValue);
            }
            m_bPressed = false;
        } else if (rightClick) {
            m_bPressed = false;
        }
        restyleAndRepaint();
        return;
    }

    if (rightClick) {
        // This is the secondary clickButton function,
        // due the leak of visual feedback we do not allow a toggle
        // function
        m_bPressed = false;
        if (m_rightButtonMode == ControlPushButton::PUSH
                || m_iNoStates == 1) {
            setControlParameterRightUp(0.0);
        }
        restyleAndRepaint();
        return;
    }

    if (leftClick) {
        m_bPressed = false;
        double emitValue = getControlParameterLeft();
        if (m_leftButtonMode == ControlPushButton::PUSH
                || m_iNoStates == 1) {
            // This is a Pushbutton
            emitValue = 0.0;
        } else {
            if (m_leftButtonMode == ControlPushButton::LONGPRESSLATCHING
                    && m_clickTimer.isActive() && emitValue >= 1.0) {
                // revert toggle if button is released too early
                if (!util_isnan(emitValue) && m_iNoStates > 0) {
                    emitValue = static_cast<int>(emitValue - 1.0) % m_iNoStates;
                }
            } else {
                // Nothing special happens when releasing a normal toggle button
            }
        }
        setControlParameterLeftUp(emitValue);
        restyleAndRepaint();
    }
}

void WPushButton::fillDebugTooltip(QStringList* debug) {
    WWidget::fillDebugTooltip(debug);
    *debug << QString("NumberStates: %1").arg(m_iNoStates)
           << QString("LeftCurrentState: %1").arg(
               static_cast<int>(getControlParameterLeft()) %
               (m_iNoStates > 0 ? m_iNoStates : 1))
           << QString("Pressed: %1").arg(toDebugString(m_bPressed))
           << QString("LeftButtonMode: %1")
            .arg(ControlPushButton::buttonModeToString(m_leftButtonMode))
           << QString("RightButtonMode: %1")
            .arg(ControlPushButton::buttonModeToString(m_rightButtonMode));
}
