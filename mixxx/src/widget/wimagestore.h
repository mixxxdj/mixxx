
#ifndef WIMAGESTORE_H
#define WIMAGESTORE_H

#include <QHash>
#include <QSharedPointer>

#include "skin/imgsource.h"

class QImage; 

class WImageStore {
  public:
    static QImage* getImage(const QString &fileName);
    static QImage* getImageNoCache(const QString &fileName);
    static void deleteImage(QImage* p);
    static void setLoader(QSharedPointer<ImgSource> ld);
  private:
    struct ImageInfoType {
        QImage *image;
        int instCount;
    };

    /** Dictionary of Images already instantiated */
    static QHash<QString, ImageInfoType*> m_dictionary;
    static QSharedPointer<ImgSource> m_loader;
};

#endif
