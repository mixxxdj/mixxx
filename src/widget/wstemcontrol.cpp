#include "widget/wstemcontrol.h"

#include <QCoreApplication>
#include <QDragEnterEvent>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QPalette>
#include <QPushButton>
#include <QStyleOption>
#include <QVBoxLayout>

#include "control/controlproxy.h"
#include "moc_wstemcontrol.cpp"
#include "track/track.h"
#include "util/math.h"
#include "util/parented_ptr.h"
#include "widget/controlwidgetconnection.h"
#include "widget/wglwidget.h"
#include "widget/wknob.h"
#include "widget/wknobcomposed.h"
#include "widget/wlabel.h"

namespace {
// FIXME(XXX) this is a workaround to ensure that that knob keep the aspect
// ratio as defined in its maxSize. This needs to be moved an implemented by the
// component directly
void ensureWidgetRatio(QLayout* layout) {
    for (int i = 0; i < layout->count(); i++) {
        auto w = layout->itemAt(i)->widget();
        if (!qobject_cast<WKnobComposed*>(w) && !qobject_cast<WKnob*>(w)) {
            continue;
        }
        float ratio = static_cast<float>(w->maximumWidth()) /
                static_cast<float>(w->maximumHeight());
        int maxSize = qMin(w->width(), w->height());
        QSize currentSize = w->size();
        QSize newSize;

        if (ratio > 1) {
            newSize = QSize(maxSize, static_cast<int>(static_cast<float>(maxSize) / ratio));
        } else {
            newSize = QSize(static_cast<int>(static_cast<float>(maxSize) * ratio), maxSize);
        }

        if (currentSize == newSize) {
            continue;
        }

        w->resize(newSize);
        layout->itemAt(i)->invalidate();
    }
}
} // namespace

WStemControlBox::WStemControlBox(
        const QString& group, QWidget* parent)
        : WWidgetGroup(parent), m_group(group), m_hasStem(false), m_displayed(true) {
    auto pLayout = new QVBoxLayout(this);
    pLayout->setSpacing(0);
    pLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(pLayout);

    setObjectName("StemControlBox");

    setWindowFlag(Qt::Sheet);
    setWindowFlag(Qt::FramelessWindowHint);
    setWindowFlag(Qt::NoDropShadowWindowHint);
    setWindowFlag(Qt::WindowDoesNotAcceptFocus);

    // setWindowFlag(Qt::BypassWindowManagerHint); // Make it fly over?

    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TranslucentBackground);
    setFocusPolicy(Qt::NoFocus);
}

void WStemControlBox::setDisplayed(bool displayed) {
    m_displayed = displayed;
    emit displayedChanged(m_displayed);
}

void WStemControlBox::slotTrackLoaded(TrackPointer track) {
    m_hasStem = false;
    if (!track) {
        return;
    }

    auto stemInfo = track->getStemInfo();

    if (stemInfo.isEmpty()) {
        return;
    }

    m_hasStem = true;

    int stemCount = qMin(m_stemControl.size(),
            static_cast<std::size_t>(stemInfo.size()));
    for (int stemIdx = 0; stemIdx < stemCount; stemIdx++) {
        m_stemControl[stemIdx]->setStem(stemInfo[stemIdx]);
    }
}

void WStemControlBox::addControl(QWidget* control) {
    auto pWidget = std::make_unique<WStemControl>(control, this, m_group, m_stemControl.size());
    layout()->addWidget(pWidget.get());
    m_stemControl.push_back(std::move(pWidget));
}

WStemControl::WStemControl(QWidget* widgetGroup, QWidget* parent, const QString& group, int stemIdx)
        : WWidget(parent),
          m_widget(widgetGroup),
          m_mutedCo(std::make_unique<ControlProxy>(
                  group, QString("stem_%1_mute").arg(stemIdx + 1))) {
    auto pLayout = new QHBoxLayout(this);
    pLayout->setSpacing(0);
    pLayout->setContentsMargins(0, 0, 0, 0);

    m_widget->setParent(this);
    setMinimumSize(m_widget->minimumSize());
    setMaximumSize(m_widget->maximumSize());
    setSizePolicy(m_widget->sizePolicy());

    layout()->addWidget(m_widget);

    m_mutedCo->connectValueChanged(this, [this](double value) {
        m_stemColor.setAlphaF(value == 1 ? 0.5 : 1.0);
        updateStyle();
    });
}

void WStemControl::setStem(const StemInfo& stemInfo) {
    m_stemColor = stemInfo.getColor();
    m_stemColor.setAlphaF(m_mutedCo->get() == 1 ? 0.5 : 1.0);
    updateStyle();
    WLabel* label = findChild<WLabel*>("stem_label");
    VERIFY_OR_DEBUG_ASSERT(label) {
        qWarning() << "Cannot find the Label \"stem_label\" in the Stem control";
        return;
    }
    label->setText(stemInfo.getLabel());
}

void WStemControl::updateStyle() {
    setStyleSheet(QString("WStemControl { background-color: %1; }")
                          .arg(m_stemColor.name(QColor::HexArgb)));
}

void WStemControl::resizeEvent(QResizeEvent* e) {
    WWidget::resizeEvent(e);
    ensureWidgetRatio(m_widget->layout());
}

void WStemControl::showEvent(QShowEvent* e) {
    WWidget::showEvent(e);
    ensureWidgetRatio(m_widget->layout());
}

void WStemControl::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
