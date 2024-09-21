#include "util/valuetransformer.h"

#include <QDomElement>
#include <QDomNode>
#include <QDomNodeList>
#include <memory>
#include <numeric>

#include "skin/legacy/skincontext.h"

class TransformNode {
  public:
    TransformNode() {
    }
    virtual ~TransformNode() {
    }

    virtual double transform(double argument) const = 0;
    virtual double transformInverse(double argument) const = 0;
};

namespace {

class TransformAdd : public TransformNode {
  public:
    TransformAdd(double addend)
            : m_addend(addend) {
    }

    double transform(double argument) const override {
        return argument + m_addend;
    }

    double transformInverse(double argument) const override {
        return argument - m_addend;
    }

  private:
    double m_addend;
};

class TransformInvert : public TransformNode {
  public:
    double transform(double argument) const override {
        return -argument;
    }

    double transformInverse(double argument) const override {
        return -argument;
    }
};

class TransformNot : public TransformNode {
  public:
    double transform(double argument) const override {
        return !static_cast<bool>(argument);
    }

    double transformInverse(double argument) const override {
        return !static_cast<bool>(argument);
    }
};

class TransformIsEqual : public TransformNode {
  public:
    TransformIsEqual(double compareValue)
            : m_compareValue(compareValue) {
    }

    double transform(double argument) const override {
        return argument == m_compareValue;
    }

    double transformInverse(double argument) const override {
        if (argument > 0.0) {
            return m_compareValue;
        }
        return 0.0;
    }

  private:
    double m_compareValue;
};

} // namespace

void ValueTransformer::addTransformer(std::unique_ptr<TransformNode> pTransformer) {
    m_transformers.push_back(std::move(pTransformer));
}

double ValueTransformer::transform(double argument) const {
    return std::accumulate(m_transformers.cbegin(),
            m_transformers.cend(),
            argument,
            [&](double argument, const auto& pNode) {
                return pNode->transform(argument);
            });
}

double ValueTransformer::transformInverse(double argument) const {
    return std::accumulate(m_transformers.crbegin(),
            m_transformers.crend(),
            argument,
            [&](double argument, const auto& pNode) {
                return pNode->transformInverse(argument);
            });
}

// static
std::unique_ptr<ValueTransformer> ValueTransformer::parseFromXml(
        const QDomElement& transformElement, const SkinContext& context) {
    if (transformElement.isNull() || !transformElement.hasChildNodes()) {
        return nullptr;
    }

    // constructor is private, so we can't use std::make_unique
    auto pTransformer = std::unique_ptr<ValueTransformer>(new ValueTransformer());

    auto maybeAddFromElement = [&]<typename T>(auto element) {
        QString valueStr = context.nodeToString(element);
        bool ok = false;
        double value = valueStr.toDouble(&ok);
        if (ok) {
            pTransformer->addTransformer(std::make_unique<T>(value));
        }
    };

    QDomNodeList children = transformElement.childNodes();
    for (int i = 0; i < children.count(); ++i) {
        QDomNode node = children.at(i);
        if (!node.isElement()) {
            continue;
        }

        QDomElement element = node.toElement();
        QString name = element.nodeName();
        if (name == "Invert") {
            pTransformer->addTransformer(std::make_unique<TransformInvert>());
        } else if (name == "Add") {
            maybeAddFromElement.operator()<TransformAdd>(element);
        } else if (name == "Not") {
            pTransformer->addTransformer(std::make_unique<TransformNot>());
        } else if (name == "IsEqual") {
            maybeAddFromElement.operator()<TransformIsEqual>(element);
        }
    }

    return pTransformer;
}

ValueTransformer::ValueTransformer() = default;

ValueTransformer::~ValueTransformer() = default;
