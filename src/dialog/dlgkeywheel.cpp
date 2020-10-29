#include <QDir>
#include <QKeyEvent>
#include <QMouseEvent>

#include "dialog/dlgkeywheel.h"

using namespace mixxx::track::io::key;

namespace {
auto kKeywheelSVG = QStringLiteral("images/keywheel/keywheel.svg");
const KeyUtils::KeyNotation kNotationHidden[]{
        KeyUtils::KeyNotation::Traditional,
        KeyUtils::KeyNotation::OpenKeyAndTraditional,
        KeyUtils::KeyNotation::LancelotAndTraditional};
} // namespace

DlgKeywheel::DlgKeywheel(QWidget *parent, UserSettingsPointer pConfig) :
    QDialog(parent),
    ui(new Ui::DlgKeywheel),
    m_pConfig(pConfig)
{
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
    auto pKeyNotation = new ControlProxy(ConfigKey("[Library]", "key_notation"), this);
    m_notation = static_cast<KeyUtils::KeyNotation>(static_cast<int>(pKeyNotation->get()));
    // we skip the TRADITIONAL display, because it shows redundant informations only
    if (m_notation == KeyUtils::KeyNotation::Traditional) {
        m_notation = KeyUtils::KeyNotation::OpenKey;
    }
    updateDisplay();
}

bool DlgKeywheel::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        // we handle TAB + Shift TAB to cycle through the display types
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Tab) {
            switchDisplay(+1);
            return true;
        } else if (keyEvent->key() == Qt::Key_Backtab) {
            switchDisplay(-1);
            return true;
        }
    } else if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        // first button forward, other buttons backward cycle
        switchDisplay(mouseEvent->button() == Qt::LeftButton ? +1 : -1);
        return true;
    }
    // standard event processing
    return QObject::eventFilter(obj, event);
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
    KeyUtils::KeyNotation newNotation = static_cast<KeyUtils::KeyNotation>(
            static_cast<int>(m_notation) + step);
    // we skip variants with redundant information
    while (newNotation <= KeyUtils::KeyNotation::Invalid ||
            newNotation >= KeyUtils::KeyNotation::NumKeyNotations ||
            isHiddenNotation(newNotation)) {
        newNotation = static_cast<KeyUtils::KeyNotation>(static_cast<int>(newNotation) + step);
        if (newNotation >= KeyUtils::KeyNotation::NumKeyNotations) {
            newNotation = KeyUtils::KeyNotation::Custom;
        } else if (newNotation <= KeyUtils::KeyNotation::Invalid) {
            newNotation = static_cast<KeyUtils::KeyNotation>(
                    static_cast<int>(KeyUtils::KeyNotation::NumKeyNotations) - 1);
        }
    }
    m_notation = newNotation;
    // we update the SVG nodes with the new value
    updateDisplay();
}

void DlgKeywheel::updateDisplay() {
    /* update the svg with new values to display, then cause a an update on the widget */
    QDomElement topElement = m_domDocument.documentElement();
    QDomNode domNode = topElement.firstChild();

    QDomElement domElement;

    QDomNodeList node_list = m_domDocument.elementsByTagName(QString("text"));
    for (int i = 0; i < node_list.count(); i++) {
        QDomNode node = node_list.at(i);
        domElement = node.toElement();
        QString id = domElement.attribute("id", "UNKNOWN");
        // we identify the text node by the id und use the suffix to lookup the
        // chromakey id
        if (id.startsWith("k_")) {
            // in this svg the text is inside a span which we need to look up first
            QDomNode tspan = node.firstChild();
            QDomNode text = tspan.firstChild();

            if (text.isText()) {
                QDomText textNode = text.toText();
                ChromaticKey key = static_cast<ChromaticKey>(id.midRef(2).toInt());
                QString keyString = KeyUtils::keyToString(key, m_notation);
                if (keyString.length() > 4) {
                    keyString.replace(QChar(' '), QChar('\n'));
                }
                textNode.setData(keyString);
            }
        }

    }

    QString str;
    QTextStream stream(&str);

    m_domDocument.save(stream, QDomNode::EncodingFromDocument);

    ui->graphic->load(str.toUtf8());
}

DlgKeywheel::~DlgKeywheel()
{
    delete ui;
}
