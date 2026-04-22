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

Both units run `mftestapp --report`, which prints the System and
Libraries probes to stdout without a GUI.

## Building

```sh
meson setup build
ninja -C build -j12
meson test -C build
```

Needs Qt 6 (`qt6-base-dev` on Debian).
