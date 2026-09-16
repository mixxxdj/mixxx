#pragma once

#include <QUndoStack>

#include "util/singleton.h"

class UndoStack : public QUndoStack, public Singleton<UndoStack> {
};
