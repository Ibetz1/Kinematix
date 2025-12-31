g++ src/*.cpp src/libs/glad/*.c -Iinc -Isrc/libs -I. -lSDL2 -lopengl32 -o main.exe
./main.exe
rm main.exe