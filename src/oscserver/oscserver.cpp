#include "oscserver.h"

namespace {
void oscErrorHandler(int err, const char *msg, const char *path) {
  qWarning() << QString("%1. OSC path: %2. Error Code: %3.")
                    .arg(QString::fromLatin1(msg))
                    .arg(QString::fromLatin1(path))
                    .arg(err);
}

int oscMsgHandler(const char *path, const char *types, lo_arg **argv, int argc,
                  void *data, void *userData) {
  Q_UNUSED(userData);

  QRegularExpression pathRegEx("\\/mixxx\\/(.+)\\/(.+)");
  QRegularExpressionMatch pathMatch =
      pathRegEx.match(QString::fromLatin1(path));

  if (!pathMatch.hasMatch()) {
    qWarning() << "Invalid OSC path: " << QString::fromLatin1(path);
    qWarning() << "Proper OSC path format: /mixxx/<group>/<control>";
    return 1;
  }

  ConfigKey key = ConfigKey(pathMatch.captured(1), pathMatch.captured(2));

  if (key.isNull() || key.isEmpty()) {
    qWarning() << "Invalid group/key pair specified in OSC path: "
               << QString::fromLatin1(path);
    return 1;
  }

  for (int i = 0; i < argc; ++i) {
    lo_type argType = static_cast<lo_type>(types[i]);

    if ((argType == LO_BLOB) || (argType == LO_TIMETAG) ||
        (argType == LO_MIDI) || (argType == LO_NIL) ||
        (argType == LO_INFINITUM)) {
      continue;
    }

    if ((argType == LO_STRING) || (argType == LO_SYMBOL)) {
      if (reinterpret_cast<char *>(argv[i])[0] == '?') {
        lo_address msgTo = lo_message_get_source(static_cast<lo_message>(data));
        if (lo_send(msgTo, path, "d", ControlObject::get(key)) == -1) {
          char *targetUrl = lo_address_get_url(msgTo);
          qWarning() << "Failed to reply to OSC get parameter message from: "
                     << QString::fromLatin1(targetUrl);
          free(targetUrl);
        }
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
}; // namespace

OscServer::OscServer(UserSettingsPointer pConfig)
    : m_pConfig(pConfig), m_st(nullptr) {
  m_pUpdateProxy =
      std::make_unique<ControlProxy>(OSC_SERVER_PREF_KEY, "NeedsUpdate");
  m_pUpdateProxy->connectValueChanged(this, &OscServer::slotNeedsUpdate);
  m_pErrorProxy = std::make_unique<ControlProxy>(OSC_SERVER_PREF_KEY, "Error");

  init();
}

OscServer::~OscServer() { quit(); }

bool OscServer::init() {
  // Return true in these two cases because there was no error
  // (init was simply not necessary in these cases)
  if (m_st) {
    return true;
  }

  if (!m_pConfig->getValue<bool>(ConfigKey(OSC_SERVER_PREF_KEY, "Enabled"))) {
    return true;
  }

  QString port =
      m_pConfig->getValueString(ConfigKey(OSC_SERVER_PREF_KEY, "Port"));
  // If user has not chosen a port, fall back to default.
  if (port.isEmpty() || port.isNull()) {
    port = OSC_SERVER_DEFAULT_PORT;
  }

  m_st = lo_server_thread_new(port.toLatin1().data(), oscErrorHandler);
  if (!m_st) {
    setError(true);
    return false;
  }

  lo_server_thread_add_method(m_st, nullptr, nullptr, oscMsgHandler, nullptr);
  if (lo_server_thread_start(m_st) < 0) {
    // m_st != nullptr, so call quit() to free it
    quit();

    setError(true);
    return false;
  }

  // Clear any previous failed init errors and update status to show OSC enabled
  setError(false);
  return true;
}

void OscServer::quit() {
  if (!m_st) {
    return;
  }

  lo_server_thread_free(m_st);
  m_st = nullptr;

  // Clear any previous errors and update status to show OSC disabled
  setError(false);
}

void OscServer::slotNeedsUpdate(double needsUpdate) {
  if (needsUpdate <= 0.0) {
    return;
  }

  quit();
  init();

  setNeedsUpdate(false);
}

void OscServer::setNeedsUpdate(bool needsUpdate) {
  m_pUpdateProxy->set(needsUpdate ? 1.0 : 0.0);
}

void OscServer::setError(bool error) { m_pErrorProxy->set(error ? 1.0 : 0.0); }