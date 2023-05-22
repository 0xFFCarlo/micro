# Micro Game Engine 

Welcome to Micro, a C99 2D video game engine.

## Dependencies

This project depends on the following libraries:

- SDL2
- SDL2_mixer

You will need to install these dependencies to be able to build the project. Here's how you can do it on some popular operating systems:

### Ubuntu

You can use the package manager `apt` to install the dependencies. Open a terminal and type:

```bash
sudo apt update
sudo apt install libsdl2-dev libsdl2-mixer-dev
```
### MacOS
If you're on MacOS, you can use the brew package manager:

```bash
brew install sdl2 sdl2_mixer

```
### Windows

On Windows, you can download the development libraries from the SDL website:

- [SDL2 Development Libraries](https://www.libsdl.org/download-2.0.php)
- [SDL2_mixer Development Libraries](https://www.libsdl.org/projects/SDL_mixer/)


## Building the Project

This project uses CMake as its build system. If you haven't installed CMake, you can download it from [here](https://cmake.org/download/).

To build the project, follow these steps:

1. Open a terminal and navigate to the root directory of the project.

2. Run CMake to generate the build files:

```bash
cmake .
```

3. Build the project:
```bash
make 
```

4. Run the game:
```bash
./bin/main
```

