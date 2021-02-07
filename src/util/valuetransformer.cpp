#include <QtDebug>

#include "util/valuetransformer.h"

ValueTransformer::ValueTransformer() {
}

void ValueTransformer::addTransformer(TransformNode* pTransformer) {
    m_transformers.append(pTransformer);
}

double ValueTransformer::transform(double argument) const {
    foreach (const TransformNode* pNode, m_transformers) {
        argument = pNode->transform(argument);
    }
    return argument;
}

double ValueTransformer::transformInverse(double argument) const {
    for (int i = m_transformers.size() - 1; i >= 0; --i) {
        const TransformNode* pNode = m_transformers[i];
        argument = pNode->transformInverse(argument);
    }
    return argument;
}

// static
ValueTransformer* ValueTransformer::parseFromXml(const QDomElement& transformElement,
        const SkinContext& context) {
    if (transformElement.isNull() || !transformElement.hasChildNodes()) {
        return nullptr;
    }

    ValueTransformer* pTransformer = new ValueTransformer();
    QDomNodeList children = transformElement.childNodes();
    for (int i = 0; i < children.count(); ++i) {
        QDomNode node = children.at(i);
        if (!node.isElement()) {
            continue;
        }

        QDomElement element = node.toElement();
        if (element.nodeName() == "Invert") {
            pTransformer->addTransformer(new TransformInvert());
        } else if (element.nodeName() == "Add") {
            QString value = context.nodeToString(element);
            bool ok = false;
            double addend = value.toDouble(&ok);
            if (ok) {
                pTransformer->addTransformer(new TransformAdd(addend));
            }
        } else if (element.nodeName() == "Not") {
            pTransformer->addTransformer(new TransformNot());
        } else if (element.nodeName() == "IsEqual") {
            QString value = context.nodeToString(element);
            bool ok = false;
            double compareValue = value.toDouble(&ok);
            if (ok) {
                pTransformer->addTransformer(new TransformIsEqual(compareValue));
            }
        }
    }

    return pTransformer;
}

ValueTransformer::~ValueTransformer() {
    foreach (TransformNode* node, m_transformers) {
        delete node;
    }
}
