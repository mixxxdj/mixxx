#include "wfeatureclickbutton.h"
#include "util/assert.h"

const int WFeatureClickButton::kHoverTime = 250; // milliseconds

WFeatureClickButton::WFeatureClickButton(LibraryFeature* feature, QWidget* parent)
        : QToolButton(parent),
          m_feature(feature) {
    DEBUG_ASSERT_AND_HANDLE(feature != nullptr) {
        return;
    }

    setIcon(m_feature->getIcon());
    setText(m_feature->title().toString());
    m_data = m_feature->getViewName();

    connect(this, SIGNAL(clicked()), this, SLOT(slotClicked()));
    setAcceptDrops(true);
}

void WFeatureClickButton::setData(const QString& data) {
    m_data = data;
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
    if (m_feature->dragMoveAccept(event->mimeData()->urls().first())) {
        event->acceptProposedAction();
        m_hoverTimer.start(kHoverTime, this);
    }
}

void WFeatureClickButton::dragLeaveEvent(QDragLeaveEvent*) {
    m_hoverTimer.stop();
}

void WFeatureClickButton::dropEvent(QDropEvent* event) {
    event->acceptProposedAction();
    if (!event->mimeData()->hasUrls() || event->source() == this) {
        event->ignore();
        return;
    }

    if (m_feature->dropAccept(event->mimeData()->urls(), event->source())) {
        event->acceptProposedAction();
    }
}

void WFeatureClickButton::timerEvent(QTimerEvent* event) {
    if (event->timerId() != m_hoverTimer.timerId()) {
        QToolButton::timerEvent(event);
        return;
    }
    emit(hoverShow(m_data));
}

void WFeatureClickButton::slotClicked() {
    emit(clicked(m_data));
}

