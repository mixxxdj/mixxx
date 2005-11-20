#include "sdatetime.h"

SDateTime::SDateTime(QDateTime *src) : QDateTime(*src) {
	//qDebug("Created SDateTime");
}

SDateTime::~SDateTime() {
}

int SDateTime::msecsTo(QDateTime *that) {
	int delta = 0;
	delta = this->secsTo(*that) * 1000;
	int msecsnow = this->time().msec();
	int msecsthen = that->time().msec();
	delta += msecsthen - msecsnow;

	return delta;
}

SDateTime* SDateTime::now() {
	QDateTime bob = QDateTime::currentDateTime();
	QDateTime* bill = new QDateTime();
	*bill = bob;
	return new SDateTime(bill);
}
