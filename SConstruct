# vim: ft=python
# $Id$
# Copyright (C) 2004 Scott Lamb <slamb@slamb.org>.
# This file is part of sigsafe, which is released under the MIT license.

import os
import re

# Determine our platform
arch = os.uname()[4]
if arch == 'Power Macintosh': arch = 'ppc'
if re.compile('i[3456]86').match(arch): arch = 'i386'
Export('arch')
os_name = os.uname()[0].lower().replace('-','')
Export('os_name')
type = 'debug'
buildDir = 'build-%s-%s-%s' % (arch, os_name, type)

env = Environment(
    CPPPATH = [
        '#/src',
        '#/src/' + arch + '-' + os_name
    ],
    CPPDEFINES = [
        '_XOPEN_SOURCE=600',
        '_REENTRANT',
        '_THREAD_SAFE',
    ],
    LIBPATH = [
        '#/' + buildDir
    ]
)

env.Append(LIBS = ['pthread'])

if os_name == 'darwin' and arch == 'ppc':
    env.Append(CPPDEFINES = ['HAVE_KEVENT'])
elif os_name == 'linux' and arch == 'i386':
    env.Append(CPPDEFINES = ['HAVE_POLL','HAVE_EPOLL'])

if env['CC'] == 'gcc':
    env.Append(CCFLAGS = ['-Wall'])
    if type == 'debug':
        env.Append(CCFLAGS=['-g'], LINKFLAGS=['-g'])
    else:
        env.Append(CPPDEFINES=['NDEBUG'],CCFLAGS=['-O2'],LINKFLAGS=['-O2'])

Export('env')
BuildDir(buildDir, 'src', 0)
SConscript(buildDir + '/SConscript')
SConscript('tests/SConscript')
