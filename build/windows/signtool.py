#!/usr/bin/env python

import os
from SCons.Builder import Builder

def signtool_path(subject_name, path):
    print("Running signtool: ", path)
    command = "signtool sign /sm /n \"%s\" /v %s" % (subject_name, path)
    if os.system(command) != 0:
        raise Exception('signtool failed: ' + command)

def do_signtool(target, source, env):
    print('do_signtool', target, source, env)
    subject_name = env.get('SUBJECT_NAME', '')

    for s in source:
        path = str(s)
        if path.endswith('.exe') or path.endswith('.dll'):
            signtool_path(subject_name, path)

SignTool = Builder(action = do_signtool)

def generate(env):
    env['BUILDERS']['SignTool'] = SignTool

def exists(env):
    return os.platform == 'windows'
