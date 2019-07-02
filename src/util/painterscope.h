//
// Created by Ferran Pujol Camins on 2019-06-22.
//

#pragma once

#include <QPainter>

// This class provides RAII style management of a QPainter properties.
//
// PainterScope will save the painter state on creation, and restore it
// on destruction.
class PainterScope final {
  public:
    PainterScope() = delete;
    PainterScope(const PainterScope&) = delete;
    PainterScope& operator=(const PainterScope&) = delete;

    explicit PainterScope(QPainter* pPainter)
            : m_pPainter(pPainter) {
        DEBUG_ASSERT(m_pPainter);
        m_pPainter->save();
    }

    ~PainterScope() {
        DEBUG_ASSERT(m_pPainter);
        m_pPainter->restore();
    }

  private:
    QPainter* const m_pPainter;
};
