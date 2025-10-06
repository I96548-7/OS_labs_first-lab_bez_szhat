#include "tree.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

enum ErrorCodes createNewNode(struct Node** nodePtr, const char *directory_name, enum NodeTypes nodeType)
{
    if ((nodeType == FILE_NODE) + (nodeType == FOLDER_NODE) != 1)
        return UNREGISTERED_TYPE_OF_NODE;

    (*nodePtr) = malloc(sizeof(struct Node));
    
    (*nodePtr)->type = nodeType;

    (*nodePtr)->data = NULL;
    (*nodePtr)->dataSize = 0;
    
    // копируем название по-сишному
    (*nodePtr)->name = malloc(sizeof(char)*strlen(directory_name));
    strcpy((*nodePtr)->name, directory_name);

    return OK;
};

enum ErrorCodes deleteTree(struct Node* tree)
{
    if (tree == NULL)
        return CLEAR_NULL_PTR;

    // если папка -- удаляем всё содержимое как отдельные Node
    if (tree->type == FOLDER_NODE)
    {
        for (int i = 0; i < tree->dataSize; ++i)
        {
            // удаляем все объекты Node в папке по очереди
            deleteTree(((struct Node**)tree->data)[i]);
        };
        
        // и просто очищаем выделенную память
        free(tree->data);
        free(tree->name);

        // здесь можно было написать вместо двух free один конгениальный, но неочевидный ход:
        // tree->type = FILE_NODE;
        // тогда бы следующий if сам всё удалил. 
        // Но это уже из разряда олимпиадного программирования))
    };

    // если файл -- просто чистим содержимое
    if (tree->type == FILE_NODE)
    {
        free(tree->data);
        free(tree->name);
    };

    return OK;
};

enum ErrorCodes addNewObjectToFolderNode(struct Node* object, struct Node* folderNode)
{
    if (folderNode->type != FOLDER_NODE)
        return NODE_TYPE_ERROR;

    // увеличиваем размер массива на 1
    folderNode->dataSize += 1;
    folderNode->data = realloc(folderNode->data, 
        sizeof(struct Node**) * folderNode->dataSize);

    // записываем в конец массива указатель на новый объект
    ((struct Node**)folderNode->data)[folderNode->dataSize - 1] = object;

    return OK;
};