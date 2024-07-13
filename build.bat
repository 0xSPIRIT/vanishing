@echo off
cl main.cpp /nologo /Z7 /GS- /EHa- /W4 /wd4244 /link /STACK:16777216 /incremental:no raylibdll.lib
