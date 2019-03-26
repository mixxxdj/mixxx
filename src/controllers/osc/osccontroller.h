#ifndef OSCCONTROLLER_H
#define OSCCONTROLLER_H

#include <lo/lo.h>

#include "control/controlproxy.h"
#include "controllers/defs_controllers.h"
#include "controllers/controller.h"
#include "controllers/hid/hidcontrollerpreset.h"
#include "controllers/hid/hidcontrollerpresetfilehandler.h"

#define OSC_SERVER_PORT "9000"

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
      return false;
    }

    static void quitServer();

    static void oscErrorHandler(int err, const char* msg, const char* path);
    static int oscMsgHandler(const char* path, const char* types, lo_arg** argv, int argc, void* data, void* userData);

    HidControllerPreset m_preset;

    static lo_server_thread m_st;
};

#endif /* OSCCONTROLLER_H */
