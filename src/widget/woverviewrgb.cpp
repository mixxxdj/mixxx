#include "widget/woverviewrgb.h"

#include <QPainter>

#include "moc_woverviewrgb.cpp"
#include "util/colorcomponents.h"
#include "util/math.h"
#include "util/timer.h"
#include "waveform/waveform.h"

WOverviewRGB::WOverviewRGB(
        const QString& group,
        PlayerManager* pPlayerManager,
        UserSettingsPointer pConfig,
        QWidget* parent)
        : WOverview(group, pPlayerManager, std::move(pConfig), parent) {
}

bool WOverviewRGB::drawNextPixmapPart() {
    ScopedTimer t("WOverviewRGB::drawNextPixmapPart");

    //qDebug() << "WOverview::drawNextPixmapPart()";

    int currentCompletion;

    ConstWaveformPointer pWaveform = getWaveform();
    if (!pWaveform) {
        return false;
    }

    const int dataSize = pWaveform->getDataSize();
    const double audioVisualRatio = pWaveform->getAudioVisualRatio();
    const double trackSamples = getTrackSamples();
    if (dataSize <= 0 || audioVisualRatio <= 0 || trackSamples <= 0) {
        return false;
    }

    if (m_waveformSourceImage.isNull()) {
        // Waveform pixmap twice the height of the viewport to be scalable
        // by total_gain
        // We keep full range waveform data to scale it on paint
        m_waveformSourceImage = QImage(
                static_cast<int>(trackSamples / audioVisualRatio / 2) + 1,
                2 * 255,
                QImage::Format_ARGB32_Premultiplied);
        m_waveformSourceImage.fill(QColor(0, 0, 0, 0).value());
        if (dataSize / 2 != m_waveformSourceImage.width()) {
            qWarning() << "Track duration has changed since last analysis"
                       << m_waveformSourceImage.width() << "!=" << dataSize / 2;
        }
    }
    DEBUG_ASSERT(!m_waveformSourceImage.isNull());

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

    QColor color;

    float lowColor_r, lowColor_g, lowColor_b;
    getRgbF(m_signalColors.getRgbLowColor(), &lowColor_r, &lowColor_g, &lowColor_b);

    float midColor_r, midColor_g, midColor_b;
    getRgbF(m_signalColors.getRgbMidColor(), &midColor_r, &midColor_g, &midColor_b);

    float highColor_r, highColor_g, highColor_b;
    getRgbF(m_signalColors.getRgbHighColor(), &highColor_r, &highColor_g, &highColor_b);

    for (currentCompletion = m_actualCompletion;
            currentCompletion < nextCompletion; currentCompletion += 2) {

        unsigned char left = pWaveform->getAll(currentCompletion);
        unsigned char right = pWaveform->getAll(currentCompletion + 1);

        // Retrieve "raw" LMH values from waveform
        float low = static_cast<float>(pWaveform->getLow(currentCompletion));
        float mid = static_cast<float>(pWaveform->getMid(currentCompletion));
        float high = static_cast<float>(pWaveform->getHigh(currentCompletion));

        // Do matrix multiplication
        float red = low * lowColor_r + mid * midColor_r + high * highColor_r;
        float green = low * lowColor_g + mid * midColor_g + high * highColor_g;
        float blue = low * lowColor_b + mid * midColor_b + high * highColor_b;

        // Normalize and draw
        float max = math_max3(red, green, blue);
        if (max > 0.0) {
            color.setRgbF(red / max, green / max, blue / max);
            painter.setPen(color);
            painter.drawLine(QPointF(currentCompletion / 2, -left),
                    QPointF(currentCompletion / 2, 0));
        }

        // Retrieve "raw" LMH values from waveform
        low = static_cast<float>(pWaveform->getLow(currentCompletion + 1));
        mid = static_cast<float>(pWaveform->getMid(currentCompletion + 1));
        high = static_cast<float>(pWaveform->getHigh(currentCompletion + 1));

        // Do matrix multiplication
        red = low * lowColor_r + mid * midColor_r + high * highColor_r;
        green = low * lowColor_g + mid * midColor_g + high * highColor_g;
        blue = low * lowColor_b + mid * midColor_b + high * highColor_b;

        // Normalize and draw
        max = math_max3(red, green, blue);
        if (max > 0.0) {
            color.setRgbF(red / max, green / max, blue / max);
            painter.setPen(color);
            painter.drawLine(QPointF(currentCompletion / 2, 0),
                    QPointF(currentCompletion / 2, right));
        }
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
