# URG-3D Python ライブラリ

北陽電機のURG-3Dセンサを、Pythonから利用するためのライブラリです。

センサとの通信とプロトコル処理には、C++版の`urg3d_library`を使用しています。Pythonからセンサの接続、初期化、計測データの取得、センサ情報の読み出しなどを行えます。

## 対応環境

- Python 3.9以上
- WindowsまたはLinux
- センサとPCがネットワーク接続されていること

公開済みのwheelを利用する場合は、通常、CMakeやC++コンパイラは必要ありません。ソースからビルドする場合は、Pythonに加えてCMake、C++コンパイラ、pybind11、scikit-build-coreが必要です。

## インストール

### wheelを利用する場合

Pythonの実行環境へインストールします。仮想環境は必須ではありません。プロジェクトごとの依存関係を分けたい場合は、仮想環境を利用してください。

```powershell
mkdir my_urg3d_app
cd my_urg3d_app
python -m venv .venv
.\.venv\Scripts\Activate.ps1
python -m pip install --upgrade pip
python -m pip install urg3d
```

仮想環境を使わない場合は、仮想環境の作成と有効化を省略して、使用するPython環境で次を実行します。

```powershell
python -m pip install urg3d
```

仮想環境を使う場合、Linuxでは次のコマンドで有効にします。

```bash
source .venv/bin/activate
```

### ソースからビルドする場合

ソースからビルドする場合は、次の開発ツールが必要です。

- Python 3.9以上
- CMake 3.18以上
- C++コンパイラ
- pybind11
- scikit-build-core

Windowsでは、MSVC Build Tools、LLVM/Clang、MinGW-w64などのC++コンパイラを使用できます。

インストール方法の一例：

- [CMakeのダウンロード](https://cmake.org/download/)
- [Microsoft C++ Build Tools](https://visualstudio.microsoft.com/visual-cpp-build-tools/)

Linuxでは、GCCまたはClangなどのC++コンパイラとCMakeをインストールしてください。

必要なPythonパッケージは、リポジトリの`python`ディレクトリで次のようにインストールします。

```powershell
python -m pip install pybind11 scikit-build-core
python -m pip install .
```

`pip install .`は、手元のソースからビルドしてインストールする方法です。wheelを事前に用意できない場合も、この方法で利用できます。ソースを変更するたびに、再度インストールしてください。

## 基本的な使い方

具体的な使い方は、実行可能なサンプルを参照してください。`Sensor`を使った処理の基本的な流れは次のとおりです。

1. `open()`でセンサへ接続する
2. `initialize()`でセンサを初期化する
3. `start_data()`で取得するデータの種類を指定する
4. `get_measurement_data()`または`get_auxiliary_data()`でデータを取得する
5. `stop_data()`と`close()`で終了する

処理結果は整数で返ります。一般に、正数はデータを取得できたこと、`0`はまだデータが到着していないこと、負数はエラーを表します。データを返すメソッドは、処理結果とデータのタプルを返します。

## サンプル

サンプルは`python/samples`にあります。`python`ディレクトリをカレントディレクトリにして実行してください。

センサ情報を表示する例：

```powershell
python .\samples\get_sensor_info.py --ip 192.168.0.10 --port 10940
```

距離・強度データを指定回数取得する例：

```powershell
python .\samples\get_measurement.py --ip 192.168.0.10 --port 10940 --count 10
```

指定したスポットを停止するまで取得する例：

```powershell
python .\samples\get_spot.py --ip 192.168.0.10 --port 10940
```

フィールド・ライン・スポットは使用するセンサに合わせて調整してください。停止するには`Ctrl+C`を押してください。

補助データを取得する例：

```powershell
python .\samples\get_auxiliary.py --ip 192.168.0.10 --port 10940 --count 1
```

VSSPコマンドを直接送信する例：

```powershell
python .\samples\low_user_request.py VER --ip 192.168.0.10 --port 10940
```

## 主なPython API

- `Sensor`：センサへの接続、初期化、データ取得
- `MeasurementType`：取得する計測データの種類
- `SensorInfo`、`SensorSpec`、`SensorVersion`：センサ情報
- `MeasurementData`、`Spot`、`Point`、`Polar`：計測結果
- `AuxiliaryData`、`AuxiliaryRecord`：補助データ

## ライブラリ開発者向けメモ

wheelは、ライブラリのソースコードをインストールしやすい形式にまとめた配布ファイルです。利用者へ配布するwheelは、対象とするOS、Pythonバージョン、アーキテクチャごとに作成します。

wheelを作成する場合は、リポジトリの`python`ディレクトリで次を実行してください。

```powershell
python -m pip install build pybind11 scikit-build-core
python -m build --wheel
```

wheelの作成にもCMakeとC++コンパイラが必要です。CIで作成する場合は、CIの実行環境にこれらのツールを用意してください。作成済みのwheelをインストールするだけの利用者には、通常これらのツールは必要ありません。

作成されたwheelは`dist`ディレクトリに出力されます。作成したwheelを確認する場合は、次のようにインストールします。

```powershell
python -m pip install .\dist\urg3d-2.1.0-*.whl
```


