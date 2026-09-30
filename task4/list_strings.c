#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Node {
    char *data;
    struct Node *next;
};

void append(struct Node **head, const char *str)
{
    struct Node *new_node = malloc(sizeof(struct Node));
    if (new_node == NULL) {
        perror("malloc node");
        exit(EXIT_FAILURE);
    }

    size_t len = strlen(str);
    new_node->data = malloc(len + 1);
    if (new_node->data == NULL) {
        perror("malloc data");
        free(new_node);
        exit(EXIT_FAILURE);
    }
    strcpy(new_node->data, str);
    new_node->next = NULL;

    if (*head == NULL) {
        *head = new_node;
        return;
    }

    struct Node *cur = *head;
    while (cur->next != NULL) {
        cur = cur->next;
    }
    cur->next = new_node;
}

void print_list(const struct Node *head)
{
    while (head != NULL) {
        printf("%s\n", head->data);
        head = head->next;
    }
}

void free_list(struct Node *head)
{
    while (head != NULL) {
        struct Node *next = head->next;
        free(head->data);
        free(head);
        head = next;
    }
}

int main(void)
{
    char buffer[1024];
    struct Node *head = NULL;

    printf("Вводите строки. Для завершения введите строку, начинающуюся с '.'\n");

    while (1) {
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            break; /* EOF */
        }

        if (buffer[0] == '.') {
            break;
        }

        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
            len--;
        }

        if (len > 0) {
            append(&head, buffer);
        }
    }

    printf("\nВы ввели:\n");
    print_list(head);

    free_list(head);
    return 0;
}
