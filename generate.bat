@echo off
conan install . -of build -s build_type=Release --build missing
utils\bin\premake5 %* vs2022
