{
  inputs = {
    nix-ros-overlay.url = "github:lopsided98/nix-ros-overlay/master";
    nixpkgs.follows = "nix-ros-overlay/nixpkgs"; # IMPORTANT!!!
  };
  outputs =
    {
      self,
      nix-ros-overlay,
      nixpkgs,
    }:
    nix-ros-overlay.inputs.flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = import nixpkgs {
          inherit system;
          overlays = [ nix-ros-overlay.overlays.default ];
        };

        rosDistro = "kilted";

        rosPkgs = (pkgs.rosPackages.${rosDistro}).overrideScope (
          self: super: {
            # Fixes https://github.com/ros/joint_state_publisher/issues/105
            joint-state-publisher = super.joint-state-publisher.overrideAttrs (oldAttrs: {
              patches = (oldAttrs.patches or [ ]) ++ [
                (pkgs.fetchpatch {
                  url = "https://patch-diff.githubusercontent.com/raw/ros/joint_state_publisher/pull/106.patch";
                  sha256 = "sha256-ll88Yy8R9mHLKPZ7GX+GbBJtMtDLJ1X0YLV1pcZbRs4=";
                  stripLen = 1;
                  includes = [ "joint_state_publisher/joint_state_publisher.py" ];
                })
              ];
            });
          }
        );

        colconWrapped = pkgs.writeShellScriptBin "colcon" ''
          # Exclude lint from colcon test due to ament_xmllint errors with nix
          if [ "$1" = "test" ]; then
            shift
            exec ${pkgs.colcon}/bin/colcon test "$@" --ctest-args -LE lint
          else
            exec ${pkgs.colcon}/bin/colcon "$@"
          fi
        '';

        devTools = with pkgs; [
          commitlint
          gh
          git
          git-lfs
          prek
          prettier
        ];

        # Define the ROS environment with all necessary dependencies
        rosEnv = rosPkgs.buildEnv {
          wrapPrograms = false;
          paths =
            with pkgs;
            with rosPkgs;
            [
              colcon
              ros-core

              # Work around https://github.com/lopsided98/nix-ros-overlay/pull/624
              ament-cmake-core
              python-cmake-module

              # Dependencies from package.xml
              ament-cmake
              ament-copyright
              ament-flake8
              ament-pep257
              ament-xmllint
              robot-state-publisher
              ros2launch
              rviz2
              sdformat-urdf
              xacro

              # Tools
              joint-state-publisher-gui
              rqt-common-plugins
            ];
        };
      in
      {
        formatter = pkgs.nixfmt-tree;
        legacyPackages = rosPkgs;

        devShells.default = pkgs.mkShell {
          name = "variobot-shell";
          packages = [
            colconWrapped
            devTools
            rosEnv
          ];

          shellHook = ''
            # Setup ROS 2 shell completion. Doing it in direnv is useless.
            if [[ ! $DIRENV_IN_ENVRC ]]; then
                eval "$(${pkgs.python3Packages.argcomplete}/bin/register-python-argcomplete ros2)"
                eval "$(${pkgs.python3Packages.argcomplete}/bin/register-python-argcomplete colcon)"
            fi
          '';
        };
      }
    );
  nixConfig = {
    # Cache details from:
    # 1. https://github.com/lopsided98/nix-ros-overlay#configure-binary-cache
    # 2. https://github.com/wentasah/nix-ros-hydra#binary-cache
    extra-substituters = [
      "https://ros.cachix.org"
      "https://attic.iid.ciirc.cvut.cz/ros"
    ];
    extra-trusted-public-keys = [
      "ros.cachix.org-1:dSyZxI8geDCJrwgvCOHDoAfOm5sV1wCPjBkKL+38Rvo="
      "ros:JR95vUYsShSqfA1VTYoFt1Nz6uXasm5QrcOsGry9f6Q="
    ];
  };
}
