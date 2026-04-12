# jni_srcs = files('opusrx_jni.c')

jni_dep = dependency('jni', required: true)

shared_library(
  'opusrx-jni',
  # jni_srcs,
  dependencies: [
    opusrx_dep,
    jni_dep
  ],
  install: true
)
