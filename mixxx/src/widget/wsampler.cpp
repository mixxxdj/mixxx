#include <QtDebug>
#include "wsampler.h"

WSampler::WSampler(QWidget* parent) : WWidget(parent) {
}

WSampler::~WSampler() {

}

void WSampler::setup(QDomNode node) {
    WWidget::setup(node);
}