# for mac users ( with homebrew using M series chips )
COMPILE CODE 

gcc uphill_car.c -o main \
  -I/opt/homebrew/include \
  -L/opt/homebrew/lib \
  -lraylib \
  -framework Cocoa -framework IOKit -framework CoreVideo -framework OpenGL

RUN 
./main

# for window users to compile this use the following code
# 1) make a file build.sh and write the code 
      gcc -o game game.c -lm -lraylib
# 2) to compile now do this 
     chmod +x build.sh
# 3) to run 
      ./build.sh