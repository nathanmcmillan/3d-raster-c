/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "list.h"

list *list_init() {
    return safe_calloc(1, sizeof(list));
}

void list_add(list *this, void *item) {
    if (this->item == NULL) {
        this->item = item;
        return;
    }
    list *link = this->next;
    list *node = safe_calloc(1, sizeof(list));
    node->item = item;
    node->next = link;
    this->next = node;
}

void list_push(list *this, void *item) {
    if (this->item == NULL) {
        this->item = item;
        return;
    }
    list *tail = safe_calloc(1, sizeof(list));
    tail->item = item;
    while (true) {
        list *next = this->next;
        if (next == NULL) {
            this->next = tail;
            return;
        }
        this = next;
    }
}

void list_remove(list *this, void *item) {
    if (this->item == NULL) {
        return;
    }
    do {
        list *next = this->next;
        if (this->item == item) {
            if (next == NULL) {
                this->item = NULL;
            } else {
                this->item = next->item;
                this->next = next->next;
                free(next);
            }
            return;
        }
        this = next;
    } while (this);
}

void list_remove_index(list *this, unsigned int index) {
    if (this->item == NULL) {
        return;
    }
    unsigned int i = 0;
    do {
        list *next = this->next;
        if (i == index) {
            if (next == NULL) {
                this->item = NULL;
            } else {
                this->item = next->item;
                this->next = next->next;
                free(next);
            }
            return;
        }
        this = next;
    } while (this);
}

void list_insert(list *this, unsigned int index, void *item) {
    if (this->item == NULL) {
        this->item = item;
        return;
    }
    list *tail = safe_calloc(1, sizeof(list));
    tail->item = item;
    unsigned int i = 0;
    while (true) {
        list *next = this->next;
        if (next == NULL or i == index) {
            tail->next = next;
            this->next = tail;
            return;
        }
        i++;
        this = next;
    }
}

void list_insert_sort(list *this, int (*compare)(void *, void *), void *item) {
    if (this->item == NULL) {
        this->item = item;
        return;
    }
    list *tail = safe_calloc(1, sizeof(list));
    tail->item = item;
    while (true) {
        list *next = this->next;
        if (next == NULL or compare(item, this->item) <= 0) {
            tail->next = next;
            this->next = tail;
            return;
        }
        this = next;
    }
}

void *list_find(list *this, bool(find)(void *x, void *), void *has) {
    if (this->item == NULL) {
        return NULL;
    }
    while (true) {
        void *item = this->item;
        if (find(item, has)) {
            return item;
        }
        this = this->next;
        if (!this) {
            return NULL;
        }
    }
}

void *list_get(list *this, unsigned int index) {
    if (this->item == NULL) {
        return NULL;
    }
    for (unsigned int i = 0; i < index; i++) {
        this = this->next;
        if (!this) {
            return NULL;
        }
    }
    return this->item;
}

bool list_is_empty(list *this) {
    return this->item == NULL;
}

bool list_not_empty(list *this) {
    return this->item != NULL;
}

unsigned int list_size(list *this) {
    if (this->item == NULL) {
        return 0;
    }
    unsigned int size = 0;
    while (true) {
        size++;
        this = this->next;
        if (!this) {
            return size;
        }
    }
}

void **list_to_array(list *this) {
    unsigned int size = list_size(this);
    if (size == 0) {
        return NULL;
    }
    void **array = safe_malloc(size * sizeof(void *));
    for (unsigned int i = 0; i < size; i++) {
        array[i] = this->item;
        this = this->next;
    }
    return array;
}

void list_free(list *this) {
    do {
        list *node = this->next;
        free(this);
        this = node;
    } while (this);
}
