/**
  * @file ctlracontroller.h
  * @author Harry van Haaren harryhaaren@gmail.com
  * @date Thu Dec 22 2016
  * @brief Ctlra backend header
  */

#ifndef CTLRACONTROLLER_H
#define CTLRACONTROLLER_H

#include "controllers/ctlra/ctlra.h"
#include "controllers/hid/hidcontrollerpreset.h"

#include <QAtomicInt>

#include "controllers/controller.h"
#include "util/duration.h"

// forward declaration
struct ctlra_dev_info_t;
struct ctlra_dev_t;
struct ctlra_event_t;

class CtlraController : public Controller
{
	Q_OBJECT
public:
	CtlraController(const struct ctlra_dev_info_t* info);
	~CtlraController() override;

	// the function that handles any input data
	void event_func(struct ctlra_dev_t* dev, uint32_t num_events,
			struct ctlra_event_t** events);

	bool isMappable() const override
	{
		return false;
	}

	/* = 0 pure virtuals needed to compile */
	virtual void visit(const MidiControllerPreset* preset) override {}
	virtual void visit(const HidControllerPreset* preset) override {}
	virtual QString presetExtension()
	{
		return "";
	}
	virtual void accept(ControllerVisitor* visitor) {}
	bool savePreset(const QString filename) const override
	{
		return false;
	}


	ControllerPresetPointer getPreset() const override
	{
		HidControllerPreset* pClone = new HidControllerPreset();
		*pClone = m_preset;
		return ControllerPresetPointer(pClone);
	}
	virtual bool matchPreset(const PresetInfo& preset)
	{
		return false;
	}
	virtual void send(QByteArray data) {}
	virtual ControllerPreset* preset() override
	{
		return &m_preset;
	}

protected:
	//Q_INVOKABLE void send(QList<int> data, unsigned int length, unsigned int reportID = 0);

private slots:
	int open() override;
	int close() override;

private:
	HidControllerPreset m_preset;
};

#endif
