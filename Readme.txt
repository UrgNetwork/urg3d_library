URG3D Library

About this software:
  This software have been developed to provide a library to use
  3D scanning range sensors of Hokuyo Automatic Corporation.  Samples
  will help to know how to use them.

  *** Cautions! ***
  This library support ILP32 or LLP64 or LP64.
  These "int" type is 32bit.


Author:
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

 Build the static library "urg3d.lib" with urg3d_library-X.X.X/vs201*/urg3d.sln.
 (Visual studio 2010 project and visual studio 2012 project exist.)

 Sample solutions that use urg3d.lib exist in urg3d_library-X.X.X/vs201*/.
 Each sample can be tested on the above visual studio project if it is set as the startup project.

##### Visual Studio bat compile (Windows) #####

 Copy vsvars32.bat to urg3d_library-X.X.X/windowsexe.
 Execute urg3d_library-X.X.X/windowsexe/compile.bat to generate the static library urg3d.lib and samples.

 (If you want to clear created files, you can use urg3d_library-X.X.X/windowsexe/cleanobj.bat.)

##### gcc (Linux, MinGW) #####

 Type "make" to compile the static library "liburg3d.a" and the shared library "liburg3d.so".
 If the compilation is successful, these libraries are created in urg3d_library-X.X.X/src/.

 (If you want to install the library to your system, execute "make install" as root.)

 Example programs exist in urg3d_library-X.X.X/samples.

Header and Source List:

 ## urg3d_connection.h, urg3d_connection.c ##

  functions to handle communication with urg sensor (serial/ethernet)

 ## urg3d_detect_os.h ##

  header to detect OS type

 ## urg3d_errno.h ##

  header for error code

 ## urg3d_ring_buffer.h, urg3d_ring_buffer.c ##

  functions to handle ring buffer

 ## urg3d_tcpclient.h, urg3d_tcpclient.c ##

   functions to handle ethernet communication

 ## urg3d_ticks.h, urg3d_ticks.c ##

   functions to handle timer

 ## urg3d_sensor.h, urg3d_sensor.c ##

   functions to handle VSSP on TCP/IP

