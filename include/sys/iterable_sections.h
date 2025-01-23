#ifndef __ITERABLE_SECTIONS_H__
#define __ITERABLE_SECTIONS_H__

#include "sys/__assert.h"
#include "device.h"

/**
 * @brief iterable section start symbol for a generic type
 *
 * will return '_[OUT_TYPE]_list_start'.
 *
 * @param[in]  secname type name of iterable section.  For 'struct foobar' this
 * would be TYPE_SECTION_START(foobar)
 *
 */
#define TYPE_SECTION_START(secname) CONCAT(_##secname, _list_start)

/**
 * @brief iterable section end symbol for a generic type
 *
 * will return '_<SECNAME>_list_end'.
 *
 * @param[in]  secname type name of iterable section.  For 'struct foobar' this
 * would be TYPE_SECTION_START(foobar)
 */
#define TYPE_SECTION_END(secname) CONCAT(_##secname, _list_end)

/**
 * @brief iterable section extern for start symbol for a generic type
 *
 * Helper macro to give extern for start of iterable section.  The macro
 * typically will be called TYPE_SECTION_START_EXTERN(struct foobar, foobar).
 * This allows the macro to hand different types as well as cases where the
 * type and section name may differ.
 *
 * @param[in]  type data type of section
 * @param[in]  secname name of output section
 */
#define TYPE_SECTION_START_EXTERN(type, secname) \
	extern type TYPE_SECTION_START(secname)[]

/**
 * @brief iterable section extern for end symbol for a generic type
 *
 * Helper macro to give extern for end of iterable section.  The macro
 * typically will be called TYPE_SECTION_END_EXTERN(struct foobar, foobar).
 * This allows the macro to hand different types as well as cases where the
 * type and section name may differ.
 *
 * @param[in]  type data type of section
 * @param[in]  secname name of output section
 */
#define TYPE_SECTION_END_EXTERN(type, secname) \
	extern type TYPE_SECTION_END(secname)[]

/**
 * @brief Iterate over a specified iterable section for a generic type
 *
 * @details
 * Iterator for structure instances gathered by TYPE_SECTION_ITERABLE().
 * The linker must provide a _<SECNAME>_list_start symbol and a
 * _<SECNAME>_list_end symbol to mark the start and the end of the
 * list of struct objects to iterate over. This is normally done using
 * ITERABLE_SECTION_ROM() or ITERABLE_SECTION_RAM() in the linker script.
 */
#define TYPE_SECTION_FOREACH(type, secname, iterator)		\
	TYPE_SECTION_START_EXTERN(type, secname);		\
	TYPE_SECTION_END_EXTERN(type, secname);		\
	for (type * iterator = TYPE_SECTION_START(secname); ({	\
		__ASSERT(iterator <= TYPE_SECTION_END(secname),\
			      "unexpected list end location");	\
		     iterator < TYPE_SECTION_END(secname);	\
	     });						\
	     iterator++)

/**
 * @brief Iterate over a specified iterable section (alternate).
 *
 * @details
 * Iterator for structure instances gathered by STRUCT_SECTION_ITERABLE().
 * The linker must provide a _<SECNAME>_list_start symbol and a
 * _<SECNAME>_list_end symbol to mark the start and the end of the
 * list of struct objects to iterate over. This is normally done using
 * ITERABLE_SECTION_ROM() or ITERABLE_SECTION_RAM() in the linker script.
 */
#define STRUCT_SECTION_FOREACH_ALTERNATE(secname, struct_type, iterator) \
	TYPE_SECTION_FOREACH(struct struct_type, secname, iterator)

/**
 * @brief Iterate over a specified iterable section.
 *
 * @details
 * Iterator for structure instances gathered by STRUCT_SECTION_ITERABLE().
 * The linker must provide a _<struct_type>_list_start symbol and a
 * _<struct_type>_list_end symbol to mark the start and the end of the
 * list of struct objects to iterate over. This is normally done using
 * ITERABLE_SECTION_ROM() or ITERABLE_SECTION_RAM() in the linker script.
 */
#define STRUCT_SECTION_FOREACH(struct_type, iterator) \
	STRUCT_SECTION_FOREACH_ALTERNATE(struct_type, struct_type, iterator)

#endif // __ITERABLE_SECTIONS_H__