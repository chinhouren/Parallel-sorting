#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include "helper.h"


int main(int argc, char *argv[]){
        FILE *fp = fopen("d-sorted-records.b", "r");
        FILE *fp_write = fopen("sorted_records.txt", "w");
        struct rec hold;
        int current = -1;
        for (int i = 0; i < 127128; i++){
            fread(&hold, sizeof(struct rec), 1, fp);
            if (current > hold.freq){
                fprintf(stderr, "Fail to sort\n");
                exit(1);
            }
            else {
                current = hold.freq;
            }
            fprintf(fp_write,"%s: %d\n",hold.word, current);
        }
        fclose(fp);
        fclose(fp_write);
        return 0;
    }
