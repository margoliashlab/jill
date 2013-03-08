import os

if hasattr(os,'uname'):
    system = os.uname()[0]
else:
    system = 'Windows'

version = '1.2.0b1'
libname = 'jill'

# install location
AddOption('--prefix',
          dest='prefix',
          type='string',
          nargs=1,
          action='store',
          metavar='DIR',
          help='installation prefix')
AddOption('--bindir',
          dest='bindir',
          type='string',
          nargs=1,
          action='store',
          metavar='DIR',
          help='binary installation dir')
AddOption('--libdir',
          dest='libdir',
          type='string',
          nargs=1,
          action='store',
          metavar='DIR',
          help='library installation')
# debug flags for compliation
debug = ARGUMENTS.get('debug',1)

if not GetOption('prefix')==None:
    install_prefix = GetOption('prefix')
else:
    install_prefix = '/usr/local/'
if not GetOption('bindir')==None:
    install_bindir = GetOption('bindir')
else:
    install_bindir = os.path.join(install_prefix,'bin')
if not GetOption('libdir')==None:
    install_libdir = GetOption('libdir')
else:
    install_libdir = os.path.join(install_prefix,'bin')


Help("""
Type: 'scons library' to build the library
      'scons install' to install library and headers under %s
      'scons examples' to compile examples
      (use --prefix  to change library installation location)

Options:
      debug=1      to enable debug compliation
""" % install_prefix)



env = Environment(ENV=os.environ,
		  PREFIX=install_prefix,
		  LIBDIR=install_libdir,
		  BINDIR=install_bindir,
                  tools=['default'])

if os.environ.has_key('CFLAGS'):
    env.Append(CCFLAGS=os.environ['CFLAGS'].split())
if os.environ.has_key('CXXFLAGS'):
    env.Append(CXXFLAGS=os.environ['CXXFLAGS'].split())
if os.environ.has_key('LDFLAGS'):
    env.Append(LINKFLAGS=os.environ['LDFLAGS'].split())

if system=='Darwin':
    env.Append(CPPPATH=['/opt/local/include'],
               LIBPATH=['/opt/local/lib'])
if int(debug):
    env.Append(CCFLAGS=['-g2', '-Wall','-DDEBUG=1'],
               CPPPATH=['#/arf'])
else:
    env.Append(CCFLAGS=['-O2'])

lib = SConscript('jill/SConscript', exports='env libname')
SConscript('modules/SConscript', exports='env lib')
SConscript('test/SConscript', exports='env lib')

if hasattr(env,'Doxygen'):
    dox = env.Doxygen('doc/doxy.cfg')
    env.Alias("docs",dox)
