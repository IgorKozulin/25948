#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Node {
    char *data;
    struct Node *next;
};

void append(struct Node **head, const char *s){
    struct Node *node = malloc(sizeof(struct Node));
    if (node == NULL) {
        perror("malloc(node)");
        exit(EXIT_FAILURE);
    }

    node->data = malloc(strlen(s) + 1);
    if (node->data == NULL) {
        perror("malloc(data)");
        free(node);
        exit(EXIT_FAILURE);
    }
    strcpy(node->data, s);
    node->next = NULL;

    if (*head == NULL) {
        *head = node;
        return;
    }

    struct Node *cur = *head;
    while (cur->next != NULL) {
        cur = cur->next;
    }
    cur->next = node;
}
//
//static void reverse_list(struct Node **head){
//    struct Node *prev = NULL;      
//    struct Node *cur  = *head;     
//    struct Node *next = NULL;      

//    while (cur != NULL) {
//        next = cur->next;          
//        cur->next = prev;          
//        prev = cur;                
//        cur = next;                
//    }

//    *head = prev;                 
//}

static void print_list(const struct Node *head)
{
    const struct Node *cur = head;
    while (cur != NULL) {
        printf("%s ->", cur->data);
        cur = cur->next;
        if(cur==NULL) printf(" NULL\n");
    }
}

static void print_fr_ls(const struct Node *head){
    if (head == NULL) {
        printf("(empty list)\n");
        return;
    }
    const struct Node *cur = head;
    printf("%s ", cur->data);
    while (cur->next != NULL) {
        cur = cur->next;
    }
    printf("%s ", cur->data);
}

static void free_list(struct Node *head)
{
    while (head != NULL) {
        struct Node *next = head->next;
        free(head->data);
        free(head);
        head = next;
    }
}

int main(void){
    char buffer[1024];
    struct Node *head = NULL;

    printf("Enter lines (a line starting with '.' to finish):\n");

    while (1) {
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            break;
        }

        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }

        if (buffer[0] == '.') {
            break;
        }

        append(&head, buffer);
    }

    print_list(head);

    //reverse_list(&head);
    //print_list(head);
    print_fr_ls(head);
    free_list(head);
    return 0;
}
