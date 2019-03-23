#include "controllers/osc/osccontroller.h"

OscController::OscController() : m_polling(false) {
    QString deviceName = QString("OSC Server on Port %1").arg(OSC_SERVER_PORT);

    setDeviceName(deviceName);
    m_preset.setName(deviceName);
}

OscController::~OscController() {
    if (isOpen()) {
        close();
    }
}

int OscController::open() {
    m_sock.bindTo(OSC_SERVER_PORT);

    if (m_sock.isOk()) {
        m_polling = true;
        setOpen(true);
        return 0;
    }

    setOpen(false);
    qWarning() << "Failed to open OSC server on port " << QString::number(OSC_SERVER_PORT);
    return -1;
}

int OscController::close() {
    if (!isOpen()) {
        return 0;
    }

    m_polling = false;
    setOpen(false);
    m_sock.close();

    return 0;
}

bool OscController::poll() {
    if (!isOpen()) {
        return false;
    }

    if (!m_sock.isOk()) {
        m_polling = false;
        setOpen(false);
        m_sock.close();
        return false;
    }

    double dValue;
    int64_t i64Value;
    float fValue;
    int32_t i32Value;
    bool bValue;

    std::string group, key, location;

    // Should 30ms timeout be changed?
    if (m_sock.receiveNextPacket(30)) {
        m_pr.init(m_sock.packetData(), m_sock.packetSize());
        oscpkt::Message *msg;
        while (m_pr.isOk() && (msg = m_pr.popMessage()) != 0) {
            if (msg->match("/getparameter").popStr(group).popStr(key).isOkNoMoreArgs()) {
                getParameter(group, key);
            } else if (msg->match("/setparameter").popStr(group).popStr(key).popDouble(dValue).isOkNoMoreArgs()) {
                setDouble(group, key, dValue);
            } else if (msg->match("/setparameter").popStr(group).popStr(key).popFloat(fValue).isOkNoMoreArgs()) {
                setFloat(group, key, fValue);
            } else if (msg->match("/setparameter").popStr(group).popStr(key).popInt32(i32Value).isOkNoMoreArgs()) {
                setI32(group, key, i32Value);
            } else if (msg->match("/setparameter").popStr(group).popStr(key).popInt64(i64Value).isOkNoMoreArgs()) {
                setI64(group, key, i64Value);
            } else if (msg->match("/setparameter").popStr(group).popStr(key).popBool(bValue).isOkNoMoreArgs()) {
                setBool(group, key, bValue);
            } else {
                qWarning() << "Unhandled OSC message" << QString::fromStdString(msg->addressPattern());
            }
        }
    }

    return true;
}

QString OscController::presetExtension() {
    return OSC_PRESET_EXTENSION;
}

bool OscController::savePreset(const QString fileName) const {
    HidControllerPresetFileHandler handler;
    return handler.save(m_preset, getName(), fileName);
}

bool OscController::matchPreset(const PresetInfo& preset) {
    Q_UNUSED(preset);
    return false;
}

void OscController::send(QByteArray data) {
    Q_UNUSED(data);
}

void OscController::visit(const MidiControllerPreset* preset) {
    Q_UNUSED(preset);
    qWarning() << "ERROR: Attempting to load a MidiControllerPreset to an OscController!";
}

void OscController::visit(const HidControllerPreset* preset) {
    Q_UNUSED(preset);
    qWarning() << "ERROR: Attempting to load a HidControllerPreset to an OscController!";
}

void OscController::getParameter(const std::string& group, const std::string& key) {
    ControlProxy proxy(QString::fromStdString(group), QString::fromStdString(key));

    // If the specified parameter does not exist or is inaccessible,
    // don't send a response (because it will be invalid).
    // Is this the best behavior here or is it better to return 0?
    if (!proxy.valid()) {
        return;
    }

    oscpkt::Message msg;
    msg.init("/mixxxparameter").pushStr(group).pushStr(key).pushDouble(proxy.get());
    m_pw.init().addMessage(msg);
    // m_sock.packetOrigin() is safe here because this function is called
    // immediately after receiving a /get packet
    m_sock.sendPacketTo(m_pw.packetData(), m_pw.packetSize(), m_sock.packetOrigin());
}

void OscController::setDouble(const std::string& group, const std::string& key, double value) {
    ControlProxy proxy(QString::fromStdString(group), QString::fromStdString(key));
    proxy.set(value);
}

void OscController::setFloat(const std::string& group, const std::string& key, float value) {
    setDouble(group, key, static_cast<double>(value));
}

void OscController::setI32(const std::string& group, const std::string& key, int32_t value) {
    setDouble(group, key, static_cast<double>(value));
}

void OscController::setI64(const std::string& group, const std::string& key, int64_t value) {
    setDouble(group, key, static_cast<double>(value));
}

void OscController::setBool(const std::string& group, const std::string& key, bool value) {
    setDouble(group, key, value ? 1.0 : 0.0);
}
