#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* read_file(const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char* content = malloc(size + 1);
    fread(content, 1, size, fp);
    content[size] = '\0';
    fclose(fp);

    return content;
}

int main(int argc, char *argv[])
{
    char* text = read_file("data.txt");
    if (!text) {
        fprintf(stderr, "Error: Could not read data.txt\n");
        return 1;
    }

    int len = strlen(text);
    if(text[len-1]=='\n'){
        text[len-1]='\0';
        len--;
    }
    printf("%s\n", text);
    return 0;
}
