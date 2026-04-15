#include "waveformmarkrange.h"

#include <QPainter>
#include <QtDebug>

#include "skin/legacy/skincontext.h"
#include "waveformsignalcolors.h"
#include "widget/wskincolor.h"

WaveformMarkRange::WaveformMarkRange(
        const QString& group,
        const QDomNode& node,
        const SkinContext& context,
        const WaveformSignalColors& signalColors)
        : m_activeColor(context.selectString(node, "Color")),
          m_disabledColor(context.selectString(node, "DisabledColor")),
          m_enabledOpacity(context.selectDouble(node, "Opacity", 0.5)),
          m_disabledOpacity(context.selectDouble(node, "DisabledOpacity", 0.5)),
          m_durationTextColor(context.selectString(node, "DurationTextColor")) {
    QString startControl = context.selectString(node, "StartControl");
    if (!startControl.isEmpty()) {
        DEBUG_ASSERT(!m_markStartPointControl); // has not been created yet
        m_markStartPointControl = std::make_unique<ControlProxy>(group, startControl);
    }
    QString endControl = context.selectString(node, "EndControl");
    if (!endControl.isEmpty()) {
        DEBUG_ASSERT(!m_markEndPointControl); // has not been created yet
        m_markEndPointControl = std::make_unique<ControlProxy>(group, endControl);
    }

    QString enabledControl = context.selectString(node, "EnabledControl");
    if (!enabledControl.isEmpty()) {
        DEBUG_ASSERT(!m_markEnabledControl); // has not been created yet
        m_markEnabledControl = std::make_unique<ControlProxy>(group, enabledControl);
    }
    QString visibilityControl = context.selectString(node, "VisibilityControl");
    if (!visibilityControl.isEmpty()) {
        DEBUG_ASSERT(!m_markVisibleControl); // has not been created yet
        ConfigKey key = ConfigKey::parseCommaSeparated(visibilityControl);
        m_markVisibleControl = std::make_unique<ControlProxy>(key);
    }

    QString durationTextLocation = context.selectString(node, "DurationTextLocation");
    if (durationTextLocation == "before") {
        m_durationTextLocation = DurationTextLocation::Before;
    } else {
        m_durationTextLocation = DurationTextLocation::After;
    }

    if (!m_activeColor.isValid()) {
        //vRince kind of legacy fallback ...
        // As a fallback, grab the mark color from the parent's MarkerColor
        QString rangeSuffix = QStringLiteral("_start_position");
        QString rangeName = startControl.remove(rangeSuffix);
        m_activeColor = signalColors.getAxesColor();
        qDebug() << "Didn't get Color for mark range" << rangeName
                << "- using parent's AxesColor:" << m_activeColor;
    } else {
        m_activeColor = WSkinColor::getCorrectColor(m_activeColor);
    }

    if (!m_disabledColor.isValid()) {
        if (enabledControl.isEmpty()) {
            m_disabledColor = QColor(Qt::transparent);
        } else {
            // Show warning only when there's no EnabledControl,
            // like for intro & outro ranges.
            QString rangeSuffix = QStringLiteral("_start_position");
            QString rangeName = startControl.remove(rangeSuffix);
            int gray = qGray(m_activeColor.rgb());
            m_disabledColor = QColor(gray, gray, gray);
            qDebug() << "Didn't get DisabledColor for mark range" << rangeName
                    << "- using desaturated Color:" << m_disabledColor;
        }
    }
}

