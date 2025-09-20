/*
 * Solution of SICP JS Exercise 5.53
 *
 * Copyright (c) 2025 Yuichiro MORIGUCHI
 *
 * This software is released under the MIT License.
 * http://opensource.org/licenses/mit-license.php
 **/
#include <stdio.h>
#include <stdlib.h>
#include "memory.h"

int main(int argc, char **argv) {
    if(argc < 2) {
        fprintf(stderr, "usage: %s <program>\n", argv[0]);
    } else {
        init_memory();
        init_cons();

        cell r = execute(argv[1]);
        display(r);
    }
}


