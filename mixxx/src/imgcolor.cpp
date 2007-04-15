#include "imgcolor.h"

QColor ImgHueInv::doColorCorrection(QColor c) {
	int h, s, v;
	c.getHsv(&h, &s, &v);
	int r, g, b;
	c.getRgb(&r, &g, &b);
	c.setRgb(0xff - r, 0xff - g, 0xff - b);
	int hi, si, vi;
	c.getHsv(&hi, &si, &vi);
	c.setHsv(hi, s, v);
	return c;
}

QColor ImgHueRot::doColorCorrection(QColor c) {
	int h, s, v;
	c.getHsv(&h, &s, &v);
	h = (h + m_amt) % 256;
	if (h < 0) { h += 256; }
	c.setHsv(h, s, v);
	return c;
}

QColor ImgScaleWhite::doColorCorrection(QColor c) {
	int h, s, v;
	c.getHsv(&h, &s, &v);
	if (s < 50) { v *= m_amt; }
	if (v > 255) { v = 255; }
	c.setHsv(h, s, v);
	return c;
}

ImgAdd::ImgAdd(ImgSource *parent, int amt)
		: ImgColorProcessor(parent), m_amt(amt) {
		// Nothing left to do
}

QColor ImgAdd::doColorCorrection(QColor c) {
	int r = c.red() + m_amt;
	int g = c.green() + m_amt;
	int b = c.blue() + m_amt;
	if (r < 0) { r = 0; }
	if (g < 0) { g = 0; }
	if (b < 0) { b = 0; }
	if (r > 255) { r = 255; }
	if (g > 255) { g = 255; }
	if (b > 255) { b = 255; }
	return QColor(r, g, b);
}

ImgMax::ImgMax(ImgSource *parent, int amt)
		: ImgColorProcessor(parent), m_amt(amt) {
}

QColor ImgMax::doColorCorrection(QColor c) {
	int r = c.red();
	int g = c.green();
	int b = c.blue();
	if (r > m_amt) { r = m_amt; }
	if (g > m_amt) { g = m_amt; }
	if (b > m_amt) { b = m_amt; }
	return QColor(r, g, b);
}