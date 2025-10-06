#ifndef TREE_H
#define TREE_H

#include "errors_codes.h"

enum NodeTypes
{
    FILE_NODE, 
    FOLDER_NODE,
    NUMBER_OF_NODE_TYPES
};

struct Node
{
    enum NodeTypes type;    // папка или файл?
    char *name;             // имя файла/папки

    // в переменных data / dataSize может быть два типа данных:
    // 1. если type == FILE, то в переменную data побайтово копируется содержание файла,
    // а в переменную dataSize -- число байтов в файле.
    // 2. если type == FOLDER, то в переменную data помещается массив указателей на Node 
    // -- на объекты в папке; а в переменную dataSize -- число объектов в папке 
    char *data; 
    long dataSize;


};

enum ErrorCodes createNewNode(struct Node** nodePtr, const char *directory_name, enum NodeTypes nodeType);
enum ErrorCodes deleteTree(struct Node* tree);
enum ErrorCodes addNewObjectToFolderNode(struct Node* object, struct Node* folderNode);

#endif //TREE_H