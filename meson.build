project('libgb', 'c',
  version: '0.1',
  default_options: ['warning_level=3', 'werror=true', 'c_std=c11'])

include_directories = include_directories('include')

install_headers('include/libgb.h')

src = [
  'src/libgb.c'
]

libgb = library(
  'gb',
  src,
  install: true,
  include_directories: include_directories,
)

pkg = import('pkgconfig')
pkg.generate(libgb)

libgb_dep = declare_dependency(
  include_directories: include_directories,
  link_with: libgb
)
