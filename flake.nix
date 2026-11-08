{
  description = "UTF-8 utilities for C++";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs =
    { self, nixpkgs, ... }:
    let
      systems = [
        "aarch64-darwin"
        "aarch64-linux"
        "x86_64-linux"
      ];
      forAllSystems = f: nixpkgs.lib.genAttrs systems (system: f system nixpkgs.legacyPackages.${system});
    in
    {
      overlays.default = final: _: {
        utf8 = final.callPackage ./nix/package.nix { };
      };

      packages = forAllSystems (
        _: pkgs: {
          default = pkgs.callPackage ./nix/package.nix { };
        }
      );

      checks = forAllSystems (
        system: _: {
          utf8 = self.packages.${system}.default;
        }
      );

      devShells = forAllSystems (
        system: pkgs: {
          default = pkgs.callPackage ./nix/shell.nix {
            utf8 = self.packages.${system}.default;
          };
        }
      );

      formatter = forAllSystems (_: pkgs: pkgs.nixfmt);
    };
}
