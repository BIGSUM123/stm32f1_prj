#ifndef __DRIVERS_FLASH_H__
#define __DRIVERS_FLASH_H__

#include <stddef.h>
#include <stdio.h>
#include "device.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Flash memory parameters. Contents of this structure suppose to be
 * filled in during flash device initialization and stay constant
 * through a runtime.
 */
 struct flash_parameters {
	/** Minimal write alignment and size */
	const size_t write_block_size;

	/** @cond INTERNAL_HIDDEN */
	/* User code should call flash_params_get_ functions on flash_parameters
	 * to get capabilities, rather than accessing object contents directly.
	 */
	struct {
		/* Device has no explicit erase, so it either erases on
		 * write or does not require it at all.
		 * This also includes devices that support erase but
		 * do not require it.
		 */
		bool no_explicit_erase: 1;
	} caps;
	/** @endcond */
	/** Value the device is filled in erased areas */
	uint8_t erase_value;
};

/**
 * @addtogroup flash_internal_interface
 * @{
 */

typedef int (*flash_api_read)(const struct device *dev, off_t offset,
			      void *data,
			      size_t len);
/**
 * @brief Flash write implementation handler type
 *
 * @note Any necessary write protection management must be performed by
 * the driver, with the driver responsible for ensuring the "write-protect"
 * after the operation completes (successfully or not) matches the write-protect
 * state when the operation was started.
 */
typedef int (*flash_api_write)(const struct device *dev, off_t offset,
			       const void *data, size_t len);

/**
 * @brief Flash erase implementation handler type
 *
 * @note Any necessary erase protection management must be performed by
 * the driver, with the driver responsible for ensuring the "erase-protect"
 * after the operation completes (successfully or not) matches the erase-protect
 * state when the operation was started.
 *
 * The callback is optional for RAM non-volatile devices, which do not
 * require erase by design, but may be provided if it allows device to
 * work more effectively, or if device has a support for internal fill
 * operation the erase in driver uses.
 */
typedef int (*flash_api_erase)(const struct device *dev, off_t offset,
			       size_t size);

typedef const struct flash_parameters* (*flash_api_get_parameters)(const struct device *dev);

#if defined(CONFIG_FLASH_PAGE_LAYOUT)
/**
 * @brief Retrieve a flash device's layout.
 *
 * A flash device layout is a run-length encoded description of the
 * pages on the device. (Here, "page" means the smallest erasable
 * area on the flash device.)
 *
 * For flash memories which have uniform page sizes, this routine
 * returns an array of length 1, which specifies the page size and
 * number of pages in the memory.
 *
 * Layouts for flash memories with nonuniform page sizes will be
 * returned as an array with multiple elements, each of which
 * describes a group of pages that all have the same size. In this
 * case, the sequence of array elements specifies the order in which
 * these groups occur on the device.
 *
 * @param dev         Flash device whose layout to retrieve.
 * @param layout      The flash layout will be returned in this argument.
 * @param layout_size The number of elements in the returned layout.
 */
typedef void (*flash_api_pages_layout)(const struct device *dev,
				       const struct flash_pages_layout **layout,
				       size_t *layout_size);
#endif /* CONFIG_FLASH_PAGE_LAYOUT */

typedef int (*flash_api_sfdp_read)(const struct device *dev, off_t offset,
				   void *data, size_t len);
typedef int (*flash_api_read_jedec_id)(const struct device *dev, uint8_t *id);
typedef int (*flash_api_ex_op)(const struct device *dev, uint16_t code,
			       const uintptr_t in, void *out);

struct flash_driver_api {
	flash_api_read read;
	flash_api_write write;
	flash_api_erase erase;
	flash_api_get_parameters get_parameters;
#if defined(CONFIG_FLASH_PAGE_LAYOUT)
	flash_api_pages_layout page_layout;
#endif /* CONFIG_FLASH_PAGE_LAYOUT */
#if defined(CONFIG_FLASH_JESD216_API)
	flash_api_sfdp_read sfdp_read;
	flash_api_read_jedec_id read_jedec_id;
#endif /* CONFIG_FLASH_JESD216_API */
#if defined(CONFIG_FLASH_EX_OP_ENABLED)
	flash_api_ex_op ex_op;
#endif /* CONFIG_FLASH_EX_OP_ENABLED */
};

