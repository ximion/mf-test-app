project('mftestapp', 'cpp', 'c',
  version: '0.1.0',
  license: 'MIT',
  meson_version: '>=1.0',
  default_options: ['cpp_std=c++17', 'c_std=c11', 'warning_level=2'],
)

plugindir = get_option('plugindir')
if plugindir == ''
  plugindir = get_option('prefix') / get_option('libdir') / 'mftestapp/plugins'
endif

subdir('lib')

qt6_dep = dependency('qt6', modules: ['Core', 'Gui', 'Widgets', 'OpenGL', 'OpenGLWidgets', 'DBus'])

executable('mftestapp', 'src/main.cpp',
  dependencies: [qt6_dep, libmftestapp_dep],
  cpp_args: [
    '-DAPP_VERSION="' + meson.project_version() + '"',
    '-DPLUGINDIR="' + plugindir + '"',
  ],
  install: true,
)

subdir('data')
subdir('examples')
subdir('tests')

devenv = environment()
devenv.set('MFTESTAPP_PLUGIN_PATH', meson.current_build_dir() / 'examples')
meson.add_devenv(devenv)
