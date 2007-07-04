#include "imgloader.h"
#include "wwidget.h"

ImgLoader::ImgLoader() {
}

QImage* ImgLoader::getImage(QString img) {
	return new QImage(img);
}

