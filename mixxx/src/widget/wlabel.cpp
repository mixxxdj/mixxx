#include "widget/wlabel.h"

#include <QEvent>
#include <QFont>

#include "moc_wlabel.cpp"
#include "skin/legacy/skincontext.h"
#include "widget/wskincolor.h"

WLabel::WLabel(QWidget* pParent)
        : QLabel(pParent),
          WBaseWidget(this),
          m_skinText(),
          m_longText(),
          m_elideMode(Qt::ElideNone),
          m_scaleFactor(1.0),
          m_highlight(0),
          m_widthHint(0) {
}

void WLabel::setup(const QDomNode& node, const SkinContext& context) {
    m_scaleFactor = context.getScaleFactor();

    // Colors
    QPalette pal = palette(); // we have to copy out the palette to edit it since it's const (probably for threadsafety)

    QDomElement bgColor = context.selectElement(node, "BgColor");
    if (!bgColor.isNull()) {
        m_qBgColor = QColor(context.nodeToString(bgColor));
        pal.setColor(this->backgroundRole(), WSkinColor::getCorrectColor(m_qBgColor));
        setAutoFillBackground(true);
    }

    m_qFgColor = QColor(context.selectString(node, "FgColor"));
    pal.setColor(this->foregroundRole(), WSkinColor::getCorrectColor(m_qFgColor));
    setPalette(pal);

    // Font size
    QString strFontSize;
    if (context.hasNodeSelectString(node, "FontSize", &strFontSize)) {
        bool widthOk = false;
        double dFontSize = strFontSize.toDouble(&widthOk);
        if (widthOk && dFontSize >= 0) {
            QFont fonti = font();
            // We do not scale the font here, because in most cases
            // this is overridden by the style sheet font size
            fonti.setPointSizeF(dFontSize);
            setFont(fonti);
        }
    }

    // Text
    if (context.hasNodeSelectString(node, "Text", &m_skinText)) {
        setText(m_skinText);
    }

    // Alignment
    QString alignment;
    if (context.hasNodeSelectString(node, "Alignment", &alignment)) {
        alignment = alignment.toLower();
        if (alignment == "right") {
            setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        } else if (alignment == "center") {
            setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        } else if (alignment == "left") {
            setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        } else {
            qDebug() << "WLabel::setup(): Alignment =" << alignment <<
                    " unknown, use right, center or left";
        }
    }

    // Adds an ellipsis to truncated text
    QString elide;
    if (context.hasNodeSelectString(node, "Elide", &elide)) {
        elide = elide.toLower();
        if (elide == "right") {
            m_elideMode = Qt::ElideRight;
        } else if (elide == "middle") {
            m_elideMode = Qt::ElideMiddle;
        } else if (elide == "left") {
            m_elideMode = Qt::ElideLeft;
        } else if (elide == "none") {
            m_elideMode = Qt::ElideNone;
        } else {
            qDebug() << "WLabel::setup(): Elide =" << elide <<
                    "unknown, use right, middle, left or none.";
        }
    }
}

QString WLabel::text() const {
    return m_longText;
}

void WLabel::setText(const QString& text) {
    m_longText = text;
    if (m_elideMode != Qt::ElideNone) {
        QFontMetrics metrics(font());
        // Measure the text for the optimum label width
        // frameWidth() is the maximum of the sum of margin, border and padding
        // width of the left and the right side.
        m_widthHint = metrics.size(0, m_longText).width() + 2 * frameWidth();
        QString elidedText = metrics.elidedText(
                m_longText, m_elideMode, width() - 2 * frameWidth());
        QLabel::setText(elidedText);
    } else {
        QLabel::setText(m_longText);
    }
}

bool WLabel::event(QEvent* pEvent) {
    if (pEvent->type() == QEvent::ToolTip) {
        updateTooltip();
    } else if (pEvent->type() == QEvent::FontChange) {
        const QFont& fonti = font();
        // Change the new font on the fly by casting away its constancy
        // using setFont() here, would results into a recursive loop
        // resetting the font to the original css values.
        // Only scale pixel size fonts, point size fonts are scaled by the OS
        if (fonti.pixelSize() > 0) {
            const_cast<QFont&>(fonti).setPixelSize(
                    static_cast<int>(fonti.pixelSize() * m_scaleFactor));
        }
        // measure text with the new font
        setText(m_longText);
    }
    return QLabel::event(pEvent);
}

void WLabel::resizeEvent(QResizeEvent* event) {
    QLabel::resizeEvent(event);
    setText(m_longText);
}

void WLabel::fillDebugTooltip(QStringList* debug) {
    WBaseWidget::fillDebugTooltip(debug);
    *debug << QString("Text: \"%1\"").arg(text());
}

int WLabel::getHighlight() const {
    return m_highlight;
}

void WLabel::setHighlight(int highlight) {
    if (m_highlight == highlight) {
        return;
    }
    m_highlight = highlight;
    emit highlightChanged(m_highlight);
}

QSize WLabel::sizeHint() const {
    // make sure the sizeHint fits for the entire string.
    QSize size = QLabel::sizeHint();
    if (m_elideMode != Qt::ElideNone) {
        size.setWidth(m_widthHint);
    }
    return size;
}
