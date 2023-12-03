#include "controllers/controllerscreenpreview.h"

#include <QHBoxLayout>
#include <QVBoxLayout>

#include "moc_controllerscreenpreview.cpp"

ControllerScreenPreview::ControllerScreenPreview(
        QWidget* parent, const LegacyControllerMapping::ScreenInfo& screen)
        : QWidget(parent),
          m_screenInfo(screen),
          m_pFrame(make_parented<QLabel>(this)),
          m_pStat(make_parented<QLabel>("- FPS", this)),
          m_frameDurationHistoryIdx(0),
          m_lastFrameTimespamp(mixxx::Time::elapsed()) {
    size_t frameDurationHistoryLenght = sizeof(m_frameDurationHistory) / sizeof(uint);
    memset(m_frameDurationHistory, 0, frameDurationHistoryLenght);
    m_pFrame->setFixedSize(screen.size);
    setMaximumWidth(screen.size.width());
    m_pStat->setAlignment(Qt::AlignRight);
    auto pLayout = make_parented<QVBoxLayout>(this);
    auto pBottomLayout = new QHBoxLayout();
    pLayout->addWidget(m_pFrame);
    pBottomLayout->addWidget(make_parented<QLabel>(
            QString("\"<i>%0</i>\"")
                    .arg(m_screenInfo.identifier.isEmpty()
                                    ? QStringLiteral("Unnamed")
                                    : m_screenInfo.identifier),
            this));
    pBottomLayout->addWidget(m_pStat);
    pLayout->addItem(pBottomLayout);
    pLayout->addItem(new QSpacerItem(
            1, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));
}
void ControllerScreenPreview::updateFrame(
        const LegacyControllerMapping::ScreenInfo& screen, const QImage& frame) {
    if (m_screenInfo.identifier != screen.identifier) {
        return;
    }
    size_t frameDurationHistoryLenght = sizeof(m_frameDurationHistory) / sizeof(uint);
    auto currentTimestamp = mixxx::Time::elapsed();
    m_frameDurationHistory[m_frameDurationHistoryIdx++] =
            (currentTimestamp - m_lastFrameTimespamp).toIntegerMillis();
    m_frameDurationHistoryIdx %= frameDurationHistoryLenght;

    double durationSinceLastFrame = 0.0;
    for (uint8_t i = 0; i < frameDurationHistoryLenght; i++) {
        durationSinceLastFrame += (double)m_frameDurationHistory[i];
    }
    durationSinceLastFrame /= (double)frameDurationHistoryLenght;

    if (durationSinceLastFrame > 0.0) {
        m_pStat->setText(QString("<i>FPS: %0/%1</i>")
                                 .arg((int)(1000.0 / durationSinceLastFrame))
                                 .arg(m_screenInfo.target_fps));
    }
    m_pFrame->setPixmap(QPixmap::fromImage(frame));
    m_lastFrameTimespamp = currentTimestamp;
}
