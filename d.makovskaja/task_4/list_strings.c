#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 1. Описание структуры узла
struct Node {
    char *data;
    struct Node *next;
};

// 2. Функция добавления (append)
void append(struct Node **head, const char *str) {
    // Выделяем память под сам узел
    struct Node *new_node = (struct Node *)malloc(sizeof(struct Node));
    
    // Выделяем память под строку и копируем данные
    new_node->data = (char *)malloc(strlen(str) + 1);
    strcpy(new_node->data, str);
    new_node->next = NULL;

    // Если список пуст, делаем новый узел головой
    if (*head == NULL) {
        *head = new_node;
    } 
    // Иначе находим последний элемент и присоединяем новый узел
    else {
        struct Node *current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_node;
    }
}

// 3. Функция освобождения памяти (free_list)
void free_list(struct Node *head) {
    struct Node *current = head;
    while (current != NULL) {
        struct Node *next_node = current->next; // запоминаем следующий
        free(current->data);                    // сначала освобождаем строку
        free(current);                          // затем освобождаем сам узел
        current = next_node;                    // переходим к следующему
    }
}

// 4. Основная логика (main)
int main() {
    struct Node *head = NULL;
    char buffer[1024];

    while (1) {
        // Читаем ввод
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            break;
        }

        // Удаляем символ новой строки '\n' из конца буфера, если он есть
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }

        // Проверяем первый символ: если это '.', прерываем цикл
        if (buffer[0] == '.') {
            break;
        }

        // Если строка не пустая, вызываем append
        if (strlen(buffer) > 0) {
            append(&head, buffer);
        }
    }

    // 5. Вывод результатов
    struct Node *current = head;
    while (current != NULL) {
        printf("%s\n", current->data);
        current = current->next;
    }

    // Вызываем free_list перед завершением программы
    free_list(head);

    return 0;
}

