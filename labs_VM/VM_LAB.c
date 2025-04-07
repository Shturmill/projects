#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void process_csv_line(const char *line) {
    char *line_copy = strdup(line);
    if (!line_copy) {
        perror("Erroe wiht reading");
        return;
    }

    line_copy[strcspn(line_copy, "\n")] = 0;


    char *x = strtok(line_copy, ",");
    char *y = strtok(NULL, ",");

    if (x && y) {
        printf("dot (%s, %s)\n", x, y);
    } else {
        printf("Error in row: %s\n", line);
    }

    free(line_copy);
}

void read_csv_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("");
        return;
    }

    char line[32];
    while (fgets(line, sizeof(line), file)) {
        process_csv_line(line);
    }

    fclose(file);
}

int main() {
    read_csv_file("sample_11.csv");
    
    return 0;
}