# Cubrium
This project depicts my journey learning OpenGL, turning this into a voxel-based sandbox named Cubrium.

## Controls
* Movement: **Mouse** plus **WASD** or **Arrow Keys**.
* Zoom: **Scroll Wheel**.
* Framebuffer Rotation: **I** key.
* Exit: **Escape** key.

---

## Prerequisites
Before running the application, verify GLFW is installed.

### Ubuntu/Debian:

```bash
sudo apt update
sudo apt install libglfw3
ldd main
```
### Fedora:

```bash
sudo dnf install glfw
ldd main
```
### Arch Linux:

```bash
sudo pacman -S glfw-x11 # or glfw-wayland
ldd main
```
## Linux (x86_64)
1. Download the `.zip` file from the [Github Releases](https://github.com/alexanderwdougherty/opengl-learn/releases) page.
2. Open the terminal and extract the downloaded `.zip` file:

    ```bash
    unzip opengl-learn-v0.0.3-linux.zip -d opengl-learn
    ```
3. Navigate to the new folder:

    ```bash
    cd opengl-learn
    ```
4. Ensure the binary has executable permissions:

    ```bash
    chmod +x main
    ```
5. Launch the application:

    ```bash
    ./main
    ```

## Windows (64-Bit)
Currently not supported.

## macOS
Currently not supported.

## Credits & Acknowledgements
* **LearnOpenGL Course:** This project is following the [LearnOpenGL](https://learnopengl.com/) course by Joey de Vries.
* **Material Data:** The Ambient, Diffuse, Specular, and Shininess value are from [OpenGl Materials Reference](http://devernay.free.fr/cours/opengl/materials.html) by Frédéric Devernay.
* **Mapping:** All Normal Maps, Specular Maps, Ambient Maps, and Displacement Maps were generated using the [NormalMap-Online](https://cpetry.github.io/NormalMap-Online/) tool by Christian Petry.

### Assets Used
* **[Skybox](assets/skybox/)** by Joey de Vries.
* **[Window](assets/window/diffuse.png)** by Joey de Vries.
* **[Crate](assets/crate/diffuse.png)** by Joey de Vries
* **[Emerald](assets/emerald/diffuse.png)** by Mojang Studios.
* **[Obsidian](assets/obsidian/diffuse.png)** by Mojang Studios.
* **[Ice](assets/ice/diffuse.png)** by Mojang Studios.
* **[Pepe](assets/pepe/diffuse.png)** - Creator Unknown.