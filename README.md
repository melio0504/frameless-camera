# Frameless Camera

A tiny, mirrored camera preview I built for myself. The window has no title bar,
toolbar, buttons, or other visible interface.

![Frameless Camera running as a movable webcam overlay](asset/screenshot.png)

## Why I made this

I made this app because I wanted a webcam window that I could freely move and
resize while recording with OBS. Its frameless design keeps the camera feed
clean, without a title bar, toolbar, or other controls appearing in the
recording. I also made it to learn more about developing applications in C++
with Qt. I use GNOME's built-in option to toggle whether the window appears on
all workspaces or only one.

## Resource usage

The compiled executable is about 44 KB on my system and uses around 84 MB of
RAM while displaying the camera preview. The executable is small because it
links to the system's Qt libraries rather than bundling them. Actual disk and
memory usage may vary depending on the Qt version, camera resolution, graphics
driver, and Linux distribution.

## Install the build dependencies

```sh
sudo apt update
sudo apt install build-essential curl pkg-config qt6-base-dev qt6-multimedia-dev qt6-wayland
```

## Build and run

```sh
make
./build/frameless-camera
```

You can also build and launch it in one step with `make run`.

## Build a portable AppImage

For a portable, self-contained version, build an AppImage. It requires more
disk space than the regular executable because it includes the required Qt
libraries and plugins.

```sh
make appimage
```

This downloads the official `linuxdeploy` and Qt deployment tools into the
ignored `build/appimage-tools/` directory, bundles the executable with its Qt
libraries and plugins, and creates:

```text
dist/frameless-camera-x86_64.AppImage
```

The resulting file can be moved and launched directly on compatible x86-64
Linux systems:

```sh
chmod +x dist/frameless-camera-x86_64.AppImage
./dist/frameless-camera-x86_64.AppImage
```

An AppImage built on a newer Linux system may not run on substantially older
distributions because core system libraries such as glibc are not bundled.

## Controls

- Drag anywhere away from an edge to move the window.
- Drag an edge or corner to resize it.

The application uses the system's default camera, starts at 440 × 280, stays
on top of regular windows, fills its entire window with the camera preview, and
mirrors the image horizontally.

On Wayland, the desktop compositor has final control over window positioning
and whether an always-on-top request is honored. The app uses the compositor's
native operations for moving and resizing whenever they are available.

## License

This project is licensed under the [MIT License](LICENSE).
