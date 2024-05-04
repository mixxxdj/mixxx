#include "controllers/controllerscreenpreview.h"

#include <QHBoxLayout>
#include <QVBoxLayout>

#include "moc_controllerscreenpreview.cpp"
#include "util/time.h"

ControllerScreenPreview::ControllerScreenPreview(
        QWidget* parent, const LegacyControllerMapping::ScreenInfo& screen)
        : QWidget(parent),
          m_screenInfo(screen),
          m_pFrame(make_parented<QLabel>(this)),
          m_pStat(make_parented<QLabel>("- FPS", this)),
          m_frameDurationHistoryIdx(0),
          m_lastFrameTimestamp(mixxx::Time::elapsed()) {
    std::fill(m_frameDurationHistory.begin(), m_frameDurationHistory.end(), 0);
    m_pFrame->setFixedSize(screen.size);
    setMaximumWidth(screen.size.width());
    m_pStat->setAlignment(Qt::AlignRight);
    auto pLayout = make_parented<QVBoxLayout>(this);
    auto* pBottomLayout = new QHBoxLayout();
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
    size_t frameDurationHistoryLength = sizeof(m_frameDurationHistory) / sizeof(uint);
    auto currentTimestamp = mixxx::Time::elapsed();
    m_frameDurationHistory[m_frameDurationHistoryIdx++] =
            (currentTimestamp - m_lastFrameTimestamp).toIntegerMillis();
    m_frameDurationHistoryIdx %= frameDurationHistoryLength;

    double durationSinceLastFrame = 0.0;
    for (uint i = 0; i < frameDurationHistoryLength; i++) {
        durationSinceLastFrame += static_cast<double>(m_frameDurationHistory[i]);
    }
    durationSinceLastFrame /= static_cast<double>(frameDurationHistoryLength);

    if (durationSinceLastFrame > 0.0) {
        m_pStat->setText(QString("<i>FPS: %0/%1</i>")
                                 .arg(static_cast<int>(1000.0 / durationSinceLastFrame))
                                 .arg(m_screenInfo.target_fps));
    }
    m_pFrame->setPixmap(QPixmap::fromImage(frame));
    m_lastFrameTimestamp = currentTimestamp;
}
