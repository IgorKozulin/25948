#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 1. Описание структуры узла
typedef struct Node {
    char *data;
    struct Node *next;
} Node;

// 2. Функция добавления узла в конец списка
void append(Node **head, const char *str) {
    // Выделяем память под новый узел
    Node *new_node = (Node *)malloc(sizeof(Node));
    if (new_node == NULL) {
        perror("Ошибка выделения памяти для узла");
        exit(EXIT_FAILURE);
    }

    // Выделяем память под строку (длина + 1 для нуль-терминатора '\0')
    new_node->data = (char *)malloc(strlen(str) + 1);
    if (new_node->data == NULL) {
        perror("Ошибка выделения памяти для строки");
        free(new_node);
        exit(EXIT_FAILURE);
    }
    
    // Копируем строку в выделенную память
    strcpy(new_node->data, str);
    new_node->next = NULL;

    // Если список пуст, новый узел становится головой
    if (*head == NULL) {
        *head = new_node;
        return;
    }

    // Иначе проходим до конца списка и добавляем узел
    Node *current = *head;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = new_node;
}

// 3. Функция освобождения памяти
void free_list(Node *head) {
    Node *current = head;
    while (current != NULL) {
        Node *temp = current;
        current = current->next; // Сохраняем указатель на следующий узел
        
        free(temp->data);        // Сначала освобождаем память строки
        free(temp);              // Затем освобождаем память самого узла
    }
}

void print_list(Node *head) {
    // Если список пуст, выводить нечего
    if (head == NULL) {
        printf("list is empty");
        return;
    }

    // 1. Находим и выводим последний элемент
    Node *current = head;
    while (current->next != NULL) {
        current = current->next; // Шагаем строго по указателям ->next
    }
    printf("%s\n", current->data);

    // 2. Выводим весь список с самого начала
    current = head;
    current = current->next;
    while (current != NULL && current->next != NULL) {
        printf("%s\n", current->data);
        current = current->next;
    }

    // 3. Выводим первый элемент
    // Голова списка (head) всегда указывает на первый элемент
    printf("%s\n", head->data);
}
void swap_first_and_last_data(Node *head) {
    // Если список пуст или в нем только 1 элемент - ничего не делаем
    if (head == NULL || head->next == NULL) {
        return;
    }

    // Находим последний элемент
    Node *last = head;
    while (last->next != NULL) {
        last = last->next;
    }

    // Меняем местами указатели на строки
    char *temp = head->data;
    head->data = last->data;
    last->data = temp;
}
// 4. Основная логика
int main() {
    Node *head = NULL;
    char buffer[1024];

    printf("Введите строки (введите '.' в начале строки для завершения):\n");

    while (1) {
        // Читаем строку из стандартного ввода
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            break; // Выход при EOF или ошибке чтения
        }

        // Если первый символ '.', завершаем ввод
        if (buffer[0] == '.') {
            break;
        }

        // Удаляем символ переноса строки '\n' (если он считался)
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }

        // Добавляем очищенную строку в список
        append(&head, buffer);
    }

    // 5. Вывод результатов
    printf("\n--- Содержимое списка ---\n");
    print_list(head);

    // Освобождение памяти перед выходом
    free_list(head);

    return 0;
}
