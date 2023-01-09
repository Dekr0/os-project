#include "main.h"


int main(int argc, char *args[]) {

    if (argc == 4) {
        char *filename = args[1];
        int monitorTime = atoi(args[2]);
        int numIteration = atoi(args[3]);

        Simulator simulator = Simulator(filename, monitorTime, numIteration);
        simulator.start();
    }

    return 0;
}
