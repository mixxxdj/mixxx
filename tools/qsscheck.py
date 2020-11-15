#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import argparse
import fnmatch
import os
import os.path
import re
import sys
import tinycss.css21


RE_CPP_CLASSNAME = re.compile(r"^\s*class\s+([\w_]+)")
RE_CPP_OBJNAME = re.compile(r'setObjectName\(.*"([^"]+)"')
RE_UI_OBJNAME = re.compile(r'<widget[^>]+name="([^"]+)"')
RE_XML_OBJNAME = re.compile(r"<ObjectName>(.*)</ObjectName>")
RE_XML_OBJNAME_SETVAR = re.compile(
    r'<SetVariable\s+name="ObjectName">(.*)</SetVariable>'
)
RE_CLASSNAME = re.compile(r"^[A-Z]\w+$")
RE_OBJNAME_VARTAG = re.compile(r"<.*>")

# List of Qt Widgets, generated with:
#     python -c 'import inspect, from PyQt5 import QtWidgets;
#     print([k for k, v in QtWidgets.__dict__.items() if inspect.isclass(v)])'
QTWIDGETS = [
    "QWidget",
    "QAbstractButton",
    "QGraphicsItem",
    "QAbstractGraphicsShapeItem",
    "QAbstractItemDelegate",
    "QFrame",
    "QAbstractScrollArea",
    "QAbstractItemView",
    "QAbstractSlider",
    "QAbstractSpinBox",
    "QAction",
    "QActionGroup",
    "QApplication",
    "QLayoutItem",
    "QLayout",
    "QBoxLayout",
    "QButtonGroup",
    "QCalendarWidget",
    "QCheckBox",
    "QDialog",
    "QColorDialog",
    "QColumnView",
    "QComboBox",
    "QPushButton",
    "QCommandLinkButton",
    "QStyle",
    "QCommonStyle",
    "QCompleter",
    "QDataWidgetMapper",
    "QDateTimeEdit",
    "QDateEdit",
    "QDial",
    "QDialogButtonBox",
    "QDirModel",
    "QDockWidget",
    "QDoubleSpinBox",
    "QErrorMessage",
    "QFileDialog",
    "QFileIconProvider",
    "QFileSystemModel",
    "QFocusFrame",
    "QFontComboBox",
    "QFontDialog",
    "QFormLayout",
    "QGesture",
    "QGestureEvent",
    "QGestureRecognizer",
    "QGraphicsAnchor",
    "QGraphicsLayoutItem",
    "QGraphicsLayout",
    "QGraphicsAnchorLayout",
    "QGraphicsEffect",
    "QGraphicsBlurEffect",
    "QGraphicsColorizeEffect",
    "QGraphicsDropShadowEffect",
    "QGraphicsEllipseItem",
    "QGraphicsGridLayout",
    "QGraphicsItemGroup",
    "QGraphicsLineItem",
    "QGraphicsLinearLayout",
    "QGraphicsObject",
    "QGraphicsOpacityEffect",
    "QGraphicsPathItem",
    "QGraphicsPixmapItem",
    "QGraphicsPolygonItem",
    "QGraphicsWidget",
    "QGraphicsProxyWidget",
    "QGraphicsRectItem",
    "QGraphicsTransform",
    "QGraphicsRotation",
    "QGraphicsScale",
    "QGraphicsScene",
    "QGraphicsSceneEvent",
    "QGraphicsSceneContextMenuEvent",
    "QGraphicsSceneDragDropEvent",
    "QGraphicsSceneHelpEvent",
    "QGraphicsSceneHoverEvent",
    "QGraphicsSceneMouseEvent",
    "QGraphicsSceneMoveEvent",
    "QGraphicsSceneResizeEvent",
    "QGraphicsSceneWheelEvent",
    "QGraphicsSimpleTextItem",
    "QGraphicsTextItem",
    "QGraphicsView",
    "QGridLayout",
    "QGroupBox",
    "QHBoxLayout",
    "QHeaderView",
    "QInputDialog",
    "QItemDelegate",
    "QItemEditorCreatorBase",
    "QItemEditorFactory",
    "QKeyEventTransition",
    "QKeySequenceEdit",
    "QLCDNumber",
    "QLabel",
    "QLineEdit",
    "QListView",
    "QListWidget",
    "QListWidgetItem",
    "QMainWindow",
    "QMdiArea",
    "QMdiSubWindow",
    "QMenu",
    "QMenuBar",
    "QMessageBox",
    "QMouseEventTransition",
    "QOpenGLWidget",
    "QPanGesture",
    "QPinchGesture",
    "QPlainTextDocumentLayout",
    "QPlainTextEdit",
    "QProgressBar",
    "QProgressDialog",
    "QProxyStyle",
    "QRadioButton",
    "QRubberBand",
    "QScrollArea",
    "QScrollBar",
    "QScroller",
    "QScrollerProperties",
    "QShortcut",
    "QSizeGrip",
    "QSizePolicy",
    "QSlider",
    "QSpacerItem",
    "QSpinBox",
    "QSplashScreen",
    "QSplitter",
    "QSplitterHandle",
    "QStackedLayout",
    "QStackedWidget",
    "QStatusBar",
    "QStyleFactory",
    "QStyleHintReturn",
    "QStyleHintReturnMask",
    "QStyleHintReturnVariant",
    "QStyleOption",
    "QStyleOptionButton",
    "QStyleOptionComplex",
    "QStyleOptionComboBox",
    "QStyleOptionDockWidget",
    "QStyleOptionFocusRect",
    "QStyleOptionFrame",
    "QStyleOptionGraphicsItem",
    "QStyleOptionGroupBox",
    "QStyleOptionHeader",
    "QStyleOptionMenuItem",
    "QStyleOptionProgressBar",
    "QStyleOptionRubberBand",
    "QStyleOptionSizeGrip",
    "QStyleOptionSlider",
    "QStyleOptionSpinBox",
    "QStyleOptionTab",
    "QStyleOptionTabBarBase",
    "QStyleOptionTabWidgetFrame",
    "QStyleOptionTitleBar",
    "QStyleOptionToolBar",
    "QStyleOptionToolBox",
    "QStyleOptionToolButton",
    "QStyleOptionViewItem",
    "QStylePainter",
    "QStyledItemDelegate",
    "QSwipeGesture",
    "QSystemTrayIcon",
    "QTabBar",
    "QTabWidget",
    "QTableView",
    "QTableWidget",
    "QTableWidgetItem",
    "QTableWidgetSelectionRange",
    "QTapAndHoldGesture",
    "QTapGesture",
    "QTextEdit",
    "QTextBrowser",
    "QTimeEdit",
    "QToolBar",
    "QToolBox",
    "QToolButton",
    "QToolTip",
    "QTreeView",
    "QTreeWidget",
    "QTreeWidgetItem",
    "QTreeWidgetItemIterator",
    "QUndoCommand",
    "QUndoGroup",
    "QUndoStack",
    "QUndoView",
    "QVBoxLayout",
    "QWhatsThis",
    "QWidgetAction",
    "QWidgetItem",
    "QWizard",
    "QWizardPage",
]


