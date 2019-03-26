#include "controllers/osc/osccontroller.h"

lo_server_thread OscController::m_st = nullptr;

OscController::OscController() {
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
    if (m_st) {
        return -1;
    }

    m_st = lo_server_thread_new(OSC_SERVER_PORT, OscController::oscErrorHandler);
    lo_server_thread_add_method(m_st, nullptr, nullptr, OscController::oscMsgHandler, nullptr);
    lo_server_thread_start(m_st);

    setOpen(true);
    return 0;
}

int OscController::close() {
    quitServer();
    setOpen(false);
    return 0;
}

bool OscController::poll() {
    return false;
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

void OscController::quitServer() {
    if (!m_st) {
        return;
    }

    lo_server_thread_free(m_st);
    m_st = nullptr;
}

void OscController::oscErrorHandler(int err, const char* msg, const char* path) {
    qWarning() << QString("ERROR: %1 in OSC path %2 (code %3)").arg(QString::fromLatin1(msg)).arg(QString::fromLatin1(path)).arg(err);
    quitServer();
}

int OscController::oscMsgHandler(const char* path, const char* types, lo_arg** argv, int argc, void* data, void* userData) {
    Q_UNUSED(userData);

    QRegularExpression pathRegEx("\\/(.+)\\/(.+)");
    QRegularExpressionMatch pathMatch = pathRegEx.match(QString::fromLatin1(path));

    if (!pathMatch.hasMatch()) {
        qWarning() << "ERROR: Invalid OSC path! Proper format: /<group>/<control>";
        return 1;
    }

    ControlProxy proxy(pathMatch.captured(1), pathMatch.captured(2));

    if (!proxy.valid()) {
        qWarning() << "ERROR: Invalid group/key pair specified in OSC path!";
        return 1;
    }

    for (int i = 0; i < argc; ++i) {
        lo_type argType = static_cast<lo_type>(types[i]);

        if ((argType == LO_BLOB) ||
            (argType == LO_TIMETAG) ||
            (argType == LO_MIDI) ||
            (argType == LO_NIL) ||
            (argType == LO_INFINITUM)) {
            continue;
        }

        if ((argType == LO_STRING) ||
            (argType == LO_SYMBOL)) {
            if (OscController::m_st && (reinterpret_cast<char*>(argv[i])[0] == '?')) {
                lo_address msgTo = lo_message_get_source(static_cast<lo_message>(data));
                lo_send(msgTo, path, "d", proxy.get());
            }

            continue;
        }

        if (argType == LO_INT32) {
            proxy.set(static_cast<double>(argv[i]->i));
            continue;
        }

        if (argType == LO_FLOAT) {
            proxy.set(static_cast<double>(argv[i]->f));
            continue;
        }

        if (argType == LO_INT64) {
            proxy.set(static_cast<double>(argv[i]->h));
            continue;
        }

        if (argType == LO_DOUBLE) {
            proxy.set(argv[i]->d);
            continue;
        }

        if (argType == LO_CHAR) {
            proxy.set(static_cast<double>(argv[i]->c));
            continue;
        }

        if (argType == LO_TRUE) {
            proxy.set(1.0);
            continue;
        }

        if (argType == LO_FALSE) {
            proxy.set(0.0);
            continue;
        }
    }

    return 0;
}
