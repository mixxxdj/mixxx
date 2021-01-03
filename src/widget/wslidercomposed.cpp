#include "widget/wslidercomposed.h"

#include <QStyleOption>
#include <QStylePainter>
#include <QtDebug>

#include "moc_wslidercomposed.cpp"
#include "util/debug.h"
#include "util/duration.h"
#include "util/math.h"
#include "widget/controlwidgetconnection.h"
#include "widget/wpixmapstore.h"
#include "widget/wskincolor.h"

WSliderComposed::WSliderComposed(QWidget * parent)
    : WWidget(parent),
      m_dHandleLength(0.0),
      m_dSliderLength(0.0),
      m_bHorizontal(false),
      m_dBarWidth(0.0),
      m_dBarBgWidth(0.0),
      m_dBarStart(0.0),
      m_dBarEnd(0.0),
      m_dBarBgStart(0.0),
      m_dBarBgEnd(0.0),
      m_dBarAxisPos(0.0),
      m_bBarUnipolar(true),
      m_barColor(nullptr),
      m_barBgColor(nullptr),
      m_barPenCap(Qt::FlatCap),
      m_pSlider(nullptr),
      m_pHandle(nullptr),
      m_renderTimer(mixxx::Duration::fromMillis(20),
                    mixxx::Duration::fromSeconds(1)) {
    connect(&m_renderTimer,
            &WidgetRenderTimer::update,
            this,
            QOverload<>::of(&QWidget::update));
}

WSliderComposed::~WSliderComposed() {
    unsetPixmaps();
}

void WSliderComposed::setup(const QDomNode& node, const SkinContext& context) {
    // Setup pixmaps
    unsetPixmaps();

    double scaleFactor = context.getScaleFactor();
    QDomElement slider = context.selectElement(node, "Slider");
    if (!slider.isNull()) {
        // The implicit default in <1.12.0 was FIXED so we keep it for backwards
        // compatibility.
        PixmapSource sourceSlider = context.getPixmapSource(slider);
        setSliderPixmap(
                sourceSlider,
                context.selectScaleMode(slider, Paintable::FIXED),
                scaleFactor);
    }

    m_dSliderLength = m_bHorizontal ? width() : height();
    m_handler.setSliderLength(m_dSliderLength);

    QDomElement handle = context.selectElement(node, "Handle");
    PixmapSource sourceHandle = context.getPixmapSource(handle);
    bool h = context.selectBool(node, "Horizontal", false);
    // The implicit default in <1.12.0 was FIXED so we keep it for backwards
    // compatibility.
    setHandlePixmap(h, sourceHandle,
                    context.selectScaleMode(handle, Paintable::FIXED),
                    scaleFactor);

    // Set up the level bar.
    QColor barColor = context.selectColor(node, "BarColor");
    context.hasNodeSelectDouble(node, "BarWidth", &m_dBarWidth);
    if (barColor.isValid() && m_dBarWidth > 0.0) {
        m_barColor = WSkinColor::getCorrectColor(barColor);
        m_dBarWidth *= scaleFactor;
        QString margins;
        QString bgMargins;
        if (context.hasNodeSelectString(node, "BarMargins", &margins)) {
            int comma = margins.indexOf(",");
            bool m1ok;
            bool m2ok;
            double m1 = (margins.leftRef(comma)).toDouble(&m1ok);
            double m2 = (margins.midRef(comma + 1)).toDouble(&m2ok);
            if (m1ok && m2ok) {
                m_dBarStart = m1 * scaleFactor;
                m_dBarEnd = m2 * scaleFactor;
            }
        }

        // Set up the bar background if there's a valid color set.
        // If the background width and margins are not set explicitly
        // we simply adopt the settings of the level bar.
        QColor barBgColor = context.selectColor(node, "BarBgColor");
        if (barBgColor.isValid()) {
            m_barBgColor = WSkinColor::getCorrectColor(barBgColor);
            if (context.hasNodeSelectDouble(node, "BarBgWidth", &m_dBarBgWidth)) {
                if (m_dBarBgWidth > 0.0) {
                    m_dBarBgWidth *= scaleFactor;
                }
            } else {
                m_dBarBgWidth = m_dBarWidth;
            }
            if (context.hasNodeSelectString(node, "BarBgMargins", &bgMargins)) {
                int comma = bgMargins.indexOf(",");
                bool m1ok;
                bool m2ok;
                double m1 = (bgMargins.leftRef(comma)).toDouble(&m1ok);
                double m2 = (bgMargins.midRef(comma + 1)).toDouble(&m2ok);
                if (m1ok && m2ok) {
                    m_dBarBgStart = m1 * scaleFactor;
                    m_dBarBgEnd = m2 * scaleFactor;
                }
            } else {
                m_dBarBgStart = m_dBarStart;
                m_dBarBgEnd = m_dBarEnd;
            }
        }
        // Shift the bar center line to the right or to the bottom (horizontal sliders)
        if (context.hasNodeSelectDouble(node, "BarAxisPos", &m_dBarAxisPos)) {
            m_dBarAxisPos *= scaleFactor;
        }
        // Draw the bar from 0 by default, from bottom or from left (horizontal)
        m_bBarUnipolar = context.selectBool(node, "BarUnipolar", true);
        if (context.selectBool(node, "BarRoundCaps", false)) {
            m_barPenCap = Qt::RoundCap;
        }
    }

    QString eventWhileDrag;
    if (context.hasNodeSelectString(node, "EventWhileDrag", &eventWhileDrag)) {
        if (eventWhileDrag.contains("no")) {
            m_handler.setEventWhileDrag(false);
        }
    }
    if (!m_connections.isEmpty()) {
        ControlParameterWidgetConnection* defaultConnection = m_connections.at(0);
        if (defaultConnection) {
            if (defaultConnection->getEmitOption() &
                    ControlParameterWidgetConnection::EMIT_DEFAULT) {
                // ON_PRESS means here value change on mouse move during press
                defaultConnection->setEmitOption(
                        ControlParameterWidgetConnection::EMIT_ON_PRESS_AND_RELEASE);
            }
        }
    }

    setFocusPolicy(Qt::NoFocus);
}