def get_skins(path):
    """Yields (skin_name, skin_path) tuples for each skin directory in path."""
    for entry in os.scandir(path):
        if entry.is_dir():
            yield entry.name, os.path.join(path, entry.name)


def get_global_names(mixxx_path):
    """Returns 2 sets with all class and object names in the Mixxx codebase."""
    classnames = set()
    objectnames = set()
    for root, dirs, fnames in os.walk(os.path.join(mixxx_path, "src")):
        for fname in fnames:
            ext = os.path.splitext(fname)[1]
            if ext in (".h", ".cpp"):
                fpath = os.path.join(root, fname)
                with open(fpath, mode="r") as f:
                    for line in f:
                        classnames.update(set(RE_CPP_CLASSNAME.findall(line)))
                        objectnames.update(set(RE_CPP_OBJNAME.findall(line)))
            elif ext == ".ui":
                fpath = os.path.join(root, fname)
                with open(fpath, mode="r") as f:
                    objectnames.update(set(RE_UI_OBJNAME.findall(f.read())))
    return classnames, objectnames


def get_skin_objectnames(skin_path):
    """
    Yields all object names in the skin_path.

    Note the names may contain one or more <Variable name="x"> tags, so it's
    not enough to check if a name CSS object name is in this list using "in".
    """
    for root, dirs, fnames in os.walk(skin_path):
        for fname in fnames:
            if os.path.splitext(fname)[1] != ".xml":
                continue

            fpath = os.path.join(root, fname)
            with open(fpath, mode="r") as f:
                for line in f:
                    yield from RE_XML_OBJNAME.findall(line)
                    yield from RE_XML_OBJNAME_SETVAR.findall(line)


def get_stylesheets(path):
    """Yields (qss_path, stylesheet) tuples for each qss file in path)."""
    cssparser = tinycss.css21.CSS21Parser()
    for filename in os.listdir(path):
        if os.path.splitext(filename)[1] != ".qss":
            continue
        qss_path = os.path.join(path, filename)
        stylesheet = cssparser.parse_stylesheet_file(qss_path)
        yield qss_path, stylesheet


