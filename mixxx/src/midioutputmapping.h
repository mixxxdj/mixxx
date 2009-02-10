
#include <QtCore>

class MidiOutputMapping : public QMap<MidiControl, MidiCommand>
{
	public:
		MidiOutputMapping() {};
		~MidiOutputMapping() {};
	private:
};
