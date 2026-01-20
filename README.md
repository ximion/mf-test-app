# Manifold Test App

A small Qt application for testing Manifold.

## Building

```sh
meson setup build
ninja -C build -j12
meson test -C build
```

Needs Qt 6 (`qt6-base-dev` on Debian).
