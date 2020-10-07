void initializeArrays(char** , char**, int, int);
void initializeEnvironment(char** , int, int);
void produceNextPlain(char**, char**, int, int);
void swapPlains(char** , char **, char**);
int getNumOfNeighbours(char**, int, int, int, int);
int updateState(char**, int, int, int);
void freeArrays(char**, char**, int, int);
double calculateTime(struct timeval, struct timeval);
