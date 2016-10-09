#include <QHBoxLayout>
#include <QKeyEvent>
#include <QVBoxLayout>

#include "wbuttonbar.h"
#include "library/libraryfeature.h"

WButtonBar::WButtonBar(QWidget* parent)
        : QFrame(parent),
          m_focusItem(0),
          m_focusFromButton(false) {
    
    QHBoxLayout* pHb = new QHBoxLayout(this);
    pHb->setContentsMargins(0,0,0,0);

    QWidget* w1 = new QWidget(this);
    
    // QSizePolicy::Maximum -> treat the size hint as maximum. This protects us 
    // from growing to the scroll area size which includes the Scroll bar. 
    w1->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
    
    pHb->addWidget(w1);
    pHb->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding,
                                       QSizePolicy::Minimum));
    
    m_pLayout = new QVBoxLayout(this);
    m_pLayout->setContentsMargins(0,0,0,0);
    m_pLayout->setSpacing(0);
    m_pLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    
    w1->setLayout(m_pLayout);
    setLayout(pHb);
    
    setFocusPolicy(Qt::StrongFocus);
    setAutoFillBackground(true);
}

WFeatureClickButton* WButtonBar::addButton(LibraryFeature* pFeature) {
    WFeatureClickButton* button = new WFeatureClickButton(pFeature, this);    
    m_pLayout->addWidget(button);
    updateGeometry();
    return button;
}

void WButtonBar::keyPressEvent(QKeyEvent* event) {
    bool focusFound = false;
    if (event->modifiers() == Qt::NoModifier) {
        switch(event->key()) {
        case Qt::Key_Up:
            for (int i = m_pLayout->count() - 1; i >= 0; --i) {
                QLayoutItem* item = m_pLayout->itemAt(i);
                if (item) {
                    QWidget* widget = item->widget();
                    if (widget) {
                        if (widget->hasFocus()) {
                            m_focusItem = i;
                            focusFound = true;
                        } else if (focusFound == true) {
                            widget->setFocus();
                            emit ensureVisible(widget);
                            m_focusItem = i;
                            qDebug() << "Focus to" << i;
                            event->accept();
                            return;
                        }
                    }
                }
            }
            if (focusFound == false) {
                QLayoutItem* item = m_pLayout->itemAt(m_focusItem);
                if (item) {
                    QWidget* widget = item->widget();
                    if (widget) {
                        widget->setFocus();
                        emit ensureVisible(widget);
                        event->accept();
                        return;
                    }
                }
            }
            break;
        case Qt::Key_Down:
            for (int i = 0; i < m_pLayout->count(); ++i) {
                QLayoutItem* item = m_pLayout->itemAt(i);
                if (item) {
                    QWidget* widget = item->widget();
                    if (widget) {
                        if (widget->hasFocus()) {
                            m_focusItem = i;
                            focusFound = true;
                        } else if (focusFound == true) {
                            widget->setFocus();
                            emit ensureVisible(widget);
                            m_focusItem = i;
                            qDebug() << "Focus to" << i;
                            event->accept();
                            return;
                        }
                    }
                }
            }
            if (focusFound == false) {
                QLayoutItem* item = m_pLayout->itemAt(m_focusItem);
                if (item) {
                    QWidget* widget = item->widget();
                    if (widget) {
                        widget->setFocus();
                        emit ensureVisible(widget);
                        event->accept();
                        return;
                    }
                }
            }
            break;
        }
    }
    QFrame::keyPressEvent(event);
}

void WButtonBar::focusInEvent(QFocusEvent* event) {
    QFrame::focusInEvent(event);
    if (m_focusFromButton) {
        // don't re-focus buttons, when the focus was just there before
        focusPreviousChild();
        m_focusFromButton = false;
    } else {
        QLayoutItem* item = m_pLayout->itemAt(m_focusItem);
        if (item) {
            QWidget* widget = item->widget();
            if (widget) {
                widget->setFocus();
                emit ensureVisible(widget);
            }
        }
    }
}

bool WButtonBar::focusNextPrevChild(bool next) {
    // focus changing by keyboard
    // Old item has still the focus, save it if it is one of our buttons
    m_focusFromButton = false;
    for (int i = 0; i < m_pLayout->count(); ++i) {
        QLayoutItem* item = m_pLayout->itemAt(i);
        if (item) {
            QWidget* widget = item->widget();
            if (widget) {
                if (widget->hasFocus()) {
                    m_focusItem = i;
                    if (!next) {
                        // WButtonBar::focusInEvent() is called short after.
                        m_focusFromButton = true;
                    }
                    break;
                }
            }
        }
    }

    return QFrame::focusNextPrevChild(next);
}
