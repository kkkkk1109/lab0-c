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
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
// list_sort
typedef int (*list_cmp_func_t)(const struct list_head *,
                               const struct list_head *);

static int cmp(const struct list_head *a, const struct list_head *b)
{
    element_t *ele_a = list_entry(a, element_t, list);
    element_t *ele_b = list_entry(b, element_t, list);

    return (strcmp(ele_a->value, ele_b->value));
}

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
        strncpy(sp, rv_node->value, bufsize);
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
        strncpy(sp, rv_node->value, bufsize);
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
    if (!head)
        return false;
    if (list_empty(head) || list_is_singular(head))
        return true;
    element_t *node, *safe;
    bool dup = 0;
    list_for_each_entry_safe (node, safe, head, list) {
        if (node->list.next != head && !strcmp(safe->value, node->value)) {
            list_del(&node->list);
            q_release_element(node);
            dup = 1;
        } else if (dup) {
            list_del(&node->list);
            q_release_element(node);
            dup = 0;
        }
    }
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
    if (!head || list_empty(head))
        return;
    int count = 0;
    struct list_head *cur, *safe, *record;
    record = head;
    list_for_each_safe (cur, safe, head) {
        count++;
        if (count != k) {
            continue;
        }
        LIST_HEAD(new_head);
        list_cut_position(&new_head, record, cur);
        q_reverse(&new_head);
        list_splice(&new_head, record);
        record = safe->prev;
        count = 0;
    }
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
// void q_sort(struct list_head *head, bool descend)
// {
//     if (!head || list_empty(head) || list_is_singular(head))
//         return;
//     head->prev->next = NULL;
//     head->next = merge_divide(head->next);
//     struct list_head *fir = head, *sec = head->next;
//     for (; sec != NULL; sec = sec->next) {
//         sec->prev = fir;
//         fir = sec;
//     }
//     fir->next = head;
//     head->prev = fir;
//     if (descend)
//         q_reverse(head);
// }

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;
    if (list_is_singular(head))
        return 1;

    struct list_head *cur, *min;
    min = head->prev;
    cur = min->prev;
    while (cur != head) {
        if (strcmp(list_entry(min, element_t, list)->value,
                   list_entry(cur, element_t, list)->value) < 0) {
            list_del(cur);
            q_release_element(list_entry(cur, element_t, list));
            cur = min->prev;

        } else if (strcmp(list_entry(min, element_t, list)->value,
                          list_entry(cur, element_t, list)->value) >= 0) {
            min = cur;
            cur = cur->prev;
        }
    }
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    return q_size(head);
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;
    if (list_is_singular(head))
        return 1;
    struct list_head *cur, *big;
    big = head->prev;
    cur = big->prev;
    while (cur != head) {
        if (strcmp(list_entry(big, element_t, list)->value,
                   list_entry(cur, element_t, list)->value) > 0) {
            list_del(cur);
            q_release_element(list_entry(cur, element_t, list));
            cur = big->prev;


        } else if (strcmp(list_entry(big, element_t, list)->value,
                          list_entry(cur, element_t, list)->value) <= 0) {
            big = cur;
            cur = cur->prev;
        }
    }
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    return q_size(head);
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/

    if (!head || list_empty(head))
        return 0;

    int total_size = 0;
    struct list_head *first = list_entry(head->next, queue_contex_t, chain)->q;
    total_size += q_size(first);
    first->prev->next = NULL;
    struct list_head *node = head->next->next;

    while (node != head) {
        struct list_head *add_q = list_entry(node, queue_contex_t, chain)->q;
        total_size += q_size(add_q);
        add_q->prev->next = NULL;
        first->next = merge_two_nodes(first->next, add_q->next);
        INIT_LIST_HEAD(add_q);
        node = node->next;
    }

    struct list_head *before = first, *after = first->next;
    for (; after != NULL; after = after->next) {
        after->prev = before;
        before = after;
    }
    before->next = first;
    first->prev = before;

    return total_size;
}

