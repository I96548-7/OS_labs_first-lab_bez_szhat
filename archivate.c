#include "tree.h"
#include "archivator.h"
#include "errors_codes.h"
#include "console_functions.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
	char directoryToCompress[MAX_LEN_OF_STRING_NAME];
	char nameOfFileWithCompressedDirectory[MAX_LEN_OF_STRING_NAME];

	printf("Вас приветствует программа-архиватор!\n");
	printf("Введите директорию, которую хотели бы сжать.\n");

	getFileOrFolderNameFromKeyboard(directoryToCompress, "> ");
 	printf("Вас интересует директория: \"%s\"\n\n", directoryToCompress);

	struct Node *directoryAsTree;
	enum ErrorCodes errCode = formTreeWithDirectory(&directoryAsTree, directoryToCompress);
	processError(errCode);
	
	printf("Директория была успешно распознана и считана.\n");
	printf("Директория, которую вы хотите сжать, имеет вид: \n");
	printTree(directoryAsTree);
	
	printf("\n\nТеперь укажите имя файла, в который должна быть сжата директория:\n");
	getFileOrFolderNameFromKeyboard(nameOfFileWithCompressedDirectory, "> ");

	char *codedTree = NULL;
	int numberOfBytesInCodedTree;	
	
	numberOfBytesInCodedTree = 0;
	errCode = codeTreeAsArrayOfBytes(directoryAsTree, &codedTree, &numberOfBytesInCodedTree);
	processError(errCode);
	
	errCode = saveArrayOfBytesToFile(codedTree, numberOfBytesInCodedTree, nameOfFileWithCompressedDirectory);
	processError(errCode);

	printf("Директория была успешно сжата! Размер архива: %d байт\n", numberOfBytesInCodedTree);
	errCode = deleteTree(directoryAsTree);
	processError(errCode);

	free(codedTree);

	return 0;
}