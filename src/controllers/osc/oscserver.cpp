#include "controllers/osc/oscserver.h"

#include <QHostAddress>
#include <QUdpSocket>
#include <QtDebug>

#include <cstring>
#include <utility>

#include "control/controlobject.h"
#include "preferences/configobject.h"
#include "util/assert.h"

#include "moc_oscserver.cpp"

namespace {

const QString kConfigGroup = QStringLiteral("[OSC]");
const QString kEnabledKey = QStringLiteral("Enabled");
const QString kInputPortKey = QStringLiteral("InputPort");
const QString kOutputPortKey = QStringLiteral("OutputPort");
const QString kOutputHostKey = QStringLiteral("OutputHost");
const QString kBindAddressKey = QStringLiteral("BindAddress");

constexpr int kDefaultInputPort = 11119;
constexpr int kDefaultOutputPort = 11118;
// 25 Hz is plenty for tracking deck state while staying cheap.
constexpr int kFeedbackIntervalMs = 40;

/// Reads an OSC string: null terminated and padded to a multiple of 4 bytes.
bool readOscString(const QByteArray& data, int* pOffset, QString* pString) {
    const int start = *pOffset;
    if (start < 0 || start >= data.size()) {
        return false;
    }
    const int end = data.indexOf('\0', start);
    if (end < 0) {
        return false;
    }
    *pString = QString::fromUtf8(data.constData() + start, end - start);
    int next = end + 1;
    while (next % 4 != 0) {
        ++next;
    }
    *pOffset = next;
    return true;
}

quint32 readBigEndianUint32(const QByteArray& data, int offset) {
    return (static_cast<quint32>(static_cast<quint8>(data.at(offset))) << 24) |
            (static_cast<quint32>(static_cast<quint8>(data.at(offset + 1))) << 16) |
            (static_cast<quint32>(static_cast<quint8>(data.at(offset + 2))) << 8) |
            static_cast<quint32>(static_cast<quint8>(data.at(offset + 3)));
}

void appendOscString(QByteArray* pData, const QString& string) {
    pData->append(string.toUtf8());
    pData->append('\0');
    while (pData->size() % 4 != 0) {
        pData->append('\0');
    }
}

void appendBigEndianUint32(QByteArray* pData, quint32 value) {
    pData->append(static_cast<char>((value >> 24) & 0xff));
    pData->append(static_cast<char>((value >> 16) & 0xff));
    pData->append(static_cast<char>((value >> 8) & 0xff));
    pData->append(static_cast<char>(value & 0xff));
}

} // namespace

OscServer::OscServer(UserSettingsPointer pConfig, QObject* parent)
        : QObject(parent),
          m_pConfig(std::move(pConfig)),
          m_pSocket(nullptr),
          m_outputPort(kDefaultOutputPort),
          m_outputHost(QStringLiteral("127.0.0.1")) {
    m_feedbackTimer.setInterval(kFeedbackIntervalMs);
    connect(&m_feedbackTimer, &QTimer::timeout, this, &OscServer::slotSendFeedback);
}

OscServer::~OscServer() = default;

// static
bool OscServer::splitOscAddress(
        const QString& address, QString* pGroup, QString* pControl) {
    if (!address.startsWith(QLatin1Char('/'))) {
        return false;
    }
    const int separator = address.indexOf(QLatin1Char('/'), 1);
    if (separator <= 1 || separator >= address.size() - 1) {
        return false;
    }
    const QString group = address.mid(1, separator - 1);
    const QString control = address.mid(separator + 1);
    // Group and control names never contain a slash in Mixxx.
    if (group.isEmpty() || control.isEmpty() || control.contains(QLatin1Char('/'))) {
        return false;
    }
    *pGroup = group;
    *pControl = control;
    return true;
}

