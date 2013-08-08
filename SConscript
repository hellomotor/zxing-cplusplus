# -*- python -*-

#
# SConscript file to specify the build process, see:
# http://scons.org/doc/production/HTML/scons-man.html
#
Decider('MD5')
import platform
import fnmatch
import os

vars = Variables()
vars.Add(BoolVariable('DEBUG', 'Set to disable optimizations', True))
vars.Add(BoolVariable('PIC', 'Set to 1 for to always generate PIC code', False))
env = Environment(variables = vars)
#env.Replace(CXX = 'clang++')

compile_options = {}
if platform.system() is 'Windows':
  compile_options['CXXFLAGS'] = '-D_CRT_SECURE_NO_WARNINGS /fp:fast /EHsc'
else:
  # Force ANSI (C++98) to ensure compatibility with MSVC.
  cxxflags = ['-ansi -pedantic']
  if env['DEBUG']:
    #compile_options['CPPDEFINES'] = '-DDEBUG'
    cxxflags.append('-O0 -g3 -ggdb')
    cxxflags.append('-Wall -Wextra -Werror -fPIC')
    # -Werror
  else:
    cxxflags.append('-Os -g3 -ggdb -Wall -Wextra -fPIC')
  compile_options['CXXFLAGS'] = ' '.join(cxxflags)
  compile_options['LINKFLAGS'] = '-ldl -L/usr/lib -L/opt/local/lib -L/usr/local/lib'

def all_files(dir, ext='.cpp', level=6):
  files = []
  for i in range(1, level):
    files += Glob(dir + ('/*' * i) + ext)
  return files

def all_libs(name, dir):
  matches = []
  for root, dirnames, filenames in os.walk(dir):
    for filename in fnmatch.filter(filenames, name):
      matches.append(os.path.join(root, filename))
  return matches

# Setup libiconv, if possible
libiconv_include = []
libiconv_libs = []
if all_libs('libiconv.*', '/opt/local/lib'):
  libiconv_include.append('/opt/local/include/')
  libiconv_libs.append('iconv')
else:
  if all_libs('libiconv.*', '/usr/lib'):
    libiconv_libs.append('iconv')

# Add libzxing library.
libzxing_files = all_files('core/src')+all_files('core/src', '.cc')
libzxing_include = ['core/src']
if platform.system() is 'Windows':
  libzxing_files += all_files('core/src/win32')
  libzxing_include += ['core/src/win32']
libzxing = env.Library('zxing', source=libzxing_files,
  CPPPATH=libzxing_include + libiconv_libs, **compile_options)

# Add cli.
zxing_files = all_files('cli/src')
zxing = env.Program('zxing', zxing_files,
  CPPPATH=libzxing_include,
  LIBS=libzxing + libiconv_libs, **compile_options)

# Setup CPPUnit.
cppunit_include = ['/opt/local/include/']
cppunit_libs = ['cppunit']

# Add testrunner program.
test_files = all_files('core/tests/src')
test = env.Program('testrunner', test_files,
  CPPPATH=libzxing_include + cppunit_include,
  LIBS=libzxing + cppunit_libs, **compile_options)

# Setup some aliases.
Alias('lib', libzxing)
Alias('zxing', zxing)
Alias('tests', test)
