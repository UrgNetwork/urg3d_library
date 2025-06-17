URG3D Library

ソフトウェアについて:
  このソフトウェアは北陽電機株式会社の三次元測域センサを利用するためのライブラリです。
  センサの利用方法を示すためのサンプルが同梱されています。
  * 通信プロトコルにVSSPを採用しているセンサに対応しています。YLMシリーズは対象外です。

開発者:
  Takeshi Aoki (Hokuyo Automatic Co., Ltd.) aoki@hokuyo-aut.co.jp
  Kristou Mehrez (Hokuyo Automatic Co., Ltd.) kristou@hokuyo-aut.jp
  Jun Fujimoto (Hokuyo Automatic Co., Ltd.) j_fujimoto@hokuyo-aut.co.jp
  HUANG da (Hokuyo Automatic Co., Ltd.) d-huang@hokuyo-aut.co.jp
  Kazuma Arimori (Hokuyo Automatic Co., Ltd.) k-arimori@hokuyo-aut.co.jp

ライセンス:
  (C)Simplified BSD License.
  See COPYRIGHT file.

コミュニティ:
 URG Network
  http://sourceforge.net/projects/urgnetwork/

ライブラリの使用方法:

  ##### Visual Studio Solution (Windows) #####

  urg3d_library-X.X.X/vs20**/urg3d.sln をビルドします。
  (vs2019, vs2022に対応しています)

  urg3d.lib のスタティックライブラリと各サンプルの実行ファイルが生成されます。

  visual studio 向けのサンプルプロジェクトは urg3d_library-X.X.X/vs20**/ 以下にあります。
  サンプルを実行するには、ビルド時に出力された実行ファイルを実行するか、実行するプロジェクトをスタートアッププロジェクトに設定し、実行してください。

  *ライブラリを利用するための Visual C++ 設定*

    生成された urg3d.lib と urg3d_library-*.*.*\include\ をコピーして利用します。
    プロジェクトに以下の３つの設定をおこなうことで、ライブラリが利用できます。

    1. プロジェクトのプロパティ "構成プロパティ" -> "C/C++" -> "全般" のフォームにある "追加のインクルードディレクトリ" に urg3d_library-*.*.*\include\ のパスを記述します。
    2. プロジェクトのプロパティ "リンカ" -> "全般" のフォームにある "追加のライブラリディレクトリ" に urg3d.lib をコピーしたディレクトリを記述します。
    3. プロジェクトのプロパティ "リンカ" -> "入力" のフォームにある "追加の依存ファイル" に urg3d.lib;ws2_32.lib;setupapi.lib を追記します。

  ##### gcc (Linux, MinGW) #####

  必要であらば urg3d_library-X.X.X/Makefile の先頭にある PREFIX を編集して
  インストール先を変更してください。現状は下記の通りになっています。
  PREFIX = /usr/local
  #PREFIX = /mingw

  コンパイルとインストールを行います。

  % make
  # make install

Header and Source List:

  ## Urg3dConnection.h, Urg3dConnection.cpp ##

     イーサネットへの接続機能

  ## Urg3dDetectOS.h ##

     OS識別機能

  ## Urg3dRrrno.h ##

     エラー番号定義

  ## Urg3dRingBuffer.h, Urg3dRingBuffer.cpp ##

     リングバッファ機能

  ## Urg3dTcpclient.h, Urg3dTcpclient.cpp ##

     TCPによる通信機能

  ## Urg3dTicks.h, Urg3dTicks.cpp ##

     複数OSに対応した時刻機能

  ## Urg3dSensor.h, Urg3dSensor.cpp ##

     VSSPによる通信機能とユーザ関数群

  ## Urg3d_t.h ##

     定数、構造体、列挙子の定義
