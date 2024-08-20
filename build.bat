@echo off
cl src/main.cpp /nologo /Z7 /GS- /EHa- /W4 /wd4244 /wd4838 /wd4201 /link /STACK:16777216 /incremental:no raylibdll.lib kernel32.lib