void WSliderComposed::setSliderPixmap(const PixmapSource& sourceSlider,
        Paintable::DrawMode drawMode,
        double scaleFactor) {
    m_pSlider = WPixmapStore::getPaintable(sourceSlider, drawMode, scaleFactor);
    if (!m_pSlider) {
        qDebug() << "WSliderComposed: Error loading slider pixmap:" << sourceSlider.getPath();
    } else if (drawMode == Paintable::FIXED) {
        // Set size of widget, using size of slider pixmap
        setFixedSize(m_pSlider->size());
    }
}

void WSliderComposed::setHandlePixmap(bool bHorizontal,
        const PixmapSource& sourceHandle,
        Paintable::DrawMode mode,
        double scaleFactor) {
    m_bHorizontal = bHorizontal;
    m_handler.setHorizontal(m_bHorizontal);
    m_pHandle = WPixmapStore::getPaintable(sourceHandle, mode, scaleFactor);
    m_dHandleLength = calculateHandleLength();
    m_handler.setHandleLength(m_dHandleLength);
    if (!m_pHandle) {
        qDebug() << "WSliderComposed: Error loading handle pixmap:" << sourceHandle.getPath();
    } else {
        // Value is unused in WSliderComposed.
        onConnectedControlChanged(getControlParameter(), 0);
        update();
    }
}

void WSliderComposed::unsetPixmaps() {
    m_pSlider.clear();
    m_pHandle.clear();
}

void WSliderComposed::mouseMoveEvent(QMouseEvent * e) {
    m_handler.mouseMoveEvent(this, e);
}

void WSliderComposed::wheelEvent(QWheelEvent *e) {
    m_handler.wheelEvent(this, e);
}

void WSliderComposed::mouseReleaseEvent(QMouseEvent * e) {
    m_handler.mouseReleaseEvent(this, e);
}

void WSliderComposed::mousePressEvent(QMouseEvent * e) {
    m_handler.mousePressEvent(this, e);
}

void WSliderComposed::paintEvent(QPaintEvent * /*unused*/) {
    QStyleOption option;
    option.initFrom(this);
    QStylePainter p(this);
    p.drawPrimitive(QStyle::PE_Widget, option);

    if (!m_pSlider.isNull() && !m_pSlider->isNull()) {
        m_pSlider->draw(rect(), &p);
    }

    // Draw level bar underneath handle
    if (m_barColor.isValid() && m_dBarWidth > 0.0) {
        drawBar(&p);
    }

    if (!m_pHandle.isNull() && !m_pHandle->isNull()) {
        // Slider position rounded, verify this for HiDPI : bug 1479037
        double drawPos = round(m_handler.parameterToPosition(getControlParameterDisplay()));
        QRectF targetRect;
        if (m_bHorizontal) {
            // The handle's draw mode determines whether it is stretched.
            targetRect = QRectF(drawPos, 0, m_dHandleLength, height());
        } else {
            // The handle's draw mode determines whether it is stretched.
            targetRect = QRectF(0, drawPos, width(), m_dHandleLength);
        }
        m_pHandle->draw(targetRect, &p);
    }
}

