{
  mkShell,
  cmake,
  ninja,
  gtest,
  clang-tools,
  utf8,
}:
mkShell {
  inputsFrom = [ utf8 ];

  packages = [
    cmake
    ninja
    gtest
    clang-tools
  ];
}
