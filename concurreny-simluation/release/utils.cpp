#include "utils.h"


int getSplitLine(FILE *file, vector<string> &tokens) {
    string line;

    char buffer[BUFFER_SIZE];

    tokens.clear();
    memset(&buffer, 0, sizeof(buffer));

    if (fgets(buffer, BUFFER_SIZE, file)) {
        strip(buffer);
        if (isComment(buffer)) {
            tokens.clear();
            return 0;
        }

        line = string(buffer);
        split(line, " ", tokens);

        return 1;
    }

    return EOF;
}


int isComment(char *buffer) {
    return strlen(buffer) == 0 || isspace(buffer[0]) || buffer[0] == '#';
}


void split(const string &str, const string &del, vector<string> &tokens) {
    int start = 0;
    int end = str.find(del);

    while (end != -1) {
        tokens.push_back(str.substr(start, end - start));
        start = end + del.size();
        end = str.find(del, start);
    }

    tokens.push_back(str.substr(start, end - start));
}


void strip(char *str) {
    unsigned long i, j, len = strlen(str);

    char out[len];
    memset(&out, 0, sizeof(out));

    for (i = 0; i < len; i++) {
        if (str[i] == '\n' || str[i] == '\r')
            str[i] = '\0';
        if (str[i] == '\t')
            str[i] = ' ';
    }

    i = 0, j = 0;
    while (str[i] != '\0') {
        out[j] = str[i];
        if (isspace(str[i]))
            while(isspace(str[i+1])) i++;
        i++;
        j++;
    }

    strcpy(str, out);
}

