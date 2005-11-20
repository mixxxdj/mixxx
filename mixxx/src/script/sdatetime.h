#ifndef SCRIPT_SDATETIME_H
#define SCRIPT_SDATETIME_H

#include <qdatetime.h>

class SDateTime : public QDateTime {

	public:
		SDateTime(QDateTime* src);
		~SDateTime();

		// TODO: Should be long
		int msecsTo(QDateTime* that);

		static SDateTime* now();
};

#endif
