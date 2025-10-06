#ifndef CONSOLE_FUNCTIONS_H
#define CONSOLE_FUNCTIONS_H

#include "tree.h"
#include "errors_codes.h"

static const int MAX_LEN_OF_STRING_NAME = 255;

// местные ф-ии не возвращают код ошибок, потому что они работают с консолью и, в случае ошибки,
// сразу печатают о ней и завершают программу

void printTree(struct Node *tree);  // симметрично обходит дерево и печатает
void processError(enum ErrorCodes code);
void getFileOrFolderNameFromKeyboard(char *result, char *textOfPromt);

#endif //CONSOLE_FUNCTIONS_H