#include "archivator.h"
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>

// приватные функции
char* _formSubdirectoryFullName(const char *directoryName, const char *subdirectoryName);
char* _formFileFullName(const char *directoryName, const char *fileName);
char* _getFolderPersonalName(const char *directoryFullName);
enum ErrorCodes _decodeTreeFromArrayOfBytes(struct Node **tree, char *arrayOfBytes, int sizeOfArray, int *position);

const char codesOfTypesOfNodes[NUMBER_OF_NODE_TYPES] = {0, 1};
const enum NodeTypes decodedTypesOfNodes[NUMBER_OF_NODE_TYPES] = {FILE_NODE, FOLDER_NODE};

enum ErrorCodes formTreeWithDirectory(struct Node **tree, const char *directoryName)
{
    enum ErrorCodes errCode;
    DIR *directory;
    struct dirent *currentObject;

    directory = opendir(directoryName);
    if (!directory) {
        printf("Ошибка открытия папки '%s'\n", directoryName);
        return DIRECTORY_NOT_OPENED;
    };

    // создаём дерево; первая вершина, очевидно, папка
    char *nodePersonalName = _getFolderPersonalName(directoryName);
    createNewNode(tree, nodePersonalName, FOLDER_NODE);
    free(nodePersonalName);

    while ( (currentObject = readdir(directory)) != NULL) {

        // скрытые папки и файлы не рассматриваем
        if (currentObject->d_name[0] == '.')
            continue;

        if (currentObject->d_type == 4)
        {
            // добавляем в дерево FOLDER_NODE
            char *subdirectoryFullName = _formSubdirectoryFullName(directoryName, currentObject->d_name);

            struct Node *tmp;

            errCode = formTreeWithDirectory(&tmp, subdirectoryFullName);
            if (errCode != OK) {
                free(subdirectoryFullName);
                return errCode;
            }

            errCode = addNewObjectToFolderNode(tmp, *tree);
            if (errCode != OK) {
                free(subdirectoryFullName);
                return errCode;
            }

            free(subdirectoryFullName);
        }
        else
        {
            // добавляем в дерево FILE_NODE
            struct Node* fileNode;

            errCode = createNewNode(&fileNode, currentObject->d_name, FILE_NODE);
            if (errCode != OK)
                return errCode;

            errCode = addNewObjectToFolderNode(fileNode, *tree);
            if (errCode != OK)
                return errCode;

            // формируем имя файла целиком
            char *fileFullName = _formFileFullName(directoryName, currentObject->d_name);

            // считываем его содержимое
            errCode = getBytesArrayFromFile(fileFullName, &fileNode->data, &fileNode->dataSize);
            if (errCode != OK) {
                free(fileFullName);
                return errCode;
            }

            free(fileFullName);
        }
    };

    closedir(directory);
    return OK;
};

char* _formSubdirectoryFullName(const char *directoryName, const char *subdirectoryName)
{
    // Убедимся, что directoryName не заканчивается на /
    int dir_len = strlen(directoryName);
    int needs_slash = (dir_len > 0 && directoryName[dir_len-1] != '/');
    
    char *subdirectoryFullName = malloc(dir_len + strlen(subdirectoryName) + (needs_slash ? 2 : 1) + 1);
    
    strcpy(subdirectoryFullName, directoryName);
    if (needs_slash) {
        strcat(subdirectoryFullName, "/");
    }
    strcat(subdirectoryFullName, subdirectoryName);
    strcat(subdirectoryFullName, "/");
    
    return subdirectoryFullName;
};

char* _formFileFullName(const char *directoryName, const char *fileName)
{
    // Убедимся, что directoryName не заканчивается на /
    int dir_len = strlen(directoryName);
    int needs_slash = (dir_len > 0 && directoryName[dir_len-1] != '/');
    
    char *fileFullName = malloc(dir_len + strlen(fileName) + (needs_slash ? 2 : 1));
    
    strcpy(fileFullName, directoryName);
    if (needs_slash) {
        strcat(fileFullName, "/");
    }
    strcat(fileFullName, fileName);
    
    return fileFullName;
};

