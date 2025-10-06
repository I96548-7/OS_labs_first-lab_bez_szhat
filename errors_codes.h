#ifndef ERRORS_CODES_H
#define ERRORS_CODES_H

enum ErrorCodes
{
    DIRECTORY_NOT_OPENED,
    FILE_NOT_OPENED,
    CLEAR_NULL_PTR,
    NODE_TYPE_ERROR,
    TREE_PTR_ERROR,
    UNREGISTERED_ERROR_CODE,
    DIRECTORY_ALREADY_EXISTS,
    UNREGISTERED_TYPE_OF_NODE,
    NUMBER_OF_REGISTERED_ERRORS,
    OK
};

static const char* ERROR_MESSAGES[NUMBER_OF_REGISTERED_ERRORS] = 
{
    "Ошибка файловой системы: не удалось открыть папку!\n",
    "Ошибка файловой системы: не удалось открыть файл!\n",
    "Ошибка с указателями: произведена попытка удалить NULL!\n",
    "Ошибка работы с Node: обращение к папке как к файлу или к файлу как к папке!\n",
    "Ошибка работы с Node: передан объект типа Node*, равный NULL!\n",
    "Ошибка в коде ошибки: обнаружен некорректный код ошибки!\n",
    "Ошибка файловой системы: попытка создать папку поверх уже существующей папки!\n",
    "Ошибка работы с Node: передан неверный тип Node!\n",
};

#endif //ERRORS_CODES_H