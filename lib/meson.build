libmftestapp = shared_library('mftestapp', 'mftestapp.c',
  version: meson.project_version(),
  soversion: '0',
  dependencies: dependency('dl'),
  install: true,
)
install_headers('mftestapp.h')

libmftestapp_dep = declare_dependency(
  link_with: libmftestapp,
  include_directories: include_directories('.'),
)

import('pkgconfig').generate(libmftestapp,
  name: 'mftestapp',
  description: 'Plugin API of the Manifold test app',
  variables: ['plugindir=' + plugindir],
)
