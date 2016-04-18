/* Copyright (c) 2013 Nordic Semiconductor. All Rights Reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the license.txt file.
 */

#ifndef BLE_DISPLAYS_H__
#define BLE_DISPLAYS_H__

#include <stdint.h>
#include <stdbool.h>
#include "ble.h"
#include "ble_srv_common.h"

#define DISPLAYS_UUID_BASE {0x23, 0xD1, 0xBC, 0xEA, 0x5F, 0x78, 0x23, 0x15, 0xDE, 0xEF, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00}

#define DISPLAYS_UUID_SERVICE_BUTTON 0x1523
#define DISPLAYS_UUID_SERVICE_LED 0x1524

#define DISPLAYS_UUID_BUTTON1 0x1601
#define DISPLAYS_UUID_BUTTON2 0x1602
#define DISPLAYS_UUID_BUTTON3 0x1603
#define DISPLAYS_UUID_BUTTON4 0x1604
#define DISPLAYS_UUID_DPAD    0x1605

#define DISPLAYS_UUID_LED 0x1701

#define DISPLAYS_UUID_FULL_COLOR 0x1801
#define DISPLAYS_UUID_BITMAP     0x1802

// Forward declaration of the ble_displays_t type.
typedef struct ble_displays_s ble_displays_t;

typedef void (*ble_displays_led_write_handler_t) (ble_displays_t * p_dis, uint8_t new_state);
typedef void (*ble_displays_full_color_write_handler_t) (ble_displays_t * p_dis, uint8_t red, uint8_t green, uint8_t blue);
typedef void (*ble_displays_bitmap_write_handler_t) (ble_displays_t * p_dis, ble_gatts_evt_write_t * p_evt_write);

typedef struct
{
	ble_displays_led_write_handler_t           led_write_handler;                    /**< Event handler to be called when LED characteristic is written. */
	ble_displays_full_color_write_handler_t    full_color_handler;
	ble_displays_bitmap_write_handler_t        bitmap_handler;
} ble_displays_init_t;


/**@brief LED Button Service structure. This contains various status information for the service. */
typedef struct ble_displays_s
{
	uint16_t                                 service_handle;
	ble_gatts_char_handles_t                 display_full_color_handles;
	ble_gatts_char_handles_t                 display_bitmap_handles;
	ble_gatts_char_handles_t                 dpad_handles;
	ble_gatts_char_handles_t                 led_char_handles;
	ble_gatts_char_handles_t                 button1_handles;
	ble_gatts_char_handles_t                 button2_handles;
	ble_gatts_char_handles_t                 button3_handles;
	ble_gatts_char_handles_t                 button4_handles;
	uint8_t                                  uuid_type;
	uint16_t                                 conn_handle;
	ble_displays_led_write_handler_t         led_write_handler;
	ble_displays_full_color_write_handler_t  full_color_handler;
	ble_displays_bitmap_write_handler_t      bitmap_handler;
} ble_displays_t;

/**@brief Function for initializing the LED Button Service.
 *
 * @param[out]  p_dis       LED Button Service structure. This structure will have to be supplied by
 *                          the application. It will be initialized by this function, and will later
 *                          be used to identify this particular service instance.
 * @param[in]   p_dis_init  Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on successful initialization of service, otherwise an error code.
 */
uint32_t ble_displays_init(ble_displays_t * p_dis, const ble_displays_init_t * p_dis_init);

/**@brief Function for handling the Application's BLE Stack events.
 *
 * @details Handles all events from the BLE stack of interest to the LED Button Service.
 *
 *
 * @param[in]   p_dis      LED Button Service structure.
 * @param[in]   p_ble_evt  Event received from the BLE stack.
 */
void ble_displays_on_ble_evt(ble_displays_t * p_dis, ble_evt_t * p_ble_evt);

/**@brief Function for sending a button state notification.
 */
uint32_t ble_displays_on_button_change(ble_displays_t * p_dis, uint8_t button_state, ble_gatts_char_handles_t *characteristic_handler);

#endif // BLE_DISPLAYS_H__

/** @} */