WaveformMarkRange::WaveformMarkRange(
        const QString& group,
        const QColor& activeColor,
        const QColor& disabledColor,
        double enabledOpacity,
        double disabledOpacity,
        const QColor& durationTextColor,
        const QString& startControl,
        const QString& endControl,
        const QString& enabledControl,
        const QString& visibilityControl,
        const QString& durationTextLocation)
        : m_activeColor(activeColor),
          m_disabledColor(disabledColor),
          m_enabledOpacity(enabledOpacity),
          m_disabledOpacity(disabledOpacity),
          m_durationTextColor(durationTextColor) {
    if (!startControl.isEmpty()) {
        DEBUG_ASSERT(!m_markStartPointControl); // has not been created yet
        m_markStartPointControl = std::make_unique<ControlProxy>(group, startControl);
    }
    if (!endControl.isEmpty()) {
        DEBUG_ASSERT(!m_markEndPointControl); // has not been created yet
        m_markEndPointControl = std::make_unique<ControlProxy>(group, endControl);
    }

    if (!enabledControl.isEmpty()) {
        DEBUG_ASSERT(!m_markEnabledControl); // has not been created yet
        m_markEnabledControl = std::make_unique<ControlProxy>(group, enabledControl);
    }
    if (!visibilityControl.isEmpty()) {
        DEBUG_ASSERT(!m_markVisibleControl); // has not been created yet
        ConfigKey key = ConfigKey::parseCommaSeparated(visibilityControl);
        m_markVisibleControl = std::make_unique<ControlProxy>(key);
    }

    if (durationTextLocation == "before") {
        m_durationTextLocation = DurationTextLocation::Before;
    } else {
        m_durationTextLocation = DurationTextLocation::After;
    }

    m_activeColor = WSkinColor::getCorrectColor(m_activeColor);

    if (!m_disabledColor.isValid()) {
        if (enabledControl.isEmpty()) {
            m_disabledColor = QColor(Qt::transparent);
        } else {
            // Show warning only when there's no EnabledControl,
            // like for intro & outro ranges.
            constexpr static QStringView rangeSuffix = u"_start_position";
            QStringView rangeName = startControl;
            if (startControl.endsWith(rangeSuffix)) {
                rangeName.chop(rangeSuffix.length());
            }
            int gray = qGray(m_activeColor.rgb());
            m_disabledColor = QColor(gray, gray, gray);
            qDebug() << "Didn't get DisabledColor for mark range" << rangeName
                     << "- using desaturated Color:" << m_disabledColor;
        }
    }
}

bool WaveformMarkRange::active() const {
    const double startValue = start();
    const double endValue = end();
    return startValue != endValue && startValue != -1.0 && endValue != -1.0;
}

bool WaveformMarkRange::enabled() const {
    // Default to enabled if there is no enabled control.
    return !m_markEnabledControl || !m_markEnabledControl->valid() ||
            m_markEnabledControl->get() > 0.0;
}

bool WaveformMarkRange::visible() const {
    // Default to visible if there is no visible control.
    return !m_markVisibleControl || !m_markVisibleControl->valid() ||
            m_markVisibleControl->get() > 0.0;
}

double WaveformMarkRange::start() const {
    double start = -1.0;
    if (m_markStartPointControl && m_markStartPointControl->valid()) {
        start = m_markStartPointControl->get();
    }
    return start;
}

double WaveformMarkRange::end() const {
    double end = -1.0;
    if (m_markEndPointControl && m_markEndPointControl->valid()) {
        end = m_markEndPointControl->get();
    }
    return end;
}

bool WaveformMarkRange::showDuration() const {
    return m_durationTextColor.isValid() && start() != end() && start() != -1 && end() != -1;
}

void WaveformMarkRange::generateImage(int weidth, int height) {
    m_activeImage = QImage(weidth, height, QImage::Format_ARGB32_Premultiplied);
    m_disabledImage = QImage(weidth, height, QImage::Format_ARGB32_Premultiplied);

    // fill needed cause they remain transparent
    m_activeImage.fill(QColor(0,0,0,0).rgba());
    m_disabledImage.fill(QColor(0,0,0,0).rgba());

    QColor activeColor = m_activeColor;
    activeColor.setAlphaF(0.3f);
    QBrush brush(activeColor);

    QPainter painter;
    painter.begin(&m_activeImage);
    painter.fillRect(m_activeImage.rect(), brush);
    painter.end();

    QColor disabledColor = m_disabledColor;
    disabledColor.setAlphaF(0.3f);
    brush = QBrush(disabledColor);

    painter.begin(&m_disabledImage);
    painter.fillRect(m_disabledImage.rect(), brush);
    painter.end();
}
