
#include <QtCore>

class MidiOutputMapping : public QHash<MixxxControl, MidiMessage>
{
	public:
		MidiOutputMapping() {};
		~MidiOutputMapping() {};
	private:
};
