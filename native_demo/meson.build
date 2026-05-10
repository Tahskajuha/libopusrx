demo_srcs = files('main.c')
alsa_dep = dependency('alsa')
glib_dep = dependency('glib-2.0')

executable(
  'opusrx',
  demo_srcs,
  dependencies: [opusrx_dep, alsa_dep, glib_dep],
  install: get_option('install_demo')
)
