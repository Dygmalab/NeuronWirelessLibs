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

#include "link_list.h"

typedef struct item item_t;

struct item
{
    void * p_instance;

    item_t * p_prev;
    item_t * p_next;
};

struct linklist
{
    item_t * p_head;
    item_t * p_tail;
    item_t * p_current;

    uint16_t count;
};

/*******************************************************************/
/*                              Items                              */
/*******************************************************************/

static INLINE result_t _item_init( item_t ** pp_item )
{
    item_t * p_item;

    p_item = heap_alloc( sizeof( item_t ) );

    /* Initialize the p_item */
    p_item->p_instance = NULL;
    p_item->p_prev = NULL;
    p_item->p_next = NULL;

    *pp_item = p_item;

    return RESULT_OK;
}

static INLINE void _item_set_instance( item_t * p_item, void * p_instance )
{
    p_item->p_instance = p_instance;
}

static INLINE void _item_set_previous( item_t * p_item, item_t * p_previous )
{
    p_item->p_prev = p_previous;
}

static INLINE void _item_set_next( item_t * p_item, item_t * p_next )
{
    p_item->p_next = p_next;
}

static INLINE void * _item_get_instance( item_t * p_item )
{
    return p_item->p_instance;
}

static INLINE item_t * _item_get_previous( item_t * p_item )
{
    return p_item->p_prev;
}

static INLINE item_t * _item_get_next( item_t * p_item )
{
    return p_item->p_next;
}

/*******************************************************************/
/*                              List                               */
/*******************************************************************/

static INLINE result_t _list_init( linklist_t * p_linklist )
{
    p_linklist->p_head = NULL;
    p_linklist->p_tail = NULL;
    p_linklist->count = 0;

    return RESULT_OK;
}

static INLINE result_t _list_add_item( linklist_t * p_linklist, item_t * p_item )
{
    /* Check if this is the first item */
    if( p_linklist->count == 0 )
    {
        p_linklist->p_head = p_item;
        p_linklist->p_tail = p_item;
        p_linklist->p_current = p_item;
    }
    else
    {
        /* Add the item to the tail of the list */
        _item_set_next( p_linklist->p_tail, p_item );
        _item_set_previous( p_item, p_linklist->p_tail );
        p_linklist->p_tail = p_item;
    }

    /* Increase the number of items in the list */
    p_linklist->count++;

    return RESULT_OK;
}

/*******************************************************************/
/*                          Link List API                          */
/*******************************************************************/

result_t linklist_init( linklist_t ** pp_linklist )
{
    result_t result = RESULT_ERR;

    *pp_linklist = heap_alloc( sizeof( linklist_t ) );

    result = _list_init( *pp_linklist );
    EXIT_IF_ERR( result, "_list_init failed" );

_EXIT:
    return result;
}

result_t linklist_add( linklist_t * p_linklist, void * p_instance )
{
    result_t result = RESULT_ERR;
    item_t * p_item;

    /* Create new item */
    result = _item_init( &p_item );
    EXIT_IF_ERR( result, "_item_init failed" );

    /* Assign the p_instance to the list item */
    _item_set_instance( p_item, p_instance );

    /* Add the item into the list */
    result = _list_add_item( p_linklist, p_item );
    EXIT_IF_ERR( result, "_add_item failed" );

_EXIT:
    return result;
}

void * linklist_get( linklist_t * p_linklist )
{
    if( p_linklist->p_current == NULL )
    {
        return NULL;
    }

    return _item_get_instance( p_linklist->p_current );
}

void linklist_nav_head( linklist_t * p_linklist )
{
    p_linklist->p_current = p_linklist->p_head;
}

void linklist_nav_tail( linklist_t * p_linklist )
{
    p_linklist->p_current = p_linklist->p_tail;
}

void linklist_nav_next( linklist_t * p_linklist )
{
    if( p_linklist->p_current == NULL )
    {
        return;
    }

    p_linklist->p_current = _item_get_next( p_linklist->p_current );
}

void linklist_nav_prev( linklist_t * p_linklist )
{
    if( p_linklist->p_current == NULL )
    {
        return;
    }

    p_linklist->p_current = _item_get_previous( p_linklist->p_current );
}

