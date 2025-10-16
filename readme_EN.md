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




# Configuration Instructions

- Configuration items are located in the `scsp-config.json` file.

| Configuration Item    | Type      | Default Value                         | Description                                            |
| --------------------- | --------- | ------------------------------------- | ------------------------------------------------------ |
| enableConsole         | Bool      | `true`                                | Enable console                                         |
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
| moveStep           | Float  | `50`          | Camera movement speed |
| mouseSpeed         | Float  | `35`          | Mouse sensitivity for camera movement |



### Resolution

| Configuration Item | Type | Default Value | Description    |
| ------------------ | ---- | ------------- | -------------- |
| w                  | Int  | `1280`        | Window width   |
| h                  | Int  | `720`         | Window height  |
| isFull             | Bool | `false`       | Is full screen |



# Free Camera Instructions

- Set `enable` under `baseFreeCamera` in `scsp-config.json` to `true`.
- Scope of application: All 3D scenes. Including but not limited to homepage, story, Live.
> With the unity engine updating in game v2.6.1, there're still unfixed bugs about free camera feature now.



## Free Camera Operation Method

- Movement: `W`, `S`, `A`, `D`
- Ascend: `Space`, Descend: `Ctrl`
- Reset camera: `R`

- Camera Rotation: 
  - Keyboard: `↑`, `↓`, `←`, `→`
  - Mouse: 
    - Press the ` key (located to the left of the number keys, above the TAB key)
    - Or **hold down** the right mouse button
- Adjust Field of View (FOV)
  - Keyboard: `Q`, `E`
  - Or mouse scroll wheel



# Live MV Instructions

- When `Save & Replace costume changes` is checked, all costume changes will be recorded, locked costumes can also be recorded by clicking the Try-On button in the game, or casual costumes can be selected in the DressOrder interface, and all the changes will be applied automatically when MV starts; unwanted records can be removed using the `Remove` button in the "Saved Costume Data" sub-window to cancel them.
- When `Save & Replace costume changes` is checked and `Override MV unit idols` is checked, the last costume data can be saved by clicking the `Slot X` buttons in the "Override MvUnit Idols" sub-window; different slots can be used to record the same idol to achieve multiple appearances of the same idol with different costumes, and unrecorded slots will inherit the original data of the idols in the selected live unit.
- In "Override MvUnit Idols" sub-window, clicking data to edit JSON data manually. (Note: When editing `CharaId` manually, it's suggested to use `1` (the default) for `HairId` to avoid freezing)



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


# How to build
- Install `conan 2`, `cmake`
- Run `generate.bat` to resolve dependencies
- Open `build/ImasSCSP-localify.sln` in `Visual Studio 2022` to build