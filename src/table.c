#include "table.h"

static const float LOAD_FACTOR = 0.80;

static const unsigned int INITIAL_BINS = 1 << 3;

static const unsigned int MAXIMUM_BINS = 1 << 30;

bool table_string_equal(void *a, void *b) {
    return strcmp(a, b) == 0;
}

unsigned long table_string_hashcode(void *key) {
    char *str_key = key;
    int length = strlen(str_key);
    unsigned long hash = 0;
    for (int i = 0; i < length; i++) {
        hash = 31 * hash + (unsigned long)str_key[i];
    }
    return hash;
}

bool table_address_equal(void *a, void *b) {
    return a == b;
}

unsigned long table_address_hashcode(void *key) {
    return (unsigned long)key;
}

table *create_table(bool (*equals_fn)(void *, void *), unsigned long (*hashcode_fn)(void *)) {
    table *this = safe_malloc(sizeof(table));
    this->equals_fn = equals_fn;
    this->hashcode_fn = hashcode_fn;
    this->size = 0;
    this->bins = INITIAL_BINS;
    this->items = safe_calloc(this->bins, sizeof(table_item *));
    return this;
}

static unsigned int get_bin(table *this, unsigned long hash) {
    return (this->bins - 1) & hash;
}

static unsigned long hash_mix(unsigned long hash) {
    return hash ^ (hash >> 16);
}

static void resize(table *this) {

    unsigned int old_bins = this->bins;
    unsigned int bins = old_bins << 1;

    if (bins > MAXIMUM_BINS) {
        return;
    }

    table_item **old_items = this->items;
    table_item **items = safe_calloc(bins, sizeof(table_item *));

    for (unsigned int i = 0; i < old_bins; i++) {
        table_item *item = old_items[i];
        if (item == NULL) {
            continue;
        }
        if (item->next == NULL) {
            items[(bins - 1) & item->hash] = item;
        } else {
            table_item *low_head = NULL;
            table_item *low_tail = NULL;
            table_item *high_head = NULL;
            table_item *high_tail = NULL;
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

void table_put(table *this, void *key, void *value) {
    unsigned long hash = hash_mix((*this->hashcode_fn)(key));
    unsigned int bin = get_bin(this, hash);
    table_item *item = this->items[bin];
    table_item *previous = NULL;
    while (item != NULL) {
        if (hash == item->hash and this->equals_fn(key, item->key)) {
            item->value = value;
            return;
        }
        previous = item;
        item = item->next;
    }
    item = safe_malloc(sizeof(table_item));
    item->hash = hash;
    item->key = key;
    item->value = value;
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

void *table_get(table *this, void *key) {
    unsigned long hash = hash_mix((*this->hashcode_fn)(key));
    unsigned int bin = get_bin(this, hash);
    table_item *item = this->items[bin];
    while (item != NULL) {
        if (hash == item->hash and this->equals_fn(key, item->key)) {
            return item->value;
        }
        item = item->next;
    }
    return NULL;
}

bool table_has(table *this, void *key) {
    return table_get(this, key) != NULL;
}

void *table_remove(table *this, void *key) {
    unsigned long hash = hash_mix((*this->hashcode_fn)(key));
    unsigned int bin = get_bin(this, hash);
    table_item *item = this->items[bin];
    table_item *previous = NULL;
    while (item != NULL) {
        if (hash == item->hash and this->equals_fn(key, item->key)) {
            if (previous == NULL) {
                this->items[bin] = item->next;
            } else {
                previous->next = item->next;
            }
            this->size -= 1;
            return item->value;
        }
        previous = item;
        item = item->next;
    }
    return NULL;
}

void table_clear(table *this) {
    unsigned int bins = this->bins;
    for (unsigned int i = 0; i < bins; i++) {
        table_item *item = this->items[i];
        while (item != NULL) {
            table_item *next = item->next;
            free(item);
            item = next;
        }
        this->items[i] = NULL;
    }
    this->size = 0;
}

bool table_is_empty(table *this) {
    return this->size == 0;
}

bool table_not_empty(table *this) {
    return this->size != 0;
}

unsigned int table_size(table *this) {
    return this->size;
}

void release_table(table *this) {
    table_clear(this);
    free(this->items);
}

void delete_table(table *this) {
    release_table(this);
    free(this);
}

table_iterator create_table_iterator(table *this) {
    table_iterator iter;
    iter.pointer = this;
    if (this->size == 0) {
        iter.bin = 0;
        iter.item = NULL;
    } else {
        unsigned int bins = this->bins;
        for (unsigned int i = 0; i < bins; i++) {
            table_item *start = this->items[i];
            if (start) {
                iter.bin = i;
                iter.item = start;
                break;
            }
        }
    }
    return iter;
}

bool table_iterator_has_next(table_iterator *iter) {
    return iter->item;
}

table_pair table_iterator_next(table_iterator *iter) {
    table_item *item = iter->item;
    if (item == NULL) {
        return (table_pair){NULL, NULL};
    }
    table_pair pair = {item->key, item->value};
    item = item->next;
    if (item == NULL) {
        unsigned int bin = iter->bin;
        unsigned int stop = iter->pointer->bins;
        for (bin = bin + 1; bin < stop; bin++) {
            table_item *start = iter->pointer->items[bin];
            if (start) {
                item = start;
                break;
            }
        }
        iter->bin = bin;
    }
    iter->item = item;
    return pair;
}
