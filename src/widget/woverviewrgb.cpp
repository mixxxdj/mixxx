#include "widget/woverviewrgb.h"

#include <QPainter>

#include "util/timer.h"
#include "util/math.h"
#include "waveform/waveform.h"

WOverviewRGB::WOverviewRGB(const char* pGroup,
                           ConfigObject<ConfigValue>* pConfig, QWidget* parent)
        : WOverview(pGroup, pConfig, parent)  {
}

bool WOverviewRGB::drawNextPixmapPart() {
    ScopedTimer t("WOverviewRGB::drawNextPixmapPart");

    //qDebug() << "WOverview::drawNextPixmapPart() - m_waveform" << m_waveform;

    int currentCompletion;

    if (!m_pWaveform) {
        return false;
    }

    const int dataSize = m_pWaveform->getDataSize();
    if (dataSize == 0 ) {
        return false;
    }

    if (!m_pWaveformSourceImage) {
        // Waveform pixmap twice the height of the viewport to be scalable
        // by total_gain
        // We keep full range waveform data to scale it on paint
        m_pWaveformSourceImage = new QImage(dataSize / 2, 2 * 255,
                QImage::Format_ARGB32_Premultiplied);
        m_pWaveformSourceImage->fill(QColor(0,0,0,0).value());
    }

    // Always multiple of 2
    const int waveformCompletion = m_pWaveform->getCompletion();
    // Test if there is some new to draw (at least of pixel width)
    const int completionIncrement = waveformCompletion - m_actualCompletion;

    int visiblePixelIncrement = completionIncrement * width() / dataSize;
    if (completionIncrement < 2 || visiblePixelIncrement == 0) {
        return false;
    }

    if (!m_pWaveform->getMutex()->tryLock()) {
        return false;
    }

    const int nextCompletion = m_actualCompletion + completionIncrement;

    //qDebug() << "WOverview::drawNextPixmapPart() - nextCompletion:"
    //         << nextCompletion
    //         << "m_actualCompletion:" << m_actualCompletion
    //         << "waveformCompletion:" << waveformCompletion
    //         << "completionIncrement:" << completionIncrement;

    QPainter painter(m_pWaveformSourceImage);
    painter.translate(0.0,(double)m_pWaveformSourceImage->height()/2.0);

    QColor color;

    unsigned char low, mid, high;

    // Maximum is needed for normalization
    float max;

    for (currentCompletion = m_actualCompletion;
            currentCompletion < nextCompletion; currentCompletion += 2) {

        unsigned char left = m_pWaveform->getAll(currentCompletion);
        unsigned char right = m_pWaveform->getAll(currentCompletion + 1);

        low = m_pWaveform->getLow(currentCompletion);
        mid = m_pWaveform->getMid(currentCompletion);
        high = m_pWaveform->getHigh(currentCompletion);

        max = (float) math_max3(low, mid, high);
        if (max > 0.0f) {
            color.setRgbF(low / max, mid / max, high / max);
            painter.setPen(color);
            painter.drawLine(currentCompletion / 2, -left, currentCompletion / 2, 0);
        }

        low = m_pWaveform->getLow(currentCompletion + 1);
        mid = m_pWaveform->getMid(currentCompletion + 1);
        high = m_pWaveform->getHigh(currentCompletion + 1);

        max = (float) math_max3(low, mid, high);
        if (max > 0.0f) {
            color.setRgbF(low / max, mid / max, high / max);
            painter.setPen(color);
            painter.drawLine(currentCompletion / 2, 0, currentCompletion / 2, right);
        }
    }

    // Evaluate waveform ratio peak
    for (currentCompletion = m_actualCompletion;
            currentCompletion < nextCompletion; currentCompletion += 2) {
        m_waveformPeak = math_max(m_waveformPeak,
                (float)m_pWaveform->getAll(currentCompletion));
        m_waveformPeak = math_max(m_waveformPeak,
                (float)m_pWaveform->getAll(currentCompletion+1));
    }

    m_actualCompletion = nextCompletion;
    m_waveformImageScaled = QImage();
    m_diffGain = 0;

    // Test if the complete waveform is done
    if (m_actualCompletion >= dataSize - 2) {
        m_pixmapDone = true;
        //qDebug() << "m_waveformPeakRatio" << m_waveformPeak;
    }

    m_pWaveform->getMutex()->unlock();
    return true;
}
