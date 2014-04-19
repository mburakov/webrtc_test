webrtc_test
===========
Peerconnection client example from webrtc migrated to Qt5

Building for GNU/Linux
======================
Are you serious?

Building for Windows 8.1
========================
WebRTC
------
1. Install [DirectX SDK](http://www.microsoft.com/en-us/download/details.aspx?id=6812)
2. Get [Depot Tools](https://src.chromium.org/svn/trunk/tools/depot_tools.zip)
3. Add directory of depot_tools to global PATH environment variable
4. Make root directory for webrtc, go to this directory
5. Call gclient without arguments - it will fetch all the necessary components
6. gclient may not succeed at first launch - run it again
7. Ensure that gclient does not do anything other than just displaying usage info
8. In webrtc root dir call: `gclient config http://webrtc.googlecode.com/svn/trunk`
9. In webrtc root dit call: `gclient sync --force`
10. In the trunk subdirectory start building: `ninja -C out\Release`
11. Terminate upon first object file
12. Remove all the generated lib and obj files from trunk\out\Release subdir
13. Replace "/MT" with "/MD" in all the generated dot ninja files in trunk\out subdir
14. In the trunk subdirectory start building again: `ninja -C out\Release`

Qt5
---
1. Install [ActiveState Perl](http://downloads.activestate.com/ActivePerl/releases/5.16.3.1604/ActivePerl-5.16.3.1604-MSWin32-x64-298023.msi)
2. Checkout stable branch qt5 code proxy: `git clone git://gitorious.org/qt/qt5.git qt5`
3. In qt5 directory get the real source code: `perl ./init-repository --no-webkit`
4. Start a new terminal, execute SetEnv.cmd in depot_tools\win_toolchain\vs2013_files\win8sdk\bin
5. Add VS2013 runtime dir to PATH: `set PATH=%PATH%;c:\Projects\depot_tools\win_toolchain\vs2013_files\sys32` assuming your root directory for the project is c:\Projects
6. Change directory to the root of Qt5
7. Explicitly specify platform: `set QMAKESPEC=win32-msvc2013`
8. Configure Qt5: `configure -developer-build -opensource -mp -nomake examples -nomake tests -debug-and-release -c++11 -no-warnings-are-errors -platform win32-msvc2013`
9. Accept the license proposal
10. After configuration is finished, run `nmake`

webrtc_test
-----------
1. Get latest source code of webrtc_test: `git clone https://github.com/Satius/webrtc_test`
2. Gather all libraries (dot lib) from webrtc\trunk\out\Release and place them to webrtc_test\libs.release directory
3. Start a new terminal, execute SetEnv.cmd in depot_tools\win_toolchain\vs2013_files\win8sdk\bin
4. Add VS2013 runtime dir to PATH: `set PATH=%PATH%;c:\Projects\depot_tools\win_toolchain\vs2013_files\sys32` assuming your root directory for the project is c:\Projects
5. Change directory to webrtc_test
6. Start build: `nmake`
