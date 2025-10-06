#include "console_functions.h"
#include <stdio.h>

// private functions of .c file
void _printTree(int numberOfTabs, struct Node *tree);

void printTree(struct Node *tree)
{
    if (tree == NULL)
        processError(TREE_PTR_ERROR);

    // т.к. в языке СИ нет параметров по умолчанию, то будет вот такая конструкция
    _printTree(0, tree);
};

void _printTree(int numberOfTabs, struct Node *tree)
{
    for (int i = 0; i < numberOfTabs; ++i)
        printf("\t");
    
    if (tree->type == FILE_NODE)
        printf("%s (файл; %d байт)\n", tree->name, tree->dataSize);
    
    if (tree->type == FOLDER_NODE)
    {
        printf("%s (папка; %d объектов)\n", tree->name, tree->dataSize);
        for (int i = 0; i < tree->dataSize; ++i)
            _printTree(numberOfTabs + 1, ((struct Node**)tree->data)[i]);
    }
}

void processError(enum ErrorCodes code)
{
    if (code == OK)
        return;
    
    if (code < DIRECTORY_NOT_OPENED || code >= NUMBER_OF_REGISTERED_ERRORS)
        code = UNREGISTERED_ERROR_CODE;

    printf("Error: %s", ERROR_MESSAGES[code]);
    exit(code);
}

void getFileOrFolderNameFromKeyboard(char *result, char *textOfPromt)
{
	// особенности национального ввода с клавиатуры:
    // мне надо, чтоб строка заканчивалась на терминальный нуль,
    // а по дефолту она заканчивается на перенос строки...

    printf("%s", textOfPromt);
    fgets(result, MAX_LEN_OF_STRING_NAME, stdin);
	result[strlen(result)-1] = '\0';
};
