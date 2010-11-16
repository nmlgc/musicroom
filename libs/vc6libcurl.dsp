# Microsoft Developer Studio Project File - Name="libcurl" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102
# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libcurl - Win32 LIB Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE
!MESSAGE NMAKE /f "libcurl.mak".
!MESSAGE
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE
!MESSAGE NMAKE /f "libcurl.mak" CFG="libcurl - Win32 LIB Debug"
!MESSAGE
!MESSAGE Possible choices for configuration are:
!MESSAGE
!MESSAGE "libcurl - Win32 DLL Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "libcurl - Win32 DLL Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "libcurl - Win32 LIB Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "libcurl - Win32 LIB Release" (based on "Win32 (x86) Static Library")
!MESSAGE

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""

!IF  "$(CFG)" == "libcurl - Win32 DLL Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DLL-Debug"
# PROP BASE Intermediate_Dir "DLL-Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DLL-Debug"
# PROP Intermediate_Dir "DLL-Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
CPP=cl.exe
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "." /I "curl\include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "BUILDING_LIBCURL" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I "." /I "curl\include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "BUILDING_LIBCURL" /FD /GZ /c
MTL=midl.exe
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib ws2_32.lib wldap32.lib /nologo /dll /incremental:no /map /debug /machine:I386 /out:"libcurld.dll" /implib:"libcurld_imp.lib" /pdbtype:sept
# ADD LINK32 kernel32.lib ws2_32.lib wldap32.lib /nologo /dll /incremental:no /map /debug /machine:I386 /out:"libcurld.dll" /implib:"libcurld_imp.lib" /pdbtype:sept

!ELSEIF  "$(CFG)" == "libcurl - Win32 DLL Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ""
# PROP BASE Intermediate_Dir "DLL-Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ""
# PROP Intermediate_Dir "DLL-Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
CPP=cl.exe
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /I "." /I "curl\include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "BUILDING_LIBCURL" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "." /I "curl\include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "BUILDING_LIBCURL" /FD /c
MTL=midl.exe
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib ws2_32.lib wldap32.lib /nologo /dll /pdb:none /machine:I386 /out:"libcurl.dll" /implib:"libcurl_imp.lib"
# ADD LINK32 kernel32.lib ws2_32.lib wldap32.lib /nologo /dll /pdb:none /machine:I386 /out:"libcurl.dll" /implib:"libcurl_imp.lib"

!ELSEIF  "$(CFG)" == "libcurl - Win32 LIB Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ""
# PROP BASE Intermediate_Dir "LIB-Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ""
# PROP Intermediate_Dir "LIB-Debug"
# PROP Target_Dir ""
CPP=cl.exe
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "." /I "curl\include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "BUILDING_LIBCURL" /D "CURL_STATICLIB" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I "." /I "curl\include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "BUILDING_LIBCURL" /D "CURL_STATICLIB" /FD /GZ /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"libcurld.lib" /machine:I386
# ADD LIB32 /nologo /out:"libcurld.lib" /machine:I386

!ELSEIF  "$(CFG)" == "libcurl - Win32 LIB Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ""
# PROP BASE Intermediate_Dir "LIB-Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ""
# PROP Intermediate_Dir "LIB-Release"
# PROP Target_Dir ""
CPP=cl.exe
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /I "." /I "curl\include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "BUILDING_LIBCURL" /D "CURL_STATICLIB" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "." /I "curl\include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "BUILDING_LIBCURL" /D "CURL_STATICLIB" /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"libcurl.lib" /machine:I386
# ADD LIB32 /nologo /out:"libcurl.lib" /machine:I386

!ENDIF

# Begin Target

# Name "libcurl - Win32 DLL Debug"
# Name "libcurl - Win32 DLL Release"
# Name "libcurl - Win32 LIB Debug"
# Name "libcurl - Win32 LIB Release"
# Begin Group "Source Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=curl\lib\base64.c
# End Source File
# Begin Source File

SOURCE=curl\lib\connect.c
# End Source File
# Begin Source File

SOURCE=curl\lib\content_encoding.c
# End Source File
# Begin Source File

SOURCE=curl\lib\cookie.c
# End Source File
# Begin Source File

SOURCE=curl\lib\curl_addrinfo.c
# End Source File
# Begin Source File

SOURCE=curl\lib\curl_fnmatch.c
# End Source File
# Begin Source File

SOURCE=curl\lib\curl_gethostname.c
# End Source File
# Begin Source File

SOURCE=curl\lib\curl_memrchr.c
# End Source File
# Begin Source File

