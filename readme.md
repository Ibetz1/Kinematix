# Kinematix

**Kinematix** is a lightweight **2D rigid body physics simulation** written in C++, focused on clarity, correctness, and real-time visualization. It serves as a sandbox for experimenting with core physics concepts and as a foundation for a more complete physics or game engine.

---

## Features
- 2D rigid body simulation (position, velocity, rotation, angular velocity)
- Collision detection and impulse-based resolution
- Real-time rendering and debug visualization
- Clean, extensible architecture for future expansion

---

## Tech Stack
- **C++**
- **OpenGL** for rendering
- **SDL2** for windowing, input, and context creation

---

## Build

### Example (Windows / MinGW)
```bash
g++ src/*.cpp src/libs/glad/*.c -Iinc -Isrc/libs -I. -lSDL2 -lopengl32
```
Ensure SDL2 is correctly installed and that its DLL is available in your PATH or alongside the executable.

## Project Structure
- `src/` - Implementation and main.cpp with a pre-written test
- `inc/` - Core utilities (headers)

## Purpose
This project exists to explore and validate fundamental physics engine ideas, including stability, collision accuracy, and performance, while remaining simple enough to iterate on quickly.
