app_id = 'com.example.mftestapp'
prefix = get_option('prefix')
datadir = get_option('datadir')

install_data(app_id + '.desktop', app_id + '.open.desktop', install_dir: datadir / 'applications')
install_data(app_id + '.metainfo.xml', install_dir: datadir / 'metainfo')
install_data(app_id + '.svg', install_dir: datadir / 'icons/hicolor/scalable/apps')
install_data(app_id + '.xml', install_dir: datadir / 'mime/packages')
install_data('60-mftestapp.rules', install_dir: prefix / 'lib/udev/rules.d')

foreach kind, wanted_by : {'system': 'multi-user.target', 'user': 'default.target'}
  unit = configure_file(
    input: app_id + '-report.service.in',
    output: app_id + '-report.' + kind + '.service',
    configuration: {
      'BINDIR': prefix / get_option('bindir'),
      'KIND': kind,
      'WANTEDBY': wanted_by,
    },
  )
  install_data(unit,
    install_dir: prefix / 'lib/systemd' / kind,
    rename: app_id + '-report.service',
  )
endforeach

appstreamcli = find_program('appstreamcli', required: false)
if appstreamcli.found()
  test('validate-metainfo', appstreamcli,
    args: ['validate', '--no-net', '--explain', files(app_id + '.metainfo.xml')])
endif
desktop_file_validate = find_program('desktop-file-validate', required: false)
if desktop_file_validate.found()
  test('validate-desktop', desktop_file_validate,
    args: files(app_id + '.desktop', app_id + '.open.desktop'))
endif
udevadm = find_program('udevadm', required: false)
if udevadm.found()
  test('validate-udev', udevadm, args: ['verify', files('60-mftestapp.rules')])
endif
