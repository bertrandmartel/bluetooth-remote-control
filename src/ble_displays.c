/* Copyright (c) 2012 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

/* Attention!
*  To maintain compliance with Nordic Semiconductor ASA’s Bluetooth profile
*  qualification listings, this section of source code must not be modified.
*/

#include "ble_displays.h"
#include <string.h>
#include "nordic_common.h"
#include "ble_srv_common.h"
#include "app_util.h"
#include "SEGGER_RTT.h"

/**@brief Function for handling the Connect event.
 *
 * @param[in]   p_dis       Battery Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_connect(ble_displays_t * p_dis, ble_evt_t * p_ble_evt)
{
    p_dis->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
}


/**@brief Function for handling the Disconnect event.
 *
 * @param[in]   p_dis       Battery Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_disconnect(ble_displays_t * p_dis, ble_evt_t * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_dis->conn_handle = BLE_CONN_HANDLE_INVALID;
}

/**@brief Function for handling the Write event.
 *
 * @param[in]   p_dis       LED Button Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_write(ble_displays_t * p_dis, ble_evt_t * p_ble_evt)
{
    ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;


    if ((p_evt_write->handle == p_dis->led_char_handles.value_handle) &&
            (p_evt_write->len == 1) &&
            (p_dis->led_write_handler != NULL))
    {
        p_dis->led_write_handler(p_dis, p_evt_write->data[0]);
    }
    else if ((p_evt_write->handle == p_dis->display_full_color_handles.value_handle) &&
             (p_evt_write->len == 3) &&
             (p_dis->full_color_handler != NULL))
    {
        p_dis->full_color_handler(p_dis, p_evt_write->data[0], p_evt_write->data[1], p_evt_write->data[2]);
    }
    else if ((p_evt_write->handle == p_dis->display_bitmap_handles.value_handle) &&
             (p_evt_write->len > 0) &&
             (p_dis->bitmap_handler != NULL))
    {
        p_dis->bitmap_handler(p_dis, p_evt_write);
    }
}

void ble_displays_on_ble_evt(ble_displays_t * p_dis, ble_evt_t * p_ble_evt)
{
    if (p_dis == NULL || p_ble_evt == NULL)
    {
        return;
    }

    switch (p_ble_evt->header.evt_id)
    {
    case BLE_GAP_EVT_CONNECTED:
        on_connect(p_dis, p_ble_evt);
        break;

    case BLE_GAP_EVT_DISCONNECTED:
        on_disconnect(p_dis, p_ble_evt);
        break;

    case BLE_GATTS_EVT_WRITE:
        SEGGER_RTT_printf(0, "\x1B[32min BLE_GATTS_EVT_WRITE\x1B[0m\n");
        on_write(p_dis, p_ble_evt);
        break;

    default:
        // No implementation needed.
        break;
    }
}


/**@brief Function for adding the LED characteristic.
 *
 */

static uint32_t led_char_add(ble_displays_t * p_dis, const ble_displays_init_t * p_dis_init, uint16_t uuid, ble_gatts_char_handles_t *characteristic_handler)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read   = 1;
    char_md.char_props.write  = 1;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = NULL;
    char_md.p_sccd_md         = NULL;

    ble_uuid.type = p_dis->uuid_type;
    ble_uuid.uuid = uuid;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid       = &ble_uuid;
    attr_char_value.p_attr_md    = &attr_md;
    attr_char_value.init_len     = sizeof(uint8_t);
    attr_char_value.init_offs    = 0;
    attr_char_value.max_len      = sizeof(uint8_t);
    attr_char_value.p_value      = NULL;

    return sd_ble_gatts_characteristic_add(p_dis->service_handle, &char_md,
                                           &attr_char_value,
                                           characteristic_handler);
}

/**@brief Function for adding the LED characteristic.
 *
 */

