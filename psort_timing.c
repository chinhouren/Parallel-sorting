#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include "helper.h"


int main(int argc, char *argv[]){
        struct timeval starttime, endtime;
        double timediff;

        if ((gettimeofday(&starttime, NULL)) == -1) {
            perror("gettimeofday");
            exit(1);
        }

        // Sets the variables to be read from the command line arguments
        int max_process;
        char *input_file_name;
        char *output_filename;
        int opt;

        if (argc != 7){
              fprintf(stderr, "Usage: psort -n <number of processes> -f <inputfile> -o <outputfile>\n");
              exit(1);
        }
        while ((opt = getopt(argc, argv, "n:f:o:")) != -1) {
            switch (opt) {
            case 'n':
                if ((max_process = atoi(optarg)) == 0){
                    fprintf(stderr, "Usage: psort -n <number of processes> -f <inputfile> -o <outputfile>\n");
                    exit(1);
                };
                break;
            case 'f':
                input_file_name = optarg;
                break;
            case 'o':
                output_filename = optarg;
                break;
            default: /* '?' */
                fprintf(stderr, "Usage: psort -n <number of processes> -f <inputfile> -o <outputfile>\n");
                exit(1);
            }
        }

        // Creates the relevant values required to start the spilting, sorting
        // and merging process
        int num_records, num_process, base_num, additional;

        // Calculate the number of records in the input file
        num_records =  get_file_size(input_file_name) / sizeof(struct rec);

        // Set the values for the number of processes to run, the base number of
        // data read by each process, and the number of leftover data that will be
        // distributed evenly to the number of processes running
        set_values(&num_process, &base_num, &additional, num_records, max_process);

        // Create n child processes and have each child process sort their respective
        // sets of records and write those records to the parent from lowest to
        // highest frequency one at a time concurrently.
        int num_data;
        int processed = 0;
        int pipe_fd[num_process - 1][2]; // One pipe for each child
        for (int i = 0; i < num_process; i++){
            // Number of records for the child to process
            if (i < additional){
                num_data = base_num + 1;
            }
            else {
                num_data = base_num;
            }
            // Pipes before the fork to make sure the child inherits the pipe
            Pipe(pipe_fd[i]);
            int n = Fork();
            if (n == 0){ //Child process
                // Opens the file
                FILE *fp_i = Fopen(input_file_name, "r");
                // Close reading end of inherited pipes as well as child's own
                // reading end
                for (int j = 0; j < (i + 1); j++){
                    Close(pipe_fd[j][0]);
                }

                // Seeks to the appropriate position of the file for the Child
                // to process.
                Fseek(fp_i, processed * sizeof(struct rec), SEEK_SET);

                // Creates a array of records for the child to store and sort
                struct rec *records_array;
                records_array = Malloc(sizeof(struct rec) * num_data);
                Fread(records_array, sizeof(struct rec) * num_data, 1, fp_i);

                // Sorts the records given to the child
                qsort(records_array, num_data, sizeof(struct rec), compare_freq);

                // Continually writes to the parent until all records are read
                int current = 0;
                while (current < num_data){
                    Write(pipe_fd[i][1], &(records_array[current]), sizeof(struct rec));
                    current++;
                }
                // Closes the writing pipe after finishing and free the relevant
                // data that were allocated.
                Close(pipe_fd[i][1]);
                free(records_array);
                Fclose(fp_i);
                // Exits normally to prevent more forks from child process
                exit(0);
            }
            else{ //Parent process
              // Increment data so that data that are already allocated to
              // be processed by a child process is not processed again
              // by another child
              processed += num_data;
              // Close writing end of the Parent process
              Close(pipe_fd[i][1]);
            }
        }

        // Reads from all the children and sort the outputs accordingly.
        struct rec *sorted_records;
        int status;
        // Merge function which reads from the fronts of the pipes of all
        // the children and extract the minimum frequency 1 at a time into
        // an array of records that will be returned
        sorted_records = merge(num_process, num_records, pipe_fd);
        for (int i = 0; i < num_process; i++){
            Wait(&status);
            if (!WIFEXITED(status) || status != 0){
                fprintf(stderr, "Child terminated abnormally\n");
                exit(1);
            }
        }
        if ((gettimeofday(&endtime, NULL)) == -1) {
            perror("gettimeofday");
            exit(1);
        }
        timediff = (endtime.tv_sec - starttime.tv_sec) +
            (endtime.tv_usec - starttime.tv_usec) / 1000000.0;
        fprintf(stdout, "%.4f\n", timediff);

        FILE *fp_o = Fopen(output_filename, "w");
        Fwrite(sorted_records, sizeof(struct rec), num_records, fp_o);
        Fclose(fp_o);
        free(sorted_records);
        exit(0);
    }
