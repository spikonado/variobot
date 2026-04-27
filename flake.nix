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
            # Fixes https://github.com/ros/xacro/issues/380
            xacro = super.xacro.overrideAttrs (oldAttrs: {
              patches = (oldAttrs.patches or [ ]) ++ [
                (pkgs.fetchpatch {
                  url = "https://patch-diff.githubusercontent.com/raw/ros/xacro/pull/380.patch";
                  sha256 = "sha256-gIsdi7/SucXCXJrmmUfYHM+Dv3H3ZB0kVGqyDWRZOx8=";
                })
              ];
            });
          }
        );

        devTools = with pkgs; [
          bun
          colcon
          commitlint
          gcc
          gh
          git
          git-lfs
          prek
          prettier
        ];

        # Define the ROS environment with all necessary dependencies
        rosEnv = rosPkgs.buildEnv {
          paths =
            with pkgs;
            with rosPkgs;
            [
              ros-core
              ament-lint-common

              # Work around https://github.com/lopsided98/nix-ros-overlay/pull/624
              ament-cmake-core
              python-cmake-module

              # Dependencies from package.xml files
              ament-cmake
              controller-manager
              gz-ros2-control
              joint-state-broadcaster
              joint-state-topic-hardware-interface
              mecanum-drive-controller
              pid-controller
              robot-state-publisher
              ros-gz-bridge
              ros-gz-sim
              ros2launch
              rviz2
              sdformat-urdf
              xacro

              # micro_ros_agent dependencies
              fmt_9
              micro-ros-msgs
              rcutils
              rmw
              rmw-dds-common
              rmw-fastrtps-shared-cpp

              # Tools
              libsForQt5.qt5.qtwayland
              joint-state-publisher-gui
              plotjuggler-ros
              ros2controlcli
              rqt-common-plugins
            ];
        };
      in
      {
        formatter = pkgs.nixfmt-tree;
        legacyPackages = rosPkgs;

        devShells.default = pkgs.mkShell.override { stdenv = pkgs.clangStdenv; } {
          name = "variobot-shell";
          packages = [
            devTools
            rosEnv
          ];

          LD_LIBRARY_PATH = pkgs.lib.makeLibraryPath [ pkgs.mesa ];

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
    extra-substituters = [
      "https://spikonado.cachix.org"
      "https://ros.cachix.org"
      "https://attic.iid.ciirc.cvut.cz/ros"
    ];
    extra-trusted-public-keys = [
      "spikonado.cachix.org-1:MwA4hqRN0+DdP7/UnTn0yvJgVu65S1S0QVnAnsguev4="
      "ros.cachix.org-1:dSyZxI8geDCJrwgvCOHDoAfOm5sV1wCPjBkKL+38Rvo="
      "ros:JR95vUYsShSqfA1VTYoFt1Nz6uXasm5QrcOsGry9f6Q="
    ];
  };
}
