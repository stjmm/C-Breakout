# Breakout (C Implementation of LearnOpenGL Game)

A C port of the **Breakout game** from [LearnOpenGL.com](https://learnopengl.com/In-Practice/2D-Game/Breakout), implemented from scratch with **modern OpenGL** and a minimal C toolchain.

Written entirely in C.  
Uses original **assets** from LearnOpenGL.com.  
Dependencies: **GLFW**, **GLAD**, **linmath.h**, **stb_image.h**, **miniaudio.h**.

---

## Features

- Implemented every feature from the original c++ version.

---

## Build Instructions (CMake)

### Requirements

- A C compiler: `gcc`, `clang`, or MSVC
- `cmake` ≥ 3.15
- `git`
- OpenGL 3.3-capable GPU

---

```bash
# Clone the repository and its submodules
git clone --recurse-submodules https://github.com/yourusername/breakout-c.git
cd breakout-c

# Create build directory
mkdir build && cd build

# Configure and build
cmake ..
make -j

# Run the game
./breakout

# Use Visual Studio or a terminal on Windows
