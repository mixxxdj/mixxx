#include <QHBoxLayout>
#include <QKeyEvent>
#include <QVBoxLayout>

#include "wbuttonbar.h"
#include "library/libraryfeature.h"

WButtonBar::WButtonBar(QWidget* parent)
        : QFrame(parent),
          m_focusItem(0) {
    
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
                    return;
                }
            }
        }
        QWidget::keyPressEvent(event);
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
                    return;
                }
            }
        }
        QWidget::keyPressEvent(event);
        break;
    default:
        QWidget::keyPressEvent(event);
        break;
    }
}

void WButtonBar::focusInEvent(QFocusEvent* event) {
    QWidget::focusInEvent(event);
    QLayoutItem* item = m_pLayout->itemAt(m_focusItem);
    if (item) {
        QWidget* widget = item->widget();
        if (widget) {
            widget->setFocus();
            emit ensureVisible(widget);
        }
    }
}

