# Manifold Test App

A small Qt application for testing Manifold.

## Host integration

To exercise Manifold's integration of an application into the host, the
app installs, below its prefix:

| File | What to check |
|---|---|
| `lib/systemd/system/com.example.mftestapp-report.service` | `systemctl start com.example.mftestapp-report`, then `journalctl -u com.example.mftestapp-report` |
| `lib/systemd/user/com.example.mftestapp-report.service` | the same with `--user` |
| `lib/udev/rules.d/60-mftestapp.rules` | `udevadm info /dev/null` shows `MFTESTAPP=1` |
| `share/mime/packages/com.example.mftestapp.xml` | `xdg-mime query filetype foo.mftest` gives `application/x-mftestapp` |
| `share/applications/com.example.mftestapp.open.desktop` | `xdg-open foo.mftest` opens the app on that file |

Both units run `mftestapp --report`, which prints the System, Plugins
and Libraries probes to stdout without a GUI.

## Shared library and plugins

The app links against its own shared library, `libmftestapp`, installed to
the usual `libdir` with its header and a pkg-config file.

The library is also the plugin API, see `lib/mftestapp.h`. A plugin is a
shared object exporting `mftest_plugin_info()`. The app loads every `*.so`
it finds, recursively, below the directories in `MFTESTAPP_PLUGIN_PATH`
(colon-separated) and below the `plugindir` build option, which defaults
to `LIBDIR/mftestapp/plugins`.

`examples/plugin` is an example plugin.

## Building

```sh
meson setup build
ninja -C build -j12
meson test -C build
```

Needs Qt 6 (`qt6-base-dev` on Debian).