char *_getFolderPersonalName(const char *directoryFullName)
{
    // берем часть после последнего слеша
    const char *name = directoryFullName;
    const char *last_slash = strrchr(directoryFullName, '/');
    
    if (last_slash != NULL) {
        name = last_slash + 1;
    }
    
    // Если имя пустое (путь заканчивается на /), берем предыдущую часть
    if (*name == '\0') {
        // Ищем предпоследний слеш
        const char *prev_slash = directoryFullName;
        if (last_slash != directoryFullName) {
            // Ищем слеш перед последним
            for (const char *p = directoryFullName; p < last_slash; p++) {
                if (*p == '/') {
                    prev_slash = p;
                }
            }
            if (prev_slash != directoryFullName) {
                name = prev_slash + 1;
            } else {
                name = directoryFullName;
            }
        }
    }
    
    return strdup(name);
};

enum ErrorCodes getBytesArrayFromFile(const char *fullFilename, char **bytesArray, long *lengthOfArray)
{
    FILE *fIn;

    fIn = fopen(fullFilename, "rb");
    if (fIn == NULL) {
        printf("Ошибка открытия файла '%s'\n", fullFilename);
        return FILE_NOT_OPENED;
    }

    fseek(fIn, 0, SEEK_END);
    *lengthOfArray = ftell(fIn);
    rewind(fIn);

    *bytesArray = malloc(*lengthOfArray * sizeof(char));
    fread(*bytesArray, *lengthOfArray, 1, fIn);
    fclose(fIn);

    return OK;
};

// кодирует дерево как массив байтов и возвращает указатель на этот массив 
enum ErrorCodes codeTreeAsArrayOfBytes(struct Node *tree, char **startOfArray, int *sizeOfArray)
{
    if (tree == NULL)
        return TREE_PTR_ERROR;

    // основная информация об узле
    long sizeOfCodedNodeInBytes = 0;
    sizeOfCodedNodeInBytes += sizeof(char);        // тип Node: файл или папка?
    sizeOfCodedNodeInBytes += sizeof(int);         // длина имени (названия)
    sizeOfCodedNodeInBytes += strlen(tree->name) * sizeof(char);    // имя файла/папки
    sizeOfCodedNodeInBytes += sizeof(long);        // число объектов в папке / длина файла (в зависимости от типа Node)

    char signatureOfNode = codesOfTypesOfNodes[tree->type];
    int lengthOfNodesName = strlen(tree->name);

    long shift = *sizeOfArray;

    // выделяем память под запись текущего узла
    *sizeOfArray += sizeOfCodedNodeInBytes;
    *startOfArray = realloc(*startOfArray, *sizeOfArray + (tree->type == FILE_NODE ? tree->dataSize : 0));

    memcpy(*startOfArray + shift, &signatureOfNode, sizeof(char));
    shift += sizeof(char);
    memcpy(*startOfArray + shift, &lengthOfNodesName, sizeof(int));
    shift += sizeof(int);
    memcpy(*startOfArray + shift, tree->name, lengthOfNodesName * sizeof(char));
    shift += sizeof(char) * lengthOfNodesName;
    memcpy(*startOfArray + shift, &tree->dataSize, sizeof(long));
    shift += sizeof(long); 

    // дальше:
    // 1. для файла -- просто печатаем его байты подряд
    // 2. для папки -- рекурсивно печатаем её содержимое
    if (tree->type == FILE_NODE)
    {
        memcpy(*startOfArray + shift, tree->data, tree->dataSize);
        *sizeOfArray += tree->dataSize;
    }

    if (tree->type == FOLDER_NODE)
    {
        enum ErrorCodes errCode;
        for(int i = 0; i < tree->dataSize; ++i)
        {
            errCode = codeTreeAsArrayOfBytes(((struct Node**)tree->data)[i], startOfArray, sizeOfArray);
            if (errCode != OK)
                return errCode;
        }
    }

    return OK;
};

enum ErrorCodes saveArrayOfBytesToFile(char *arrayOfBytes, int length, char *fileName)
{
    FILE *fOut = fopen(fileName, "wb");

    if (fOut == NULL)
    {
        return FILE_NOT_OPENED;
    }

