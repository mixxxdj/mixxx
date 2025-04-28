#include "controllers/controllerscreenpreview.h"

#include <QHBoxLayout>
#include <QVBoxLayout>

#include "moc_controllerscreenpreview.cpp"
#include "util/time.h"

namespace {
/// Number of sample frame timestamp sample to perform a smooth average FPS label.
constexpr double kFrameSmoothAverageFactor = 20;
} // namespace

using Clock = std::chrono::steady_clock;

ControllerScreenPreview::ControllerScreenPreview(
        QWidget* parent, const LegacyControllerMapping::ScreenInfo& screen)
        : QWidget(parent),
          m_screenInfo(screen),
          m_pFrame(make_parented<QLabel>(this)),
          m_pStat(make_parented<QLabel>(tr("FPS: n/a"), this)) {
    m_pFrame->setFixedSize(screen.size);
    setMaximumWidth(screen.size.width());
    m_pStat->setAlignment(Qt::AlignRight);
    auto pLayout = make_parented<QVBoxLayout>(this);
    pLayout->setContentsMargins(0, 0, 0, 0);
    auto* pBottomLayout = new QHBoxLayout();
    pLayout->addWidget(m_pFrame);
    pBottomLayout->addWidget(make_parented<QLabel>(
            QStringLiteral("\"<i>%0</i>\"")
                    .arg(m_screenInfo.identifier.isEmpty()
                                    ? tr("Unnamed")
                                    : m_screenInfo.identifier),
            this));
    pBottomLayout->addWidget(m_pStat);
    pLayout->addLayout(pBottomLayout);
    pLayout->addSpacerItem(new QSpacerItem(
            1, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));
}
void ControllerScreenPreview::updateFrame(
        const LegacyControllerMapping::ScreenInfo& screen, const QImage& frame) {
    if (m_screenInfo.identifier != screen.identifier) {
        return;
    }
    m_pFrame->setPixmap(QPixmap::fromImage(frame));

    auto currentTimestamp = Clock::now();
    if (m_lastFrameTimestamp == Clock::time_point()) {
        m_lastFrameTimestamp = currentTimestamp;
        return;
    }

    if (m_averageFrameDuration == 0) {
        m_averageFrameDuration =
                std::chrono::duration_cast<std::chrono::microseconds>(
                        currentTimestamp - m_lastFrameTimestamp)
                        .count();
    } else {
        m_averageFrameDuration = std::lerp(m_averageFrameDuration,
                std::chrono::duration_cast<std::chrono::microseconds>(
                        currentTimestamp - m_lastFrameTimestamp)
                        .count(),
                1.0 / kFrameSmoothAverageFactor);
    }
    m_lastFrameTimestamp = currentTimestamp;
    m_pStat->setText(tr("<i>FPS: %0/%1</i>")
                             .arg(QString::number(static_cast<int>(
                                          1000000 / m_averageFrameDuration)),
                                     QString::number(m_screenInfo.target_fps)));
}
