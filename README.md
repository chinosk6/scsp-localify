<div align="center">

# SCSP-localify

简体中文 | [English](readme_EN.md)

偶像大师 闪耀色彩 棱镜之歌 **DMM 版** 本地化插件。

**注意：使用外部插件属于违反游戏条款的行为。若使用插件后账号被封禁，造成的后果由用户自行承担。**

</div>



# 使用说明：

- 将插件本体解压到游戏安装目录内即可 (`version.dll` 和 `imasscprism.exe` 在同一级目录)
- 启动游戏后看见控制台（需打开`enableConsole` ）即安装成功



# 功能列表

- dump 文本
- 汉化、替换字体
- 解锁帧数
- 切换窗口不暂停
- Free Camera 自由视角
- Live MV 相关 **(在 GUI 中修改)**
  - 自由选择服装，可以穿别人的衣服
  - 允许相同偶像登场
  - 手动编辑登场偶像，可以选择未解锁偶像
- 角色身体参数实时修改，可修改 身高、头部、胸部、手臂、手掌 大小 **(在 GUI 中修改)**
- 运行时模型贴图提取和替换
- 身体姿势数据复制



# 配置说明

- 配置项位于 `scsp-config.json` 文件中

| 配置项                     | 类型                                     | 默认值                                 | 说明                                                 |
| -------------------------- | ---------------------------------------- | -------------------------------------- | ---------------------------------------------------- |
| enableConsole              | Bool                                     | `true`                                 | 是否开启控制台                                       |
| showStartCommand           | Bool                                     | `false`                                | 是否显示游戏启动参数<br>*注意：启动参数中包含个人信息token* |
| enableVSync                | Bool                                     | `false`                                | 是否启用垂直同步                                     |
| maxFps                     | Int                                      | `60`                                   | 最大帧数<br>当启用 `enableVSync` 时，此项配置失效    |
| 3DResolutionScale          | Float                                    | `1.0`                                  | 3D 渲染分辨率倍率                                    |
| localifyBasePath           | String                                   | `scsp_localify`                        | 本地化文件目录                                       |
| hotKey                     | String (Char)                            | `u`                                    | 按下 `Ctrl` + 此项配置的热键，**打开插件 GUI**       |
| dumpUntransLyrics          | Bool                                     | `false`                                | dump 未翻译的歌词                                    |
| dumpUntransLocal2          | Bool                                     | `false`                                | dump 未翻译的文本                                    |
| autoDumpAllJson            | Bool                                     | `false`                                | dump 所有游戏加载的 JSON                             |
| ~~extraAssetBundlePaths~~  | ~~String[]~~                             | ~~`["scsp_localify/scsp-bundle"]`~~    | ~~自定义数据包路径~~<br> **此设置已过时** <br>使用 `asset_bundle_path::asset_path` 格式对具体要使用的资源进行指定。      |
| customFontPath             | String                                   | `scsp_localify/scsp-bundle::assets/font/sbtphumminge-regular.ttf` | 自定义数据包中字体路径<br>用于替换游戏内置字体       |
| blockOutOfFocus            | Bool                                     | `true`                                 | 拦截窗口失焦事件<br>切换到其它窗口后不会触发游戏暂停 |
| baseFreeCamera             | [BaseFreeCamera](#BaseFreeCamera) Object | [BaseFreeCamera](#BaseFreeCamera)      | 自由视角配置                                         |
| unlockPIdolAndSCharaEvents | Bool                                     | `false`                                | 解锁 `角色` - `一览` 中的P卡和S卡事件                |
| startResolution            | [Resolution](#Resolution) Object         | [Resolution](#Resolution)              | 启动游戏初始分辨率                                   |



### BaseFreeCamera

| 配置项     | 类型  | 默认值  | 说明               |
| ---------- | ----- | ------- | ------------------ |
| enable     | Bool  | `false` | 启用自由视角       |
| enableCameraOffset | Bool  | `false` | 启用镜头偏移模式（与 enable 互斥） |
| moveStep   | Float | `50`    | 摄像机移动速度     |
| mouseSpeed | Float | `35`    | 鼠标移动视角灵敏度 |



### Resolution

| 配置项 | 类型 | 默认值  | 说明     |
| ------ | ---- | ------- | -------- |
| w      | Int  | `1280`  | 窗口宽度 |
| h      | Int  | `720`   | 窗口高度 |
| isFull | Bool | `false` | 是否全屏 |



# 自由视角说明 (Free Camera)

- 将 `scsp-config.json` 中 `baseFreeCamera` - `enable` 设置为 `true` 即可。
- 生效范围：所有 3D 场景。包括但不限于主页、故事、Live
> 由于游戏v2.6.1的引擎升级，自由视角目前仍存在尚未完全修复的BUG



## 自由视角操作方法

> 下述为默认按键，具体按键可以通过配置文件进行自定义

- 移动: `W`, `S`, `A`, `D`
- 上移: `Space` （插件v1.3.6开始分发的配置文件中重载为 `Alt`），下移: `Ctrl`
- 摄像头复位: `R`

- 视角转动: 
  - 键盘: `↑`, `↓`, `←`, `→`
  - 鼠标: 
    - 按 ` 键（数字键最左边，TAB 键上方）切换
    - 或者**按住**鼠标右键
- 调整视场角 (FOV)
  - 键盘: `Q`, `E`
  - 或者鼠标滚轮


 ### 自由视角按键设置

- 所有配置项同样位于 `scsp-config.json` 文件中
- 由于游戏v2.9.0更新了键盘操作，该插件既往默认的自由视角上移按键 `Space` 从插件v1.3.6开始分发的配置文件中修改为 `Alt`，但若配置文件没有修改，默认仍保持 `Space` 不变

| 配置项                       | 默认值                     |
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


JSON值说明：对于按键设置接受以下两种类型的值：
- `char[1]` 单字节字符串，即用双引号包绕的单个字母或数字，表示相应的按键，如 `"W"` 表示W键
- `int` 整数数值，直接绑定到相应的 windows virtual key，可参考 https://learn.microsoft.com/zh-cn/windows/win32/inputdev/virtual-key-codes


 # Live MV 功能说明

- 在开启 `Save & Replace costume changes` 选项后打开后可以记录所有服装变化，打开游戏内的试用按钮后也可以记录未解锁的服装，或在DressOrder界面中选择私服，并在MV播放时自动应用所记录的服装信息；在子窗口 "Saved Costume Data" 中可以通过 `Remove` 按钮移除不需要的记录以取消
- 在开启 `Save & Replace costume changes` 后再开启 `Override MV unit idols` 选项，在 "Override MvUnit Idols" 子窗口中通过 `Slot X` 按钮进行保存上一次修改时的服装信息，不同槽位可以记录同一个偶像以实现相同偶像登场并使用不同服装，无记录的位置将继承当前编队中的原始信息
- "Override MvUnit Idols" 子窗口中可以通过点击数据直接编辑JSON数据进行手动修改（备注：当直接编辑`CharaId`时，建议使用`1`（默认）作为`HairId`的值以避免游戏卡住）


# Magica Cloth 相关设置

所有相关设置均可在游戏中通过GUI进行修改，以下在 `scsp-config.json` 配置文件中的值只影响初始化。

具体属性说明可参考官方文档：https://magicasoft.jp/mc2_about/

| 配置项                               | 默认值及说明                   |
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

† 此处`Inertia`和`Radius`属性来自`MagicaClothController`


# 如何汉化

- 将 dumps 目录内的 Json 文件汉化后，放进 `scsp_localify` 目录即可。
- 汉化仓库：[SCSPTranslationData](https://github.com/ShinyGroup/SCSPTranslationData) 欢迎各位贡献自己的翻译~



## 自行 dump 原文
- 游戏内的 UI 文本大致可以分为三类。

  - 1、通过游戏内的 `Localify` 接口加载

  - 2、不通过 `Localify`接口加载

  - 3、直接通过 Json 加载（这部分不止文本，还有其它诸如镜头数据、人物动作等，插件也支持替换。）

  

- 第一类对应 `localify.json`

- 第二类对应 `local2.json` 和 ` lyrics.json`

- 除此之外的文件都对应第三类

- （UI 文本一部分走 `Localify`，一部分不走，很奇怪。）



### 故事和部分 UI 文本 dump
登录游戏后，进入故事阅读界面，按下 `ctrl` + `u`，会弹出控制窗口，勾选 `Waiting Extract Text`，然后点击任意故事标题，之后会自动 dump 故事文本和 `localify.json`



### 歌词和另一部分 UI 文本 dump
将 `scsp-config.json` 内 `dumpUntransLyrics` 和 `dumpUntransLocal2` 设置为 `true`，然后打开游戏。插件会实时将未翻译的部分 dump 到 Json 中。


# 运行时模型贴图提取和替换

- 提取：在GUI中勾选 `Extract assets of：` 以及相应的筛选器后，会在程序目录的 `TextureDump` 文件夹下分类存放提取的纹理。
- 替换：在程序的 `scsp_localify\textures` 文件夹内放置与提取得到的纹理相同名称的图片会自动在启动时加载并替换。


# 身体姿势数据复制
一个简单的使用介绍：https://github.com/chinosk6/scsp-localify/discussions/101


# 如何编译
- 安装 `conan 2`、`cmake`
- 运行 `generate.bat` 获取依赖包
- 通过 `build/ImasSCSP-localify.sln` 使用 `Visual Studio 2022` 进行编译

## 预处理器命令 `__SAFETYHOOK`

### 什么是`safetyhook`以及为什么要使用
`safetyhook`相比默认的`minhook`在插入hook时有更高的成功率。

如果插件因为`MH_ERROR_MEMORY_ALLOC`而无法成功插入hook，就可以尝试`safetyhook`。

## 如何编译并使用`safetyhook`
- 在 `deps/safetyhook` 路径下运行 `cmake . -B build -G "Visual Studio 17 2022"` 初始化safetyhook工程（需要手动创建`build`文件夹）
- 在Release模式下编译 `deps/safetyhook/build/safetyhook.sln`
- 在当前工程中添加引用
    - C/C++ - General | Additional Include Directories: `..\deps\safetyhook\include`
    - Linker - General | Additional Library Directories: `..\deps\safetyhook\build\Release`, `..\deps\safetyhook\build\_deps\zydis-build\Release`
    - Linker - Input | Additional Dependencies: `Zydis.lib`, `safetyhook.lib`
- 在 C/C++ - Preprocessor | Preprocessor Definitions 下添加: `__SAFETYHOOK`
