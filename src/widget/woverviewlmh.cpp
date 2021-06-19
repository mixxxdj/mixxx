#include "widget/woverviewlmh.h"

#include <QPen>
#include <QPainter>
#include <QColor>

#include "util/timer.h"
#include "util/math.h"
#include "waveform/waveform.h"

WOverviewLMH::WOverviewLMH(
        const QString& group,
        PlayerManager* pPlayerManager,
        UserSettingsPointer pConfig,
        QWidget* parent)
        : WOverview(group, pPlayerManager, pConfig, parent) {
}

bool WOverviewLMH::drawNextPixmapPart() {
    ScopedTimer t("WOverviewLMH::drawNextPixmapPart");

    //qDebug() << "WOverview::drawNextPixmapPart()";

    int currentCompletion;

    ConstWaveformPointer pWaveform = getWaveform();
    if (!pWaveform) {
        return false;
    }

    const int dataSize = pWaveform->getDataSize();
    if (dataSize == 0) {
        return false;
    }

    if (m_waveformSourceImage.isNull()) {
        // Waveform pixmap twice the height of the viewport to be scalable
        // by total_gain
        // We keep full range waveform data to scale it on paint
        m_waveformSourceImage = QImage(dataSize / 2, 2 * 255,
                QImage::Format_ARGB32_Premultiplied);
        m_waveformSourceImage.fill(QColor(0, 0, 0, 0).value());
    }

    // Always multiple of 2
    const int waveformCompletion = pWaveform->getCompletion();
    // Test if there is some new to draw (at least of pixel width)
    const int completionIncrement = waveformCompletion - m_actualCompletion;

    int visiblePixelIncrement = completionIncrement * length() / dataSize;
    if (waveformCompletion < (dataSize - 2) &&
            (completionIncrement < 2 || visiblePixelIncrement == 0)) {
        return false;
    }

    const int nextCompletion = m_actualCompletion + completionIncrement;

    //qDebug() << "WOverview::drawNextPixmapPart() - nextCompletion:"
    //         << nextCompletion
    //         << "m_actualCompletion:" << m_actualCompletion
    //         << "waveformCompletion:" << waveformCompletion
    //         << "completionIncrement:" << completionIncrement;


    QPainter painter(&m_waveformSourceImage);
    painter.translate(0.0, static_cast<double>(m_waveformSourceImage.height()) / 2.0);

    QColor lowColor = m_signalColors.getLowColor();
    QPen lowColorPen(QBrush(lowColor), 1);

    QColor midColor = m_signalColors.getMidColor();
    QPen midColorPen(QBrush(midColor), 1);

    QColor highColor = m_signalColors.getHighColor();
    QPen highColorPen(QBrush(highColor), 1);

    for (currentCompletion = m_actualCompletion;
            currentCompletion < nextCompletion; currentCompletion += 2) {
        unsigned char lowNeg = pWaveform->getLow(currentCompletion);
        unsigned char lowPos = pWaveform->getLow(currentCompletion+1);
        if (lowPos || lowNeg) {
            painter.setPen(lowColorPen);
            painter.drawLine(QPoint(currentCompletion / 2, -lowNeg),
                             QPoint(currentCompletion / 2, lowPos));
        }
    }

    for (currentCompletion = m_actualCompletion;
            currentCompletion < nextCompletion; currentCompletion += 2) {
        painter.setPen(midColorPen);
        painter.drawLine(QPoint(currentCompletion / 2,
                -pWaveform->getMid(currentCompletion)),
                QPoint(currentCompletion / 2,
                pWaveform->getMid(currentCompletion+1)));
    }

    for (currentCompletion = m_actualCompletion;
            currentCompletion < nextCompletion; currentCompletion += 2) {
        painter.setPen(highColorPen);
        painter.drawLine(QPoint(currentCompletion / 2,
                -pWaveform->getHigh(currentCompletion)),
                QPoint(currentCompletion / 2,
                pWaveform->getHigh(currentCompletion+1)));
    }

    // Evaluate waveform ratio peak

    for (currentCompletion = m_actualCompletion;
            currentCompletion < nextCompletion; currentCompletion += 2) {
        m_waveformPeak = math_max3(
                m_waveformPeak,
                static_cast<float>(pWaveform->getAll(currentCompletion)),
                static_cast<float>(pWaveform->getAll(currentCompletion + 1)));
    }

    m_actualCompletion = nextCompletion;
    m_waveformImageScaled = QImage();
    m_diffGain = 0;

    // Test if the complete waveform is done
    if (m_actualCompletion >= dataSize - 2) {
        m_pixmapDone = true;
        //qDebug() << "m_waveformPeakRatio" << m_waveformPeak;
    }

    return true;
}
