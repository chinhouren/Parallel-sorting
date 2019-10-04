#ifndef _HELPER_H
#define _HELPER_H

#define SIZE 44

struct rec {
    int freq;
    char word[SIZE];
};

int get_file_size(char *filename);
int compare_freq(const void *rec1, const void *rec2);
// Functions with error checking
int Fork(void);
void *Malloc(size_t size);
void Fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
void Fseek(FILE *stream, long int offset, int whence);
void Pipe(int filedes[2]);
void Close(int fd);
void Write(int fd, void *buf, size_t count);
int Read(int fd, void *buf, size_t count);
FILE *Fopen(char *filename, char *mode);
void Fclose(FILE *stream);
void Fwrite(void *ptr, size_t size, size_t nmemb, FILE *stream);
void Wait(int *wstatus);
// Functions to merge and to set values required for psort
struct rec *merge(int num_process, int num_records, int pipe_fd[num_process][2]);
void set_values(int *num_process, int *base_num, int *additional, int num_records, int max_process);
#endif /* _HELPER_H */
