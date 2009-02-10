
#include <QtCore>

class MidiOutputMapping : public QMap<MixxxControl, MidiMessage>
{
	public:
		MidiOutputMapping() {};
		~MidiOutputMapping() {};
	private:
};
