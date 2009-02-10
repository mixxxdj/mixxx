
#include <QtCore>

class MidiInputMapping : public QMap<MidiCommand, MidiControl>
{
	public:
		MidiInputMapping() {};
		~MidiInputMapping() {};
	private:
};
