@echo off

del oppai.dll
del oppai.obj
cl  -D_CRT_SECURE_NO_WARNINGS=1 ^
	-DOPPAI_IMPLEMENTATION=1 ^
    -DNOMINMAX=1 ^
    -Ot ^
    -nologo -MD -Gm -GR -EHsc -W4 ^
    /LD oppai.c
