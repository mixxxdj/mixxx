#include "widget/woverviewhsv.h"

#include <QPainter>
#include <QColor>

#include "util/timer.h"
#include "util/math.h"
#include "waveform/waveform.h"

WOverviewHSV::WOverviewHSV(const char* pGroup,
                           ConfigObject<ConfigValue>* pConfig, QWidget* parent)
        : WOverview(pGroup, pConfig, parent)  {
}

bool WOverviewHSV::drawNextPixmapPart() {
    ScopedTimer t("WOverviewHSV::drawNextPixmapPart");

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
    // << nextCompletion
    // << "m_actualCompletion:" << m_actualCompletion
    // << "waveformCompletion:" << waveformCompletion
    // << "completionIncrement:" << completionIncrement;


    QPainter painter(m_pWaveformSourceImage);
    painter.translate(0.0,(double)m_pWaveformSourceImage->height()/2.0);

    // Get HSV of low color. NOTE(rryan): On ARM, qreal is float so it's
    // important we use qreal here and not double or float or else we will get
    // build failures on ARM.
    qreal h, s, v;
    m_signalColors.getLowColor().getHsvF(&h, &s, &v);

    QColor color;
    float lo, hi, total;

    unsigned char maxLow[2] = {0, 0};
    unsigned char maxHigh[2] = {0, 0};
    unsigned char maxMid[2] = {0, 0};
    unsigned char maxAll[2] = {0, 0};

    for (currentCompletion = m_actualCompletion;
            currentCompletion < nextCompletion; currentCompletion += 2) {
        maxAll[0] = m_pWaveform->getAll(currentCompletion);
        maxAll[1] = m_pWaveform->getAll(currentCompletion+1);
        if (maxAll[0] || maxAll[1]) {
            maxLow[0] = m_pWaveform->getLow(currentCompletion);
            maxLow[1] = m_pWaveform->getLow(currentCompletion+1);
            maxMid[0] = m_pWaveform->getMid(currentCompletion);
            maxMid[1] = m_pWaveform->getMid(currentCompletion+1);
            maxHigh[0] = m_pWaveform->getHigh(currentCompletion);
            maxHigh[1] = m_pWaveform->getHigh(currentCompletion+1);

            total = (maxLow[0] + maxLow[1] + maxMid[0] + maxMid[1] +
                     maxHigh[0] + maxHigh[1]) * 1.2;

            // Prevent division by zero
            if( total > 0 )
            {
                // Normalize low and high
                // (mid not need, because it not change the color)
                lo = (maxLow[0] + maxLow[1]) / total;
                hi = (maxHigh[0] + maxHigh[1]) / total;
            }
            else
                lo = hi = 0.0;

            // Set color
            color.setHsvF(h, 1.0-hi, 1.0-lo);

            painter.setPen(color);
            painter.drawLine(QPoint(currentCompletion / 2, -maxAll[0]),
                    QPoint(currentCompletion / 2, maxAll[1]));
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
