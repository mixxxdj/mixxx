dir ~/workspace/qt/src/corelib
dir ~/workspace/qt/src/corelib/io
dir ~/workspace/qt/src/gui
dir ~/workspace/qt/src/gui/image
dir ~/workspace/qt/src/gui/kernel
dir ~/workspace/qt/src/network
dir ~/workspace/qt/src/sql
dir ~/workspace/qt/src/sql/drivers/sqlit
dir ~/workspace/qt/src/opengl
dir ~/workspace/sqlite/sqlite3-3.7.4
dir ~/workspace/daschuer-mixxx-clementine/src
dir ~/workspace/daschuer-mixxx-clementine/src/core
dir ~/workspace/daschuer-mixxx-clementine/ext
dir ~/workspace/daschuer-mixxx-clementine/3rdparty


python
import sys

sys.path.insert(0, '/home/daniel/workspace')
from qt4 import register_qt4_printers
register_qt4_printers (None)
end
set print pretty 1
set charset UTF-8