SOURCE=curl\lib\curl_rand.c
# End Source File
# Begin Source File

SOURCE=curl\lib\curl_rtmp.c
# End Source File
# Begin Source File

SOURCE=curl\lib\curl_sspi.c
# End Source File
# Begin Source File

SOURCE=curl\lib\curl_threads.c
# End Source File
# Begin Source File

SOURCE=curl\lib\dict.c
# End Source File
# Begin Source File

SOURCE=curl\lib\easy.c
# End Source File
# Begin Source File

SOURCE=curl\lib\escape.c
# End Source File
# Begin Source File

SOURCE=curl\lib\file.c
# End Source File
# Begin Source File

SOURCE=curl\lib\fileinfo.c
# End Source File
# Begin Source File

SOURCE=curl\lib\formdata.c
# End Source File
# Begin Source File

SOURCE=curl\lib\ftp.c
# End Source File
# Begin Source File

SOURCE=curl\lib\ftplistparser.c
# End Source File
# Begin Source File

SOURCE=curl\lib\getenv.c
# End Source File
# Begin Source File

SOURCE=curl\lib\getinfo.c
# End Source File
# Begin Source File

SOURCE=curl\lib\gtls.c
# End Source File
# Begin Source File

SOURCE=curl\lib\hash.c
# End Source File
# Begin Source File

SOURCE=curl\lib\hmac.c
# End Source File
# Begin Source File

SOURCE=curl\lib\hostares.c
# End Source File
# Begin Source File

SOURCE=curl\lib\hostasyn.c
# End Source File
# Begin Source File

SOURCE=curl\lib\hostip4.c
# End Source File
# Begin Source File

SOURCE=curl\lib\hostip6.c
# End Source File
# Begin Source File

SOURCE=curl\lib\hostip.c
# End Source File
# Begin Source File

SOURCE=curl\lib\hostsyn.c
# End Source File
# Begin Source File

SOURCE=curl\lib\hostthre.c
# End Source File
# Begin Source File

SOURCE=curl\lib\http.c
# End Source File
# Begin Source File

SOURCE=curl\lib\http_chunks.c
# End Source File
# Begin Source File

SOURCE=curl\lib\http_digest.c
# End Source File
# Begin Source File

SOURCE=curl\lib\http_negotiate.c
# End Source File
# Begin Source File

SOURCE=curl\lib\http_ntlm.c
# End Source File
# Begin Source File

SOURCE=curl\lib\if2ip.c
# End Source File
# Begin Source File

SOURCE=curl\lib\imap.c
# End Source File
# Begin Source File

SOURCE=curl\lib\inet_ntop.c
# End Source File
# Begin Source File

SOURCE=curl\lib\inet_pton.c
# End Source File
# Begin Source File

SOURCE=curl\lib\krb4.c
# End Source File
# Begin Source File

SOURCE=curl\lib\krb5.c
# End Source File
# Begin Source File

SOURCE=curl\lib\ldap.c
# End Source File
# Begin Source File

SOURCE=curl\lib\llist.c
# End Source File
# Begin Source File

SOURCE=curl\lib\md4.c
# End Source File
# Begin Source File

SOURCE=curl\lib\md5.c
# End Source File
# Begin Source File

SOURCE=curl\lib\memdebug.c
# End Source File
# Begin Source File

SOURCE=curl\lib\mprintf.c
# End Source File
# Begin Source File

SOURCE=curl\lib\multi.c
# End Source File
# Begin Source File

SOURCE=curl\lib\netrc.c
# End Source File
# Begin Source File

SOURCE=curl\lib\nonblock.c
# End Source File
# Begin Source File

SOURCE=curl\lib\nss.c
# End Source File
# Begin Source File

SOURCE=curl\lib\openldap.c
# End Source File
# Begin Source File

SOURCE=curl\lib\parsedate.c
# End Source File
# Begin Source File

SOURCE=curl\lib\pingpong.c
# End Source File
# Begin Source File

SOURCE=curl\lib\polarssl.c
# End Source File
# Begin Source File

SOURCE=curl\lib\pop3.c
# End Source File
# Begin Source File

SOURCE=curl\lib\progress.c
# End Source File
# Begin Source File

SOURCE=curl\lib\qssl.c
# End Source File
# Begin Source File

SOURCE=curl\lib\rawstr.c
# End Source File
# Begin Source File

SOURCE=curl\lib\rtsp.c
# End Source File
# Begin Source File

SOURCE=curl\lib\security.c
# End Source File
# Begin Source File

