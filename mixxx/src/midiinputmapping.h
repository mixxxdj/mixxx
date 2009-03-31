
#include <QtCore>

class MidiInputMapping : public QHash<MidiMessage, MixxxControl>
{
	public:
		MidiInputMapping() {};
		~MidiInputMapping() {};
	private:
};
