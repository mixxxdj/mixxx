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

class TransformMatch : public TransformNode {
  public:
    TransformMatch(double match) : m_match(match) { }

    double transform(double argument) const {
        return argument == m_match ? 1.0 : 0.0;
    }

    double transformInverse(double argument) const {
        if (argument) {
            return m_match;
        }
        // Inverse is lossy -- we don't know what the original value
        // was based on the matched boolean success or failure.  But we can
        // at least return a value that would correctly fail a match.
        if (m_match == 0) {
            return 1.0;
        }
        return 0;
    }

  private:
    double m_match;
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