SOURCE=curl\lib\select.c
# End Source File
# Begin Source File

SOURCE=curl\lib\sendf.c
# End Source File
# Begin Source File

SOURCE=curl\lib\share.c
# End Source File
# Begin Source File

SOURCE=curl\lib\slist.c
# End Source File
# Begin Source File

SOURCE=curl\lib\smtp.c
# End Source File
# Begin Source File

SOURCE=curl\lib\socks.c
# End Source File
# Begin Source File

SOURCE=curl\lib\socks_gssapi.c
# End Source File
# Begin Source File

SOURCE=curl\lib\socks_sspi.c
# End Source File
# Begin Source File

SOURCE=curl\lib\speedcheck.c
# End Source File
# Begin Source File

SOURCE=curl\lib\splay.c
# End Source File
# Begin Source File

SOURCE=curl\lib\ssh.c
# End Source File
# Begin Source File

SOURCE=curl\lib\sslgen.c
# End Source File
# Begin Source File

SOURCE=curl\lib\ssluse.c
# End Source File
# Begin Source File

SOURCE=curl\lib\strdup.c
# End Source File
# Begin Source File

SOURCE=curl\lib\strequal.c
# End Source File
# Begin Source File

SOURCE=curl\lib\strerror.c
# End Source File
# Begin Source File

SOURCE=curl\lib\strtok.c
# End Source File
# Begin Source File

SOURCE=curl\lib\strtoofft.c
# End Source File
# Begin Source File

SOURCE=curl\lib\telnet.c
# End Source File
# Begin Source File

SOURCE=curl\lib\tftp.c
# End Source File
# Begin Source File

SOURCE=curl\lib\timeval.c
# End Source File
# Begin Source File

SOURCE=curl\lib\transfer.c
# End Source File
# Begin Source File

SOURCE=curl\lib\url.c
# End Source File
# Begin Source File

SOURCE=curl\lib\version.c
# End Source File
# Begin Source File

SOURCE=curl\lib\warnless.c
# End Source File
# Begin Source File

SOURCE=curl\lib\wildcard.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=curl\lib\arpa_telnet.h
# End Source File
# Begin Source File

SOURCE=curl\lib\config-win32.h
# End Source File
# Begin Source File

SOURCE=curl\lib\connect.h
# End Source File
# Begin Source File

SOURCE=curl\lib\content_encoding.h
# End Source File
# Begin Source File

SOURCE=curl\lib\cookie.h
# End Source File
# Begin Source File

SOURCE=curl\lib\curl_addrinfo.h
# End Source File
# Begin Source File

SOURCE=curl\lib\curl_base64.h
# End Source File
# Begin Source File

SOURCE=curl\lib\curl_fnmatch.h
# End Source File
# Begin Source File

SOURCE=curl\lib\curl_gethostname.h
# End Source File
# Begin Source File

SOURCE=curl\lib\curl_hmac.h
# End Source File
# Begin Source File

SOURCE=curl\lib\curl_ldap.h
# End Source File
# Begin Source File

SOURCE=curl\lib\curl_md4.h
# End Source File
# Begin Source File

SOURCE=curl\lib\curl_md5.h
# End Source File
# Begin Source File

SOURCE=curl\lib\curl_memory.h
# End Source File
# Begin Source File

SOURCE=curl\lib\curl_memrchr.h
# End Source File
# Begin Source File

SOURCE=curl\lib\curl_rand.h
# End Source File
# Begin Source File

SOURCE=curl\lib\curl_rtmp.h
# End Source File
# Begin Source File

SOURCE=curl\lib\curl_sspi.h
# End Source File
# Begin Source File

SOURCE=curl\lib\curl_threads.h
# End Source File
# Begin Source File

SOURCE=curl\lib\curlx.h
# End Source File
# Begin Source File

SOURCE=curl\lib\dict.h
# End Source File
# Begin Source File

SOURCE=curl\lib\easyif.h
# End Source File
# Begin Source File

SOURCE=curl\lib\escape.h
# End Source File
# Begin Source File

SOURCE=curl\lib\file.h
# End Source File
# Begin Source File

SOURCE=curl\lib\fileinfo.h
# End Source File
# Begin Source File

SOURCE=curl\lib\formdata.h
# End Source File
# Begin Source File

SOURCE=curl\lib\ftp.h
# End Source File
# Begin Source File

SOURCE=curl\lib\ftplistparser.h
# End Source File
# Begin Source File

SOURCE=curl\lib\getinfo.h
# End Source File
# Begin Source File

