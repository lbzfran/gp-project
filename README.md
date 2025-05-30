# 🎮 GraPhics Project – 3D Graphics Engine Mockup

This project is a 3D graphics engine mockup developed as the final project for my Computer Graphics course. It demonstrates real-time rendering capabilities, including model loading, shading, and camera manipulation, using modern C++ and OpenGL libraries. It also features drawing on a framebuffer and several key optimizations such as caching, back-face culling.

## 📸 Features

- **3D Model Loading**: Import and render complex 3D models using Assimp.
- **Shader Management**: Utilize GLSL shaders for dynamic lighting and material effects.
- **Camera Controls**: Navigate the 3D environment with interactive camera movement.
- **Cross-Platform Support**: Build and run on both Windows and Linux systems.

## 🛠️ Technologies Used

- **C++17**: Core programming language.
- **SFML (≥ 3.0)**: Window creation and input handling.
- **Assimp**: Asset Import Library for loading 3D models.
- **GLAD (≥ 4.0)**: OpenGL function loader.
- **GLSL**: OpenGL Shading Language for custom shaders.

## 🚀 Getting Started

### Prerequisites

Ensure the following are installed on your system:

- C++17 compatible compiler
- [SFML 3.0](https://www.sfml-dev.org/)
- [Assimp](https://www.assimp.org/)
- [GLAD](https://glad.dav1d.de/)

### Building the Project

#### Using Make

```bash
git clone https://github.com/lbzfran/gp-project.git
cd gp-project
make
```

### Running the Application

```bash
./bin/pj
```

## 📂 Project Structure

```makefile
gp-project/
├── include/        # Header files
├── models/         # 3D model assets
├── shaders/        # GLSL shader programs
├── src/            # Source code files
├── bin/            # output files
├── CMakeLists.txt  # CMake build configuration
├── Makefile        # Makefile for Linux builds
└── README.md       # Project documentation
```

## 📄 License
This project is licensed under the MIT License. See the LICENSE file for details.

## 🙌 Acknowledgments
Developed in tangent with coursework from the Computer Graphics (CS449) at CSULB.

Special thanks to the open-source community for providing invaluable resources and libraries.
