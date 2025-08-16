#include "widget/wpushbutton.h"

#include <QApplication>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QStyleOption>
#include <QStylePainter>
#include <QtDebug>

#include "control/controlbehavior.h"
#include "control/controlobject.h"
#include "control/controlpushbutton.h"
#include "moc_wpushbutton.cpp"
#include "skin/legacy/skincontext.h"
#include "util/debug.h"
#include "widget/controlwidgetconnection.h"
#include "widget/wpixmapstore.h"

WPushButton::WPushButton(QWidget* pParent)
        : WWidget(pParent),
          m_leftButtonMode(mixxx::control::ButtonMode::Push),
          m_rightButtonMode(mixxx::control::ButtonMode::Push) {
    setStates(0);
}

WPushButton::WPushButton(QWidget* pParent,
        mixxx::control::ButtonMode leftButtonMode,
        mixxx::control::ButtonMode rightButtonMode)
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
                    context.selectScaleMode(backPathNode, Paintable::DrawMode::Fixed),
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
                        stateContext->selectScaleMode(unpressedNode, Paintable::DrawMode::Fixed);
                if (!pixmapSource.isEmpty()) {
                    setPixmap(iState, false, pixmapSource,
                              unpressedMode, context.getScaleFactor());
                }

                QDomElement pressedNode = stateContext->selectElement(state, "Pressed");
                pixmapSource = stateContext->getPixmapSource(pressedNode);
                // The implicit default in <1.12.0 was FIXED so we keep it for
                // backwards compatibility.
                Paintable::DrawMode pressedMode =
                        stateContext->selectScaleMode(pressedNode, Paintable::DrawMode::Fixed);
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
    if (m_leftConnections.empty()) {
        if (!m_connections.empty()) {
            // If no left connection is set, the this is the left connection
            leftConnection = m_connections[0].get();
        }
    } else {
        leftConnection = m_leftConnections[0].get();
    }

    if (leftConnection) {
        bool leftClickForcePush = context.selectBool(node, "LeftClickIsPushButton", false);
        m_leftButtonMode = mixxx::control::ButtonMode::Push;
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
            case mixxx::control::ButtonMode::Push:
            case mixxx::control::ButtonMode::PowerWindow:
                leftConnection->setEmitOption(
                        ControlParameterWidgetConnection::EMIT_ON_PRESS_AND_RELEASE);
                break;
            case mixxx::control::ButtonMode::LongPressLatching:
                leftConnection->setEmitOption(
                        ControlParameterWidgetConnection::EMIT_ON_PRESS_AND_RELEASE);
                m_pLongPressLatching = std::make_unique<LongPressLatching>(this);
                break;
            case mixxx::control::ButtonMode::Toggle:
            case mixxx::control::ButtonMode::Trigger:
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

    if (!m_rightConnections.empty()) {
        auto& rightConnection = m_rightConnections[0];
        bool rightClickForcePush = context.selectBool(node, "RightClickIsPushButton", false);
        m_rightButtonMode = mixxx::control::ButtonMode::Push;
        if (!rightClickForcePush) {
            const ConfigKey configKey = rightConnection->getKey();
            ControlPushButton* p = qobject_cast<ControlPushButton*>(
                    ControlObject::getControl(configKey));
            if (p) {
                m_rightButtonMode = p->getButtonMode();
                if (m_rightButtonMode != mixxx::control::ButtonMode::Push &&
                        m_rightButtonMode != mixxx::control::ButtonMode::Toggle &&
                        m_rightButtonMode != mixxx::control::ButtonMode::Trigger) {
                    SKIN_WARNING(node,
                            context,
                            "WPushButton::setup: Connecting a Pushbutton not "
                            "in Push, Trigger or Toggle mode is not "
                            "implemented\n Please consider to set "
                            "<RightClickIsPushButton>true</"
                            "RightClickIsPushButton>");
                }
            }
        }
        if (rightConnection->getEmitOption() &
                ControlParameterWidgetConnection::EMIT_DEFAULT) {
            switch (m_rightButtonMode) {
            case mixxx::control::ButtonMode::Push:
            case mixxx::control::ButtonMode::PowerWindow:
            case mixxx::control::ButtonMode::LongPressLatching:
                rightConnection->setEmitOption(
                        ControlParameterWidgetConnection::EMIT_ON_PRESS_AND_RELEASE);
                break;
            case mixxx::control::ButtonMode::Toggle:
            case mixxx::control::ButtonMode::Trigger:
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
    m_dragging = false;
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
    if (!pPixmap || pPixmap->isNull()) {
        // Only log if it looks like the user tried to specify a pixmap.
        if (!source.isEmpty()) {
            qDebug() << metaObject()->className() << objectName()
                     << "Error loading pixmap" << source.getPath()
                     << "for state" << iState;
        }
    } else if (mode == Paintable::DrawMode::Fixed) {
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
            (!m_pPixmapBack || m_pPixmapBack->isNull())) {
        // Only log if it looks like the user tried to specify a pixmap.
        qDebug() << metaObject()->className() << objectName()
                 << "Error loading background pixmap:" << source.getPath();
    }
}

void WPushButton::restyleAndRepaint() {
    emit displayValueChanged(readDisplayValue());

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
    if (m_pLongPressLatching) {
        if (dValue == 1.0) {
            m_pLongPressLatching->start();
        } else {
            m_pLongPressLatching->stop();
        }
    }
    restyleAndRepaint();
}

void WPushButton::paintEvent(QPaintEvent* e) {
    Q_UNUSED(e);
    paintOnDevice(nullptr, readDisplayValue());
}

void WPushButton::paintOnDevice(QPaintDevice* pd, int idx) {
    QStyleOption option;
    option.initFrom(this);
    std::unique_ptr<QStylePainter> p(pd ? new QStylePainter(pd, this) : new QStylePainter(this));
    p->drawPrimitive(QStyle::PE_Widget, option);

    if (m_iNoStates == 0) {
        return;
    }

    if (m_pPixmapBack) {
        m_pPixmapBack->draw(rect(), p.get());
    }

    const QVector<PaintablePointer>& pixmaps = m_bPressed ?
            m_pressedPixmaps : m_unpressedPixmaps;


    // m_text, m_pressedPixmaps and m_unpressedPixmaps are all the same size (as
    // per setup()) so if one is empty, all are empty.
    if (pixmaps.isEmpty()) {
        return;
    }

    // Just in case m_iNoStates is somehow different from pixmaps.size().
    if (idx < 0) {
        idx = 0;
    } else if (idx >= pixmaps.size()) {
        idx = pixmaps.size() - 1;
    }

    PaintablePointer pPixmap = pixmaps.at(idx);
    if (pPixmap && !pPixmap->isNull()) {
        pPixmap->draw(rect(), p.get());
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
        p->drawText(rect(), m_align.at(idx), elidedText);
    }

    if (pd == nullptr && m_pLongPressLatching) {
        m_pLongPressLatching->paint(p.get());
    }
}

void WPushButton::mousePressEvent(QMouseEvent * e) {
    const bool leftClick = e->button() == Qt::LeftButton;
    const bool rightClick = e->button() == Qt::RightButton;

    if (m_leftButtonMode == mixxx::control::ButtonMode::PowerWindow && m_iNoStates == 2) {
        if (leftClick) {
            m_clickTimer.setSingleShot(true);
            m_clickTimer.start(ControlPushButtonBehavior::kPowerWindowTimeMillis);
            m_bPressed = true;

            double emitValue = getControlParameterLeft() == 0.0 ? 1.0 : 0.0;
            setControlParameterLeftDown(emitValue);
            restyleAndRepaint();
        }
        // discharge right clicks here, because is used for latching in PowerWindow mode
        return;
    }

    if (rightClick) {
        // This is the secondary button function always a Pushbutton
        // due the lack of visual feedback we do not allow a toggle function
        if (m_rightButtonMode == mixxx::control::ButtonMode::Push ||
                m_rightButtonMode == mixxx::control::ButtonMode::Trigger ||
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
        if (m_leftButtonMode == mixxx::control::ButtonMode::Push || m_iNoStates == 1) {
            // This is either forced to behave like a push button on left-click
            // or this is a push button.
            emitValue = 1.0;
        } else {
            // Toggle through the states
            emitValue = getControlParameterLeft();
            const auto oldValue = emitValue;
            if (!util_isnan(emitValue) && m_iNoStates > 0) {
                emitValue = static_cast<int>(emitValue + 1.0) % m_iNoStates;
            }
            if (m_leftButtonMode == mixxx::control::ButtonMode::LongPressLatching) {
                m_clickTimer.setSingleShot(true);
                m_clickTimer.start(ControlPushButtonBehavior::kLongPressLatchingTimeMillis);
                if (oldValue == 0.0 && m_pLongPressLatching) {
                    m_pLongPressLatching->start();
                }
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
        // Leave might occur sporadically while dragging (swapping) a WHotcueButton.
        // Don't release in that case.
        if (m_bPressed && !m_dragging) {
            // A Leave event is send instead of a mouseReleaseEvent()
            // fake it to get not stuck in pressed state
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

void WPushButton::mouseReleaseEvent(QMouseEvent * e) {
    // Note. when changing any of these actions, also take care of
    // WHotcueButton::release()
    const bool leftClick = e->button() == Qt::LeftButton;
    const bool rightClick = e->button() == Qt::RightButton;

    if (m_pLongPressLatching) {
        m_pLongPressLatching->stop();
    }

    if (m_leftButtonMode == mixxx::control::ButtonMode::PowerWindow && m_iNoStates == 2) {
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
        // due the lack of visual feedback we do not allow a toggle
        // function
        m_bPressed = false;
        if (m_rightButtonMode == mixxx::control::ButtonMode::Push || m_iNoStates == 1) {
            setControlParameterRightUp(0.0);
        }
        restyleAndRepaint();
        return;
    }

    if (leftClick) {
        m_bPressed = false;
        double emitValue = getControlParameterLeft();
        if (m_leftButtonMode == mixxx::control::ButtonMode::Push || m_iNoStates == 1) {
            // This is a Pushbutton
            emitValue = 0.0;
        } else {
            if (m_leftButtonMode == mixxx::control::ButtonMode::LongPressLatching &&
                    m_clickTimer.isActive() && emitValue >= 1.0) {
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

void WPushButton::updateSlot() {
    update();
}

WPushButton::LongPressLatching::LongPressLatching(WPushButton* pButton)
        : m_pButton(pButton) {
    // To animate the long press latching
    connect(&m_animTimer, &QTimer::timeout, m_pButton, &WPushButton::updateSlot);
}

void WPushButton::LongPressLatching::paint(QPainter* p) {
    if (m_animTimer.isActive()) {
        // Animate the long press latching by capturing the off state in a pixmap
        // and gradually draw less of it over the duration of the long press latching.

        int remainingTime = std::max(0,
                ControlPushButtonBehavior::kLongPressLatchingTimeMillis -
                        static_cast<int>(
                                m_sinceStart.elapsed().toIntegerMillis()));

        if (remainingTime == 0) {
            m_animTimer.stop();
        } else {
            qreal x = m_pButton->width() * static_cast<qreal>(remainingTime) /
                    static_cast<qreal>(ControlPushButtonBehavior::
                                    kLongPressLatchingTimeMillis);
            p->drawPixmap(QPointF(m_pButton->width() - x, 0),
                    m_preLongPressPixmap,
                    QRectF((m_pButton->width() - x) * m_pButton->devicePixelRatio(),
                            0,
                            x * m_pButton->devicePixelRatio(),
                            m_pButton->height() * m_pButton->devicePixelRatio()));
        }
    }
}

void WPushButton::LongPressLatching::start() {
    if (m_animTimer.isActive()) {
        // already running
        return;
    }
    // Capture pixmap with the off state ...
    m_preLongPressPixmap = QPixmap(static_cast<int>(m_pButton->width() *
                                           m_pButton->devicePixelRatio()),
            static_cast<int>(
                    m_pButton->height() * m_pButton->devicePixelRatio()));
    m_preLongPressPixmap.setDevicePixelRatio(m_pButton->devicePixelRatio());
    m_pButton->paintOnDevice(&m_preLongPressPixmap, 0);
    // ... and start the long press latching animation
    m_sinceStart.start();
    m_animTimer.start(1000 / 60);
}

void WPushButton::LongPressLatching::stop() {
    m_animTimer.stop();
}
