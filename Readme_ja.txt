URG3D Library

About this software:
  このソフトウェアは北陽電機株式会社の三次元測域センサを利用するためのライブラリです。
  センサの利用方法を示すためのサンプルが同梱されています。

  *** Cautions! ***
  This library support ILP32 or LLP64 or LP64.
  These "int" type is 32bit.


Authors:
  Akira Oshima (Doog Inc.) software@doog-inc.com
  Takeshi Aoki (Hokuyo Automatic Co., Ltd.) aoki@hokuyo-aut.co.jp
  Kristou Mehrez (Hokuyo Automatic Co., Ltd.) kristou@hokuyo-aut.jp
  Kiyoshi Matsuo (Hokuyo Automatic Co., Ltd.) k-matsuo@hokuyo-aut.co.jp
  Jun Fujimoto (Hokuyo Automatic Co., Ltd.) j_fujimoto@hokuyo-aut.co.jp
  HUANG da (Hokuyo Automatic Co., Ltd.) d-huang@hokuyo-aut.co.jp

License:
  (C)Simplified BSD License.
  See COPYRIGHT file.

Community site:
 3D-URG User's Group (English):
  https://groups.google.com/forum/?hl=ja#!forum/3d-urg-users-group
  https://sites.google.com/site/3durgusersgroup/

 3D-URG User's Group (Japanese):
  https://groups.google.com/forum/?hl=ja#!forum/3d-urg-users-group-jp
  https://sites.google.com/site/3durgusersgroupjp/

 URG Network
  http://sourceforge.net/projects/urgnetwork/

Library usage:

  ##### Visual Studio Solution (Windows) #####

  urg3d_library-X.X.X/vs201*/urg3d.sln をビルドします。
  (vs2010 と vs2012 に対応しています)

  urg3d.lib のスタティックライブラリと各サンプルの実行ファイルが生成されます。

  visual studio 向けのサンプルプロジェクトは urg3d_library-X.X.X/vs201*/ 以下にあります。
  サンプルを実行するには、ビルド時に出力された実行ファイルを実行するか、実行するプロジェクトをスタートアッププロジェクトに設定し、実行してください。

  *ライブラリを利用するための Visual C++ 設定*

    生成された urg3d.lib と urg3d_library-*.*.*\include\ をコピーして利用します。
    プロジェクトに以下の３つの設定をおこなうことで、ライブラリが利用できます。

    1. プロジェクトのプロパティ "構成プロパティ" -> "C/C++" -> "全般" のフォームにある "追加のインクルードディレクトリ" に urg3d_library-*.*.*\include\ のパスを記述します。
    2. プロジェクトのプロパティ "リンカ" -> "全般" のフォームにある "追加のライブラリディレクトリ" に urg3d.lib をコピーしたディレクトリを記述します。
    3. プロジェクトのプロパティ "リンカ" -> "入力" のフォームにある "追加の依存ファイル" に urg3d.lib;ws2_32.lib;setupapi.lib を追記します。

  ##### Visual Studio bat compile (Windows) #####

  1. 環境変数を設定するために Visual Studio が提供している bat ファイルを
     コピーします。

  Microsoft Visual Studio XXX/Common7/Tools/vsvars32.bat を
  urg3d_library-X.X.X/windowsexeにコピーする。


  2. コンパイル用のbatファイルを実行する。

  urg3d_library-X.X.X/windowsexe/compile.batを実行する。


  3. 生成されたサンプルの実行ファイルを動かす。

  urg3d_library-X.X.X/windowsexeに生成されるexeを実行する。


  4. 生成されたファイルを削除する。

  urg3d_library-X.X.X/windowsexe/cleanobj.batを実行し
  生成されたファイルを削除する。


  ##### gcc (Linux, MinGW) #####

  必要ならば urg3d_library-X.X.X/Makefile の先頭にある PREFIX を編集して
  インストール先を変更します。現状は下記の通りになっています。
  PREFIX = /usr/local
  #PREFIX = /mingw

  コンパイルとインストールを行います。

  % make
  # make install

  ライブラリの使い方は、urg3d_library-X.X.X/samples/ 中の Makefile を参照して下さい。


Header and Source List:

  ## urg3d_connection.h, urg3d_connection.c ##

     イーサネットとシリアルポートに対応した接続機能
     （urg_libraryからの移植で改変なし）

  ## urg3d_detect_os.h ##

     OS識別機能
     （urg_libraryからの移植で、Windows用のM_PIはここへ記載）

  ## urg3d_errno.h ##

     エラー番号定義
     （urg_libraryからの移植で改変なし）

  ## urg3d_ring_buffer.h, urg3d_ring_buffer.c ##

     リングバッファ機能
     （urg_libraryからの移植で改変なし）

  ## urg3d_tcpclient.h, urg3d_tcpclient.c ##

     イーサネットによる通信機能
     （urg_libraryからの移植で、URG3D_MAX_RX_BUFFER_BITを12bitから16bitへ増やした）

  ## urg3d_ticks.h, urg3d_ticks.c ##

     複数OSに対応した時刻機能
     （urg_libraryからの移植で、tick_ms()を利用する構成に修正）

  ## urg3d_sensor.h, urg3d_sensor.c ##

     イーサネット限定でVSSPによる通信機能とユーザ関数群
     なお、基礎的な機能として以下の２つを追加している。
     ・urg3d_ring_bufferに実装の無い、「ポインタは動かさない仮読込」機能の追加
     ・urg3d_tcpclientに実装の無い、「ノンブロッキング読込かつ全データを
     　一旦リングバッファに入れる」機能の追加
     ・