void WSliderComposed::drawBar(QPainter* pPainter) {
    double x1;
    double x2;
    double y1;
    double y2;
    double value;

    // Draw bar background
    if (m_dBarBgWidth > 0.0) {
        QPen barBgPen = QPen(m_barBgColor);
        barBgPen.setWidthF(m_dBarBgWidth);
        barBgPen.setCapStyle(m_barPenCap);
        pPainter->setPen(barBgPen);
        QLineF barBg;
        if (m_bHorizontal) {
            barBg = QLineF(m_dBarBgStart, m_dBarAxisPos,
                    width() - m_dBarBgEnd, m_dBarAxisPos);
        } else {
            barBg = QLineF(m_dBarAxisPos, height() - m_dBarBgEnd,
                    m_dBarAxisPos, m_dBarBgStart);
        }
        pPainter->drawLine(barBg);
    }

    QPen barPen = QPen(m_barColor);
    barPen.setWidthF(m_dBarWidth);
    barPen.setCapStyle(m_barPenCap);
    pPainter->setPen(barPen);

    if (m_bHorizontal) {
        // Left to right increases the parameter
        value = getControlParameterDisplay();
        if (m_bBarUnipolar) {
            // draw from the left
            x1 = m_dBarStart;
        } else {
            // draw from center
            x1 = m_dBarStart + (width() - m_dBarStart -m_dBarEnd) / 2;
        }
        x2 = m_dBarStart + value * (width() - m_dBarStart - m_dBarEnd);
        y1 = m_dBarAxisPos;
        y2 = y1;
    } else { // vertical slider
        // Sliders usually increase parameters when moved UP, but pixels
        // are count top to bottom, so we flip the scale
        value = 1.0 - getControlParameterDisplay();
        x1 = m_dBarAxisPos;
        x2 = x1;
        if (m_bBarUnipolar) {
            // draw from bottom
            y1 = height() - m_dBarEnd;
        } else {
            // draw from center
            y1 = m_dBarEnd + (height() - m_dBarStart - m_dBarEnd) / 2;
        }
        y2 = m_dBarStart + value * (height() - m_dBarStart - m_dBarEnd);
    }
    pPainter->drawLine(QLineF(x1, y1, x2, y2));
}

void WSliderComposed::resizeEvent(QResizeEvent* pEvent) {
    Q_UNUSED(pEvent);

    m_dHandleLength = calculateHandleLength();
    m_handler.setHandleLength(m_dHandleLength);
    m_dSliderLength = m_bHorizontal ? width() : height();
    m_handler.setSliderLength(m_dSliderLength);
    m_handler.resizeEvent(this, pEvent);

    // Re-calculate state based on our new width/height.
    onConnectedControlChanged(getControlParameter(), 0);
}

void WSliderComposed::onConnectedControlChanged(double dParameter, double /*dValue*/) {
    m_handler.onConnectedControlChanged(this, dParameter);
}

void WSliderComposed::fillDebugTooltip(QStringList* debug) {
    WWidget::fillDebugTooltip(debug);
    int sliderLength = m_bHorizontal ? width() : height();
    *debug << QString("Horizontal: %1").arg(toDebugString(m_bHorizontal))
           << QString("SliderPosition: %1").arg(
                   m_handler.parameterToPosition(getControlParameterDisplay()))
           << QString("SliderLength: %1").arg(sliderLength)
           << QString("HandleLength: %1").arg(m_dHandleLength);
}

double WSliderComposed::calculateHandleLength() {
    if (m_pHandle) {
        Paintable::DrawMode mode = m_pHandle->drawMode();
        if (m_bHorizontal) {
            // Stretch the pixmap to be the height of the widget.
            if (mode == Paintable::FIXED || mode == Paintable::STRETCH ||
                    mode == Paintable::TILE || m_pHandle->height() == 0.0) {
                return m_pHandle->width();
            } else if (mode == Paintable::STRETCH_ASPECT) {
                const int iHeight = m_pHandle->height();
                if (iHeight == 0) {
                  qDebug() << "WSliderComposed: Invalid height.";
                  return 0.0;
                }
                const qreal aspect =
                  static_cast<qreal>(m_pHandle->width()) / iHeight;
                return aspect * height();
            }
        } else {
            // Stretch the pixmap to be the width of the widget.
            if (mode == Paintable::FIXED || mode == Paintable::STRETCH ||
                    mode == Paintable::TILE || m_pHandle->width() == 0.0) {
                return m_pHandle->height();
            } else if (mode == Paintable::STRETCH_ASPECT) {
                const int iWidth = m_pHandle->width();
                if (iWidth == 0) {
                  qDebug() << "WSliderComposed: Invalid width.";
                  return 0.0;
                }
                const qreal aspect =
                  static_cast<qreal>(m_pHandle->height()) / iWidth;
                return aspect * width();
            }
        }
    }
    return 0;
}

void WSliderComposed::inputActivity() {
#ifdef __APPLE__
    m_renderTimer.activity();
#else
    update();
#endif
}
