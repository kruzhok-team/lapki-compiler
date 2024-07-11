{ pkgs ? import <nixpkgs> {} }:

pkgs.callPackage ./derivation.nix {
  python3Packages = pkgs.python310Packages;
}
