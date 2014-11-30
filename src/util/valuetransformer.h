#ifndef VALUETRANSFORMER_H
#define VALUETRANSFORMER_H

#include <QList>
#include <QDomElement>

#include "skin/skincontext.h"

class TransformNode {
  public:
    TransformNode() {}

    virtual double transform(double argument) const = 0;
    virtual double transformInverse(double argument) const = 0;
};

class TransformAdd : public TransformNode {
  public:
    TransformAdd(double addend) : m_addend(addend) {}

    double transform(double argument) const {
        return argument + m_addend;
    }

    double transformInverse(double argument) const {
        return argument - m_addend;
    }

  private:
    double m_addend;
};

class TransformInvert : public TransformNode {
  public:
    TransformInvert() {}

    double transform(double argument) const {
        return -argument;
    }

    double transformInverse(double argument) const {
        return -argument;
    }
};

class TransformNot : public TransformNode {
  public:
    TransformNot() {}

    double transform(double argument) const {
        return !static_cast<bool>(argument);
    }

    double transformInverse(double argument) const {
        return !static_cast<bool>(argument);
    }
};

class ValueTransformer {
  public:
    double transform(double argument) const;
    double transformInverse(double argument) const;

    static ValueTransformer* parseFromXml(QDomElement transformElement,
                                          const SkinContext& context);

  private:
    ValueTransformer();

    void addTransformer(TransformNode* pTransformer);

    QList<TransformNode*> m_transformers;
};

#endif /* VALUETRANSFORMER_H */
