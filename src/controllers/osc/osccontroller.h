#ifndef OSCCONTROLLER_H
#define OSCCONTROLLER_H

// udp.hh needs these files
#include <ostream>
#include <unistd.h>

#include "../../../lib/oscpkt/udp.hh"
#include "../../../lib/oscpkt/oscpkt.hh"

#include "control/controlproxy.h"
#include "controllers/defs_controllers.h"
#include "controllers/controller.h"
#include "controllers/hid/hidcontrollerpreset.h"
#include "controllers/hid/hidcontrollerpresetfilehandler.h"

#define OSC_SERVER_PORT 9000

class OscController : public Controller {
    Q_OBJECT
  public:
    OscController();
    ~OscController() override;

    QString presetExtension() override;

    void accept(ControllerVisitor* visitor) override {
      if (visitor) {
        visitor->visit(this);
      }
    }

    bool savePreset(const QString fileName) const override;

    void visit(const MidiControllerPreset* preset) override;
    void visit(const HidControllerPreset* preset) override;

    ControllerPresetPointer getPreset() const override {
      HidControllerPreset* pClone = new HidControllerPreset();
      *pClone = m_preset;
      return ControllerPresetPointer(pClone);
    }

    bool isMappable() const override {
      return m_preset.isMappable();
    }

    bool matchPreset(const PresetInfo& preset) override;

  private slots:
    int open() override;
    int close() override;
    bool poll() override;

  private:
    void send(QByteArray data) override;

    ControllerPreset* preset() override {
      return &m_preset;
    }

    bool isPolling() const override {
      return m_polling;
    }

    HidControllerPreset m_preset;

    void getParameter(const std::string& group, const std::string& key);
    void setDouble(const std::string& group, const std::string& key, double value);
    void setFloat(const std::string& group, const std::string& key, float value);
    void setI32(const std::string& group, const std::string& key, int32_t value);
    void setI64(const std::string& group, const std::string& key, int64_t value);
    void setBool(const std::string& group, const std::string& key, bool value);

    oscpkt::UdpSocket m_sock;
    oscpkt::PacketReader m_pr;
    oscpkt::PacketWriter m_pw;
    bool m_polling;
};

#endif /* OSCCONTROLLER_H */