void q_shuffle(struct list_head *head)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;
    struct list_head *new = head->prev, *tmp = new->prev;
    int size = q_size(head);
    for (; new != head &&size; new = tmp, tmp = tmp->prev, size--) {
        struct list_head *old = head->next;
        int random = rand() % (size + 1);
        for (; random > 0; random--)
            old = old->next;

        if (old == new)
            continue;
        // swap
        struct list_head *prev1 = old->prev;
        struct list_head *prev2 = new->prev;
        if (old->next == new) {
            list_del(new);
            list_add(new, prev1);
        } else if (new->next == old) {
            list_del(old);
            list_add(old, prev2);
        } else {
            list_del(new);
            list_del(old);
            list_add(new, prev1);
            list_add(old, prev2);
        }
    }
}
static struct list_head *merge(list_cmp_func_t cmp,
                               struct list_head *a,
                               struct list_head *b)
{
    struct list_head *head = NULL, **tail = &head;

    for (;;) {
        /* if equal, take 'a' -- important for sort stability */
        if (cmp(a, b) <= 0) {
            *tail = a;
            tail = &a->next;
            a = a->next;
            if (!a) {
                *tail = b;
                break;
            }
        } else {
            *tail = b;
            tail = &b->next;
            b = b->next;
            if (!b) {
                *tail = a;
                break;
            }
        }
    }
    return head;
}

static void merge_final(list_cmp_func_t cmp,
                        struct list_head *head,
                        struct list_head *a,
                        struct list_head *b)
{
    struct list_head *tail = head;
    unsigned char count = 0;

    for (;;) {
        /* if equal, take 'a' -- important for sort stability */
        if (cmp(a, b) <= 0) {
            tail->next = a;
            a->prev = tail;
            tail = a;
            a = a->next;
            if (!a)
                break;
        } else {
            tail->next = b;
            b->prev = tail;
            tail = b;
            b = b->next;
            if (!b) {
                b = a;
                break;
            }
        }
    }

    /* Finish linking remainder of list b on to tail */
    tail->next = b;
    do {
        /*
         * If the merge is highly unbalanced (e.g. the input is
         * already sorted), this loop may run many iterations.
         * Continue callbacks to the client even though no
         * element comparison is needed, so the client's cmp()
         * routine can invoke cond_resched() periodically.
         */
        if (unlikely(!++count))
            cmp(b, b);
        b->prev = tail;
        tail = b;
        b = b->next;
    } while (b);

    /* And the final links to make a circular doubly-linked list */
    tail->next = head;
    head->prev = tail;
}
void list_sort(struct list_head *head, list_cmp_func_t cmp)
{
    struct list_head *list = head->next, *pending = NULL;
    size_t count = 0; /* Count of pending */

    if (list == head->prev) /* Zero or one elements */
        return;

    /* Convert to a null-terminated singly-linked list. */
    head->prev->next = NULL;

    /*
     * Data structure invariants:
     * - All lists are singly linked and null-terminated; prev
     *   pointers are not maintained.
     * - pending is a prev-linked "list of lists" of sorted
     *   sublists awaiting further merging.
     * - Each of the sorted sublists is power-of-two in size.
     * - Sublists are sorted by size and age, smallest & newest at front.
     * - There are zero to two sublists of each size.
     * - A pair of pending sublists are merged as soon as the number
     *   of following pending elements equals their size (i.e.
     *   each time count reaches an odd multiple of that size).
     *   That ensures each later final merge will be at worst 2:1.
     * - Each round consists of:
     *   - Merging the two sublists selected by the highest bit
     *     which flips when count is incremented, and
     *   - Adding an element from the input as a size-1 sublist.
     */
    do {
        size_t bits;
        struct list_head **tail = &pending;

        /* Find the least-significant clear bit in count */
        for (bits = count; bits & 1; bits >>= 1)
            tail = &(*tail)->prev;
        /* Do the indicated merge */
        if (likely(bits)) {
            struct list_head *a = *tail, *b = a->prev;

            a = merge(cmp, b, a);
            /* Install the merged result in place of the inputs */
            a->prev = b->prev;
            *tail = a;
        }

        /* Move one element from input list to pending */
        list->prev = pending;
        pending = list;
        list = list->next;
        pending->next = NULL;
        count++;
    } while (list);

    /* End of input; merge together all the pending lists. */
    list = pending;
    pending = pending->prev;
    for (;;) {
        struct list_head *next = pending->prev;

        if (!next)
            break;
        list = merge(cmp, pending, list);
        pending = next;
    }
    /* The final merge, rebuilding prev links */
    merge_final(cmp, head, pending, list);
}

void q_sort(struct list_head *head, bool descend)
{
    list_sort(head, cmp);
    if (descend)
        q_reverse(head);
}