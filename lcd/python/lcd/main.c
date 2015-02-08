#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>


#include "lcd.h"


int main(int argc, char **argv) {
  struct lcdmodule module;

  // Here we create and initialise two LCD modules.
  // The arguments are, in order, EN, RS, D4, D5, D6, D7.
  module = lcdInit(16, 21, 26, 19, 13, 6);

  prints(module, "Ti goes with Pi.");


  while(1) {

   
  }
  return 0;
} // main



