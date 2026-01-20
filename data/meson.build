app_id = 'com.example.mftestapp'
datadir = get_option('datadir')

install_data(app_id + '.desktop', install_dir: datadir / 'applications')
install_data(app_id + '.metainfo.xml', install_dir: datadir / 'metainfo')
install_data(app_id + '.svg', install_dir: datadir / 'icons/hicolor/scalable/apps')

appstreamcli = find_program('appstreamcli', required: false)
if appstreamcli.found()
  test('validate-metainfo', appstreamcli,
    args: ['validate', '--no-net', '--explain', files(app_id + '.metainfo.xml')])
endif
desktop_file_validate = find_program('desktop-file-validate', required: false)
if desktop_file_validate.found()
  test('validate-desktop', desktop_file_validate, args: files(app_id + '.desktop'))
endif
