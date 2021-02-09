#include "dialog/dlgkeywheel.h"

#include <QDir>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QSvgRenderer>

#include "control/controlobject.h"

using namespace mixxx::track::io::key;

namespace {
const auto kKeywheelSVG = QStringLiteral(":/images/keywheel/keywheel.svg");
const KeyUtils::KeyNotation kNotationHidden[]{
        KeyUtils::KeyNotation::OpenKeyAndTraditional,
        KeyUtils::KeyNotation::LancelotAndTraditional};
} // namespace

DlgKeywheel::DlgKeywheel(QWidget* parent, const UserSettingsPointer& pConfig)
        : QDialog(parent),
          m_pConfig(pConfig) {
    setupUi(this);
    QDir resourceDir(m_pConfig->getResourcePath());
    auto svgPath = resourceDir.filePath(kKeywheelSVG);

    QFile xmlFile(svgPath);
    if (!xmlFile.exists() || !xmlFile.open(QFile::ReadOnly | QFile::Text)) {
        qWarning() << "Could not load svg template: " << svgPath;
        return;
    }
    m_domDocument.setContent(&xmlFile);

    installEventFilter(this);
    graphic->installEventFilter(this);
    graphic->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    graphic->setMinimumSize(200, 200);

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    graphic->renderer()->setAspectRatioMode(Qt::KeepAspectRatio);
#endif

    connect(closeButton,
            &QAbstractButton::clicked,
            this,
            &QDialog::accept);

    // load the user configured setting as default
    const int notation = static_cast<int>(ControlObject::get(
            ConfigKey("[Library]", "key_notation")));
    m_notation = static_cast<KeyUtils::KeyNotation>(notation);
    // Display the current or next valid notation
    switchNotation(0);
}

bool DlgKeywheel::eventFilter(QObject* obj, QEvent* event) {
    if (event->type() == QEvent::KeyPress) {
        // we handle TAB + Shift TAB to cycle through the notations
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Tab) {
            switchNotation(+1);
            return true;
        } else if (keyEvent->key() == Qt::Key_Backtab) {
            switchNotation(-1);
            return true;
        }
    } else if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        // first button forward, other buttons backward cycle
        switchNotation(mouseEvent->button() == Qt::LeftButton ? +1 : -1);
        return true;
    }
    // standard event processing
    return QDialog::eventFilter(obj, event);
}

void DlgKeywheel::show() {
    QDialog::show();
    // FIXME(XXX) this is a very ugly workaround that the svg graphics gets its
    // correct form. The SVG seems only to be scaled correctly after it has been shown
    if (!m_resized) {
        resize(height() - 1, width() - 1);
        m_resized = true;
    }
}

void DlgKeywheel::resizeEvent(QResizeEvent* ev) {
    QSize newSize = ev->size();
    int size = qMin(graphic->size().height(), graphic->size().width());
    newSize = QSize(size, size);

    graphic->resize(newSize);
    updateGeometry();
    QDialog::resizeEvent(ev);
}

bool DlgKeywheel::isHiddenNotation(KeyUtils::KeyNotation notation) {
    for (KeyUtils::KeyNotation hidden : kNotationHidden) {
        if (hidden == notation) {
            return true;
        }
    }
    return false;
}

void DlgKeywheel::switchNotation(int step) {
    const int invalidNotation = static_cast<int>(KeyUtils::KeyNotation::Invalid);
    const int numNotation = static_cast<int>(KeyUtils::KeyNotation::NumKeyNotations);

    int newNotation = static_cast<int>(m_notation) + step;

    // use default step forward in case of invalid direction.
    // This only takes affects if the already selected new notation is invalid.
    if (step == 0) {
        step = 1;
    }
    // we skip variants with redundant information
    while (newNotation <= invalidNotation ||
            newNotation >= numNotation ||
            isHiddenNotation(static_cast<KeyUtils::KeyNotation>(newNotation))) {
        newNotation = newNotation + step;
        if (newNotation >= numNotation) {
            newNotation = invalidNotation + 1;
        } else if (newNotation <= invalidNotation) {
            newNotation = numNotation - 1;
        }
    }
    m_notation = static_cast<KeyUtils::KeyNotation>(newNotation);
    // we update the SVG nodes with the new value
    updateSvg();
}

void DlgKeywheel::updateSvg() {
    // update the svg with new values to display, then issue a an update on the widget
    QDomElement topElement = m_domDocument.documentElement();

    bool hideTraditional = m_notation == KeyUtils::KeyNotation::Traditional;
    QDomElement domElement;

    QDomNodeList nodeList = m_domDocument.elementsByTagName(QStringLiteral("text"));
    for (int i = 0; i < nodeList.count(); i++) {
        QDomNode node = nodeList.at(i);
        domElement = node.toElement();
        QString id = domElement.attribute("id", "UNKNOWN");

        if (id.startsWith("t_")) {
            domElement.setAttribute("visibility",
                    hideTraditional ? "hidden" : "visible");
        } else if (id.startsWith("k_")) {
            // we identify the text node by the id und use the suffix to lookup the
            // chromakey id
            // in this svg the text is inside a span which we need to look up first
            QDomNode tspan = node.firstChild();
            QDomNode text = tspan.firstChild();

            if (text.isText()) {
                QDomText textNode = text.toText();
                ChromaticKey key = static_cast<ChromaticKey>(id.midRef(2).toInt());
                QString keyString = KeyUtils::keyToString(key, m_notation);
                textNode.setData(keyString);
            }
        }
    }

    QString str;
    QTextStream stream(&str);

    m_domDocument.save(stream, QDomNode::EncodingFromDocument);

    // toUtf8 creates a ByteArray which is loaded as content. QString would be a filename
    graphic->load(str.toUtf8());
}
