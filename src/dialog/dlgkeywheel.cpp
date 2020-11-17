#include "dialog/dlgkeywheel.h"

#include <QDir>
#include <QKeyEvent>
#include <QMouseEvent>

#include "control/controlobject.h"

using namespace mixxx::track::io::key;

namespace {
auto kKeywheelSVG = QStringLiteral("images/keywheel/keywheel.svg");
const KeyUtils::KeyNotation kNotationHidden[]{
        KeyUtils::KeyNotation::OpenKeyAndTraditional,
        KeyUtils::KeyNotation::LancelotAndTraditional};
} // namespace

DlgKeywheel::DlgKeywheel(QWidget* parent, UserSettingsPointer pConfig)
        : QDialog(parent),
          ui(new Ui::DlgKeywheel),
          m_pConfig(pConfig) {
    ui->setupUi(this);
    QDir resourceDir(m_pConfig->getResourcePath());
    auto svgPath = resourceDir.filePath(kKeywheelSVG);
    qDebug() << svgPath;
    QFile xmlFile(svgPath);
    if (!xmlFile.exists() || !xmlFile.open(QFile::ReadOnly | QFile::Text)) {
        qDebug() << "Could not load template: " << svgPath;
        return;
    }
    m_domDocument.setContent(&xmlFile);

    // FIXME(poelzi): try to prevent aspect ratio. This fails unfortunatelly
    QSizePolicy qsp = ui->graphic->sizePolicy(); //.setHeightForWidth(true);
    qsp.setHeightForWidth(true);
    ui->graphic->setSizePolicy(qsp);
    installEventFilter(this);
    ui->graphic->installEventFilter(this);

    // load the user configured setting as default
    m_notation = static_cast<KeyUtils::KeyNotation>(ControlObject::get(
            ConfigKey("[Library]", "key_notation")));
    // Select a valid display and update
    switchDisplay(0);
}

bool DlgKeywheel::eventFilter(QObject* obj, QEvent* event) {
    if (event->type() == QEvent::KeyPress) {
        // we handle TAB + Shift TAB to cycle through the display types
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Tab) {
            switchDisplay(+1);
            return true;
        } else if (keyEvent->key() == Qt::Key_Backtab) {
            switchDisplay(-1);
            return true;
        }
    } else if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        // first button forward, other buttons backward cycle
        switchDisplay(mouseEvent->button() == Qt::LeftButton ? +1 : -1);
        return true;
    }
    // standard event processing
    return QObject::eventFilter(obj, event);
}

void DlgKeywheel::resizeEvent(QResizeEvent* ev) {
    QSize newSize = ev->size();
    int size = qMin(ui->graphic->size().height(), ui->graphic->size().width());
    newSize = QSize(size, size);

    ui->graphic->resize(newSize);
}

bool DlgKeywheel::isHiddenNotation(KeyUtils::KeyNotation notation) {
    for (size_t i = 0; i < sizeof(kNotationHidden) / sizeof(kNotationHidden[0]); i++) {
        if (kNotationHidden[i] == notation) {
            return true;
        }
    }
    return false;
}

void DlgKeywheel::switchDisplay(int step) {
    const int invalidNotations = static_cast<int>(KeyUtils::KeyNotation::Invalid);
    const int numNotations = static_cast<int>(KeyUtils::KeyNotation::NumKeyNotations);

    int newNotation = static_cast<int>(m_notation) + step;

    if (step == 0) {
        step = 1;
    }
    // we skip variants with redundant information
    while (newNotation <= invalidNotations ||
            newNotation >= numNotations ||
            isHiddenNotation(static_cast<KeyUtils::KeyNotation>(newNotation))) {
        newNotation = newNotation + step;
        if (newNotation >= numNotations) {
            newNotation = invalidNotations + 1;
        } else if (newNotation <= invalidNotations) {
            newNotation = numNotations - 1;
        }
    }
    m_notation = static_cast<KeyUtils::KeyNotation>(newNotation);
    // we update the SVG nodes with the new value
    updateSvg();
}

void DlgKeywheel::updateSvg() {
    /* update the svg with new values to display, then cause a an update on the widget */
    QDomElement topElement = m_domDocument.documentElement();

    bool hideTraditional = m_notation == KeyUtils::KeyNotation::Traditional;
    QDomElement domElement;

    QDomNodeList node_list = m_domDocument.elementsByTagName(QStringLiteral("text"));
    for (int i = 0; i < node_list.count(); i++) {
        QDomNode node = node_list.at(i);
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

    ui->graphic->load(str.toUtf8());
}

DlgKeywheel::~DlgKeywheel() {
    delete ui;
}
