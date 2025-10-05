{
  description = "Simple flake";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-25.05";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem ( system:
    let
      pkgs = import nixpkgs { inherit system; };
    in {
      devShell = pkgs.mkShell {
        name = "c++";

        nativeBuildInputs = with pkgs; [
          clang
          clang-tools
          boost
          (pkgs.python3.withPackages (ps: with ps; [ matplotlib pandas ]))
        ];
      };
    });
}