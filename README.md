# Serious SKA Converter

This is a simple console utility for converting between different ASCII model formats, the main format being ASCII SKA models for Serious Engine 1.

There is no real way of making SKA models in SE1 without creating some sort of plugin for exporting skeletal meshes from 3D modeling programs. I myself have no idea how to do that, so I resorted to the second best thing - converter from another ASCII format.

This project began during the development of [Dreamy Utilities](https://github.com/DreamyCecil/DreamyUtilities) and many parts of this framework exist thanks to this project. It started as an application for converting models from [SMD format](https://developer.valvesoftware.com/wiki/Studiomdl_Data) (`.smd`) to various SE1 SKA formats (`.am`, `.as` and `.aa`) but also includes limited support for conversions between ASCII formats from Serious Engine 2+.

**It currently supports such conversion methods:**
1. SMD mesh -> SE1 mesh, skeleton and default animation
2. SMD animation -> SE1 animation
3. SE2+ skeleton -> SE1 skeleton
4. SE2+ animation -> SE1 animation
5. SE1 skeleton -> SE2+ skeleton

## How to use

Place the compiled executable anywhere you want and just associate specific file formats with the application to open it upon double clicking on the file.

File formats to associate with specific conversion methods:
- `.smd` for methods 1 and 2
- `.asf` for method 3
- `.aaf` for method 4
- `.as` for method 5

### Extra features

1. There are multiple launch arguments you can specify to make the program automatically finish the conversion without extra user input.
  - `-scale` - Specify custom model scale. Example: `-scale 0.5`.
  - `-fixscale` - Convert scale from Source engine to SE1.
  - `-fixdir` - Fix model facing direction.
  - `-keepdir` - Keep model facing direction. Mostly for testing.
  - `-fixanim` - Fix facing direction for the animation. SMD animations usually face X axis instead of Z.
  - `-keepanim` - Keep facing direction for the animation. Mostly for testing.
  - `-base` - Specify base SMD model for the animation. If you don't do this, the center of the model during the converted animation may be offsetted incorrectly.
2. You can create a `!Converter.txt` file near the file that's being opened where you can specify launch arguments to add to the execution instead of writing a custom script for running the converter. Example for most SMD animation files:
```
-fixscale -fixdir -fixanim -base <main mesh file>.smd
```
3. When you get prompted with `Specify SMD model file that this animation is for:` upon converting an SMD file with an animation and don't specify any model, it will default to `!Base.smd` file that you can place near the file that's being opened that will be used as the "base" model. This is the same model as in the `-base` launch argument.
4. You can create a `!AnimInfo.json` file near the file that's being opened for specifying custom properties for specific SMD animation files. By default, converted animations have the same name as the SMD file they were created from and 30 FPS as the fixed animation speed. These properties can be overriden as such:
```json
{
  "run.smd" : {
    "name" : "Run",
    "fps" : 25,
  },
}
```

## Building

Before building the code, make sure to load in the submodules. Use `git submodule update --init --recursive` command to load files for all submodules.

The repository includes Visual Studio project files for building **Serious SKA Converter** under Windows and Linux platforms (using WSL).

Project files are compatible with Visual Studio 2019 and higher.

### Tested compilers
- **MSVC**: 6.0 (`C++98`), 12.0 (`C++11`)
- **GCC**: 9.4.0 (`C++98` and `C++11`)

## License

**Serious SKA Converter** is licensed under the MIT license (see `LICENSE`).
