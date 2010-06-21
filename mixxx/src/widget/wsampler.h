#ifndef WSAMPLER_H
#define WSAMPLER_H

#include <QString>
#include "wwidget.h"

class WSampler : public WWidget {
    Q_OBJECT
public:
    WSampler(QWidget* parent);
    virtual ~WSampler();
    void setup(QDomNode node);
    
public slots:
private:
};

#endif /* WSampler_H */