/**
 * @}
 */

/**
 * @addtogroup flash_interface
 * @{
 */

/**
 *  @brief  Read data from flash
 *
 *  All flash drivers support reads without alignment restrictions on
 *  the read offset, the read size, or the destination address.
 *
 *  @param  dev             : flash dev
 *  @param  offset          : Offset (byte aligned) to read
 *  @param  data            : Buffer to store read data
 *  @param  len             : Number of bytes to read.
 *
 *  @return  0 on success, negative errno code on fail.
 */
static inline int flash_read(const struct device *dev, 
                        off_t offset, void *data, size_t len)
{
	const struct flash_driver_api *api = 
		(const struct flash_driver_api *)dev->api;

	return api->read(dev, offset, data, len);
}

/**
 *  @brief  Write buffer into flash memory.
 *
 *  All flash drivers support a source buffer located either in RAM or
 *  SoC flash, without alignment restrictions on the source address.
 *  Write size and offset must be multiples of the minimum write block size
 *  supported by the driver.
 *
 *  Any necessary write protection management is performed by the driver
 *  write implementation itself.
 *
 *  @param  dev             : flash device
 *  @param  offset          : starting offset for the write
 *  @param  data            : data to write
 *  @param  len             : Number of bytes to write
 *
 *  @return  0 on success, negative errno code on fail.
 */
static inline int flash_write(const struct device *dev,
						off_t offset, const void *data, size_t len)
{
	const struct flash_driver_api *api = 
		(const struct flash_driver_api *)dev->api;

	return api->write(dev, offset, data, len);
}

/**
 *  @brief  Erase part or all of a flash memory
 *
 *  Acceptable values of erase size and offset are subject to
 *  hardware-specific multiples of page size and offset. Please check
 *  the API implemented by the underlying sub driver, for example by
 *  using flash_get_page_info_by_offs() if that is supported by your
 *  flash driver.
 *
 *  Any necessary erase protection management is performed by the driver
 *  erase implementation itself.
 *
 *  The function should be used only for devices that are really
 *  explicit erase devices; in case when code relies on erasing
 *  device, i.e. setting it to erase-value, prior to some operations,
 *  but should work with explicit erase and RAM non-volatile devices,
 *  then flash_flatten should rather be used.
 *
 *  @param  dev             : flash device
 *  @param  offset          : erase area starting offset
 *  @param  size            : size of area to be erased
 *
 *  @return  0 on success, negative errno code on fail.
 *
 *  @see flash_flatten()
 *  @see flash_get_page_info_by_offs()
 *  @see flash_get_page_info_by_idx()
 */
static inline int flash_erase(const struct device *dev,
						off_t offset, size_t len)
{
	int rc = -1;

	const struct flash_driver_api *api =
		(const struct flash_driver_api *)dev->api;

	if (api->erase != NULL) {
		rc = api->erase(dev, offset, len);
	}

	return rc;
}

/**
 *  @brief  Get pointer to flash_parameters structure
 *
 *  Returned pointer points to a structure that should be considered
 *  constant through a runtime, regardless if it is defined in RAM or
 *  Flash.
 *  Developer is free to cache the structure pointer or copy its contents.
 *
 *  @return pointer to flash_parameters structure characteristic for
 *          the device.
 */
static inline const struct flash_parameters *flash_get_parameters(
								const struct device *dev)
{
	const struct flash_driver_api *api =
		(const struct flash_driver_api *)dev->api;

	return api->get_parameters(dev);
}

#ifdef __cplusplus
}
#endif

#endif // __DRIVERS_FLASH_H__