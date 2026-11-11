#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node { 
    char *data; 
    struct Node *next; 
} Node;

static void append(Node **head, const char *str) {
    Node *n = malloc(sizeof *n);
    if (!n) {
        perror("malloc");
        exit(1);
    }

    n->data = malloc(strlen(str) + 1);
    if (!n->data){
        perror("malloc");
        exit(1);
    }

    strcpy(n->data, str);
    n->next = NULL;

    if (!*head) {
        *head = n;
        return;
    }

    Node *sb_node = *head;
    while (sb_node->next){
        sb_node = sb_node->next;
    }
    sb_node->next = n;
}

static void free_list(Node *head) {
    while (head) {
        struct Node *n = head->next;
        free(head->data);
        free(head);
        head = n;
    }
}

int main(void) {
    char buf[1024];
    Node *head = NULL;

    while (1) {
        if (!fgets(buf, sizeof buf, stdin)) {
            break;
        }

        size_t len = strlen(buf);
        if (len) {
            if (buf[len-1] == '\n' || buf[len-1] == '\r') {
                buf[len-1] = '\0';
            }
        }

        if (buf[0] == '.'){
            break;
        }

        append(&head, buf);
    }

    Node *p = head;
    while (p) {
        printf("%s\n", p->data);
        p = p->next;
    }

    free_list(head);

    return 0;
}