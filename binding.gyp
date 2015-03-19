{
  'targets': [
    {
      'target_name': 'isal',

      'defines': [
      ],

      'variables': {
        'ISAL_PATH': '<!(echo ${ISAL_HOME})'
      },

      'cflags!': ['-fno-exceptions'],
      'cflags_cc!':['-fnb-rtti', '-fno-exceptions', '-g', '-O0'],

      'sources': [
        'src/main.cc',
        'src/util.cc',
        'src/crcwrapper.cc',
        'src/aeswrapper.cc',
        # The following files are auto-genrated by tools.
        #'src/aes_xts_wrapper.cc',
        #'src/crc_wrapper.cc',
        #'src/erasure_code_wrapper.cc',
        #'src/gf_vect_mul_wrapper.cc',
        #'src/igzip_lib_wrapper.cc',
        'src/mb_md5_wrapper.cc',
        'src/mb_sha1_wrapper.cc',
        'src/mb_sha256_wrapper.cc',
        'src/mb_sha512_wrapper.cc',
        #'src/md5_mb_wrapper.cc',
        'src/mem_routines_wrapper.cc',
        'src/raid_wrapper.cc',
        #'src/sha1_mb_wrapper.cc',
        #'src/sha256_mb_wrapper.cc',
        #'src/sha512_mb_wrapper.cc',
        #'src/sha_wrapper.cc',
      ],

      'include_dirs': [
        '<!(node -e \"require(\'nan\')\")',
        '<(ISAL_PATH)/include'
      ],

      'libraries': [
        '-L<(ISAL_PATH)/bin',
        '-lisa-l'  # make sure "libisa-l.a" must exist in correct folder
      ],

      'conditions': [
        ['OS=="linux"', {
        }],
      ],

    }
  ],
}

