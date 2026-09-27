#!/usr/bin/env bash
# 构建 2048(n*m) Android APK（无需 Gradle）
# 前提: JDK 17+ 在 PATH 中, ANDROID_SDK_ROOT 指向已安装
#       "platforms;android-34" 和 "build-tools;34.0.0" 的 SDK
# 用法: ANDROID_SDK_ROOT=/path/to/sdk ./build-apk.sh [输出文件名]
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
BUILD="$ROOT/build"
SDK="${ANDROID_SDK_ROOT:?请设置 ANDROID_SDK_ROOT 环境变量}"
BT="$SDK/build-tools/34.0.0"
PLATFORM="$SDK/platforms/android-34/android.jar"
JAVA="${JAVA:-java}"
OUT="${1:-2048nm.apk}"

rm -rf "$BUILD"
mkdir -p "$BUILD/classes"

echo "[1/5] aapt2 link"
"$BT/aapt2" link -o "$BUILD/unsigned.apk" \
  -I "$PLATFORM" \
  --min-sdk-version 24 --target-sdk-version 34 \
  --manifest "$ROOT/AndroidManifest.xml"

echo "[2/5] javac"
if command -v cygpath >/dev/null 2>&1; then
  find "$ROOT/src" -name '*.java' | cygpath -w -f - > "$BUILD/sources.txt"
else
  find "$ROOT/src" -name '*.java' > "$BUILD/sources.txt"
fi
javac -encoding UTF-8 -classpath "$PLATFORM" -d "$BUILD/classes" @"$BUILD/sources.txt"

echo "[3/5] d8"
"$JAVA" -cp "$BT/lib/d8.jar" com.android.tools.r8.D8 --min-api 24 --output "$BUILD" $(find "$BUILD/classes" -name '*.class')

echo "[4/5] 打包 classes.dex"
if command -v zip >/dev/null 2>&1; then
  (cd "$BUILD" && zip -q unsigned.apk classes.dex)
else
  python3 -c "import zipfile,sys; zipfile.ZipFile(sys.argv[1],'a').write(sys.argv[2],'classes.dex')" \
    "$BUILD/unsigned.apk" "$BUILD/classes.dex"
fi

echo "[5/5] 签名"
if [ ! -f "$BUILD/debug.keystore" ]; then
  keytool -genkeypair -keystore "$BUILD/debug.keystore" -storepass android \
    -alias androiddebugkey -keypass android -dname "CN=2048" \
    -keyalg RSA -keysize 2048 -validity 10000
fi
"$JAVA" -jar "$BT/lib/apksigner.jar" sign --ks "$BUILD/debug.keystore" --ks-pass pass:android \
  --key-pass pass:android --out "$ROOT/$OUT" "$BUILD/unsigned.apk"
"$JAVA" -jar "$BT/lib/apksigner.jar" verify "$ROOT/$OUT"
echo "完成: $ROOT/$OUT"
