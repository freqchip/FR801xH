/**
 * Copyright (c) 2019, Freqchip
 *
 * All rights reserved.
 *
 *
 */

#ifndef OS_MEM_H_
#define OS_MEM_H_

/*
 * INCLUDES 
 */
#include <stdint.h>
#include <string.h>

#if 0

/*********************************************************************
 * @fn      os_malloc
 *
 * @brief   malloc a ram space with specfied size
 *
 * @param   size    malloc space
 *
 * @return  None.
 */
void *os_malloc(uint32_t size);
/*********************************************************************
 * @fn      os_calloc
 *
 * @brief   malloc a ram space with specfied size, real size = num * size
 *
 * @param   -num    how many blocks will be malloced
 *			-size   ram length per block
 *
 * @return  None.
 */
void *os_calloc(uint32_t num,uint32_t size);
/*********************************************************************
 * @fn      os_realloc
 *
 * @brief   malloc a ram space with specfied size, and free old ram space .
 *
 * @param   -new_size    malloc space
 *			-ptr	pointer to old ram space	
 *
 * @return  None.
 */
void *os_realloc(void *ptr,uint32_t new_size);

/*********************************************************************
 * @fn      os_zalloc
 *
 * @brief   malloc a ram space with specfied size,ram value is set to 0
 *
 * @param   size    malloc space
            type    -KE_MEM_ENV
                    -KE_MEM_ATT_DB
                    -KE_MEM_KE_MSG
                    -KE_MEM_NON_RETENTION
 *
 * @return  None.
 */
void *os_zalloc(uint32_t size);

/*********************************************************************
 * @fn      os_free
 *
 * @brief   free a malloced ram space
 *
 * @param   ptr    point to ram space,which will be freed.
 *
 * @return  None.
 */
void os_free(void *ptr);
/*********************************************************************
 * @fn      os_get_free_heap_size
 *
 * @brief   get free heap size.
 *
 * @param   None
 *
 * @return  Free heap size.
 */
uint16_t os_get_free_heap_size(void);

#else

#include "compiler.h"
void *ke_malloc(uint32_t size, uint8_t type);
void ke_free(void *mem_ptr);
uint16_t ke_get_mem_free(uint8_t type);


/*********************************************************************
 * @fn      os_malloc
 *
 * @brief   malloc a ram space with specfied size
 *
 * @param   size    malloc space
 *
 * @return  None.
 */

__STATIC __INLINE void *os_malloc(uint32_t size)
{
    return ke_malloc(size, 3);
}
/*********************************************************************
 * @fn      os_zalloc
 *
 * @brief   malloc a ram space with specfied size,ram value is set to 0
 *
 * @param   size    malloc space
            type    -KE_MEM_ENV
                    -KE_MEM_ATT_DB
                    -KE_MEM_KE_MSG
                    -KE_MEM_NON_RETENTION
 *
 * @return  None.
 */
__STATIC __INLINE void *os_zalloc(uint32_t size)
{
    void *buffer;

    buffer = ke_malloc(size, 3);

    if(buffer)
    {
        memset(buffer, 0, size);
    }

    return buffer;
}
/*********************************************************************
 * @fn      os_free
 *
 * @brief   free a malloced ram space
 *
 * @param   ptr    point to ram space,which will be freed.
 *
 * @return  None.
 */
__STATIC __INLINE void os_free(void *ptr)
{
    if(ptr)
        ke_free(ptr);
}
/*********************************************************************
 * @fn      os_calloc
 *
 * @brief   malloc a ram space with specfied size, real size = num * size
 *
 * @param   -num    how many blocks will be malloced
 *			-size   ram length per block
 *
 * @return  None.
 */
__STATIC __INLINE void *os_calloc(uint32_t num,uint32_t size)
{
    void *buffer;
    buffer = ke_malloc(num * size, 3);
	
    if(buffer)
    {
        memset(buffer, 0, num * size);
    }
    return buffer;
}
/*********************************************************************
 * @fn      os_realloc
 *
 * @brief   malloc a ram space with specfied size, and free old ram space .
 *
 * @param   -new_size    malloc space
 *			-ptr	pointer to old ram space	
 *
 * @return  None.
 */
__STATIC __INLINE void *os_realloc(void *ptr,uint32_t new_size)
{
    void *new_ptr =  ke_malloc(new_size, 3);
    if(new_ptr)
    {
        memset(new_ptr, 0, new_size);
    }
    os_free(ptr);
    return new_ptr;
}
/*********************************************************************
 * @fn      os_get_free_heap_size
 *
 * @brief   get free heap size.
 *
 * @param   None
 *
 * @return  Free heap size.
 */
__STATIC __INLINE uint16_t os_get_free_heap_size(void)
{
    return (ke_get_mem_free(3));
}

#endif


/** @functions API for memory leakage debug,
 *   before call these functions, must define USER_MEM_API_ENABLE in your project Preprocessor Symbols.
 * @{
 */
void show_msg_list(void);
void show_ke_malloc(void);
void show_mem_list(void);


#endif // APP_HT_H_

