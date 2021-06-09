/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "set.h"

static const float LOAD_FACTOR = 0.80f;

static const unsigned int INITIAL_BINS = 1 << 3;

static const unsigned int MAXIMUM_BINS = 1 << 30;

bool set_string_equal(void *a, void *b) {
    return strcmp(a, b) == 0;
}

unsigned long set_string_hashcode(void *key) {
    char *str_key = key;
    int length = strlen(str_key);
    unsigned long hash = 0;
    for (int i = 0; i < length; i++) {
        hash = 31 * hash + (unsigned long)str_key[i];
    }
    return hash;
}

bool set_address_equal(void *a, void *b) {
    return a == b;
}

unsigned long set_address_hashcode(void *key) {
    return (unsigned long)key;
}

set *new_set(bool (*equals_fn)(void *, void *), unsigned long (*hashcode_fn)(void *)) {
    set *this = safe_malloc(sizeof(set));
    this->equals_fn = equals_fn;
    this->hashcode_fn = hashcode_fn;
    this->size = 0;
    this->bins = INITIAL_BINS;
    this->items = safe_calloc(this->bins, sizeof(set_item *));
    return this;
}

static unsigned int get_bin(set *this, unsigned long hash) {
    return (this->bins - 1) & hash;
}

static unsigned long hash_mix(unsigned long hash) {
    return hash ^ (hash >> 16);
}

static void resize(set *this) {

    unsigned int old_bins = this->bins;
    unsigned int bins = old_bins << 1;

    if (bins > MAXIMUM_BINS) {
        return;
    }

    set_item **old_items = this->items;
    set_item **items = safe_calloc(bins, sizeof(set_item *));

    for (unsigned int i = 0; i < old_bins; i++) {
        set_item *item = old_items[i];
        if (item == NULL) {
            continue;
        }
        if (item->next == NULL) {
            items[(bins - 1) & item->hash] = item;
        } else {
            set_item *low_head = NULL;
            set_item *low_tail = NULL;
            set_item *high_head = NULL;
            set_item *high_tail = NULL;
            do {
                if ((old_bins & item->hash) == 0) {
                    if (low_tail == NULL) {
                        low_head = item;
                    } else {
                        low_tail->next = item;
                    }
                    low_tail = item;
                } else {
                    if (high_tail == NULL) {
                        high_head = item;
                    } else {
                        high_tail->next = item;
                    }
                    high_tail = item;
                }
                item = item->next;
            } while (item != NULL);

            if (low_tail != NULL) {
                low_tail->next = NULL;
                items[i] = low_head;
            }

            if (high_tail != NULL) {
                high_tail->next = NULL;
                items[i + old_bins] = high_head;
            }
        }
    }

    free(old_items);

    this->bins = bins;
    this->items = items;
}

void set_add(set *this, void *key) {
    unsigned long hash = hash_mix((*this->hashcode_fn)(key));
    unsigned int bin = get_bin(this, hash);
    set_item *item = this->items[bin];
    set_item *previous = NULL;
    while (item != NULL) {
        if (hash == item->hash and this->equals_fn(key, item->key)) {
            return;
        }
        previous = item;
        item = item->next;
    }
    item = safe_malloc(sizeof(set_item));
    item->hash = hash;
    item->key = key;
    item->next = NULL;
    if (previous == NULL) {
        this->items[bin] = item;
    } else {
        previous->next = item;
    }
    this->size++;
    if (this->size >= this->bins * LOAD_FACTOR) {
        resize(this);
    }
}

bool set_has(set *this, void *key) {
    unsigned long hash = hash_mix((*this->hashcode_fn)(key));
    unsigned int bin = get_bin(this, hash);
    set_item *item = this->items[bin];
    while (item != NULL) {
        if (hash == item->hash and this->equals_fn(key, item->key)) {
            return true;
        }
        item = item->next;
    }
    return false;
}

void set_remove(set *this, void *key) {
    unsigned long hash = hash_mix((*this->hashcode_fn)(key));
    unsigned int bin = get_bin(this, hash);
    set_item *item = this->items[bin];
    set_item *previous = NULL;
    while (item != NULL) {
        if (hash == item->hash and this->equals_fn(key, item->key)) {
            if (previous == NULL) {
                this->items[bin] = item->next;
            } else {
                previous->next = item->next;
            }
            this->size -= 1;
            return;
        }
        previous = item;
        item = item->next;
    }
}

void set_clear(set *this) {
    unsigned int bins = this->bins;
    for (unsigned int i = 0; i < bins; i++) {
        set_item *item = this->items[i];
        while (item != NULL) {
            set_item *next = item->next;
            free(item);
            item = next;
        }
        this->items[i] = NULL;
    }
    this->size = 0;
}

bool set_is_empty(set *this) {
    return this->size == 0;
}

bool set_not_empty(set *this) {
    return this->size != 0;
}

unsigned int set_size(set *this) {
    return this->size;
}

void set_release(set *this) {
    set_clear(this);
    free(this->items);
}

void set_delete(set *this) {
    set_release(this);
    free(this);
}

set_iterator new_set_iterator(set *this) {
    set_iterator iter;
    iter.pointer = this;
    if (this->size == 0) {
        iter.bin = 0;
        iter.item = NULL;
    } else {
        unsigned int bins = this->bins;
        for (unsigned int i = 0; i < bins; i++) {
            set_item *start = this->items[i];
            if (start) {
                iter.bin = i;
                iter.item = start;
                break;
            }
        }
    }
    return iter;
}

bool set_iterator_has_next(set_iterator *iter) {
    return iter->item;
}

void *set_iterator_next(set_iterator *iter) {
    set_item *item = iter->item;
    if (item == NULL) {
        return NULL;
    }
    void *key = item->key;
    item = item->next;
    if (item == NULL) {
        unsigned int bin = iter->bin;
        unsigned int stop = iter->pointer->bins;
        for (bin = bin + 1; bin < stop; bin++) {
            set_item *start = iter->pointer->items[bin];
            if (start) {
                item = start;
                break;
            }
        }
        iter->bin = bin;
    }
    iter->item = item;
    return key;
}