SOURCE=curl\lib\gtls.h
# End Source File
# Begin Source File

SOURCE=curl\lib\hash.h
# End Source File
# Begin Source File

SOURCE=curl\lib\hostip.h
# End Source File
# Begin Source File

SOURCE=curl\lib\http_chunks.h
# End Source File
# Begin Source File

SOURCE=curl\lib\http_digest.h
# End Source File
# Begin Source File

SOURCE=curl\lib\http.h
# End Source File
# Begin Source File

SOURCE=curl\lib\http_negotiate.h
# End Source File
# Begin Source File

SOURCE=curl\lib\http_ntlm.h
# End Source File
# Begin Source File

SOURCE=curl\lib\if2ip.h
# End Source File
# Begin Source File

SOURCE=curl\lib\imap.h
# End Source File
# Begin Source File

SOURCE=curl\lib\inet_ntop.h
# End Source File
# Begin Source File

SOURCE=curl\lib\inet_pton.h
# End Source File
# Begin Source File

SOURCE=curl\lib\krb4.h
# End Source File
# Begin Source File

SOURCE=curl\lib\llist.h
# End Source File
# Begin Source File

SOURCE=curl\lib\memdebug.h
# End Source File
# Begin Source File

SOURCE=curl\lib\multiif.h
# End Source File
# Begin Source File

SOURCE=curl\lib\netrc.h
# End Source File
# Begin Source File

SOURCE=curl\lib\nonblock.h
# End Source File
# Begin Source File

SOURCE=curl\lib\nssg.h
# End Source File
# Begin Source File

SOURCE=curl\lib\parsedate.h
# End Source File
# Begin Source File

SOURCE=curl\lib\pingpong.h
# End Source File
# Begin Source File

SOURCE=curl\lib\polarssl.h
# End Source File
# Begin Source File

SOURCE=curl\lib\pop3.h
# End Source File
# Begin Source File

SOURCE=curl\lib\progress.h
# End Source File
# Begin Source File

SOURCE=curl\lib\qssl.h
# End Source File
# Begin Source File

SOURCE=curl\lib\rawstr.h
# End Source File
# Begin Source File

SOURCE=curl\lib\rtsp.h
# End Source File
# Begin Source File

SOURCE=curl\lib\select.h
# End Source File
# Begin Source File

SOURCE=curl\lib\sendf.h
# End Source File
# Begin Source File

SOURCE=curl\lib\setup.h
# End Source File
# Begin Source File

SOURCE=curl\lib\setup_once.h
# End Source File
# Begin Source File

SOURCE=curl\lib\share.h
# End Source File
# Begin Source File

SOURCE=curl\lib\slist.h
# End Source File
# Begin Source File

SOURCE=curl\lib\smtp.h
# End Source File
# Begin Source File

SOURCE=curl\lib\sockaddr.h
# End Source File
# Begin Source File

SOURCE=curl\lib\socks.h
# End Source File
# Begin Source File

SOURCE=curl\lib\speedcheck.h
# End Source File
# Begin Source File

SOURCE=curl\lib\splay.h
# End Source File
# Begin Source File

SOURCE=curl\lib\ssh.h
# End Source File
# Begin Source File

SOURCE=curl\lib\sslgen.h
# End Source File
# Begin Source File

SOURCE=curl\lib\ssluse.h
# End Source File
# Begin Source File

SOURCE=curl\lib\strdup.h
# End Source File
# Begin Source File

SOURCE=curl\lib\strequal.h
# End Source File
# Begin Source File

SOURCE=curl\lib\strerror.h
# End Source File
# Begin Source File

SOURCE=curl\lib\strtok.h
# End Source File
# Begin Source File

SOURCE=curl\lib\strtoofft.h
# End Source File
# Begin Source File

SOURCE=curl\lib\telnet.h
# End Source File
# Begin Source File

SOURCE=curl\lib\tftp.h
# End Source File
# Begin Source File

SOURCE=curl\lib\timeval.h
# End Source File
# Begin Source File

SOURCE=curl\lib\transfer.h
# End Source File
# Begin Source File

SOURCE=curl\lib\urldata.h
# End Source File
# Begin Source File

SOURCE=curl\lib\url.h
# End Source File
# Begin Source File

SOURCE=curl\lib\warnless.h
# End Source File
# Begin Source File

SOURCE=curl\lib\wildcard.h
# End Source File
# End Group

# Begin Group "Resource Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=curl\lib\libcurl.rc
# End Source File
# End Group
# End Target
# End Project
