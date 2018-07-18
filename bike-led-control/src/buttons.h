
#include <stdlib.h>

typedef void (*press_ptr)(int);

void buttonBegin(press_ptr press, press_ptr longpress);
void buttonOnTimerTick();
void buttonOnLoop();

void buttonSetLight(uint8_t level);
