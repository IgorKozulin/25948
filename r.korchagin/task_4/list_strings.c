#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node {
    char *data;
    struct Node *next;
} Node;

void append(Node **head, const char *str) {
    Node *new_node = (Node *)malloc(sizeof(Node));
    if (new_node == NULL) {
        perror("malloc node");
        exit(EXIT_FAILURE);
    }

    new_node->data = (char *)malloc(strlen(str) + 1);
    if (new_node->data == NULL) {
        perror("malloc data");
        free(new_node);
        exit(EXIT_FAILURE);
    }
    strcpy(new_node->data, str);
    new_node->next = NULL;

    if (*head == NULL) {
        *head = new_node;
    } else {
        Node *cur = *head;
        while (cur->next != NULL) {
            cur = cur->next;
        }
        cur->next = new_node;
    }
}

void free_list(Node *head) {
    Node *cur = head;
    while (cur != NULL) {
        Node *next = cur->next;
        free(cur->data);
        free(cur);
        cur = next;
    }
}

int main(void) {
    Node *head = NULL;
    char buffer[1024];

    while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        size_t len = strlen(buffer);

        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }

        if (buffer[0] == '.') {
            break;
        }

        append(&head, buffer);   /* пустые строки тоже сохраняются */
    }

    Node *cur = head;
    while (cur != NULL) {
        printf("%s\n", cur->data);
        cur = cur->next;
    }

    free_list(head);
    return 0;
}
