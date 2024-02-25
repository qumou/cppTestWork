#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TAG_LENGTH 2
#define MAX_LENGTH_LENGTH 2
#define MAX_VALUE_LENGTH 24
#define MAX_TAG_BUFFER_SIZE 4
#define MAX_BUFFER_SIZE 100

char lenBufToConv[MAX_LENGTH_LENGTH+1];

typedef struct {
    char tag[MAX_TAG_LENGTH + 1];
    char length[MAX_LENGTH_LENGTH + 1];
    char value[MAX_VALUE_LENGTH + 1];
} TLV;

const char testStr[] = "NA0810000001AB13C000000001000LB13C000000002000UT192022-05-01 09:00:00\nNA09100000002AB13D000000005000LB13C000000000050UT192022-05-01 10:00:00\r\nNA101000000003AB13C000000010000UT192022-05-01 11:00:00\n";

int findNonZeroToken(char *str, int len)
{    
    for (int i = 0; i < len-1; i++)
    {
        if (str[i] != '0' & str[i] != '\0')
        {
            return i;
        }        
    }
return -1;
}

void printSQLInsert(const TLV *tags, int tagCount) {
    printf("INSERT INTO T_ACCOUNT (ACCOUNT_NO, AVAIL_BAL, LEDGER_BAL, UPD_DATE, UPD_TIME) VALUES (");

    for (int i = 0; i < tagCount; i++) {
        if (strcmp(tags[i].tag, "NA") == 0) {
            printf("'%s', ", tags[i].value);
        } else if (strcmp(tags[i].tag, "AB") == 0 || strcmp(tags[i].tag, "LB") == 0) {
            if(tags[i].value[0] == 'C') {
                printf("'%s', ", &tags[i].value[findNonZeroToken(&tags[i].value[1],MAX_VALUE_LENGTH)+1]);
            }else  printf("'-%s', ", &tags[i].value[findNonZeroToken(&tags[i].value[1],MAX_VALUE_LENGTH)+1]);
        } else if (strcmp(tags[i].tag, "UT") == 0) {
            printf("TO_DATE('%s', 'YYYY-MM-DD HH24:MI:SS')", tags[i].value);
        }
    }

    printf(");\n");
}

int main(int argc, char *argv[]) {
    int commitLimit = -1; // Default value, no COMMIT statements unless specified

    if (argc == 3 && strcmp(argv[1], "-c") == 0) {
        commitLimit = atoi(argv[2]);
    }

    int tagCount = 0;
    TLV tags[MAX_TAG_BUFFER_SIZE];

    char buffer[MAX_BUFFER_SIZE];
    char *line;

    while ((line = fgets(buffer, sizeof(buffer), stdin)) != NULL) {
        // Remove trailing newline character
        size_t length = strlen(line);

        // Обработка тегов в пределах одной строки
        char *currentPosition = line;
        int index = 0;
        while (index < length-1) {
            // Считываем тег
            strncpy(tags[tagCount].tag, currentPosition, MAX_TAG_LENGTH);
            tags[tagCount].tag[MAX_TAG_LENGTH] = '\0';

            // Считываем длину
            strncpy(tags[tagCount].length, currentPosition + MAX_TAG_LENGTH, MAX_LENGTH_LENGTH);
            tags[tagCount].length[MAX_LENGTH_LENGTH] = '\0';
            int tagLength = atoi(tags[tagCount].length);

            // Считываем значение
            strncpy(tags[tagCount].value, currentPosition + MAX_TAG_LENGTH + MAX_LENGTH_LENGTH, tagLength);
            tags[tagCount].value[tagLength] = '\0';

            tagCount++;

            // Перемещаем указатель на следующий тег
            currentPosition += MAX_TAG_LENGTH + MAX_LENGTH_LENGTH + tagLength;
            index += MAX_TAG_LENGTH + MAX_LENGTH_LENGTH + tagLength;
        }
        printSQLInsert(tags, tagCount);
        tagCount = 0;
    }
    return 0;
}
