{
  "targets": [
    {
      "target_name": "isal",
      "sources": [
        "src/main.cc",
        "src/util.cc"
      ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")"
      ]
    }
  ]
}

