#include "oscserver.h"

lo_server_thread OscServer::m_st = nullptr;

OscServer::OscServer(UserSettingsPointer& pConfig) {
    if (m_st) {
        return;
    }

    QString port = pConfig->getValueString(ConfigKey(OSC_SERVER_PREF_KEY, "Port"));
    m_st = lo_server_thread_new(port.toLatin1().data(), OscServer::oscErrorHandler);
    if (!m_st) {
        return;
    }

    lo_server_thread_add_method(m_st, nullptr, nullptr, OscServer::oscMsgHandler, nullptr);
    lo_server_thread_start(m_st);
}

OscServer::~OscServer() {
    quitServer();
}

void OscServer::quitServer() {
    if (!m_st) {
        return;
    }

    lo_server_thread_free(m_st);
    m_st = nullptr;
}

void OscServer::oscErrorHandler(int err, const char* msg, const char* path) {
    qWarning() << QString("ERROR: %1 in OSC path %2 (code %3)").arg(QString::fromLatin1(msg)).arg(QString::fromLatin1(path)).arg(err);
    quitServer();
}

int OscServer::oscMsgHandler(const char* path, const char* types, lo_arg** argv, int argc, void* data, void* userData) {
    Q_UNUSED(userData);

    QRegularExpression pathRegEx("\\/(.+)\\/(.+)");
    QRegularExpressionMatch pathMatch = pathRegEx.match(QString::fromLatin1(path));

    if (!pathMatch.hasMatch()) {
        qWarning() << "ERROR: Invalid OSC path! Proper format: /<group>/<control>";
        return 1;
    }

    ConfigKey key = ConfigKey(pathMatch.captured(1), pathMatch.captured(2));

    if (key.isNull() || key.isEmpty()) {
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
            if (OscServer::m_st && (reinterpret_cast<char*>(argv[i])[0] == '?')) {
                lo_address msgTo = lo_message_get_source(static_cast<lo_message>(data));
                lo_send(msgTo, path, "d", ControlObject::get(key));
            }

            continue;
        }

        if (argType == LO_INT32) {
            ControlObject::set(key, static_cast<double>(argv[i]->i));
            continue;
        }

        if (argType == LO_FLOAT) {
            ControlObject::set(key, static_cast<double>(argv[i]->f));
            continue;
        }

        if (argType == LO_INT64) {
            ControlObject::set(key, static_cast<double>(argv[i]->h));
            continue;
        }

        if (argType == LO_DOUBLE) {
            ControlObject::set(key, argv[i]->d);
            continue;
        }

        if (argType == LO_CHAR) {
            ControlObject::set(key, static_cast<double>(argv[i]->c));
            continue;
        }

        if (argType == LO_TRUE) {
            ControlObject::set(key, 1.0);
            continue;
        }

        if (argType == LO_FALSE) {
            ControlObject::set(key, 0.0);
            continue;
        }
    }

    return 0;
}
