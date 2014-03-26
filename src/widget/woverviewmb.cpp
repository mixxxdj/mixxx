#include "widget/woverviewmb.h"

#include <QPainter>

#include "util/timer.h"

#include "waveform/waveform.h"

WOverviewMB::WOverviewMB(const char* pGroup,
                           ConfigObject<ConfigValue>* pConfig, QWidget* parent)
        : WOverview(pGroup, pConfig, parent)  {
}

bool WOverviewMB::drawNextPixmapPart() {
    ScopedTimer t("WOverviewLMH::drawNextPixmapPart");

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

    for (currentCompletion = m_actualCompletion;
            currentCompletion < nextCompletion; currentCompletion += 2) {


        painter.setPen(detectColor(
                           m_pWaveform->getHigh(currentCompletion),
                           m_pWaveform->getMid(currentCompletion),
                           m_pWaveform->getLow(currentCompletion)));
        painter.drawRect(currentCompletion/2,-m_pWaveformSourceImage->height()/2,2,m_pWaveformSourceImage->height());
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

QColor WOverviewMB::detectColor(unsigned char high, unsigned char mid, unsigned char low){
    double rms = sqrt(pow(high,2)+pow(mid,2)+pow(low,2));
    //restricted the alpha value between 255 and 100
    unsigned char alpha = (unsigned char) (rms < 150)? ((rms < 50)? 100 : ceil(rms)) : 255 ;
    //color strength of 50 is given for all bands.
    high = qMin(high+50,255);
    mid  = qMin(mid+50,255);
    low  = qMin(low+50,255);

    return QColor(low,mid,high,alpha);
}


