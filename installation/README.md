# Installation

## APT - Ubuntu 24.04

### Source Installation

You should have [ROS 2 Kilted already installed](https://docs.ros.org/en/kilted/Installation.html).

#### 1. Clone the necessary repositories/packages

```bash
git clone https://github.com/spikonado/variobot.git ~/variobot_ws/src/variobot
cd ~/variobot_ws/src
vcs import ./ < ./variobot/variobot-apt-ubuntu.repos
```

#### 2. Build the packages

```bash
cd ~/variobot_ws
rosdep install --from-paths src --ignore-src -r -y
colcon build --symlink-install
```

If building on a weak system:

```bash
export MAKEFLAGS="-j 1" && colcon build --parallel-workers=1 --executor sequential --symlink-install
```

#### 3. Source the workspace

```bash
source ~/variobot_ws/install/setup.bash
```

> [!NOTE]
> You need to run `source install/setup.bash`, every time you create a new terminal and do something related to the project.

## Nix

Install Nix: https://nixos.org/nix/download.html

### Source Installation

#### 1. Clone the necessary repositories/packages

```bash
git clone https://github.com/spikonado/variobot.git ~/variobot_ws/src/variobot
```

#### 2. Activate the environment and build the packages

```bash
cd ~/variobot_ws
nix develop ./src/variobot/
colcon build --symlink-install
```

If building on a weak system:

```bash
cd ~/variobot_ws
nix develop ./src/variobot/
export MAKEFLAGS="-j 1" && colcon build --parallel-workers=1 --executor sequential --symlink-install
```

#### 4. Source the workspace

```bash
source ~/variobot_ws/install/setup.bash
```

> [!NOTE]
> You need to run `cd ~/variobot_ws && nix develop ./src/variobot/ --command "source install/setup.bash; return"`, every time you create a new terminal and do something related to the project.

#### 5. (Optional) Setup direnv

This would allow `nix develop` and `source install/setup.bash` (with colcon still working properly) to be automatically run when you `cd` the workspace and for the build dependencies to not be garbage collected.

First [install direnv](https://github.com/nix-community/nix-direnv), then execute the following:

```bash
nix profile install github:wentasah/ros-direnv --no-write-lock-file
cd ~/variobot_ws
ros-direnv-setup
echo "use flake ../src/variobot/" >> ./.buildenv/.envrc
direnv allow .buildenv
direnv allow .
```
