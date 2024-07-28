@echo off
cl main.cpp /nologo /Z7 /fsanitize=address /GS- /EHa- /W4 /wd4244 /wd4838 /link /STACK:16777216 /incremental:no raylibdll.lib kernel32.lib
