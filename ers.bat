@echo off

SET DIR=%~dp0

SET FOUND_PYTHON_EXE=""
SET FOUND_CMAKE_EXE=""

1>NUL 2>NUL CMD /c where python2
IF NOT ERRORLEVEL 1 (
   SET FOUND_PYTHON_EXE=python2
)

IF %FOUND_PYTHON_EXE% EQU "" (
   1>NUL 2>NUL CMD /c where python
   IF NOT ERRORLEVEL 1 (
      SET FOUND_PYTHON_EXE=python
   )
)

IF %FOUND_PYTHON_EXE% EQU "" (
  ECHO ERROR: no Python executable found on PATH.
  ECHO Make sure you can run 'python' or 'python2' in a Command Prompt.
  EXIT /b
)

1>NUL 2>NUL CMD /c where cmake
IF NOT ERRORLEVEL 1 (
   SET FOUND_CMAKE_EXE=cmake
)

IF %FOUND_CMAKE_EXE% EQU "" (
  ECHO ERROR: no cmake executable found on PATH.
  ECHO Make sure you can run 'cmake' in a Command Prompt.
  EXIT /b
)

1>NUL 2>NUL CMD /c where virtualenv
IF ERRORLEVEL 1 (
   ECHO ERROR: virtualenv not installed.
   EXIT /b 1
)

1>NUL 2>NUL CMD /c where npm
IF ERRORLEVEL 1 (
   ECHO ERROR: npm not installed.
   EXIT /b 1
)


SET "DIR=%DIR%\venv"
SET "DEACTIVATE=%DIR%\venv\Scripts\deactivate.bat"

IF "%1" == "setup" (
   IF NOT DEFINED DevEnvDir (
      IF NOT EXIST "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" (
         ECHO ERROR: could not find vcvarsall.bat
         ECHO Make sure you installed Visual Studio 2015.
         EXIT /b 1
      )
      CALL "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat"
   )
   GOTO :setup
) ELSE (
       IF NOT EXIST "%DIR%\Scripts\activate.bat" (
          ECHO venv not found. running "ers setup".
          ECHO enter the command again after setup is finished.
          timeout 10
          GOTO :setup
       )
       CALL "%DIR%\Scripts\activate.bat"
       python ers.py %*
       CALL "%DIR%\Scripts\deactivate.bat"
       EXIT /b %ERRORLEVEL%
       GOTO :eof
)

:setup
IF NOT EXIST "%DIR%\venv" (
       virtualenv venv
       PUSHD venv\Scripts
       CALL activate
       pip install click
       pip install wget
       CALL deactivate
       POPD
)

git submodule init
git submodule update

CALL venv\Scripts\activate.bat

python ers.py cef

CALL venv\Scripts\deactivate.bat

IF NOT EXIST "build\bin" (
   ECHO Creating build\bin
   mkdir build\bin
)

IF NOT EXIST "build\bin\Debug" (
   ECHO Creating build\bin\Debug
   mkdir build\bin\Debug
)

IF NOT EXIST "build\bin\Release" (
   ECHO Creating build\bin\Release
   mkdir build\bin\Release
)

pushd libs\cef_binary_3.2704.1414.g185cd6c_windows64\Debug
for %%f in (*.*) do mklink /H ..\..\..\build\bin\Debug\%%f %%f
popd

pushd libs\cef_binary_3.2704.1414.g185cd6c_windows64\Release
for %%f in (*.*) do mklink /H ..\..\..\build\bin\Release\%%f %%f
popd

pushd libs\cef_binary_3.2704.1414.g185cd6c_windows64\Resources
for %%f in (*.*) do mklink /H ..\..\..\build\bin\Release\%%f %%f
for %%f in (*.*) do mklink /H ..\..\..\build\bin\Debug\%%f %%f
popd

pushd build\bin\Debug
mklink /J locales ..\..\..\libs\cef_binary_3.2704.1414.g185cd6c_windows64\Resources\locales
popd

pushd build\bin\Release
mklink /J locales ..\..\..\libs\cef_binary_3.2704.1414.g185cd6c_windows64\Resources\locales
popd

pushd libs\cef_binary_3.2704.1414.g185cd6c_windows64
mkdir build && pushd build
cmake -G "Visual Studio 14 Win64" ..
msbuild /p:Configuration=Release /p:Platform="x64" /nologo /m /v:m cef.sln
msbuild /p:Configuration=Debug /p:Platform="x64" /nologo /m /v:m cef.sln
popd
popd

pushd libs\g3logger
mkdir build && pushd build
cmake .. -G "Visual Studio 14 Win64"
msbuild /p:Configuration=Debug g3log.sln
msbuild /p:Configuration=Release g3log.sln
popd
popd

tools\build\premake5.exe vs2015

CALL "%DIR%\Scripts\activate.bat"
python ers.py client
CALL "%DIR%\Scripts\deactivate.bat
