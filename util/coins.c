#include <stdio.h>

int main(int argc, char ** argv) {

  int array[5] = {2, 3, 5, 7, 9};
  int i, j, k, l, m;

  for(i = 0; i < 5; i++) 
    for(j = 0; j < 5; j++) 
      for(k = 0; k < 5; k++) 
        for(l = 0; l < 5; l++) 
           for(m = 0; m < 5; m++) 
             if ((array[i] + array[j] * (array[k]*array[k]) + (array[l]*array[l]*array[l]) - array[m]) == 399) {
              printf("%d %d %d %d %d\n", i, j, k, l, m);
             }


}
