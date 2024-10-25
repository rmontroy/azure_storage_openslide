project(
  'libremotetiff', 'cpp',
  version : '0.1.0',
  license : 'Unlicense',
  default_options : [
    'default_library=static',
    'buildtype=debugoptimized',
    'cpp_std=c++14',
    'warning_level=2',
  ],
  meson_version : '>=0.54',
)

# Compiler flags
cc = meson.get_compiler('cpp')
add_project_arguments(
  cc.get_supported_arguments(
    '-Wmissing-declarations',
    '-fno-common',
    '-ffunction-sections',
    '-fdata-sections',
  ),
  language : 'cpp'
)
add_project_link_arguments(
  cc.get_supported_link_arguments(
    '-Wl,--no-undefined',
  ),
  language : 'cpp'
)

# Library
libremotetiff_sources = [
  'remotetiff.cpp',
  'context.cpp',
]
libremotetiff = library('remotetiff',
  libremotetiff_sources,
  version : '1.0.0',
  link_args : [],
  link_language : 'cpp',
  gnu_symbol_visibility : 'hidden',
  include_directories : '.',
  dependencies : [
    dependency('azure-identity-cpp', method : 'cmake', modules : ['Threads::Threads', 'OpenSSL::Crypto', 'OpenSSL::SSL', 'OpenSSL::applink', 'ZLIB::ZLIB', 'CURL::libcurl_static', 'CURL::libcurl', 'Azure::azure-core', 'Azure::azure-identity']),
    dependency('azure-storage-blobs-cpp', method : 'cmake', modules : ['Threads::Threads', 'OpenSSL::Crypto', 'OpenSSL::SSL', 'OpenSSL::applink', 'ZLIB::ZLIB', 'CURL::libcurl_static', 'CURL::libcurl', 'Azure::azure-core', 'LibXml2::LibXml2', 'Azure::azure-storage-common', 'Azure::azure-storage-blobs']),
  ],
  install : false,
)
libremotetiff_dep = declare_dependency(
  include_directories : include_directories('.'),
  link_with : libremotetiff,
  link_args : ['-ldl'],
)