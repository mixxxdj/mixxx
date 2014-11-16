#include "widget/weffectpushbutton.h"

#include <QStylePainter>
#include <QStyleOption>
#include <QPixmap>
#include <QtDebug>
#include <QMouseEvent>
#include <QTouchEvent>
#include <QPaintEvent>
#include <QApplication>

#include "widget/wpixmapstore.h"
#include "controlobject.h"
#include "controlpushbutton.h"
#include "control/controlbehavior.h"
#include "util/debug.h"

WEffectPushButton::WEffectPushButton(QWidget* pParent, EffectsManager* pEffectsManager)
        : WWidget(pParent),
          m_leftButtonMode(ControlPushButton::PUSH),
          m_rightButtonMode(ControlPushButton::PUSH),
          m_pEffectsManager(pEffectsManager) {
    setStates(0);
}

WEffectPushButton::WEffectPushButton(QWidget* pParent, ControlPushButton::ButtonMode leftButtonMode,
                         ControlPushButton::ButtonMode rightButtonMode)
        : WWidget(pParent),
          m_leftButtonMode(leftButtonMode),
          m_rightButtonMode(rightButtonMode) {
    setStates(0);
}

WEffectPushButton::~WEffectPushButton() {
}

void WEffectPushButton::setup(QDomNode node, const SkinContext& context) {
    // Number of states
    int iNumStates = context.selectInt(node, "NumberStates");
    setStates(iNumStates);

    // Set background pixmap if available
    if (context.hasNode(node, "BackPath")) {
        QString mode_str = context.selectAttributeString(
                context.selectElement(node, "BackPath"), "scalemode", "TILE");
        QString backPath = context.getPixmapPath(context.selectNode(node, "BackPath"));
        if (!backPath.isEmpty()) {
            setPixmapBackground(backPath, Paintable::DrawModeFromString(mode_str));
        }
    }

    // Load pixmaps for associated states
    QDomNode state = context.selectNode(node, "State");
    while (!state.isNull()) {
        if (state.isElement() && state.nodeName() == "State") {
            int iState = context.selectInt(state, "Number");
            if (iState < m_iNoStates) {
                QString pixmapPath;
                
                pixmapPath = context.getPixmapPath(context.selectNode(state, "Unpressed"));
                if (!pixmapPath.isEmpty()) {
                    setPixmap(iState, false, pixmapPath);
                }
                
                pixmapPath = context.getPixmapPath(context.selectNode(state, "Pressed"));
                if (!pixmapPath.isEmpty()) {
                    setPixmap(iState, true, pixmapPath);
                }
                
                m_text.replace(iState, context.selectString(state, "Text"));
                QString alignment = context.selectString(state, "Alignment");
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

    ControlParameterWidgetConnection* leftConnection = NULL;
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
            ControlPushButton* p = dynamic_cast<ControlPushButton*>(
                    ControlObject::getControl(configKey));
            if (p) {
                m_leftButtonMode = p->getButtonMode();
            }
        }
        if (leftConnection->getEmitOption() &
                ControlParameterWidgetConnection::EMIT_DEFAULT) {
            switch (m_leftButtonMode) {
                case ControlPushButton::PUSH:
                case ControlPushButton::LONGPRESSLATCHING:
                case ControlPushButton::POWERWINDOW:
                    leftConnection->setEmitOption(
                            ControlParameterWidgetConnection::EMIT_ON_PRESS_AND_RELEASE);
                    break;
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
            ControlPushButton* p = dynamic_cast<ControlPushButton*>(
                    ControlObject::getControl(configKey));
            if (p) {
                m_rightButtonMode = p->getButtonMode();
                if (m_rightButtonMode != ControlPushButton::PUSH) {
                    qWarning()
                            << "WEffectPushButton::setup: Connecting a Pushbutton not in PUSH mode is not implemented\n"
                            << "Please set <RightClickIsPushButton>true</RightClickIsPushButton>";
                }
            }
        }
        if (rightConnection->getEmitOption() &
                ControlParameterWidgetConnection::EMIT_DEFAULT) {
            switch (m_rightButtonMode) {
                case ControlPushButton::PUSH:
                case ControlPushButton::LONGPRESSLATCHING:
                case ControlPushButton::POWERWINDOW:
                    leftConnection->setEmitOption(
                            ControlParameterWidgetConnection::EMIT_ON_PRESS_AND_RELEASE);
                    break;
                default:
                    leftConnection->setEmitOption(
                            ControlParameterWidgetConnection::EMIT_ON_PRESS);
                    break;
            }
        }
        if (rightConnection->getDirectionOption() &
                        ControlParameterWidgetConnection::DIR_DEFAULT) {
            rightConnection->setDirectionOption(ControlParameterWidgetConnection::DIR_FROM_WIDGET);
        }
    }

    m_pButtonMenu = new QMenu(this);
    connect(m_pButtonMenu, SIGNAL(triggered(QAction*)),
            this, SLOT(slotActionChosen(QAction*)));
    bool rackOk = false;
    int rackNumber = context.selectInt(node, "EffectRack", &rackOk) - 1;
    bool chainOk = false;
    int chainNumber = context.selectInt(node, "EffectUnit", &chainOk) - 1;
    bool effectOk = false;
    int effectNumber = context.selectInt(node, "Effect", &effectOk) - 1;
    bool parameterOk = false;
    int parameterNumber = context.selectInt(node, "EffectButtonParameter", &parameterOk) - 1;

    // Tolerate no <EffectRack>. Use the default one.
    if (!rackOk) {
        rackNumber = 0;
    }

    if (!chainOk) {
        qDebug() << "EffectPushButton node had invalid EffectUnit number:" << chainNumber;
    }

    if (!effectOk) {
        qDebug() << "EffectPushButton node had invalid Effect number:" << effectNumber;
    }

    if (!parameterOk) {
        qDebug() << "EffectPushButton node had invalid ButtonParameter number:" << parameterNumber;
    }

    EffectRackPointer pRack = m_pEffectsManager->getEffectRack(rackNumber);
    if (pRack) {
        EffectChainSlotPointer pChainSlot = pRack->getEffectChainSlot(chainNumber);
        if (pChainSlot) {
            EffectSlotPointer pEffectSlot = pChainSlot->getEffectSlot(effectNumber);
            if (pEffectSlot) {
                EffectParameterSlotBasePointer pParameterSlot =
                        pEffectSlot->getEffectButtonParameterSlot(parameterNumber);
                if (pParameterSlot) {
                    m_pEffectParameterSlot = pParameterSlot;
                    connect(pParameterSlot.data(), SIGNAL(updated()),
                            this, SLOT(parameterUpdated()));
                    parameterUpdated();
                } else {
                    qDebug() << "EffectPushButton node had invalid ButtonParameter number:" << parameterNumber;
                }
            } else {
                qDebug() << "EffectPushButton node had invalid Effect number:" << effectNumber;
            }
        } else {
            qDebug() << "EffectPushButton node had invalid EffectUnit number:" << chainNumber;
        }
    } else {
        qDebug() << "EffectPushButton node had invalid EffectRack number:" << rackNumber;
    }
}

void WEffectPushButton::setStates(int iStates) {
    m_bPressed = false;
    m_iNoStates = iStates;
    m_activeTouchButton = Qt::NoButton;

    m_pressedPixmaps.resize(iStates);
    m_unpressedPixmaps.resize(iStates);
    m_text.resize(iStates);
    m_align.resize(iStates);
}

void WEffectPushButton::setPixmap(int iState, bool bPressed, const QString& filename) {
    QVector<PaintablePointer>& pixmaps = bPressed ?
            m_pressedPixmaps : m_unpressedPixmaps;

    if (iState < 0 || iState >= pixmaps.size()) {
        return;
    }

    PaintablePointer pPixmap = WPixmapStore::getPaintable(filename,
                                                          Paintable::STRETCH);

    if (pPixmap.isNull() || pPixmap->isNull()) {
        // Only log if it looks like the user tried to specify a pixmap.
        if (!filename.isEmpty()) {
            qDebug() << "WEffectPushButton: Error loading pixmap:" << filename;
        }
    } else {
        // Set size of widget equal to pixmap size
        setFixedSize(pPixmap->size());
    }
    pixmaps.replace(iState, pPixmap);
}

void WEffectPushButton::setPixmapBackground(const QString &filename,
                                      Paintable::DrawMode mode) {
    // Load background pixmap
    m_pPixmapBack = WPixmapStore::getPaintable(filename, mode);
    if (!filename.isEmpty() &&
            (m_pPixmapBack.isNull() || m_pPixmapBack->isNull())) {
        // Only log if it looks like the user tried to specify a pixmap.
        qDebug() << "WEffectPushButton: Error loading background pixmap:" << filename;
    }
}

void WEffectPushButton::onConnectedControlChanged(double dParameter, double dValue) {
    Q_UNUSED(dParameter);
    // Enums are not currently represented using parameter space so it doesn't
    // make sense to use the parameter here yet.
    if (m_iNoStates == 1) {
        m_bPressed = (dValue == 1.0);
    }

    foreach (QAction* action, m_pButtonMenu->actions()) {
        if (action->data().toDouble() == dValue) {
            action->setChecked(true);
            break;
        }
    }


    double value = getControlParameterDisplay(); 
    if (isnan(value) || m_iNoStates == 0) {
	return; 
    }

    int idx = static_cast<int>(value) % m_iNoStates;
    setProperty("displayValue", idx);
    // According to http://stackoverflow.com/a/3822243 this is the least
    // expensive way to restyle just this widget.
    // Since we expect button connections to not change at high frequency we
    // don't try to detect whether things have changed for WEffectPushButton, we just
    // re-render.
    style()->unpolish(this);
    style()->polish(this);
    // These calls don't always trigger the repaint, so call it explicitly.
    repaint();
}

void WEffectPushButton::paintEvent(QPaintEvent* e) {
    Q_UNUSED(e);
    QStyleOption option;
    option.initFrom(this);
    QStylePainter p(this);
    p.drawPrimitive(QStyle::PE_Widget, option);

    if (m_iNoStates == 0) {
        return;
    }

    if (m_pPixmapBack) {
        m_pPixmapBack->draw(0, 0, &p);
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
        pPixmap->draw(0, 0, &p);
    }

    QString text = m_text.at(idx);
    if (!text.isEmpty()) {
        p.drawText(rect(), m_align.at(idx), text);
    }
}

void WEffectPushButton::mousePressEvent(QMouseEvent * e) {
    const bool leftClick = e->button() == Qt::LeftButton;
    const bool rightClick = e->button() == Qt::RightButton;

    if (rightClick && m_pButtonMenu->actions().size()) {
        m_pButtonMenu->exec(e->globalPos());
    }

    if (m_leftButtonMode == ControlPushButton::POWERWINDOW
            && m_iNoStates == 2) {
        if (leftClick) {
            if (getControlParameterLeft() == 0.0) {
                m_clickTimer.setSingleShot(true);
                m_clickTimer.start(ControlPushButtonBehavior::kPowerWindowTimeMillis);
            }
            m_bPressed = true;
            setControlParameterLeftDown(1.0);
            update();
        }
        // discharge right clicks here, because is used for latching in POWERWINDOW mode
        return;
    }

    if (leftClick) {
        double emitValue;
        if (m_leftButtonMode == ControlPushButton::PUSH
                || m_iNoStates == 1) {
            // This is either forced to behave like a push button on left-click
            // or this is a push button.
            emitValue = 1.0;
        } else {
            // Toggle thru the states
            emitValue = getControlParameterLeft(); 
            if (!isnan(emitValue) && m_iNoStates) { 
                emitValue = static_cast<int>(emitValue + 1.0) % m_iNoStates;
            }	
            if (m_leftButtonMode == ControlPushButton::LONGPRESSLATCHING) {
                m_clickTimer.setSingleShot(true);
                m_clickTimer.start(ControlPushButtonBehavior::kLongPressLatchingTimeMillis);
            }

            // Check the corresponding QAction
            foreach (QAction* action, m_pButtonMenu->actions()) {
                if (action->data().toDouble() == emitValue) {
                    action->setChecked(true);
                    break;
                }
            }
        }
        m_bPressed = true;
        setControlParameterLeftDown(emitValue);
        update();
    }
}

void WEffectPushButton::focusOutEvent(QFocusEvent* e) {
    Q_UNUSED(e);
    if (e->reason() != Qt::MouseFocusReason) {
        // Since we support multi touch there is no reason to reset
        // the pressed flag if the Primary touch point is moved to an
        // other widget
        m_bPressed = false;
        update();
    }
}

void WEffectPushButton::mouseReleaseEvent(QMouseEvent * e) {
    const bool leftClick = e->button() == Qt::LeftButton;
    const bool rightClick = e->button() == Qt::RightButton;

    if (m_leftButtonMode == ControlPushButton::POWERWINDOW
            && m_iNoStates == 2) {
        if (leftClick) {
            const bool rightButtonDown = QApplication::mouseButtons() & Qt::RightButton;
            if (m_bPressed && !m_clickTimer.isActive() && !rightButtonDown) {
                // Release button after timer, but not if right button is clicked
                setControlParameterLeftUp(0.0);
            }
            m_bPressed = false;
        } else if (rightClick) {
            m_bPressed = false;
        }
        update();
        return;
    }

    if (leftClick) {
        double emitValue = getControlParameterLeft();
        if (m_leftButtonMode == ControlPushButton::PUSH
                || m_iNoStates == 1) {
            // This is a Pushbutton
            emitValue = 0.0;
        } else {
            if (m_leftButtonMode == ControlPushButton::LONGPRESSLATCHING
                    && m_clickTimer.isActive() && emitValue >= 1.0) {
                // revert toggle if button is released too early
                if (!isnan(emitValue) && m_iNoStates) { 
                    emitValue = static_cast<int>(emitValue - 1.0) % m_iNoStates;
                }      
            } else {
                // Nothing special happens when releasing a normal toggle button
            }
        }
        m_bPressed = false;
        setControlParameterLeftUp(emitValue);
        update();
    }
}

void WEffectPushButton::fillDebugTooltip(QStringList* debug) {
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

void WEffectPushButton::parameterUpdated() {
    m_pButtonMenu->clear();
    const QList<QPair<QString, double> >& options = m_pEffectParameterSlot->getManifest().getSteps();
    // qDebug() << " HERE IS THE OPTIONS SIZE: " << options.size() << m_pEffectParameterSlot->getManifest().name();
    m_iNoStates = options.size(); 
	if (m_iNoStates == 0) {
		// Toggle button without a menu
		m_iNoStates = 2; 
		return; 
	}
    double value = getControlParameterLeft();     

    QActionGroup* actionGroup = new QActionGroup(m_pButtonMenu);
    actionGroup->setExclusive(true);
    for (int i = 0; i < options.size(); i++) {
        // action is added automatically to actionGroup
        QAction* action = new QAction(actionGroup);
        // qDebug() << options[i].first; 
        action->setText(options[i].first);
        action->setData(options[i].second);
        action->setCheckable(true);
	
        if (options[i].second == value) {
            action->setChecked(true);
        }
        m_pButtonMenu->addAction(action);
    }
}

void WEffectPushButton::slotActionChosen(QAction* action) {
    action->setChecked(true);
    setControlParameter(action->data().toDouble());
}
