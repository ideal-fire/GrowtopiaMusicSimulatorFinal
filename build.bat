cd src
gcc main.c fonthelper.c nathanList.c ./../icon/icon.res -Wall -Wno-char-subscripts -g -I ../../libgeneralgood/Include -L../lib -lgeneralgoodwindows -llua -lmingw32 -lSDL2main -lSDL2 -mwindows -lSDL2_net -lSDL2_image -lSDL2_mixer -lm -o ./../bin/GrowtopiaMusicSimulatorFinal.exe
cd ..
