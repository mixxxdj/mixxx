#ifndef SEGMENTATION_H
#define SEGMENTATION_H

#include "defs.h"

#include <QList>

/*
 * This class is basically a pirate of CLAM::DiscontinuousSegmentation
 * for storing chord detection information without copying the whole of
 * CLAM into Mixxx, thanks CLAM guys :)
 *  - Adam
 */

template <class T>
class Segmentation {

public:
	Segmentation<T>() :
		m_start(),
		m_end(),
		m_info() {
	}
	
	int getSegCount();

	// We have no typedef for time?
	float getSegStart(const int index) {
		return m_start[index];
	}

	float getSegEnd(const int index) {
		return m_end[index];
	}

	T getSegInfo(const int index) {
		return m_info[index];
	}

	void addSeg(const float start, const float end, const T& info) {
		m_start.push_back(start);
		m_end.push_back(end);
		m_info.push_back(info);
	}

private:
	QList<int> m_start;
	QList<int> m_end;
	QList<T> m_info;
};

#endif