    fwrite(arrayOfBytes, sizeof(char) * length, 1, fOut);
    fclose(fOut);

    return OK;
}

// опять скрыл параметр по-сишному
enum ErrorCodes    decodeTreeFromArrayOfBytes(struct Node **tree, char *arrayOfBytes, int sizeOfArray)
{
    int position = 0;
    return _decodeTreeFromArrayOfBytes(tree, arrayOfBytes, sizeOfArray, &position);
};

enum ErrorCodes _decodeTreeFromArrayOfBytes(struct Node **tree, char *arrayOfBytes, int sizeOfArray, int *position)
{
    enum ErrorCodes errCode;

    if (tree == NULL)
        return TREE_PTR_ERROR;

    // на сколько сдвинется arrayOfBytes для следующей Node
    int shift = 0;

    char nodeType;
    nodeType = arrayOfBytes[*position];
    shift += sizeof(char);

    int lengthOfNodesName;
    memcpy(&lengthOfNodesName, arrayOfBytes + shift + *position, sizeof(int));
    shift += sizeof(int);
    
    char *nameOfNode = malloc(sizeof(char) * (lengthOfNodesName + 1));
    memcpy(nameOfNode, arrayOfBytes + shift + *position, sizeof(char) * lengthOfNodesName);
    nameOfNode[lengthOfNodesName] = '\0';    // потому что в файле имя лежит без терминального нуля

    shift += sizeof(char) * lengthOfNodesName;

    errCode = createNewNode(tree, nameOfNode, decodedTypesOfNodes[nodeType]);
    if (errCode != OK)
        return errCode;
    
    free(nameOfNode);

    long dataSize;
    memcpy(&dataSize, arrayOfBytes + shift + *position, sizeof(long));
    shift += sizeof(long);
    
    // сдвигаем указатель чтения
    *position += shift;

    if((*tree)->type == FILE_NODE)
    {
        (*tree)->dataSize = dataSize;
        (*tree)->data = malloc(sizeof(char) * (*tree)->dataSize);
        memcpy((*tree)->data, arrayOfBytes + *position, (*tree)->dataSize);
        *position += (*tree)->dataSize;
    }

    if((*tree)->type == FOLDER_NODE)
    {
        enum ErrorCodes errCode;
        for (int i = 0; i < dataSize; ++i)
        {
            struct Node *sonNode;
            errCode = _decodeTreeFromArrayOfBytes(&sonNode, arrayOfBytes, sizeOfArray, position);
            if (errCode != OK)
                return errCode;
            
            errCode = addNewObjectToFolderNode(sonNode, *tree);
            if (errCode != OK)
                return errCode;
        }
    }

    return OK;
}

enum ErrorCodes formDirectoryWithTree(struct Node *tree, char *directory)
{
    enum ErrorCodes errCode;

    if (tree == NULL)
        return TREE_PTR_ERROR;
    
    if (tree->type == FILE_NODE)
    {
        char *fileFullName = _formFileFullName(directory, tree->name);

        // Создаем директорию если нужно
        char *dir_path = strdup(fileFullName);
        char *p = strrchr(dir_path, '/');
        if (p != NULL) {
            *p = '\0';
            mkdir(dir_path, 0700);
        }
        free(dir_path);

        errCode = saveArrayOfBytesToFile(tree->data, tree->dataSize, fileFullName);
        free(fileFullName);
        if (errCode != OK) return errCode;
    }

    if (tree->type == FOLDER_NODE)
    {
        // ВСЕГДА создаем поддиректорию для папок
        char *subdirectoryName = _formSubdirectoryFullName(directory, tree->name);

        struct stat st = {0};
        if (stat(subdirectoryName, &st) == -1) {
            mkdir(subdirectoryName, 0700);
        }

        // Обрабатываем содержимое
        for (int i = 0; i < tree->dataSize; ++i)
        {
            errCode = formDirectoryWithTree(((struct Node **)(tree->data))[i], subdirectoryName);
            if (errCode != OK) {
                free(subdirectoryName);
                return errCode;
            }
        }
        
        free(subdirectoryName);
    }

    return OK;
};