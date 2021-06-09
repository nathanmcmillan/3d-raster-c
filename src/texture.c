#include "texture.h"

Texture *new_texture() {
    return safe_calloc(sizeof(Texture), 1);
}
