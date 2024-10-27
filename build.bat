@echo off

rem cl /nologo src/pl_mpeg.cpp /O2 /c

cl src/main.cpp /nologo /Z7 /GS- /EHa- /W4 /wd4244 /wd4838 /wd4201 /link /NOIMPLIB /NOEXP /STACK:16777216 /incremental:no glad.obj raylibdll.lib kernel32.lib opengl32.lib pl_mpeg.obj
