#include "utils.h"


int getSplitLine(FILE *file, vector<string> &tokens) {
    string line = "";

    char buffer[BUFFER_SIZE];

    tokens.clear();
    memset(&buffer, 0, sizeof(buffer));

    if (fgets(buffer, BUFFER_SIZE, file)) {
        if (isComment(buffer)) {
            tokens.clear();
            return 0;
        }

        line = string(buffer);
        line = line.substr(0, line.find("\n"));
        split(line, " ", tokens);

        return 1;
    }

    return EOF;
}

int isComment(char *buffer) {
    return strlen(buffer) == 0 || buffer[0] == '#' ||
    buffer[0] == '\n' || buffer[0] == ' ' || buffer[0] == '\0';
}

int split(string str, string del, vector<string> &tokens) {
    int num = 0;
    int start = 0;
    int end = str.find(del);

    while (end != -1) {
        tokens.push_back(str.substr(start, end - start));
        start = end + del.size();
        end = str.find( del, start );
    }

    tokens.push_back(str.substr(start, end - start));

    return num;
}
