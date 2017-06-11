setlocal
cd %~dp0
REM add our nasm.exe to the path
SET PATH=%~dp0..\bin;%PATH%

SET CFG=rel

nmake -f makefile.lib.msvc EXTLIBSDIR=..\ext CFG=%CFG% libmupdf.lib %*