{
  'targets': [
    {
      'target_name': 'isal',

      'defines': [
      ],

      'variables': {
        'ISAL_PATH': '~/labs/isal/src_2.12',
      },

      'sources': [
        'src/main.cc',
        'src/util.cc',
      ],

      'include_dirs': [
        '<!(node -e \"require(\'nan\')\")',
        '<(ISAL_PATH)/include',
      ],

      'libraries': [
        '-L<(ISAL_PATH)/bin'
        '-lisa-l'
      ],

      'conditions': [
        ['OS=="linux"', {
        }],
      ],

    }
  ],
}

