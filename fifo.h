#define NUM_OF_PIPES 2
#define BUFFER_ELEMENT_SIZE (SAMPLES_PER_PASS + 10)
#define BUFFER_LENGTH 50

void initpipe();
void removepipe();
char write_pipe(int pipenum, unsigned char *data, int len);
int read_pipe_wait(int pipenum, unsigned char *data, int maxlen);
int NumberOfElementsInPipe(int pipenum);
