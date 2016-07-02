#include "wfeatureclickbutton.h"
#include "util/assert.h"

const int WFeatureClickButton::kHoverTime = 250; // milliseconds

WFeatureClickButton::WFeatureClickButton(LibraryFeature* pFeature, QWidget* parent)
        : QToolButton(parent),
          m_pFeature(pFeature) {
    DEBUG_ASSERT_AND_HANDLE(pFeature != nullptr) {
        return;
    }

    setIcon(m_pFeature->getIcon());
    setText(m_pFeature->title().toString());

    connect(this, SIGNAL(clicked()), this, SLOT(slotClicked()));
    setAcceptDrops(true);
}

void WFeatureClickButton::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::RightButton) {
        emit(rightClicked(event->globalPos()));
    }
    QToolButton::mousePressEvent(event);
}

void WFeatureClickButton::dragEnterEvent(QDragEnterEvent* event) {
    qDebug() << "WFeatureClickButton::dragEnterEvent" << event;
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

