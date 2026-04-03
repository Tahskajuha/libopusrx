project(
  'opusrx',
  'c',
  version: '0.1',
  license: 'GPL3',
  meson_version: '>=0.56.0',
  default_options: [
    'c_std=c11',
    'warning_level=2',
    'werror=true'
  ]
)

srcs = files(
  'src/rtp.c',
  'src/jb.c'
)

deps = [dependency('opus', version: '>=1.5')]

libopusrx = library(
  'opusrx',
  srcs,
  dependencies: deps,
  install: true
)

install_headers('src/rtp.h', 'src/jb.h')
