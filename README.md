# SCSP-localify

- 偶像大师 闪耀色彩 棱镜之歌 本地化插件。

- 注意：使用外部插件属于违反游戏条款的行为。若使用插件后账号被封禁，造成的后果由用户自行承担。



# 使用说明：

- 将插件本体解压到游戏安装目录内即可 (`version.dll` 和 `imasscprism.exe` 在同一级目录)
- 启动游戏后看见控制台（需打开`enableConsole` ）即安装成功
- 游戏登录界面会出现安全策略相关错误弹窗。因为插件拦截了游戏的反作弊程序，属于正常现象，关闭弹窗即可。



# 配置说明

- 配置项位于 `scsp-config.json` 文件中

| 配置项                | 类型                                     | 默认值                                 | 说明                                                 |
| --------------------- | ---------------------------------------- | -------------------------------------- | ---------------------------------------------------- |
| enableConsole         | Bool                                     | `true`                                 | 是否开启控制台                                       |
| enableVSync           | Bool                                     | `false`                                | 是否启用垂直同步                                     |
| maxFps                | Int                                      | `60`                                   | 最大帧数<br>当启用 `enableVSync` 时，此项配置失效    |
| localifyBasePath      | String                                   | `scsp_localify`                        | 本地化文件目录                                       |
| hotKey                | String (Char)                            | `u`                                    | 按下 `Ctrl` + 此项配置的热键，打开插件 GUI           |
| dumpUntransLyrics     | Bool                                     | `false`                                | dump 未翻译的歌词                                    |
| dumpUntransLocal2     | Bool                                     | `false`                                | dump 未翻译的文本                                    |
| autoDumpAllJson       | Bool                                     | `false`                                | dump 所有游戏加载的 JSON                             |
| extraAssetBundlePaths | String[]                                 | `["scsp_localify/scsp-bundle"]`        | 自定义数据包路径                                     |
| customFontPath        | String                                   | `assets/font/sbtphumminge-regular.ttf` | 自定义数据包中字体路径<br>用于替换游戏内置字体       |
| blockOutOfFocus       | Bool                                     | `true`                                 | 拦截窗口失焦事件<br>切换到其它窗口后不会触发游戏暂停 |
| baseFreeCamera        | [BaseFreeCamera](#BaseFreeCamera) Object | [BaseFreeCamera](#BaseFreeCamera)      | 自由视角配置                                         |



### BaseFreeCamera

| 配置项     | 类型  | 默认值  | 说明               |
| ---------- | ----- | ------- | ------------------ |
| enable     | Bool  | `false` | 启用自由视角       |
| moveStep   | Float | `50`    | 摄像机移动速度     |
| mouseSpeed | Float | `35`    | 鼠标移动视角灵敏度 |



# 自由视角说明 (Free Camera)

- 将 `scsp-config.json` 中 `baseFreeCamera` - `enable` 设置为 `true` 即可。
- 生效范围：所有 3D 场景。包括但不限于主页、故事、Live

## 自由视角操作方法

- 移动: `W`, `S`, `A`, `D`
- 上移: `Space`，下移: `Ctrl`
- 视角转动: 
  - 键盘: `↑`, `↓`, `←`, `→`
  - 鼠标: 
    - 按 ` 键（数字键最左边，TAB 键上方）切换
    - 或者**按住**鼠标右键

- 调整视场角 (FOV)
  - 键盘: `Q`, `E`
  - 或者鼠标滚轮



# 如何汉化：

- 将 dumps 目录内的 Json 文件汉化后，放进 scsp_localify 目录即可。
- 汉化仓库：[SCSPTranslationData](https://github.com/chinosk6/SCSPTranslationData) 欢迎各位贡献自己的翻译~



## 自行 dump 原文：
- 游戏内的 UI 文本大致可以分为三类。

  - 1、通过游戏内的 Localify 接口加载

  - 2、不通过 Localify 接口加载

  - 3、直接通过 Json 加载（这部分不止文本，还有其它诸如镜头数据、人物动作等，插件也支持替换。）

  

- 第一类对应 `localify.json`

- 第二类对应 `local2.json` 和 ` lyrics.json`

- 除此之外的文件都对应第三类

- （UI 文本一部分走 Localify，一部分不走，很奇怪。）



### 故事和部分 UI 文本 dump
登录游戏后，进入故事阅读界面，按下 `ctrl` + `u`，会弹出控制窗口，勾选 `Waiting Extract Text`，然后点击任意故事标题，之后会自动 dump 故事文本和 `localify.json`



### 歌词和另一部分 UI 文本 dump
将 `scsp-config.json` 内 `dumpUntransLyrics` 和 `dumpUntransLocal2` 设置为 `true`，然后打开游戏。插件会实时将未翻译的部分 dump 到 Json 中。