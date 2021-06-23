#pragma once

// General tool for removing concrete dependencies while still incrementing a
// reference count.
class BaseReferenceHolder {
  public:
    BaseReferenceHolder() { }
    virtual ~BaseReferenceHolder() { }
};

template <class T>
class ReferenceHolder : public BaseReferenceHolder {
  public:
    ReferenceHolder(QSharedPointer<T>& reference)
            : m_reference(reference) {
    }
    virtual ~ReferenceHolder() {}

  private:
    QSharedPointer<T> m_reference;
};
