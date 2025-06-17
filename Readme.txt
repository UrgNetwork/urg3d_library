URG3D Library

About this software:
  This software have been developed to provide a library to use
  3D scanning range sensors of Hokuyo Automatic Corporation.  Samples
  will help to know how to use them.
  * Supports sensors that use the VSSP communication protocol. YLM series is not supported.


Author:
  Takeshi Aoki (Hokuyo Automatic Co., Ltd.) aoki@hokuyo-aut.co.jp
  Jun Fujimoto (Hokuyo Automatic Co., Ltd.) j_fujimoto@hokuyo-aut.co.jp
  HUANG da (Hokuyo Automatic Co., Ltd.) d-huang@hokuyo-aut.co.jp
  Kazuma Arimori (Hokuyo Automatic Co., Ltd.) k-arimori@hokuyo-aut.co.jp

License:
  (C)Simplified BSD License.
  See COPYRIGHT file.

Community site:
 URG Network
  http://sourceforge.net/projects/urgnetwork/

Library usage:

##### Visual Studio Solution (Windows) #####

 Build the static library "urg3d.lib" with urg3d_library-X.X.X/vs20**/urg3d.sln.
 (Visual studio 2019 and Visual studio 2022 project exist.)

 Sample solutions that use urg3d.lib exist in urg3d_library-X.X.X/vs20**/.
 Each sample can be tested on the above visual studio project if it is set as the startup project.

##### gcc (Linux, MinGW) #####

 Type "make" to compile the static library "liburg3d.a" and the shared library "liburg3d.so".
 If the compilation is successful, these libraries are created in urg3d_library-X.X.X/src/.

 (If you want to install the library to your system, execute "make install" as root.)

 Example programs exist in urg3d_library-X.X.X/samples.

Header and Source List:

  ## Urg3dConnection.h, Urg3dConnection.cpp ##

  functions to handle communication with urg sensor (ethernet)

  ## Urg3dDetectOS.h ##

  header to detect OS type

  ## Urg3dRrrno.h ##

  header for error code

  ## Urg3dRingBuffer.h, Urg3dRingBuffer.cpp ##

  functions to handle ring buffer

  ## Urg3dTcpclient.h, Urg3dTcpclient.cpp ##

   functions to handle ethernet communication

  ## Urg3dTicks.h, Urg3dTicks.cpp ##

   functions to handle timer

  ## Urg3dSensor.h, Urg3dSensor.cpp ##

   functions to handle VSSP on TCP/IP

  ## Urg3d_t.h ##

   header defining constants, structures, and enumerators.
