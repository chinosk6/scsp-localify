# ShinyColors-SongForPrism-localify

- 注意：使用外部插件属于违反游戏条款的行为。若使用插件后账号被封禁，造成的后果由用户自行担责。



# 使用说明：

- 将插件本体解压到游戏安装目录内即可 (version.dll 和 imasscprism.exe 在同一级目录)
  启动游戏后看见控制台（需打开 "enableConsole"）即安装成功
  游戏登录界面会出现安全策略相关错误弹窗。因为插件拦截了游戏的反作弊程序，属于正常现象，关闭弹窗即可。



# 如何汉化：

- 将 dumps 目录内的 Json 文件汉化后，放进 scsp_localify 目录即可。



## 自行 dump 原文：
游戏内的 UI 文本大致可以分为三类。
1、通过游戏内的 Localify 接口加载
2、不通过 Localify 接口加载
3、直接通过 Json 加载（这部分不止文本，还有其它诸如镜头数据、人物动作等，插件也支持替换。）

第一类对应 localify.json
第二类对应 local2.json 和 lyrics.json
除此之外的文件都对应第三类
（UI 文本一部分走 Localify，一部分不走，很奇怪。）



### 故事和部分 UI 文本 dump
登录游戏后，进入故事阅读界面，然后切换到控制台，按下 ctrl + u，看到 "hotKey pressed." 输出，然后点击任意故事标题，之后会自动 dump 故事文本和 localify.json



### 歌词和另一部分 UI 文本 dump
压缩包内提供的 lyrics.json 和 local2.json 内容并不全，需要自行 dump。
将 scsp-config.json 内 "dumpUntransLyrics" 和 "dumpUntransLocal2" 设置为 true，然后打开游戏。插件会实时将未翻译的部分 dump 到 Json 中。