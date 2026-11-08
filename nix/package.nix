{
  lib,
  stdenv,
  cmake,
  ninja,
  gtest,
}:
stdenv.mkDerivation (finalAttrs: {
  pname = "utf8";
  version = "0.0.1";

  src = lib.cleanSource ./..;

  nativeBuildInputs = [
    cmake
    ninja
  ];

  buildInputs = [ gtest ];

  doCheck = true;

  meta = {
    description = "UTF-8 utilities for C++";
    homepage = "https://github.com/drl/utf8";
    license = lib.licenses.mit;
    platforms = lib.platforms.all;
  };
})
