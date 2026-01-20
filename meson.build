project('mftestapp', 'cpp',
  version: '0.1.0',
  license: 'MIT',
  meson_version: '>=1.0',
  default_options: ['cpp_std=c++17', 'warning_level=2'],
)

qt6_dep = dependency('qt6', modules: ['Core', 'Gui', 'OpenGL'])

executable('mftestapp', 'src/main.cpp',
  dependencies: qt6_dep,
  cpp_args: '-DAPP_VERSION="' + meson.project_version() + '"',
  install: true,
)

subdir('data')
