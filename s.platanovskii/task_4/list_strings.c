#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Node {
    char *data;
    struct Node *next;
};

static void append(struct Node **head, const char *str) {
    struct Node *new_node = malloc(sizeof(struct Node));
    if (new_node == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    new_node->data = malloc(strlen(str) + 1);
    if (new_node->data == NULL) {
        perror("malloc");
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
    while (cur->next != NULL)
        cur = cur->next;
    cur->next = new_node;
}

static void print_list(struct Node *head) {
    for (struct Node *cur = head; cur != NULL; cur = cur->next)
        printf("%s\n", cur->data);
}

static void free_list(struct Node *head) {
    while (head != NULL) {
        struct Node *next = head->next;
        free(head->data);
        free(head);
        head = next;
    }
}

int main(void) {
    char buffer[1024];
    struct Node *head = NULL;

    while (1) {
        if (fgets(buffer, sizeof(buffer), stdin) == NULL)
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
