@echo off

cl /nologo src/pl_mpeg.cpp /O2 /c
cl src/main.cpp /nologo /Z7 /O2 /GS- /EHa- /W4 /wd4244 /wd4838 /wd4201 /DRELEASE /link /STACK:16777216 /SUBSYSTEM:windows user32.lib /incremental:no glad.obj raylibdll.lib kernel32.lib opengl32.lib pl_mpeg.obj
