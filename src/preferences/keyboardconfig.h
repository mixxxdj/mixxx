#pragma once

#include <memory>

#include "preferences/configobject.h"

using KeyboardConfig = ConfigObject<ConfigValueKbd>;
using KeyboardConfigPointer = std::shared_ptr<KeyboardConfig>;
