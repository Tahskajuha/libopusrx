srcs = files(
  'rtp.c',
  'pipeline.c',
  'opusrx.c'
)

inc = include_directories('.')

deps = [dependency('opus', version: '>=1.5')]

libopusrx = library(
  'opusrx',
  srcs,
  include_directories: inc,
  dependencies: deps,
  version: '0.1.0',
  soversion: '0',
  install: true
)

opusrx_dep = declare_dependency(
  link_with: libopusrx,
  include_directories: inc,
  dependencies: deps
)

install_headers('opusrx/opusrx.h', subdir: 'opusrx')
