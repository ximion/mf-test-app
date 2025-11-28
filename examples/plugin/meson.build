# Builds the example plugin against an installed Manifold test app, the
# way a separate plugin project would.
project('mftestapp-example-plugin', 'c',
  version: '0.1.0',
  meson_version: '>=1.0',
)

mftestapp_dep = dependency('mftestapp')

shared_module('example', 'example.c',
  name_prefix: '',
  dependencies: mftestapp_dep,
  install: true,
  install_dir: mftestapp_dep.get_variable(pkgconfig: 'plugindir'),
)
