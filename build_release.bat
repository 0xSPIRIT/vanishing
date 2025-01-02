@echo off

cl src/main.cpp src/pl_mpeg.cpp /nologo /Z7 /O2 /w /GS- /EHa- /MT /wd4244 /wd4838 /wd4201 /DRELEASE /link /STACK:16777216 /SUBSYSTEM:windows user32.lib /incremental:no raylibdll.lib kernel32.lib opengl32.lib
