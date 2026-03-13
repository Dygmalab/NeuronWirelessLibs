/*
 * The MIT License (MIT)
 *
 * Copyright (C) 2025  Dygma Lab S.L.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __LINK_LIST_H_
#define __LINK_LIST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "dl_middleware.h"

typedef struct linklist linklist_t;

extern result_t linklist_init( linklist_t ** pp_linklist );

extern result_t linklist_add( linklist_t * p_linklist, void * p_instance );
extern void * linklist_get( linklist_t * p_linklist );                       /* Returns currently navigated p_instance previously
                                                                                added to the list with linklist_add */
extern void linklist_nav_head( linklist_t * p_linklist );
extern void linklist_nav_tail( linklist_t * p_linklist );
extern void linklist_nav_next( linklist_t * p_linklist );
extern void linklist_nav_prev( linklist_t * p_linklist );

#ifdef __cplusplus
}
#endif

#endif /* __LINK_LIST_H_ */
