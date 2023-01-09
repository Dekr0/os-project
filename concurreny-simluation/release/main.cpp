#include "main.h"


int main(int argc, char *args[]) {

    if (argc == 4) {
        char *filename = args[1];

        size_t i;

        for (i = 0; i < strlen(args[2]); ++i) {
            if (!isdigit(args[2][i])) {
                cerr << "a4w22: invalid monitor time" << endl;
                exit(1);
            }
        }

        for (i = 0; i < strlen(args[3]); ++i) {
            if (!isdigit(args[3][i])) {
                cerr << "a4w22: invalid number of iteration" << endl;
                exit(1);
            }
        }

        int monitorTime = atoi(args[2]);
        int numIteration = atoi(args[3]);

        Simulator simulator = Simulator(filename, monitorTime, numIteration);
        simulator.start();
    }

    return 0;
}
