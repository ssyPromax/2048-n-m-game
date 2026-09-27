# 2048 (N*M) Game

棋盘大小可自定义的 2048 图形界面游戏：行数、列数各用一个滑动条在 **2~10** 之间调节，点「开始新游戏」即可用新棋盘开局。C++ / Win32 API 编写，单文件，无第三方依赖。

![语言](https://img.shields.io/badge/language-C%2B%2B-blue) ![平台](https://img.shields.io/badge/platform-Windows-lightgrey)

## 玩法

1. 拖动「行数」「列数」滑动条选择棋盘大小（2~10）
2. 点击「开始新游戏」（游戏中按 **R** 可用当前设置重开）
3. **方向键 / WASD** 移动方块，相同数字碰撞合并得分
4. 合成 **2048** 获胜；棋盘填满且无法移动时游戏结束
5. **Esc** 退出

## 编译

需要 MinGW-w64 (g++)：

```bash
g++ -O2 -static -mwindows -o 2048nm.exe 2048nm.cpp -lcomctl32
```

`-static` 静态链接，生成的 exe 无需额外 DLL，在任何 Windows 上双击即玩。

## 文件说明

| 文件 | 说明 |
| --- | --- |
| `2048nm.cpp` | 完整源代码（单文件） |
| `release.yml` | GitHub Actions 工作流：移到 `.github/workflows/` 下可自动编译并发布 Release |
