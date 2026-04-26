demo_srcs = files('main.c')
alsa_dep = dependency('alsa')

executable(
  'opusrx',
  demo_srcs,
  dependencies: [opusrx_dep, alsa_dep],
  install: get_option('install_demo')
)
