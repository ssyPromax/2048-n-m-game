# 2048 (n*m) - Android 版

Android 版 2048：行数 / 列数滑动条（2~10）自定义棋盘，手势滑动移动方块。纯 Java + 系统 API，无第三方依赖，无需 Gradle。

## 构建 APK

前提：JDK 17+，以及安装了 `platforms;android-34` 和 `build-tools;34.0.0` 的 Android SDK。

```bash
export ANDROID_SDK_ROOT=/path/to/android-sdk
bash build-apk.sh "2048(n*m).apk"
```

构建流程（脚本内可见）：aapt2 打包清单 → javac 编译 → d8 生成 dex → 合成 APK → apksigner 签名。

## 文件说明

| 文件 | 说明 |
| --- | --- |
| `AndroidManifest.xml` | 应用清单（包名 com.ssy.game2048，minSdk 24） |
| `src/com/ssy/game2048/MainActivity.java` | 全部源码（界面 + 游戏逻辑） |
| `build-apk.sh` | 免 Gradle 构建脚本 |

## 操作

- 拖动「行数 / 列数」滑动条（2~10），点「开始新游戏」
- 在棋盘上**滑动**移动方块（上/下/左/右）
- 合成 2048 获胜；死局弹窗可一键重开
