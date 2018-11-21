# Getting Meson

Meson is implemented in Python 3, and requires 3.5 or newer. If your operating system provides a package manager, you should install it with that. On Debian, for example, you can simply:

```sh
$ sudo apt install meson
```

For platforms that don't have a package manager, you need to download it from Python's home page. See the following [link](https://mesonbuild.com/Getting-meson.html) for more information.

# Compilation using Meson

In order to build the oz2 software you need the following libraries: sdl2, sdl2_image, sdl2_gfx and sdl2_ttf. On Debian you can install these libraries as:

```sh
$ sudo apt install libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev libsdl2-gfx-dev
```

The Meson build system is generally considered stable and ready for production.

The meson program is used to configure the source directory and generates a ninja build file. Meson only supports out-of-tree builds, and must be passed a directory to put built and generated sources into. We'll call that directory "build" for examples.

```sh
$ meson build/
```

To see a description of your options you can run meson configure along with a build directory to view the selected options for. This will show your meson global arguments and project arguments, along with their defaults and your local settings.

```sh
$ meson configure build/
```

Once you've run the initial meson command successfully you can use your configured backend to build the project. With ninja, the -C option can be be used to point at a directory to build.

```sh
$ ninja -C build/
```

