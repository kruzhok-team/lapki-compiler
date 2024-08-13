{}:
let
  nixpkgs = fetchGit { 
        url="https://github.com/NixOS/nixpkgs/";
        rev="90f456026d284c22b3e3497be980b2e47d0b28ac";
  };
  pkgs = import nixpkgs {};
in 
pkgs.callPackage ./derivation.nix {
  python3Packages = pkgs.python310Packages;
}