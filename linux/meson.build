desktop_file = i18n.merge_file(
	input: 'com.github.Amide.desktop.in',
	output: 'com.github.Amide.desktop',
	type: 'desktop',
	po_dir: '../po',
	install: true,
	install_dir: get_option('datadir') / 'applications'
)

desktop_utils = find_program('desktop-file-validate', required: false)
if desktop_utils.found()
  test('Validate desktop file', desktop_utils, args: [desktop_file])
endif

appstream_file = i18n.merge_file(
        input: 'com.github.Amide.metainfo.xml.in',
       output: 'com.github.Amide.metainfo.xml',
       po_dir: '../po',
      install: true,
  install_dir: get_option('datadir') / 'metainfo'
)

appstreamcli = find_program('appstreamcli', required: false, disabler: true)
test('Validate appstream file', appstreamcli,
     args: ['validate', '--no-net', '--explain', appstream_file])

install_man('man/amide.1')
