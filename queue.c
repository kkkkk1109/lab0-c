#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */


/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *new_list = malloc(sizeof(struct list_head));
    if (!new_list)
        return NULL;
    INIT_LIST_HEAD(new_list);
    return new_list;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    if (!head)
        return;
    element_t *node, *safe;
    list_for_each_entry_safe (node, safe, head, list) {
        list_del(&node->list);
        q_release_element(node);
    }
    free(head);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *new_ele = malloc(sizeof(element_t));
    if (!new_ele)
        return false;
    new_ele->value = strdup(s);
    if (!new_ele->value) {
        q_release_element(new_ele);
        return false;
    }
    list_add(&new_ele->list, head);
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *new_ele = malloc(sizeof(element_t));
    if (!new_ele)
        return false;
    new_ele->value = strdup(s);
    if (!new_ele->value) {
        q_release_element(new_ele);
        return false;
    }

    list_add_tail(&new_ele->list, head);
    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head)) {
        return NULL;
    }
    element_t *rv_node = list_first_entry(head, element_t, list);
    list_del(&rv_node->list);
    if (sp) {
        memcpy(sp, rv_node->value, bufsize);
        sp[bufsize - 1] = '\0';
    }
    return rv_node;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head)) {
        return NULL;
    }
    element_t *rv_node = list_last_entry(head, element_t, list);
    list_del(&rv_node->list);
    if (sp) {
        memcpy(sp, rv_node->value, bufsize);
        sp[bufsize - 1] = '\0';
    }
    return rv_node;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;

    int len = 0;
    struct list_head *li;

    list_for_each (li, head)
        len++;
    return len;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    if (!head || list_empty(head))
        return false;
    struct list_head *ln, *rn;
    ln = head->prev;
    rn = head->next;
    while (1) {
        if ((ln == rn) | (rn->next == ln))
            break;
        ln = ln->prev;
        rn = rn->next;
    }
    element_t *rv_node = list_entry(rn, element_t, list);
    list_del(rn);
    q_release_element(rv_node);
    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return false;
    element_t *cur, *nxt;
    bool dup = false;
    list_for_each_entry_safe (cur, nxt, head, list) {
        if (strcmp(cur->value, nxt->value) == 0 && &nxt->list != head) {
            list_del(&cur->list);
            q_release_element(cur);
            dup = true;
        } else if (dup) {
            list_del(&cur->list);
            q_release_element(cur);
            dup = false;
        }
    }
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    int cnt = 0;
    struct list_head *fir, *mid, *sec;
    fir = head;
    list_for_each_safe (mid, sec, head) {
        if (cnt == 0 && sec != head && mid != head) {
            fir->next = sec;
            sec->prev = fir;
            mid->prev = sec;
            cnt++;
            fir = mid;
        } else if (cnt == 1) {
            mid->next = fir;
            fir->next = sec;
            sec->prev = fir;
            cnt--;
        }
    }
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head))
        return;

    struct list_head *node, *safe;
    list_for_each_safe (node, safe, head) {
        node->next = node->prev;
        node->prev = safe;
    }
    node->next = node->prev;
    node->prev = safe;
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
}

/* Sort elements of queue in ascending/descending order */

struct list_head *merge_two_nodes(struct list_head *left,
                                  struct list_head *right)
{
    struct list_head *m_head = NULL, **indirect = &m_head, **iter = NULL;
    for (; left && right; *iter = (*iter)->next) {
        iter = strcmp(list_entry(left, element_t, list)->value,
                      list_entry(right, element_t, list)->value) >= 0
                   ? &right
                   : &left;
        *indirect = *iter;
        indirect = &(*indirect)->next;
    }
    *indirect = (struct list_head *) ((u_int64_t) left | (u_int64_t) right);
    return m_head;
}

struct list_head *merge_divide(struct list_head *head)
{
    if (!head || !head->next)
        return head;
    struct list_head *fast = head, *slow = head, *mid;

    for (; fast && fast->next; fast = fast->next->next)
        slow = slow->next;
    mid = slow;
    mid->prev->next = NULL;
    struct list_head *left = merge_divide(head);
    struct list_head *right = merge_divide(mid);
    return merge_two_nodes(left, right);
}
void q_sort(struct list_head *head, bool descend)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;
    head->prev->next = NULL;
    head->next = merge_divide(head->next);
    struct list_head *fir = head, *sec = head->next;
    for (; sec != NULL; sec = sec->next) {
        sec->prev = fir;
        fir = sec;
    }
    fir->next = head;
    head->prev = fir;
    if (descend)
        q_reverse(head);
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    return 0;
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    return 0;
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    return 0;
}
