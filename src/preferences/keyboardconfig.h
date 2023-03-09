#pragma once

#include <memory>

#include "preferences/configobject.h"

typedef ConfigObject<ConfigValueKbd> KeyboardConfig;
typedef std::shared_ptr<KeyboardConfig> KeyboardConfigPointer;
