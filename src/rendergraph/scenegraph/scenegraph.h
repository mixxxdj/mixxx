#pragma once

#include <QQuickWindow>
#include <QSGNode>
#include <memory>

namespace rendergraph {
class Context;
class Node;

std::unique_ptr<Context> createSgContext(QQuickWindow* window);
QSGNode* sgNode(Node* pNode);
} // namespace rendergraph
