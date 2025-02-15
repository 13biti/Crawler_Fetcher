#include <stdio.h>
#include <pcre.h>

#define OVECCOUNT 30  // Size of the output vector for matched substrings

int main() {
    const char *pattern = ".*10000000.*";  // Your regular expression pattern
    const char *filename = "DataBaseLinkFile.txt";  // Your file name

    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        return 1;
    }

    // Read the entire file into memory (you might need to adapt this for very large files)
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *fileContent =(char *) malloc(fileSize + 1);
    fread(fileContent, 1, fileSize, file);
    fileContent[fileSize] = '\0';

    fclose(file);

    // Compile the regular expression pattern
    const char *error;
    int erroffset;
    pcre *re = pcre_compile(pattern, 0, &error, &erroffset, NULL);

    if (re == NULL) {
        fprintf(stderr, "Error compiling regex at offset %d: %s\n", erroffset, error);
        return 2;
    }

    // Perform the search
    int ovector[OVECCOUNT];
    int rc = pcre_exec(re, NULL, fileContent, fileSize, 0, 0, ovector, OVECCOUNT);

    if (rc < 0) {
        if (rc != PCRE_ERROR_NOMATCH) {
            fprintf(stderr, "Error in pcre_exec: %d\n", rc);
            return 3;
        } else {
            printf("Pattern not found.\n");
        }
    } else {
        printf("Pattern found!\n");
    }

    // Free resources
    free(fileContent);
    pcre_free(re);

    return 0;
}

