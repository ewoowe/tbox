/*!The Tiny Box Library
 * 
 * TBox is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 * 
 * TBox is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with TBox; 
 * If not, see <a href="http://www.gnu.org/licenses/"> http://www.gnu.org/licenses/</a>
 * 
 * Copyright (C) 2009 - 2011, ruki All rights reserved.
 *
 * \author		ruki
 * \file		malloc.c
 *
 */
/* /////////////////////////////////////////////////////////
 * includes
 */
#include "malloc.h"

/* /////////////////////////////////////////////////////////
 * macros
 */
#ifdef TB_DEBUG
#	define TB_MPOOL_ALLOCATE(hpool, size, func, line, file) 			tb_mpool_allocate(hpool, size, func, line, file)
# 	define TB_MPOOL_CALLOCATE(hpool, item, size, func, line, file) 		tb_mpool_callocate(hpool, item, size, func, line, file)
# 	define TB_MPOOL_REALLOCATE(hpool, data, size, func, line, file) 	tb_mpool_reallocate(hpool, data, size, func, line, file)
# 	define TB_MPOOL_DEALLOCATE(hpool, data, func, line, file) 			tb_mpool_deallocate(hpool, data, func, line, file)
#else
# 	define TB_MPOOL_ALLOCATE(hpool, size, func, line, file) 			tb_mpool_allocate(hpool, size)
# 	define TB_MPOOL_CALLOCATE(hpool, item, size, func, line, file) 		tb_mpool_callocate(hpool, item, size)
# 	define TB_MPOOL_REALLOCATE(hpool, data, size, func, line, file) 	tb_mpool_reallocate(hpool, data, size)
# 	define TB_MPOOL_DEALLOCATE(hpool, data, func, line, file) 			tb_mpool_deallocate(hpool, data)
#endif

/* /////////////////////////////////////////////////////////
 * globals
 */
static tb_handle_t g_mpool = TB_NULL;
static tb_handle_t g_mutex = TB_NULL; 

/* /////////////////////////////////////////////////////////
 * implemention
 */
tb_bool_t tb_memory_init(tb_void_t* data, tb_size_t size)
{
	tb_assert_and_check_return_val(data && size, TB_FALSE);

	if (!g_mutex) g_mutex = tb_mutex_init("the memory pool");
	tb_assert_and_check_return_val(g_mutex, TB_FALSE);

	if (!tb_mutex_lock(g_mutex)) return TB_FALSE;
	if (!g_mpool) g_mpool = tb_mpool_init(data, size);
	if (!tb_mutex_unlock(g_mutex)) return TB_FALSE;

	return g_mpool? TB_TRUE : TB_FALSE;
}
tb_void_t tb_memory_exit()
{
	if (tb_mutex_lock(g_mutex))
	{
		if (g_mpool)
		{
			tb_mpool_exit(g_mpool);
			g_mpool = TB_NULL;
		}
		tb_mutex_unlock(g_mutex);
	}

	if (g_mutex)	
	{
		tb_mutex_exit(g_mutex);
		g_mutex = TB_NULL;
	}
}

#ifndef TB_DEBUG
tb_void_t* tb_memory_allocate(tb_size_t size)
#else
tb_void_t* tb_memory_allocate(tb_size_t size, tb_char_t const* func, tb_size_t line, tb_char_t const* file)
#endif
{
	// check 
	tb_assert_and_check_return_val(g_mpool && g_mutex, TB_NULL);

	// lock
	if (!tb_mutex_lock(g_mutex)) return TB_NULL;
	tb_byte_t* p = TB_MPOOL_ALLOCATE(g_mpool, size, func, line, file);
	tb_mutex_unlock(g_mutex);

	// align by 4 bytes
	tb_assert(!(((tb_size_t)p) & TB_CPU_BITALIGN));

	tb_assert_message(p, "cannot alloc at %s(): %d, file: %s", func? func : "null", line, file? file : "null");
	return p;
}
#ifndef TB_DEBUG
tb_void_t* tb_memory_callocate(tb_size_t item, tb_size_t size)
#else
tb_void_t* tb_memory_callocate(tb_size_t item, tb_size_t size, tb_char_t const* func, tb_size_t line, tb_char_t const* file)
#endif
{
	// check 
	tb_assert_and_check_return_val(g_mpool && g_mutex, TB_NULL);

	// lock
	if (!tb_mutex_lock(g_mutex)) return TB_NULL;
	tb_byte_t* p = TB_MPOOL_CALLOCATE(g_mpool, item, size, func, line, file);
	tb_mutex_unlock(g_mutex);

	// align by 4 bytes
	tb_assert(!(((tb_size_t)p) & TB_CPU_BITALIGN));

	tb_assert_message(p, "cannot calloc at %s(): %d, file: %s", func? func : "null", line, file? file : "null");
	return p;
}

#ifndef TB_DEBUG
tb_void_t* tb_memory_reallocate(tb_void_t* data, tb_size_t size)
#else
tb_void_t* tb_memory_reallocate(tb_void_t* data, tb_size_t size, tb_char_t const* func, tb_size_t line, tb_char_t const* file)
#endif
{
	// check 
	tb_assert_and_check_return_val(g_mpool && g_mutex, TB_NULL);

	// lock
	if (!tb_mutex_lock(g_mutex)) return TB_NULL;
	tb_byte_t* p = TB_MPOOL_REALLOCATE(g_mpool, data, size, func, line, file);
	tb_mutex_unlock(g_mutex);

	// align by 4 bytes
	tb_assert(!(((tb_size_t)p) & TB_CPU_BITALIGN));

	tb_assert_message(p, "invalid realloc data address:%x at %s(): %d, file: %s", data, func? func : "null", line, file? file : "null");
	return p;
}
#ifndef TB_DEBUG
tb_void_t tb_memory_deallocate(tb_void_t* data)
#else
tb_void_t tb_memory_deallocate(tb_void_t* data, tb_char_t const* func, tb_size_t line, tb_char_t const* file)
#endif
{
	// check 
	tb_assert_and_check_return(g_mpool && g_mutex);

	// lock
	if (!tb_mutex_lock(g_mutex)) return ;
	tb_bool_t ret = TB_MPOOL_DEALLOCATE(g_mpool, data, func, line, file);
	tb_mutex_unlock(g_mutex);

	tb_assert_message(ret, "invalid free data address:%x at %s(): %d, file: %s", data, func? func : "null", line, file? file : "null");
}

#ifdef TB_DEBUG
tb_void_t tb_memory_dump()
{
	// check 
	tb_assert_and_check_return(g_mpool && g_mutex);

	// lock
	if (!tb_mutex_lock(g_mutex)) return ;
	tb_mpool_dump(g_mpool);
	tb_mutex_unlock(g_mutex);
}
tb_bool_t tb_memory_check()
{
	// check 
	tb_assert_and_check_return_val(g_mpool && g_mutex, TB_FALSE);

	// lock
	if (!tb_mutex_lock(g_mutex)) return TB_FALSE;
	tb_bool_t ret = tb_mpool_check(g_mpool);
	tb_mutex_unlock(g_mutex);
	return ret;
}
#endif
