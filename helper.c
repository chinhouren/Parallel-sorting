#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "helper.h"



int get_file_size(char *filename) {
    struct stat sbuf;

    if ((stat(filename, &sbuf)) == -1) {
       perror("stat");
       exit(1);
    }

    return sbuf.st_size;
}

/* A comparison function to use for qsort */
int compare_freq(const void *rec1, const void *rec2) {

    struct rec *r1 = (struct rec *) rec1;
    struct rec *r2 = (struct rec *) rec2;

    if (r1->freq == r2->freq) {
        return 0;
    } else if (r1->freq > r2->freq) {
        return 1;
    } else {
        return -1;
    }
}

/* A fork function that does error checking */
int Fork(void){
    int n = fork();
    if (n < 0){
        perror("fork");
        exit(1);
    }
    return n;
}

/* A malloc function that does error checking */
void *Malloc(size_t size){
    void *ptr = malloc(size);
    if (ptr == NULL){
        perror("malloc");
        exit(1);
    }
    return ptr;
}

/* A fread function that does error checking */
void Fread(void *ptr, size_t size, size_t nmemb, FILE *stream){
    if ((fread(ptr, size, nmemb, stream)) != nmemb){
        perror("fread");
        exit(1);
    }
}

/* A fseek function that does error checking */
void Fseek(FILE *stream, long int offset, int whence){
    if ((fseek(stream, offset, whence)) != 0){
        perror("fseek");
        exit(1);
    }
}


/* A pipe function that does error checking */
void Pipe(int filedes[2]){
    if ((pipe(filedes)) == -1){
        perror("pipe");
        exit(1);
    }
}


/* A close pipe function that does error checking */
void Close(int fd){
    if ((close(fd)) == -1){
        perror("close");
        exit(1);
    }
}

/* A write to file descriptor function that does error checking */
void Write(int fd, void *buf, size_t count){
    if (write(fd, buf, count) != count) {
        perror("write from child to pipe");
        exit(1);
    }
}

/* A read from file descriptor function that does error checking */
int Read(int fd, void *buf, size_t count){
    int bytes_read = read(fd, buf, count);
    if (bytes_read == -1) {
        perror("read from pipe to parent");
        exit(1);
    }
    return bytes_read;
}

/* An open file descriptor function that does error checking */
FILE *Fopen(char *filename, char *mode){
    FILE *fp;
    if ((fp = fopen(filename, mode)) == NULL){
        perror("fopen");
        exit(1);
    }
    return fp;
}

/* A close file descriptor function that does error checking */
void Fclose(FILE *stream){
    if (fclose(stream) != 0){
        perror("fclose");
        exit(1);
    }
}

/* A fwrite function that does error checking */
void Fwrite(void *ptr, size_t size, size_t nmemb, FILE *stream){
    if (fwrite(ptr, size, nmemb, stream) != nmemb){
        perror("fwrite");
        exit(1);
    }
}
/* A wait function that does error checking */
void Wait(int *wstatus){
    if (wait(wstatus) == -1){
        perror("wait");
        exit(1);
    }
}

/* Merge function which reads from the fronts of the pipes of all
   the children and extract the minimum frequency 1 at a time into
   an array of records that will be returned*/
struct rec *merge(int num_process, int num_records, int pipe_fd[num_process][2]){
    int read_over[num_process];
    int full[num_process];
    struct rec current_records[num_process];
    struct rec *sorted_records = Malloc(sizeof(struct rec) * num_records);

    // Creates a parallel list of the current reads from children's pipes
    // and whether the reads are over.
    for (int i = 0; i < num_process; i++){
        read_over[i] = 0;
        full[i] = 0;
    }

    int num_extracted = 0;
    int min = -1;
    int index = 0;
    while (num_extracted < num_records){
        // Reads into current_records[i] the corresponding i child process
        for (int i = 0; i < num_process; i++){
            // Only allows reading from pipe if the child process if not
            // yet done writing or if the element at the i-th index
            // of current_records was extracted.
            if (read_over[i] == 0 && full[i] == 0){
                if ((Read(pipe_fd[i][0], &(current_records[i]), sizeof(struct rec))) == 0){
                    Close(pipe_fd[i][0]);
                    read_over[i] = 1;
                }
                else{
                    full[i] = 1;
                }
            }
        }
        // Finds the minimum freq and index of it in current_records
        for (int i = 0; i < num_process; i++){
            if (read_over[i] == 0){
                if (min == -1 || min >= current_records[i].freq){
                    min = current_records[i].freq;
                    index = i;
                }
            }
        }
        // Extract the minimum record from the front of each pipe.
        sorted_records[num_extracted] = current_records[index];
        // Sets full of the pipe to 0 to make the program read a new
        // record into it.
        full[index] = 0;
        // Resets the min back to -1 to set and find new min of the front
        // of the pipes.
        min = -1;
        // Counts the number of extraction done, will extract exactly the
        // number of records avail
        num_extracted++;
    }
    return sorted_records;
}


// Set the values for the number of processes to run, the base number of
// data read by each process, and the number of leftover data that will be
// distributed evenly to the number of processes running
void set_values(int *num_process, int *base_num, int *additional, int num_records, int max_process){
    if (num_records <= max_process){
        *num_process = num_records;
        *base_num = 1;
        *additional = 0;
    }
    else{
        *num_process = max_process;
        *base_num = num_records / max_process;
        *additional = num_records % max_process;
    }
}
