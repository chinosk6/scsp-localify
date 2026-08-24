<div align="center">

# SCSP-localify

[简体中文](README.md) | English

iM@S SCSP localify plugin.

**Note: Using external plugins violates the game's terms of service. If your account is banned due to plugin usage, the consequences are solely your responsibility.**

</div>



# Instructions for Use

- Simply unzip the plugin into the game installation directory (`version.dll` and `imasscprism.exe` should be in the same directory).
- Upon launching the game, if you see the console (make sure to open `enableConsole`), the installation is successful.



# Function List

- Dump Text
- Localization, Font Replacement
- Unlock Frame Rate
- Switching Windows Without Pausing
- Free Camera
- Live MV Related **(Modify in GUI)**
  - Freedom to Choose Costumes, Wear Other Characters' Clothes
  - Allow Same Idol Appearance
  - Edit on-stage idols manually, allowing to select unlocked idols
- Real-time Modification of Character Body Parameters, Adjust Height, Head, Chest, Arm, and Palm Size **(Modify in GUI)**
- Runtime texture extracting and replacing
- Copying body pose data




# Configuration Instructions

- Configuration items are located in the `scsp-config.json` file.

| Configuration Item    | Type      | Default Value                         | Description                                            |
| --------------------- | --------- | ------------------------------------- | ------------------------------------------------------ |
| enableConsole         | Bool      | `true`                                | Enable console                                         |
| showStartCommand      | Bool      | `false`                               | Output start command<br>*Note: Personal token is contained in startup args* |
| enableVSync           | Bool      | `false`                               | Enable vertical sync                                   |
| maxFps                | Int       | `60`                                  | Maximum frame rate<br>When `enableVSync` is enabled, this configuration is ineffective |
| 3DResolutionScale | Float | `1.0` | 3D resolution render scale |
| localifyBasePath      | String    | `scsp_localify`                      | Localization file directory                            |
| hotKey                | String (Char) | `u`                               | Press `Ctrl` + this configured hotkey to **open the plugin GUI** |
| dumpUntransLyrics     | Bool      | `false`                               | Dump untranslated lyrics                               |
| dumpUntransLocal2     | Bool      | `false`                               | Dump untranslated text                                 |
| autoDumpAllJson       | Bool      | `false`                               | Dump all loaded JSON files                             |
| ~~extraAssetBundlePaths~~ | ~~String[]~~  | ~~`["scsp_localify/scsp-bundle"]`~~       | ~~Custom asset bundle paths~~<br> **This option is obsolete** <br>Use format `asset_bundle_path::asset_path` to specify an exact asset to use. |
| customFontPath        | String    | `scsp_localify/scsp-bundle::assets/font/sbtphumminge-regular.ttf` | Custom font path in asset bundles<br>Used for replacing built-in fonts in the game |
| blockOutOfFocus       | Bool      | `true`                                | Intercept window out-of-focus events<br>Game won't pause when switching to other windows |
| baseFreeCamera        | [BaseFreeCamera](#BaseFreeCamera) Object | [BaseFreeCamera](#BaseFreeCamera) | Free camera configuration                             |
| unlockPIdolAndSCharaEvents | Bool | `false` | Unlock Idol Event (アイドルイベント) and Support Event (サポートイベント) in `Characters` - `Overview` |
| startResolution | [Resolution](#Resolution) Object | [Resolution](#Resolution) | Game window resolution |



### BaseFreeCamera

| Configuration Item | Type   | Default Value | Description         |
| ------------------ | ------ | ------------- | --------------------|
| enable             | Bool   | `false`       | Enable free camera  |
| enableCameraOffset | Bool   | `false`       | Enable camera offset mode (mutually exclusive with `enable`) |
| moveStep           | Float  | `50`          | Camera movement speed |
| mouseSpeed         | Float  | `35`          | Mouse sensitivity for camera movement |



### Resolution

| Configuration Item | Type | Default Value | Description    |
| ------------------ | ---- | ------------- | -------------- |
| w                  | Int  | `1280`        | Window width   |
| h                  | Int  | `720`         | Window height  |
| isFull             | Bool | `false`       | Is full screen |



# Free Camera Instructions

> The following is the default key binding, and can be customized by config files.

- Set `enable` under `baseFreeCamera` in `scsp-config.json` to `true`.
- Scope of application: All 3D scenes. Including but not limited to homepage, story, Live.
> With the unity engine updating in game v2.6.1, there're still unfixed bugs about free camera feature now.



## Free Camera Operation Method

- Movement: `W`, `S`, `A`, `D`
- Ascend: `Space` (overwritten as `Alt` in config files distributed after plugin v1.3.6), Descend: `Ctrl`
- Reset camera: `R`

- Camera Rotation: 
  - Keyboard: `↑`, `↓`, `←`, `→`
  - Mouse: 
    - Press the ` key (located to the left of the number keys, above the TAB key)
    - Or **hold down** the right mouse button
- Adjust Field of View (FOV)
  - Keyboard: `Q`, `E`
  - Or mouse scroll wheel


 ## Customizing free camera key bindings
 - All config items are also located in the `scsp-config.json` file.
 - As the update of game v2.9.0 introduced some key bindings, the default camera ascending key binding `Space` is bound to `Alt` in the config files distributed after plugin v1.3.6. But if the config file isn't edited, it keeps `Space` as unchanged.

| Configuration Item          | Default Value             |
| --------------------------- | ------------------------- |
| key_w_camera_forward        | `W`                       |
| key_s_camera_back           | `S`                       |
| key_a_camera_left           | `A`                       |
| key_d_camera_right          | `D`                       |
| key_ctrl_camera_down        | `17` (ctrl)               |
| key_space_camera_up         | `18` (alt)                |
| key_up_cameralookat_up      | `38` (↑)                  |
| key_down_cameralookat_down  | `40` (↓)                  |
| key_left_cameralookat_left  | `37` (←)                  |
| key_right_cameralookat_right| `39` (→)                  |
| key_q_camera_fov_increase   | `Q`                       |
| key_e_camera_fov_decrease   | `E`                       |
| key_r_camera_reset          | `R`                       |
| key_192_camera_mouseMove    | `192` (`` ` ``, backtick) |


About JSON value: For key bindings, two types of values are acceptable:
- `char[1]` single character string, wrapped by double quotes, to express the letter key, like `"W"` for key W
- `int` an integer to bind to a windows virtual key directly, see also https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes


# Live MV Instructions

- When `Save & Replace costume changes` is checked, all costume changes will be recorded, locked costumes can also be recorded by clicking the Try-On button in the game, or casual costumes can be selected in the DressOrder interface, and all the changes will be applied automatically when MV starts; unwanted records can be removed using the `Remove` button in the "Saved Costume Data" sub-window to cancel them.
- When `Save & Replace costume changes` is checked and `Override MV unit idols` is checked, the last costume data can be saved by clicking the `Slot X` buttons in the "Override MvUnit Idols" sub-window; different slots can be used to record the same idol to achieve multiple appearances of the same idol with different costumes, and unrecorded slots will inherit the original data of the idols in the selected live unit.
- In "Override MvUnit Idols" sub-window, clicking data to edit JSON data manually. (Note: When editing `CharaId` manually, it's suggested to use `1` (the default) for `HairId` to avoid freezing)


# Magica Cloth Settings

All relative settings can be adjusted in in-game GUI, all these values inside configuration file `scsp-config.json` only affect initialization.

Detailed descriptions of properties can be checked at the official document: https://magicasoft.jp/mc2_about/


| Configuration Item                  | Default Value                |
| ----------------------------------- | ---------------------------- |
| magicacloth_override                | `false`                      |
| magicacloth_inertia_min †           | `1.0f`                       |
| magicacloth_inertia_max †           | `1.0f`                       |
| magicacloth_radius_min †            | `0.002f`                     |
| magicacloth_radius_max †            | `0.028f`                     |
| magicacloth_damping                 | `0.01f`                      |
| magicacloth_movementSpeedLimit      | `10.0f`                      |
| magicacloth_rotationSpeedLimit      | `1440.0f`                    |
| magicacloth_localMovementSpeedLimit | `10.0f`                      |
| magicacloth_localRotationSpeedLimit | `1440.0f`                    |
| magicacloth_particleSpeedLimit      | `40.0f`                      |
| magicacloth_limitAngle              | `90.0f`                      |
| magicacloth_springLimitDistance     | `0.5f`                       |
| magicacloth_springNoise             | `0.1f`                       |

† Properties `Inertia` and `Radius` come from `MagicaClothController`


# How to Localize

- After localizing the Json files in the dumps directory, place them in the `scsp_localify` directory.
- Localization Repository (Chinese): [SCSPTranslationData](https://github.com/ShinyGroup/SCSPTranslationData) Contributors are welcome to contribute their translations~



## Dump Original Text Yourself
- The UI text in the game can be roughly divided into three categories.

  - 1. Loaded through the game's `Localify` interface
  - 2. Loaded without using the `Localify` interface
  - 3. Loaded directly through Json (this part includes not only text but also other things like camera data, character actions, etc., which can be replaced by the plugin.)

  

- The first category corresponds to `localify.json`

- The second category corresponds to `local2.json` and `lyrics.json`

- Files other than these correspond to the third category

- (Some UI text goes through `Localify`, some don't, it's strange.)



### Story and Some UI Text Dump
After logging into the game, go to the story reading interface, press `ctrl` + `u`, a control window will pop up, check `Waiting Extract Text`, then click on any story title, and the story text and `localify.json` will be automatically dumped.



### Lyrics and Another Part of UI Text Dump
Set `dumpUntransLyrics` and `dumpUntransLocal2` in `scsp-config.json` to `true`, then open the game. The plugin will continuously dump untranslated parts into Json in real time.


# Runtime texture extracting and replacing
- Extracting: After checking option `Extract assets of：` and filters in GUI, textures will be extracted to `TextureDump` in the same directory to the program.
- Replacing: Textures put inside directory `scsp_localify\textures` with exactly same names of extracted will be loaded and replaced automatically.


# Copying body pose data
a breif instruction: https://github.com/chinosk6/scsp-localify/discussions/101


# How to build
- Install `conan 2`, `cmake`
- Run `generate.bat` to resolve dependencies
- Open `build/ImasSCSP-localify.sln` in `Visual Studio 2022` to build

## Preprocessor `__SAFETYHOOK`

### What is `safetyhook` and why to use it
`safetyhook` inserts hooks with a higher probability of success than the default `minhook` library.

If the plugin failed to insert hooks because of `MH_ERROR_MEMORY_ALLOC`, it's time to try `safetyhook`.

### How to compile with `safetyhook`
- Run `cmake . -B build -G "Visual Studio 17 2022"` under path `deps/safetyhook` to initialize the safetyhook project（requiring directory `build` created before）
- Compile `deps/safetyhook/build/safetyhook.sln` in release mode
- Add references in this project:
    - C/C++ - General | Additional Include Directories: `..\deps\safetyhook\include`
    - Linker - General | Additional Library Directories: `..\deps\safetyhook\build\Release`, `..\deps\safetyhook\build\_deps\zydis-build\Release`
    - Linker - Input | Additional Dependencies: `Zydis.lib`, `safetyhook.lib`
- Add `__SAFETYHOOK` under "C/C++ - Preprocessor | Preprocessor Definitions"
