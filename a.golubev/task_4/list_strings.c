#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 1024

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

    new_node->data = malloc(strlen(str) + 1);
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

void print_list(struct Node *head)
{
    while (head != NULL) {
        printf("%s\n", head->data);
        head = head->next;
    }
}

void free_list(struct Node *head)
{
    while (head != NULL) {
        struct Node *tmp = head;
        head = head->next;
        free(tmp->data);
        free(tmp);
    }
}

int main(void)
{
    char buffer[BUFFER_SIZE];
    struct Node *head = NULL;

    while (1) {
        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL)
            break;

        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n')
            buffer[len - 1] = '\0';

        if (buffer[0] == '.')
            break;

        if (buffer[0] == '\0')
            continue;

        append(&head, buffer);
    }

    print_list(head);
    free_list(head);

    return 0;
}
