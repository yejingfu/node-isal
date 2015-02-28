{
  'targets': [
    {
      'target_name': 'isal',

      'defines': [
      ],

      'variables': {
        'ISAL_PATH': '../src_2.12',
      },

      'cflags!': ['-fno-exceptions'],
      'cflags_cc!':['-fnb-rtti', '-fno-exceptions', '-g', '-O0'],

      'sources': [
        'src/main.cc',
        'src/util.cc',
        'src/crcwrapper.cc',
      ],

      'include_dirs': [
        '<!(node -e \"require(\'nan\')\")',
        '<(ISAL_PATH)/include',
      ],

      'libraries': [
        '-L$(srcdir)/<(ISAL_PATH)/bin',
        '-lisa-l'  # make sure "libisa-l.a" must exist in correct folder
      ],

      'conditions': [
        ['OS=="linux"', {
        }],
      ],

    }
  ],
}