void OscServer::start() {
    stop();

    if (!m_pConfig->getValue<bool>(ConfigKey(kConfigGroup, kEnabledKey), false)) {
        return;
    }

    const int inputPort = m_pConfig->getValue<int>(
            ConfigKey(kConfigGroup, kInputPortKey), kDefaultInputPort);
    m_outputPort = static_cast<quint16>(m_pConfig->getValue<int>(
            ConfigKey(kConfigGroup, kOutputPortKey), kDefaultOutputPort));
    m_outputHost = m_pConfig->getValueString(ConfigKey(kConfigGroup, kOutputHostKey));
    if (m_outputHost.isEmpty()) {
        m_outputHost = QStringLiteral("127.0.0.1");
    }

    QString bindAddress = m_pConfig->getValueString(ConfigKey(kConfigGroup, kBindAddressKey));
    if (bindAddress.isEmpty()) {
        bindAddress = QStringLiteral("127.0.0.1");
    }

    m_pSocket = new QUdpSocket(this);
    if (!m_pSocket->bind(QHostAddress(bindAddress), static_cast<quint16>(inputPort))) {
        qWarning() << "OSC: failed to bind to" << bindAddress << inputPort
                   << m_pSocket->errorString();
        delete m_pSocket;
        m_pSocket = nullptr;
        return;
    }
    connect(m_pSocket, &QUdpSocket::readyRead, this, &OscServer::slotReadPendingDatagrams);

    m_subscribedAddresses.clear();
    m_lastFeedbackValues.clear();
    m_feedbackTimer.start();

    qInfo() << "OSC: listening on" << bindAddress << inputPort
            << "sending feedback to" << m_outputHost << m_outputPort;
}

void OscServer::stop() {
    m_feedbackTimer.stop();
    if (m_pSocket) {
        m_pSocket->close();
        m_pSocket->deleteLater();
        m_pSocket = nullptr;
    }
}

bool OscServer::isListening() const {
    return m_pSocket != nullptr;
}

void OscServer::slotReadPendingDatagrams() {
    while (m_pSocket && m_pSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(static_cast<int>(m_pSocket->pendingDatagramSize()));
        m_pSocket->readDatagram(datagram.data(), datagram.size());
        handleMessage(datagram);
    }
}

void OscServer::handleMessage(const QByteArray& datagram) {
    int offset = 0;
    QString address;
    QString typeTags;
    if (!readOscString(datagram, &offset, &address) ||
            !readOscString(datagram, &offset, &typeTags)) {
        return;
    }
    if (!typeTags.startsWith(QLatin1Char(',')) || typeTags.size() < 2) {
        // A message without arguments cannot set a control value.
        return;
    }

    // Only the first argument is used for the control value.
    const QChar type = typeTags.at(1);
    double value = 0.0;
    if (type == QLatin1Char('f')) {
        if (offset + 4 > datagram.size()) {
            return;
        }
        const quint32 raw = readBigEndianUint32(datagram, offset);
        float floatValue;
        std::memcpy(&floatValue, &raw, sizeof(floatValue));
        value = floatValue;
    } else if (type == QLatin1Char('i')) {
        if (offset + 4 > datagram.size()) {
            return;
        }
        value = static_cast<qint32>(readBigEndianUint32(datagram, offset));
    } else {
        return;
    }

    QString group;
    QString control;
    VERIFY_OR_DEBUG_ASSERT(splitOscAddress(address, &group, &control)) {
        qWarning() << "OSC: ignoring malformed address" << address;
        return;
    }

    const ConfigKey key(group, control);
    if (ControlObject::getControl(key, ControlFlag::AllowMissingOrInvalid) == nullptr) {
        qWarning() << "OSC: ignoring unknown control for address" << address;
        return;
    }
    ControlObject::set(key, value);

    // From now on the value of this control is streamed back to the client.
    m_subscribedAddresses.insert(address);
}

void OscServer::sendControl(const QString& address, double value) {
    if (!m_pSocket) {
        return;
    }
    QByteArray packet;
    appendOscString(&packet, address);
    appendOscString(&packet, QStringLiteral(",f"));
    const float floatValue = static_cast<float>(value);
    quint32 raw;
    std::memcpy(&raw, &floatValue, sizeof(raw));
    appendBigEndianUint32(&packet, raw);
    m_pSocket->writeDatagram(packet, QHostAddress(m_outputHost), m_outputPort);
}

void OscServer::slotSendFeedback() {
    if (!m_pSocket || m_subscribedAddresses.isEmpty()) {
        return;
    }
    for (const QString& address : std::as_const(m_subscribedAddresses)) {
        QString group;
        QString control;
        if (!splitOscAddress(address, &group, &control)) {
            continue;
        }
        const ConfigKey key(group, control);
        if (ControlObject::getControl(key, ControlFlag::AllowMissingOrInvalid) == nullptr) {
            continue;
        }
        const double value = ControlObject::get(key);
        const auto it = m_lastFeedbackValues.constFind(address);
        if (it != m_lastFeedbackValues.constEnd() && *it == value) {
            continue; // Unchanged since the last feedback.
        }
        m_lastFeedbackValues.insert(address, value);
        sendControl(address, value);
    }
}
