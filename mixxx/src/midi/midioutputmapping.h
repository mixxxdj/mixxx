
#include <QtCore>

class MidiOutputMapping : public QMultiHash<MixxxControl, MidiMessage>
{
	public:
		MidiOutputMapping() {};
		~MidiOutputMapping() {};
	private:
};
