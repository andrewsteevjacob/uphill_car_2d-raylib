# for mac users ( with homebrew using M series chips )
COMPILE CODE 

gcc basic_window_try.c -o main \
  -I/opt/homebrew/include \
  -L/opt/homebrew/lib \
  -lraylib \
  -framework Cocoa -framework IOKit -framework CoreVideo -framework OpenGL

RUN 
./main