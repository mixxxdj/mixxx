#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import argparse
import fnmatch
import os
import os.path
import re
import sys
import tinycss.css21
import PyQt5.QtWidgets


RE_CPP_CLASSNAME = re.compile(r'^\s*class\s+([\w_]+)')
RE_CPP_OBJNAME = re.compile(r'setObjectName\(.*"([^"]+)"')
RE_UI_OBJNAME = re.compile(r'<widget[^>]+name="([^"]+)"')
RE_XML_OBJNAME = re.compile(r'<ObjectName>(.*)</ObjectName>')
RE_XML_OBJNAME_SETVAR = re.compile(
    r'<SetVariable\s+name="ObjectName">(.*)</SetVariable>')
RE_CLASSNAME = re.compile(r'^[A-Z]\w+$')
RE_OBJNAME_VARTAG = re.compile(r'<.*>')


def get_skins(mixxx_path):
    skins_path = os.path.join(mixxx_path, 'res', 'skins')
    for entry in os.scandir(skins_path):
        if entry.is_dir():
            yield entry.name


def make_glob(name):
    return RE_OBJNAME_VARTAG.sub('*', name)


def get_global_names(mixxx_path):
    classnames = set()
    objectnames = set()
    for root, dirs, fnames in os.walk(os.path.join(mixxx_path, 'src')):
        for fname in fnames:
            ext = os.path.splitext(fname)[1]
            if ext in ('.h', '.cpp'):
                fpath = os.path.join(root, fname)
                with open(fpath, mode='r') as f:
                    for line in f:
                        classnames.update(set(RE_CPP_CLASSNAME.findall(line)))
                        objectnames.update(set(RE_CPP_OBJNAME.findall(line)))
            elif ext == '.ui':
                fpath = os.path.join(root, fname)
                with open(fpath, mode='r') as f:
                    objectnames.update(set(RE_UI_OBJNAME.findall(f.read())))
    return classnames, objectnames


def get_skin_objectnames(mixxx_path, skin):
    skin_path = os.path.join(mixxx_path, 'res', 'skins', skin)
    for root, dirs, fnames in os.walk(skin_path):
        for fname in fnames:
            if os.path.splitext(fname)[1] != '.xml':
                continue

            fpath = os.path.join(root, fname)
            with open(fpath, mode='r') as f:
                for line in f:
                    yield from RE_XML_OBJNAME.findall(line)
                    yield from RE_XML_OBJNAME_SETVAR.findall(line)


def get_skin_stylesheets(mixxx_path, skin):
    cssparser = tinycss.css21.CSS21Parser()
    skin_path = os.path.join(mixxx_path, 'res', 'skins', skin)
    for filename in os.listdir(skin_path):
        if os.path.splitext(filename)[1] != '.qss':
            continue
        qss_path = os.path.join(skin_path, filename)
        stylesheet = cssparser.parse_stylesheet_file(qss_path)
        yield qss_path, stylesheet


def check_stylesheet(stylesheet, classnames, objectnames):
    for rule in stylesheet.rules:
        if not isinstance(rule, tinycss.css21.RuleSet):
            continue
        for token in rule.selector:
            if token.type == 'IDENT':
                if not RE_CLASSNAME.match(token.value):
                    continue
                if token.value in classnames:
                    continue
                if token.value in dir(PyQt5.QtWidgets):
                    continue
                yield (token, 'Unknown widget class "%s"' % token.value)

            elif token.type == 'HASH':
                value = token.value[1:]
                if value in objectnames:
                    continue

                if any(fnmatch.fnmatchcase(value, make_glob(objname))
                        for objname in objectnames if '<' in objname):
                    continue

                yield (token, 'Unknown object name "%s"' % token.value)


def main(argv=None):
    parser = argparse.ArgumentParser()
    parser.add_argument('-s', '--skin', help='skin name')
    parser.add_argument('-i', '--ignore', default='',
                        help='glob pattern to ignore')
    parser.add_argument('mixxx_path', help='Mixxx path')
    args = parser.parse_args(argv)

    mixxx_path = args.mixxx_path
    ignore_pattern = args.ignore.split(',')
    skins = set(get_skins(mixxx_path))
    if args.skin:
        skins = set(skin for skin in skins if skin == args.skin)

    if not skins:
        print('No skins to check')
        return 1

    status = 0
    classnames, objectnames = get_global_names(mixxx_path)
    for skin in sorted(skins):
        skin_objectnames = objectnames.union(set(
            get_skin_objectnames(mixxx_path, skin)))
        for qss_path, stylesheet in get_skin_stylesheets(mixxx_path, skin):
            for error in stylesheet.errors:
                status = 2
                print('%s:%d:%d: %s - %s' % (
                    qss_path, error.line, error.column,
                    error.__class__.__name__, error.reason,
                ))
            for token, message in check_stylesheet(
                    stylesheet, classnames, skin_objectnames):
                if any(fnmatch.fnmatchcase(token.value, pattern)
                       for pattern in ignore_pattern):
                    continue
                status = 2
                print('%s:%d:%d: %s' % (
                    qss_path, token.line, token.column, message,
                ))
    return status


if __name__ == '__main__':
    sys.exit(main())