static uint32_t display_char_add(ble_displays_t * p_dis,
                                 const ble_displays_init_t * p_dis_init,
                                 uint16_t uuid,
                                 ble_gatts_char_handles_t *characteristic_handler,
                                 uint16_t init_len,
                                 uint16_t max_len)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&char_md, 0, sizeof(char_md));

    //char_md.char_ext_props.reliable_wr = 1;
    char_md.char_props.read   = 1;
    char_md.char_props.write  = 1;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = NULL;
    char_md.p_sccd_md         = NULL;

    ble_uuid.type = p_dis->uuid_type;
    ble_uuid.uuid = uuid;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid       = &ble_uuid;
    attr_char_value.p_attr_md    = &attr_md;
    attr_char_value.init_len     = init_len;
    attr_char_value.init_offs    = 0;
    attr_char_value.max_len      = max_len;
    attr_char_value.p_value      = NULL;

    return sd_ble_gatts_characteristic_add(p_dis->service_handle, &char_md,
                                           &attr_char_value,
                                           characteristic_handler);
}



/**@brief Function for adding the Button characteristic.
 *
 */
static uint32_t adc_char_add(ble_displays_t * p_dis, const ble_displays_init_t * p_dis_init, uint16_t uuid, ble_gatts_char_handles_t *characteristic_handler)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&cccd_md, 0, sizeof(cccd_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
    cccd_md.vloc = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read   = 1;
    char_md.char_props.notify = 1;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md;
    char_md.p_sccd_md         = NULL;

    ble_uuid.type = p_dis->uuid_type;
    ble_uuid.uuid = uuid;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.write_perm);
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid       = &ble_uuid;
    attr_char_value.p_attr_md    = &attr_md;
    attr_char_value.init_len     = sizeof(uint8_t);
    attr_char_value.init_offs    = 0;
    attr_char_value.max_len      = sizeof(uint8_t);
    attr_char_value.p_value      = NULL;

    return sd_ble_gatts_characteristic_add(p_dis->service_handle, &char_md,
                                           &attr_char_value,
                                           characteristic_handler);
}

uint32_t ble_displays_init(ble_displays_t * p_dis, const ble_displays_init_t * p_dis_init)
{
    uint32_t   err_code;
    ble_uuid_t ble_uuid;

    // Initialize service structure
    p_dis->conn_handle       = BLE_CONN_HANDLE_INVALID;
    p_dis->led_write_handler = p_dis_init->led_write_handler;
    p_dis->full_color_handler = p_dis_init->full_color_handler;
    p_dis->bitmap_handler = p_dis_init->bitmap_handler;

    // Add service
    ble_uuid128_t base_uuid = {DISPLAYS_UUID_BASE};
    err_code = sd_ble_uuid_vs_add(&base_uuid, &p_dis->uuid_type);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    ble_uuid.type = p_dis->uuid_type;
    ble_uuid.uuid = DISPLAYS_UUID_SERVICE_BUTTON;

    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &p_dis->service_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    err_code = adc_char_add(p_dis, p_dis_init, DISPLAYS_UUID_BUTTON1, &p_dis->button1_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    err_code = adc_char_add(p_dis, p_dis_init, DISPLAYS_UUID_BUTTON2, &p_dis->button2_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    err_code = adc_char_add(p_dis, p_dis_init, DISPLAYS_UUID_BUTTON3, &p_dis->button3_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    err_code = adc_char_add(p_dis, p_dis_init, DISPLAYS_UUID_BUTTON4, &p_dis->button4_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    err_code = led_char_add(p_dis, p_dis_init, DISPLAYS_UUID_LED, &p_dis->led_char_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    err_code = display_char_add(p_dis, p_dis_init, DISPLAYS_UUID_FULL_COLOR, &p_dis->display_full_color_handles, sizeof(uint8_t), sizeof(uint32_t));
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    err_code = adc_char_add(p_dis, p_dis_init, DISPLAYS_UUID_DPAD, &p_dis->dpad_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    err_code = display_char_add(p_dis, p_dis_init, DISPLAYS_UUID_BITMAP, &p_dis->display_bitmap_handles, sizeof(uint8_t), 18 );
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    return NRF_SUCCESS;
}

uint32_t ble_displays_on_button_change(ble_displays_t * p_dis, uint8_t button_state, ble_gatts_char_handles_t *characteristic_handler)
{
    ble_gatts_hvx_params_t params;
    uint16_t len = sizeof(button_state);

    memset(&params, 0, sizeof(params));
    params.type = BLE_GATT_HVX_NOTIFICATION;
    params.handle = characteristic_handler->value_handle;
    params.p_data = &button_state;
    params.p_len = &len;

    return sd_ble_gatts_hvx(p_dis->conn_handle, &params);
}