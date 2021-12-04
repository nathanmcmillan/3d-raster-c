/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef RESOURCES_H
#define RESOURCES_H

#include <string.h>

#include "image.h"
#include "mem.h"
#include "super.h"

void ResourceAddImage(Image *image);
Image *ResourceImage(int index);
int ResourceImageIndex(char *name);
Image *ResourceImageSearch(char *name);

void ResourceFree();

#endif