def check_stylesheet(stylesheet, classnames, objectnames, objectnames_fuzzy):
    """Yields (token, problem) tuples for each problem found in stylesheet."""
    for rule in stylesheet.rules:
        if not isinstance(rule, tinycss.css21.RuleSet):
            continue
        for token in rule.selector:
            if token.type == "IDENT":
                if not RE_CLASSNAME.match(token.value):
                    continue
                if token.value in classnames:
                    continue
                if token.value in QTWIDGETS:
                    continue
                yield (token, 'Unknown widget class "%s"' % token.value)

            elif token.type == "HASH":
                value = token.value[1:]
                if value in objectnames:
                    continue

                if any(
                    fnmatch.fnmatchcase(value, objname)
                    for objname in objectnames_fuzzy
                ):
                    continue

                yield (token, 'Unknown object name "%s"' % token.value)


def check_skins(mixxx_path, skins, ignore_patterns=()):
    """
    Yields error messages for skins using class/object names from mixxx_path.

    By providing a list of ignore_patterns, you can ignore certain class or
    object names (e.g. #Test, #*Debug).
    """
    classnames, objectnames = get_global_names(mixxx_path)

    # Check default stylesheets
    default_styles_path = os.path.join(mixxx_path, "res", "skins")
    for qss_path, stylesheet in get_stylesheets(default_styles_path):
        for error in stylesheet.errors:
            yield "%s:%d:%d: %s - %s" % (
                qss_path,
                error.line,
                error.column,
                error.__class__.__name__,
                error.reason,
            )
        for token, message in check_stylesheet(
            stylesheet, classnames, objectnames, []
        ):
            if any(
                fnmatch.fnmatchcase(token.value, pattern)
                for pattern in ignore_patterns
            ):
                continue
            yield "%s:%d:%d: %s" % (
                qss_path,
                token.line,
                token.column,
                message,
            )

    # Check skin stylesheets
    for skin_name, skin_path in sorted(skins):
        # If the skin objectname is something like 'Deck<Variable name="i">',
        # then replace it with 'Deck*' and use glob-like matching
        skin_objectnames = objectnames.copy()
        skin_objectnames_fuzzy = set()
        for objname in get_skin_objectnames(skin_path):
            new_objname = RE_OBJNAME_VARTAG.sub("*", objname)
            if "*" in new_objname:
                skin_objectnames_fuzzy.add(new_objname)
            else:
                skin_objectnames.add(new_objname)

        for qss_path, stylesheet in get_stylesheets(skin_path):
            for error in stylesheet.errors:
                yield "%s:%d:%d: %s - %s" % (
                    qss_path,
                    error.line,
                    error.column,
                    error.__class__.__name__,
                    error.reason,
                )
            for token, message in check_stylesheet(
                stylesheet,
                classnames,
                skin_objectnames,
                skin_objectnames_fuzzy,
            ):
                if any(
                    fnmatch.fnmatchcase(token.value, pattern)
                    for pattern in ignore_patterns
                ):
                    continue
                yield "%s:%d:%d: %s" % (
                    qss_path,
                    token.line,
                    token.column,
                    message,
                )


def main(argv=None):
    """Main method for handling command line arguments."""
    parser = argparse.ArgumentParser(
        "qsscheck",
        description="Check QSS styles for non-existing object/class names",
    )
    parser.add_argument(
        "-p",
        "--extra-skins-path",
        help="Additional skin path, to check (.e.g. ~/.mixxx/skins)",
    )
    parser.add_argument("-s", "--skin", help="Only check skin with this name")
    parser.add_argument(
        "-i",
        "--ignore",
        default="",
        help="Glob pattern of class/object names to ignore (e.g. '#Test*'), "
        "separated by commas",
    )
    parser.add_argument("mixxx_path", help="Path of Mixxx sources/git repo")
    args = parser.parse_args(argv)

    mixxx_path = args.mixxx_path

    skins_path = os.path.join(mixxx_path, "res", "skins")
    skins = set(get_skins(skins_path))
    if args.extra_skins_path:
        skins.update(set(get_skins(args.extra_skins_path)))

    if args.skin:
        skins = set((name, path) for name, path in skins if name == args.skin)

    if not skins:
        print("No skins to check")
        return 1

    status = 0
    for message in check_skins(mixxx_path, skins, args.ignore.split(",")):
        print(message)
        status = 2
    return status


if __name__ == "__main__":
    sys.exit(main())
