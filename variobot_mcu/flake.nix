{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
    esp-dev.url = "github:mirrexagon/nixpkgs-esp-dev";
  };

  outputs =
    {
      self,
      nixpkgs,
      flake-utils,
      esp-dev,
    }:
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = import nixpkgs {
          inherit system;

          # esptool dependency
          config.permittedInsecurePackages = [
            "python3.13-ecdsa-0.19.1"
          ];
        };

        # Access the packages from the esp-dev flake
        esp-pkgs = esp-dev.packages.${system};
      in
      {
        formatter = pkgs.nixfmt-tree;

        devShells.default = pkgs.mkShell.override { stdenv = pkgs.clangStdenv; } {
          name = "variobot-mcu-shell";
          packages =
            with pkgs;
            with esp-pkgs;
            [
              gcc
              uv
              (esp-idf-full.override {
                rev = "v5.5.4";
                sha256 = "sha256-rItbBrwItkfJf8tKImAQsiXDR95sr0LqaM51gDZG/nI=";
              })
            ];

          shellHook = ''
            uv sync
            . .venv/bin/activate
          '';
        };
      }
    );
  nixConfig = {
    extra-substituters = [
      "https://spikonado.cachix.org"
    ];
    extra-trusted-public-keys = [
      "spikonado.cachix.org-1:MwA4hqRN0+DdP7/UnTn0yvJgVu65S1S0QVnAnsguev4="
    ];
  };
}
