<div align="center">

# SCSP-localify

[简体中文](readme.md) | English

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
  - Unlock Costumes
  - Freedom to Choose Costumes, Wear Other Characters' Clothes
  - Allow Same Idol Appearance
- Real-time Modification of Character Body Parameters, Adjust Height, Head, Chest, Arm, and Palm Size **(Modify in GUI)**




# Configuration Instructions

- Configuration items are located in the `scsp-config.json` file.

| Configuration Item    | Type      | Default Value                         | Description                                            |
| --------------------- | --------- | ------------------------------------- | ------------------------------------------------------ |
| enableConsole         | Bool      | `true`                                | Enable console                                         |
| enableVSync           | Bool      | `false`                               | Enable vertical sync                                   |
| maxFps                | Int       | `60`                                  | Maximum frame rate<br>When `enableVSync` is enabled, this configuration is ineffective |
| localifyBasePath      | String    | `scsp_localify`                      | Localization file directory                            |
| hotKey                | String (Char) | `u`                               | Press `Ctrl` + this configured hotkey to **open the plugin GUI** |
| dumpUntransLyrics     | Bool      | `false`                               | Dump untranslated lyrics                               |
| dumpUntransLocal2     | Bool      | `false`                               | Dump untranslated text                                 |
| autoDumpAllJson       | Bool      | `false`                               | Dump all loaded JSON files                             |
| extraAssetBundlePaths | String[]  | `["scsp_localify/scsp-bundle"]`       | Custom asset bundle paths                              |
| customFontPath        | String    | `assets/font/sbtphumminge-regular.ttf` | Custom font path in asset bundles<br>Used for replacing built-in fonts in the game |
| blockOutOfFocus       | Bool      | `true`                                | Intercept window out-of-focus events<br>Game won't pause when switching to other windows |
| baseFreeCamera        | [BaseFreeCamera](#BaseFreeCamera) Object | [BaseFreeCamera](#BaseFreeCamera) | Free camera configuration                             |



### BaseFreeCamera

| Configuration Item | Type   | Default Value | Description         |
| ------------------ | ------ | ------------- | --------------------|
| enable             | Bool   | `false`       | Enable free camera  |
| moveStep           | Float  | `50`          | Camera movement speed |
| mouseSpeed         | Float  | `35`          | Mouse sensitivity for camera movement |



# Free Camera Instructions

- Set `enable` under `baseFreeCamera` in `scsp-config.json` to `true`.
- Scope of application: All 3D scenes. Including but not limited to homepage, story, Live.



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