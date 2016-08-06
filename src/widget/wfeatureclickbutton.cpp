#include <QMimeData>

#include "wfeatureclickbutton.h"
#include "util/assert.h"

const int WFeatureClickButton::kHoverTime = 250; // milliseconds

WFeatureClickButton::WFeatureClickButton(LibraryFeature* pFeature, QWidget* parent)
        : QToolButton(parent),
          m_textControl(ConfigKey("[Library]", "show_icon_text"), this),
          m_pFeature(pFeature) {
    DEBUG_ASSERT_AND_HANDLE(pFeature != nullptr) {
        return;
    }
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
    setAcceptDrops(true);
    connect(this, SIGNAL(clicked()), this, SLOT(slotClicked()));

    setIcon(m_pFeature->getIcon());
    m_textControl.connectValueChanged(SLOT(slotTextDisplayChanged(double)));

    slotTextDisplayChanged(m_textControl.get());
}

void WFeatureClickButton::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::RightButton) {
        emit(rightClicked(event->globalPos()));
    }
    QToolButton::mousePressEvent(event);
}

void WFeatureClickButton::dragEnterEvent(QDragEnterEvent* event) {
    //qDebug() << "WFeatureClickButton::dragEnterEvent" << event;
    if (!event->mimeData()->hasUrls() || event->source() == this) {
        event->ignore();
        return;
    }
    if (m_pFeature->dragMoveAccept(event->mimeData()->urls().first())) {
        event->acceptProposedAction();
        m_hoverTimer.start(kHoverTime, this);
    }
}

void WFeatureClickButton::dragLeaveEvent(QDragLeaveEvent*) {
    m_hoverTimer.stop();
}

void WFeatureClickButton::dropEvent(QDropEvent* event) {
    m_hoverTimer.stop();
    event->acceptProposedAction();
    if (!event->mimeData()->hasUrls() || event->source() == this) {
        event->ignore();
        return;
    }

    if (m_pFeature->dropAccept(event->mimeData()->urls(), event->source())) {
        event->acceptProposedAction();
    }
}

void WFeatureClickButton::timerEvent(QTimerEvent* event) {
    if (event->timerId() != m_hoverTimer.timerId()) {
        QToolButton::timerEvent(event);
        return;
    }
    emit(hoverShow(m_pFeature));
}

void WFeatureClickButton::slotClicked() {
    emit(clicked(m_pFeature));
}

void WFeatureClickButton::slotTextDisplayChanged(double value) {
    if (value < 1.0) {
        setText("");
        setToolButtonStyle(Qt::ToolButtonIconOnly);
    } else {
        setText(m_pFeature->title().toString());
        setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    }
}

