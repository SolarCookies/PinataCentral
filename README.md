# PinataCentral

PinataCentral is an open-source repository dedicated to reading and modifying Viva Piñata's proprietary `.pkg` file format. Originally designed for managing piñata-related data, the project has evolved into a tool for reverse engineering and understanding the structure of these files.

## File Format Breakdown

### `.pkg` Files

Viva Piñata's `.pkg` files serve as containers for multiple `CAFF` (Common Asset File Format) files. This format is found in many Rareware games, though each game's implementation differs slightly.

Each `CAFF` file contains four streams of compressed ZLIB data. The `CAFF` header is 120 bytes long and contains offsets and sizes for the first two of these streams:

- **VREF (Reference Data)**: Stores filenames, offsets, and sizes of `VDAT` and `VGPU`. It acts as an index for locating assets.
- **VLUT (Unknown)**: Its exact function is unclear, but it may serve as a hashmap or lookup table.
- **VDAT (Asset Metadata)**: Contains essential information required to load assets, such as image dimensions, formats, model shaders, requirements, bones, scripts, localization tags, and object tags.
- **VGPU (Graphics Data)**: Primarily consists of data sent directly to the GPU, including model face/vertex data and texture pixels. Some textures include DDS headers, while others do not, likely indicating different formats.

## Features

- Parsing and extracting `.pkg` and `CAFF` file contents
- Understanding and modifying Viva Piñata's assets in order to modify the game
- Filters chunks by type and adds controls to manipulate the data
- View Images and streams without extracting anything from the .pkg
- With a built in Vulcan Pipeline we also plan on adding a model viewer down the line

## Installation

To set up *PinataCentral* locally, follow these steps:

1. Clone the repository:
   ```sh
   git clone https://github.com/Stitch-Em/PinataCentral.git
   cd PinataCentral
   ```
2. Install the latest Vulkan SDK from [LunarG](https://vulkan.lunarg.com):
   ```sh
   https://vulkan.lunarg.com
   ```
3. Run the setup script:
   ```sh
   PinataCentral\scripts\Setup-ExampleProject.bat
   ```

## Usage

- First, you will see that no `.pkg` directory has been set. Click the red button to set it. For the first game, the `.pkg` files are located in:

```
Viva Pinata/bundles_packages/
```

For the second game, *Viva Piñata: Trouble in Paradise* ("TIP"), they are located in:

```
VivaPinataTiP/Beta/packages/
```

after using an Xbox ISO extractor to extract the files.

- Extract, Analyze, and Modify chunks using a hex editor like "010 Editor"
- Use the tool to replace the modified chunk (The tool can only replace data thats exactly the same size in bytes at the moment)
- Play the game and if it crashes then the game couldn't load up the mod due to a format issue with the modified chunk
- If you found something new that the tool doesn't already have a control for then feel free to Contribute to reverse engineering efforts and documentation, you can do this by joining our [Discord community](https://discord.gg/XZY7bjCsaa) or submitting an issue on GitHub.

## License

While the source code for this research tool is open-source, modifying a Microsoft game comes with potential risks. Additionally, distributing modified game files or extracted assets may violate Microsoft's terms of service. This tool is intended for research and preservation purposes only, and we do not condone using it to rip and reupload assets elsewhere. Use at your own discretion.

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.

## Contact

For questions, suggestions, or contributions, feel free to open an issue on GitHub, contact the maintainers, or join our [Discord community](https://discord.gg/XZY7bjCsaa).

---
