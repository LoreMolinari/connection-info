#define MAXLINE 1024

char* getRequest(FILE *f) {

    int choicheFound = 0;
    char* response = malloc (sizeof (char) * MAXLINE);
    char header_line[MAXLINE];
    char* res;
    char* scan;
    int exit = 0;

    if (f == NULL) die("getReqeuest() error.\n");
    printf("\n\n");

    do {
        res = fgets(header_line, MAXLINE, f);
        if (res != NULL) {
            printf("%s", res);
            if (!choicheFound) {
                choicheFound = searchReferer(res, response);
            }
        }
    } while (res != NULL && strcmp(header_line, "\r\n") != 0);
    
    //printf("%s\n", response);
    return response;
}


int searchReferer(char *line, char *address) {

    int found = 0;
    char *ptr;
    char *name;
    char *value;

    name = strndup(line, MAXLINE);
    ptr = index(name, (int)':');
    if (ptr == NULL) return 0;
    
    *ptr = '\0';
    ptr = index(line, (int) ':');
    value = strndup(ptr + 2, MAXLINE);

    value[strlen(value)-2] = '\0';

    if (strncmp(name, "Referer", MAXLINE) == 0) {
        found = 1;
        strncpy(address, value, MAXLINE);
    }

    free(name);
    free(value);

    return found;
}