#include <QKeyEvent>
#include <QMouseEvent>

#include "dialog/dlgkeywheel.h"

using namespace mixxx::track::io::key;

DlgKeywheel::DlgKeywheel(QWidget *parent, UserSettingsPointer pConfig) :
    QDialog(parent),
    ui(new Ui::DlgKeywheel),
    m_pConfig(pConfig)
{
    ui->setupUi(this);

    auto svg = pConfig->getResourcePath() + QString("images/keywheel/keywheel.svg");
    QFile xmlFile(svg);
    if (!xmlFile.exists() || !xmlFile.open(QFile::ReadOnly | QFile::Text)) {
        qDebug() << "Could not load template: " << svg;
        return;
    }
    m_domDocument.setContent(&xmlFile);

    // FIXME(poelzi): try to prevent aspect ratio. This fails unfortunatelly
    QSizePolicy qsp = ui->graphic->sizePolicy(); //.setHeightForWidth(true);
    qsp.setWidthForHeight(true);
    ui->graphic->setSizePolicy(qsp);
    installEventFilter(this);
    ui->graphic->installEventFilter(this);

    // load the user configured setting as default
    auto pKeyNotation = new ControlProxy(ConfigKey("[Library]", "key_notation"), this);
    m_notation = static_cast<KeyUtils::KeyNotation>(static_cast<int>(pKeyNotation->get()));
    // we skip the TRADITIONAL display, because it shows redundant informations only
    if (m_notation == KeyUtils::KeyNotation::TRADITIONAL) {
            m_notation = KeyUtils::KeyNotation::OPEN_KEY;
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

void DlgKeywheel::switchDisplay(int dir) {
    m_notation = static_cast<KeyUtils::KeyNotation>(static_cast<int>(m_notation) + dir);
    // we skip the TRADITIONAL display, because it shows redundant informations only
    if (m_notation == KeyUtils::KeyNotation::TRADITIONAL) {
            m_notation = static_cast<KeyUtils::KeyNotation>(static_cast<int>(m_notation) + dir);
    }
    if (m_notation >= KeyUtils::KeyNotation::KEY_NOTATION_MAX) {
        m_notation = KeyUtils::KeyNotation::CUSTOM;
    } else if (m_notation <= KeyUtils::KeyNotation::INVALID) {
        m_notation = static_cast<KeyUtils::KeyNotation>(static_cast<int>(KeyUtils::KeyNotation::KEY_NOTATION_MAX) - 1);
    }

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
                textNode.setData(KeyUtils::keyToString(key,
                                                       m_notation));
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
