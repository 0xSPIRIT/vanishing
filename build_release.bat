@echo off
cl src/main.cpp /nologo /Z7 /O2 /GS- /EHa- /W4 /wd4244 /wd4838 /wd4201 /DRELEASE /link /STACK:16777216 /SUBSYSTEM:windows user32.lib /incremental:no raylibdll.lib kernel32.lib
