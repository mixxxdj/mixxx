//
// Created by Ferran Pujol Camins on 2019-06-22.
//

#pragma once

#include <QPainter>

// This class provides RAII style management of a QPainter properties.
//
// PainterScope will save the painter state on creation, and restore it
// on destruction.
class PainterScope {
  public:
    PainterScope() = delete;

    explicit PainterScope(QPainter& painter)
            : m_painter(painter) {
        m_painter.save();
    }

    ~PainterScope() {
        m_painter.restore();
    }

  private:
    QPainter& m_painter;
};
