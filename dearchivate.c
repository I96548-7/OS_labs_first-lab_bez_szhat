#include "tree.h"
#include "archivator.h"
#include "errors_codes.h"
#include "console_functions.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

int main(void)
{
    enum ErrorCodes errCode;

    char fileWithCompressedDirectory[MAX_LEN_OF_STRING_NAME];
    char directoryToDearchivateIn[MAX_LEN_OF_STRING_NAME];

    char *treeCodedInBytes; 
    long numberOfBytesTreeCodedWith;

    printf("Вас приветствует программа-деархиватор!\n");
    printf("Введите имя файла, который будет распакован.\n");
    getFileOrFolderNameFromKeyboard(fileWithCompressedDirectory, "> ");

    errCode = getBytesArrayFromFile(fileWithCompressedDirectory, &treeCodedInBytes, &numberOfBytesTreeCodedWith);
    processError(errCode);
    printf("Файл \"%s\" был успешно загружен из памяти! Размер: %ld байт\n", fileWithCompressedDirectory, numberOfBytesTreeCodedWith);

    struct Node *decodedTree;
    errCode = decodeTreeFromArrayOfBytes(&decodedTree, treeCodedInBytes, numberOfBytesTreeCodedWith);
    processError(errCode);

    printf("\n=== СОДЕРЖИМОЕ АРХИВА ===\n");
    printTree(decodedTree);

    printf("\nВведите имя директории, в которую будет распакован файл.\n");
    getFileOrFolderNameFromKeyboard(directoryToDearchivateIn, "> ");

    // ПРОВЕРКА СУЩЕСТВОВАНИЯ ДИРЕКТОРИИ
    struct stat st;
    if (stat(directoryToDearchivateIn, &st) == -1) {
        printf("Ошибка: Директория '%s' не существует!\n", directoryToDearchivateIn);
        printf("Создайте директорию '%s' и повторите попытку.\n", directoryToDearchivateIn);
        return 1;
    }

    // Проверяем, что это действительно директория
    if (!S_ISDIR(st.st_mode)) {
        printf("Ошибка: '%s' не является директорией!\n", directoryToDearchivateIn);
        return 1;
    }

    printf("Директория '%s' существует, начинаем распаковку...\n", directoryToDearchivateIn);
    errCode = formDirectoryWithTree(decodedTree, directoryToDearchivateIn);
    processError(errCode);

    printf("Файл был успешно распакован!\n");
    errCode = deleteTree(decodedTree);
    processError(errCode);

    free(treeCodedInBytes);
    return 0;
}