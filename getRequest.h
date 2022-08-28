#define MAX_LINE_LENGTH (1024)

char* getRequest(FILE *f) {
    int choicheFound = 0;
    char* response = malloc (sizeof (char) * MAX_LINE_LENGTH);
    char header_line[MAX_LINE_LENGTH];
    char* res;
    char* scan;
    int exit = 0;

    if (f == NULL) die("getReqeuest() error.");

    do {
        res = fgets(header_line, MAX_LINE_LENGTH, f);

        if (res != NULL) {
            printf("%s", res);

            if (!choicheFound) {
                choicheFound = searchReferer(res, response);
            }
        }
    } while (res != NULL && strcmp(header_line, "\r\n") != 0);
    
    printf("\n%s\n", response);
    return response;
}


int searchReferer(char *line, char *address) {

  int found = 0;
  char *ptr;
  char *name;
  char *value;

  name = strndup(line, MAX_LINE_LENGTH);
  ptr = index(name, (int)':');
  if (ptr == NULL) {
    return 0;
  }
  // end the string at the colon
  *ptr = '\0';

  // get the value part of the header field
  ptr = index(line, (int) ':');
  value = strndup(ptr + 2, MAX_LINE_LENGTH);

  // most ugly way to remove \r\n from the end of the string
  value[strlen(value)-2] = '\0';

  if (strncmp(name, "Referer", MAX_LINE_LENGTH) == 0) {
    found = 1;
    strncpy(address, value, MAX_LINE_LENGTH);
  }

  free(name);
  free(value);

  return found;
}