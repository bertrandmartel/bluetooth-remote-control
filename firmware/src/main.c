/* Copyright (c) 2013 Nordic Semiconductor. All Rights Reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the license.txt file.
 */

/**
 * This file is the main file for the application described in application note
 * nAN-36 Creating Bluetooth® Low Energy Applications Using nRF51822.
 */

#include <string.h>
#include <stdint.h>
#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "nrf_gpio.h"
#include "nrf51_bitfields.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_conn_params.h"
#include "boards.h"
#include "app_scheduler.h"
#include "softdevice_handler.h"
#include "app_timer_appsh.h"
#include "ble_error_log.h"
#include "app_gpiote.h"
#include "app_button.h"
#include "ble_debug_assert_handler.h"
#include "pstorage.h"
#include "ble_displays.h"
#include "bsp.h"
#include "ble_gap.h"
#include "SEGGER_RTT.h"
#include "adafruit1_8_oled_library.h"
#include "bsp.h"
#include "nrf_delay.h"
#include "unpack.h"

#define E(x,y) x = y,
enum BUTTON_STATE_ENUM {
#include "button_state.h"
};

#define E(x,y) #x,
static const char *BUTTON_STATE_STRING_ENUM[] = {
#include "button_state.h"
};

#define E(x,y) x = y,
enum TRANSMIT_STATUS_ENUM {
#include "transmit_status.h"
};

#define E(x,y) #x,
static const char *TRANSMIT_STATUS_STRING_ENUM[] = {
#include "transmit_status.h"
};

#define IS_SRVC_CHANGED_CHARACT_PRESENT 0                                           /**< Include or not the service_changed characteristic. if not enabled, the server's database cannot be changed for the lifetime of the device*/

#define BUTTON_S0         BSP_BUTTON_0                                /**< Button used to wake up the application. */
#define BUTTON_S1         BSP_BUTTON_1
#define BUTTON_S2         BSP_BUTTON_2
#define BUTTON_S3         BSP_BUTTON_3

#define DEVICE_NAME                     "BleDisplayRemote"                             /**< Name of device. Will be included in the advertising data. */

#define APP_ADV_INTERVAL                64                                          /**< The advertising interval (in units of 0.625 ms. This value corresponds to 40 ms). */
#define APP_ADV_TIMEOUT_IN_SECONDS      180                                         /**< The advertising timeout (in units of seconds). */
/**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_MAX_TIMERS            2                                           /**< Maximum number of simultaneously created timers. */
#define APP_TIMER_OP_QUEUE_SIZE         4                                           /**< Size of timer operation queues. */

#define APP_TIMER_PRESCALER             0    /**< Value of the RTC1 PRESCALER register. */

#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(30, UNIT_1_25_MS)            /**< Minimum acceptable connection interval (0.5 seconds). */
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(30, UNIT_1_25_MS)            /**< Maximum acceptable connection interval (1 second). */
#define SLAVE_LATENCY                   0                                           /**< Slave latency. */
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)             /**< Connection supervisory timeout (4 seconds). */
#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(20000, APP_TIMER_PRESCALER) /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (15 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(5000, APP_TIMER_PRESCALER)  /**< Time between each call to sd_ble_gap_conn_param_update after the first call (5 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                           /**< Number of attempts before giving up the connection parameter negotiation. */
#define ADC_SAMPLING_INTERVAL           APP_TIMER_TICKS(100, APP_TIMER_PRESCALER) /**< Sampling rate for the ADC */
#define APP_GPIOTE_MAX_USERS            1                                           /**< Maximum number of users of the GPIOTE handler. */

#define BUTTON_DETECTION_DELAY          APP_TIMER_TICKS(50, APP_TIMER_PRESCALER)    /**< Delay from a GPIOTE event until a button is reported as pushed (in number of timer ticks). */

#define SEC_PARAM_TIMEOUT               30                                          /**< Timeout for Pairing Request or Security Request (in seconds). */
#define SEC_PARAM_BOND                  1                                           /**< Perform bonding. */
#define SEC_PARAM_MITM                  0                                           /**< Man In The Middle protection not required. */
#define SEC_PARAM_IO_CAPABILITIES       BLE_GAP_IO_CAPS_NONE                        /**< No I/O capabilities. */
#define SEC_PARAM_OOB                   0                                           /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE          7                                           /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE          16                                          /**< Maximum encryption key size. */

#define DEAD_BEEF                       0xDEADBEEF                                  /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

static ble_gap_sec_params_t             m_sec_params;                               /**< Security requirements for this application. */
static uint16_t                         m_conn_handle = BLE_CONN_HANDLE_INVALID;    /**< Handle of the current connection. */
static ble_displays_t                        m_dis;

#define SCHED_MAX_EVENT_DATA_SIZE       sizeof(app_timer_event_t)                   /**< Maximum size of scheduler events. Note that scheduler BLE stack events do not contain any data, as the events are being pulled from the stack in the event handler. */
#define SCHED_QUEUE_SIZE                10                                          /**< Maximum number of events in the scheduler queue. */

APP_TIMER_DEF(m_adc_sampling_timer_id);
APP_TIMER_DEF(m_dpad_timer_id);

static volatile uint8_t button_state = BUTTON_NONE;
static volatile uint8_t transmit_state = TRANSMIT_NONE;

static bool transmit_init = false;

static volatile bool button_state_change = false;
static volatile uint16_t bitmap_length = 0;
static volatile uint32_t expecting_length = 0;

static const uint16_t MAX_SIZE_ARRAY = 10584;

static uint16_t bitmap_offset = 0;

pstorage_handle_t       pstorage_handle;

// first image buffer
uint8_t *image_part = 0;
// second image buffer
uint8_t *image_part2 = 0;

bool image_part_select = false;

uint8_t block_offset = 0;

//when this flag is set to 1, it means it is the final bitmap chunk to be stored before being processed
static volatile uint8_t final_storage_bitmap_flag = 0;

uint32_t image_index = 0;
uint16_t frame_offset = 0;

static volatile uint16_t bitmap_stop_iteration = 0;
static volatile uint16_t bitmap_count_iteration = 0;

#define BITMAP_CHUNK_SIZE 128

/**@brief Function to make the ADC start a battery level conversion.
 */
static void adc_init(void)
{
    NRF_ADC->CONFIG = (ADC_CONFIG_EXTREFSEL_None << ADC_CONFIG_EXTREFSEL_Pos) /*!< Analog external reference inputs disabled. */
                      | (ADC_CONFIG_PSEL_AnalogInput5 << ADC_CONFIG_PSEL_Pos)
                      | (ADC_CONFIG_REFSEL_VBG << ADC_CONFIG_REFSEL_Pos)   /*!< Use internal 1.2V bandgap voltage as reference for conversion. */
                      | (ADC_CONFIG_INPSEL_AnalogInputOneThirdPrescaling << ADC_CONFIG_INPSEL_Pos) /*!< Analog input specified by PSEL with 1/3 prescaling used as input for the conversion. */
                      | (ADC_CONFIG_RES_10bit << ADC_CONFIG_RES_Pos);  /*!< 10bit ADC resolution. */
    // enable ADC
    NRF_ADC->ENABLE = 1; /* Bit 0 : ADC enable. */

    NRF_ADC->INTENSET = ADC_INTENSET_END_Msk;
    NVIC_SetPriority(ADC_IRQn, 1);
    NVIC_EnableIRQ(ADC_IRQn);
}

/**@brief ADC interrupt handler.
 * @details  This function will fetch the conversion result from the ADC, convert the value into
 *           percentage and send it to peer.
 */
void ADC_IRQHandler(void)
{
    NRF_ADC->EVENTS_END = 0;
    uint16_t adc_result = NRF_ADC->RESULT;
    NRF_ADC->TASKS_STOP = 1;

    uint8_t old_state = button_state;

    if (button_state == BUTTON_NONE) {
        if (adc_result < 62) button_state = BUTTON_DOWN;
        else if (adc_result < 295) button_state = BUTTON_RIGHT;
        else if (adc_result < 465) button_state = BUTTON_SELECT;
        else if (adc_result < 620) button_state = BUTTON_UP;
        else if (adc_result < 1020) button_state = BUTTON_LEFT;
    }
    else {
        if (adc_result > 1020) button_state = BUTTON_NONE;
    }
    if (old_state != button_state) {

        SEGGER_RTT_printf(0, "\x1B[32mbutton state : %s\x1B[0m\n", BUTTON_STATE_STRING_ENUM[button_state]);
        button_state_change = true;
    }
}

/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in]   line_num   Line number of the failing ASSERT call.
 * @param[in]   file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}


/**@brief Function for the LEDs initialization.
 *
 * @details Initializes all LEDs used by the application.
 */
static void leds_init(void)
{
    // Configure LED-pins as outputs.
    LEDS_CONFIGURE(BSP_LED_0_MASK | BSP_LED_1_MASK | BSP_LED_2_MASK | BSP_LED_3_MASK);
    LEDS_OFF(BSP_LED_0_MASK | BSP_LED_1_MASK | BSP_LED_2_MASK | BSP_LED_3_MASK);
    SEGGER_RTT_printf(0, "\x1B[32mled config : %d %d %d %d\x1B[0m\n", BSP_LED_0_MASK, BSP_LED_1_MASK, BSP_LED_2_MASK, BSP_LED_3_MASK);
}

// ADC timer handler to start ADC sampling
static void adc_sampling_timeout_handler(void * p_context)
{
    NRF_ADC->EVENTS_END  = 0;
    NRF_ADC->TASKS_START = 1;            //Start ADC sampling
}

static void dpad_timeout_handler(void * p_context) {

    if (button_state_change) {

        button_state_change = false;
        SEGGER_RTT_printf(0, "\x1B[32mbutton_state_change\x1B[0m\n");
        uint16_t err_code = ble_displays_on_button_change(&m_dis, button_state, &m_dis.dpad_handles);
        if (err_code != NRF_SUCCESS &&
                err_code != BLE_ERROR_INVALID_CONN_HANDLE &&
                err_code != NRF_ERROR_INVALID_STATE)
        {
            APP_ERROR_CHECK(err_code);
        }
    }
}

/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module.
 */
static void timers_init(void)
{
    // Initialize timer module, making it use the scheduler
    APP_TIMER_APPSH_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, true);

    uint32_t err_code = app_timer_create(&m_adc_sampling_timer_id,
                                         APP_TIMER_MODE_REPEATED,
                                         adc_sampling_timeout_handler);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_create(&m_dpad_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                dpad_timeout_handler);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for the GAP initialization.
 *
 * @details This function sets up all the necessary GAP (Generic Access Profile) parameters of the
 *          device including the device name, appearance, and the preferred connection parameters.
 */
static void gap_params_init(void)
{
    uint32_t                err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *)DEVICE_NAME,
                                          strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the Advertising functionality.
 *
 * @details Encodes the required advertising data and passes it to the stack.
 *          Also builds a structure to be passed to the stack when starting advertising.
 */
static void advertising_init(void)
{
    uint32_t      err_code;
    ble_advdata_t advdata;
    ble_advdata_t scanrsp;

    ble_uuid_t adv_uuids[] = {{DISPLAYS_UUID_SERVICE_BUTTON, m_dis.uuid_type}};

    // Build and set advertising data
    memset(&advdata, 0, sizeof(advdata));

    advdata.name_type               = BLE_ADVDATA_FULL_NAME;
    advdata.include_appearance      = true;
    advdata.flags                   = BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE;


    memset(&scanrsp, 0, sizeof(scanrsp));
    scanrsp.uuids_complete.uuid_cnt = sizeof(adv_uuids) / sizeof(adv_uuids[0]);
    scanrsp.uuids_complete.p_uuids  = adv_uuids;

    err_code = ble_advdata_set(&advdata, &scanrsp);
    APP_ERROR_CHECK(err_code);
}

static void full_color_handler(ble_displays_t * p_dis, uint8_t red, uint8_t green, uint8_t blue)
{
    uint16_t data = ((red >> 3) << 11) + ((green >> 2) << 5) + (blue >> 3);
    SEGGER_RTT_printf(0, "\x1B[32mset full color : %d;%d;%d to %d\x1B[0m\n", red, green, blue, data);
    fillScreen(data);
}

static void transmit_status_handler(ble_displays_t * p_dis, uint8_t transmit_status) {

    if ( transmit_status == TRANSMITTING) {
        if (transmit_state != TRANSMIT_CANCEL) {
            transmit_state = TRANSMITTING;
        }
    }
    else {
        transmit_state = transmit_status;
    }

    SEGGER_RTT_printf(0, "\x1B[32mset transmit status to : %s\x1B[0m\n", TRANSMIT_STATUS_STRING_ENUM[transmit_state]);

}

static void dispatch_transmit_status(uint8_t transmit_status) {

    uint16_t err_code = ble_displays_on_transmit_status_change(&m_dis, transmit_status, &m_dis.transmit_status_handles);
    if (err_code != NRF_SUCCESS &&
            err_code != BLE_ERROR_INVALID_CONN_HANDLE &&
            err_code != NRF_ERROR_INVALID_STATE)
    {
        APP_ERROR_CHECK(err_code);
    }
}

void clean_complete() {

    expecting_length = 0;
    transmit_state = TRANSMIT_COMPLETE;
    image_index = 0;
    transmit_init = false;
    pstorage_clear(&pstorage_handle, 40 * 1024);
    //send TRANSMIT_COMPLETE
    dispatch_transmit_status(TRANSMIT_COMPLETE);
}

static void bitmap_handler(ble_displays_t * p_dis, ble_gatts_evt_write_t * p_evt_write) {

    SEGGER_RTT_printf(0, "\x1B[32mbitmap handler\x1B[0m\n");

    if ((transmit_state == TRANSMITTING) || (transmit_state == TRANSMIT_START)) {

        if (!transmit_init) {

            if (p_evt_write->len == 2) {

                transmit_init = true;
                bitmap_length = (p_evt_write->data[0] << 8) + p_evt_write->data[1];
                block_offset = 0;
                bitmap_offset = 0;
                bitmap_count_iteration = 0;

                free(image_part);
                image_part = NULL;
                image_part = (uint8_t*)malloc(sizeof(uint8_t) * 1024);

                free(image_part2);
                image_part2 = NULL;
                image_part2 = (uint8_t*)malloc(sizeof(uint8_t) * 1024);

                bitmap_stop_iteration = (bitmap_length / 18) + 1;

                image_index = 0;

                expecting_length = 127;
                SEGGER_RTT_printf(0, "\x1B[32mreceive total length : %d\x1B[0m\n", bitmap_length);
            }
            else {
                SEGGER_RTT_printf(0, "\x1B[32mError expecting data of size 2 (bitmap length)\x1B[0m\n");
                dispatch_transmit_status(TRANSMIT_ERROR);
            }
        }
        else {

            if (expecting_length == 0) {
                uint32_t subs = bitmap_stop_iteration - bitmap_count_iteration;
                expecting_length = (subs >= BITMAP_CHUNK_SIZE) ? BITMAP_CHUNK_SIZE : subs;
                SEGGER_RTT_printf(0, "\x1B[32mreinitializing expecting length to %d\x1B[0m\n", expecting_length);
            }

            SEGGER_RTT_printf(0, "\x1B[32madding values, image_index : %d; expecting_length : %d; length : %d\x1B[0m\n", image_index, expecting_length, p_evt_write->len);

            for (uint16_t i = 0; i < p_evt_write->len; i++) {

                if (frame_offset == 1024) {

                    SEGGER_RTT_printf(0, "\x1B[32mSTORING\x1B[0m\n");

                    uint32_t retval = store_data_pstorage();

                    if (retval == NRF_SUCCESS)
                    {
                        SEGGER_RTT_printf(0, "\x1B[32mpstorage_store SUCCESS\x1B[0m\n");
                        // Store successfully requested. Wait for operation result.
                    }
                    else
                    {
                        SEGGER_RTT_printf(0, "\x1B[32mpstorage_store FAILURE\x1B[0m\n");
                        // Failed to request store, take corrective action.
                    }
                    frame_offset = 0;
                }

                if (!image_part_select) {
                    image_part[frame_offset++] = p_evt_write->data[i];
                }
                else {
                    image_part2[frame_offset++] = p_evt_write->data[i];
                }
            }

            bitmap_count_iteration++;
            image_index += p_evt_write->len;
            expecting_length--;

            SEGGER_RTT_printf(0, "\x1B[32mpimage_index : %d et bitmap_length : %d et expecting_length : %d\x1B[0m\n", image_index, bitmap_length, expecting_length);

            if ((bitmap_count_iteration == bitmap_stop_iteration) || (image_index == bitmap_length)) {

                if (frame_offset != 0 || (image_index == bitmap_length)) {

                    //set flags for last chunk
                    final_storage_bitmap_flag = 1;
                    //store last frames in pstorage
                    uint32_t retval = store_data_pstorage();

                    if (retval == NRF_SUCCESS)
                    {
                        SEGGER_RTT_printf(0, "\x1B[32mpstorage_store SUCCESS\x1B[0m\n");
                        SEGGER_RTT_printf(0, "\x1B[32mFINISHED WAITING FOR STORAGE CALLBACK\x1B[0m\n");
                        // Store successfully requested. Wait for operation result.
                    }
                    else
                    {
                        SEGGER_RTT_printf(0, "\x1B[32mpstorage_store FAILURE\x1B[0m\n");
                        // Failed to request store, take corrective action.
                    }
                    /*
                    if (final_storage_bitmap_flag == 1) {

                    }
                    */

                    frame_offset = 0;
                }

                if (image_index == bitmap_length) {
                    expecting_length = 0;
                }
            }

            if (expecting_length == 0) {

                SEGGER_RTT_printf(0, "\x1B[32mReceived 128 frames.transmitting OK %d & %d\x1B[0m\n", image_index, bitmap_length);
                transmit_state = TRANSMIT_OK;

                //send TRANSMIT_OK
                dispatch_transmit_status(TRANSMIT_OK);

                if (image_index == bitmap_length) {

                    SEGGER_RTT_printf(0, "\x1B[32mReceived ALL frames.\x1B[0m\n");

                    free(image_part);
                    image_part = 0;
                    free(image_part2);
                    image_part2 = 0;
                }

            }
        }
    }
    else if (transmit_state == TRANSMIT_CANCEL) {

        SEGGER_RTT_printf(0, "\x1B[32mCANCEL TRANSMISSION\x1B[0m\n");
        frame_offset = 0;
        free(image_part);
        image_part = 0;
        free(image_part2);
        image_part2 = 0;
        clean_complete();
    }
}

static void led_write_handler(ble_displays_t * p_dis, uint8_t led_state)
{
    if (led_state & 0b00000001)
        nrf_gpio_pin_clear(BSP_LED_0);
    else
        nrf_gpio_pin_set(BSP_LED_0);

    if (led_state & 0b00000010)
        nrf_gpio_pin_clear(BSP_LED_1);
    else
        nrf_gpio_pin_set(BSP_LED_1);

    if (led_state & 0b00000100)
        nrf_gpio_pin_clear(BSP_LED_2);
    else
        nrf_gpio_pin_set(BSP_LED_2);

    if (led_state & 0b00001000)
        nrf_gpio_pin_clear(BSP_LED_3);
    else
        nrf_gpio_pin_set(BSP_LED_3);
}

/**@brief Function for initializing services that will be used by the application.
 */
static void services_init(void)
{
    uint32_t err_code;
    ble_displays_init_t init;

    init.led_write_handler       = led_write_handler;
    init.full_color_handler      = full_color_handler;
    init.bitmap_handler          = bitmap_handler;
    init.transmit_status_handler = transmit_status_handler;

    err_code = ble_displays_init(&m_dis, &init);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing security parameters.
 */
static void sec_params_init(void)
{

    m_sec_params.bond         = SEC_PARAM_BOND;
    m_sec_params.mitm         = SEC_PARAM_MITM;
    m_sec_params.io_caps      = SEC_PARAM_IO_CAPABILITIES;
    m_sec_params.oob          = SEC_PARAM_OOB;
    m_sec_params.min_key_size = SEC_PARAM_MIN_KEY_SIZE;
    m_sec_params.max_key_size = SEC_PARAM_MAX_KEY_SIZE;
}

/**@brief Function for handling the Connection Parameters Module.
 *
 * @details This function will be called for all events in the Connection Parameters Module which
 *          are passed to the application.
 *          @note All this function does is to disconnect. This could have been done by simply
 *                setting the disconnect_on_fail config parameter, but instead we use the event
 *                handler mechanism to demonstrate its use.
 *
 * @param[in]   p_evt   Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    uint32_t err_code;

    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}

/**@brief Function for handling a Connection Parameters error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init(void)
{
    uint32_t               err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for starting timers.
*/
static void timers_start(void)
{
    //ADC timer start
    uint32_t err_code = app_timer_start(m_adc_sampling_timer_id, ADC_SAMPLING_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);

    //dpad timer start
    err_code = app_timer_start(m_dpad_timer_id, BUTTON_DETECTION_DELAY, NULL);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for starting advertising.
 */
static void advertising_start(void)
{
    uint32_t             err_code;
    ble_gap_adv_params_t adv_params;

    // Start advertising
    memset(&adv_params, 0, sizeof(adv_params));

    adv_params.type        = BLE_GAP_ADV_TYPE_ADV_IND;
    adv_params.p_peer_addr = NULL;
    adv_params.fp          = BLE_GAP_ADV_FP_ANY;
    adv_params.interval    = APP_ADV_INTERVAL;
    adv_params.timeout     = APP_ADV_TIMEOUT_IN_SECONDS;

    err_code = sd_ble_gap_adv_start(&adv_params);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling the Application's BLE Stack events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 */
static void on_ble_evt(ble_evt_t * p_ble_evt)
{
    uint32_t                         err_code;
    //static ble_gap_evt_auth_status_t m_auth_status;
    static ble_gap_master_id_t p_master_id;
    static ble_gap_sec_keyset_t keys_exchanged;

    switch (p_ble_evt->header.evt_id)
    {
    case BLE_GAP_EVT_CONNECTED:
        m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;

        err_code = app_button_enable();
        APP_ERROR_CHECK(err_code);
        break;

    case BLE_GAP_EVT_DISCONNECTED:
        m_conn_handle = BLE_CONN_HANDLE_INVALID;

        err_code = app_button_disable();
        APP_ERROR_CHECK(err_code);

        advertising_start();
        break;

    case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
        err_code = sd_ble_gap_sec_params_reply(m_conn_handle,
                                               BLE_GAP_SEC_STATUS_SUCCESS,
                                               &m_sec_params, &keys_exchanged);
        APP_ERROR_CHECK(err_code);
        break;

    case BLE_GATTS_EVT_SYS_ATTR_MISSING:
        err_code = sd_ble_gatts_sys_attr_set(m_conn_handle, NULL, 0, BLE_GATTS_SYS_ATTR_FLAG_USR_SRVCS);
        APP_ERROR_CHECK(err_code);
        break;

    case BLE_GAP_EVT_AUTH_STATUS:
        //m_auth_status = p_ble_evt->evt.gap_evt.params.auth_status;
        break;

    case BLE_GAP_EVT_SEC_INFO_REQUEST:
        //p_enc_info = keys_exchanged.keys_central.p_enc_key

        if (p_master_id.ediv == p_ble_evt->evt.gap_evt.params.sec_info_request.master_id.ediv)
        {
            err_code = sd_ble_gap_sec_info_reply(m_conn_handle, &keys_exchanged.keys_peer.p_enc_key->enc_info, &keys_exchanged.keys_peer.p_id_key->id_info, NULL);
            APP_ERROR_CHECK(err_code);
            p_master_id.ediv = p_ble_evt->evt.gap_evt.params.sec_info_request.master_id.ediv;
        }
        else
        {
            // No keys found for this device
            err_code = sd_ble_gap_sec_info_reply(m_conn_handle, NULL, NULL, NULL);
            APP_ERROR_CHECK(err_code);
        }
        break;
    /*
       case BLE_GAP_EVT_TIMEOUT:

           if (p_ble_evt->evt.gap_evt.params.timeout.src == BLE_GAP_TIMEOUT_SRC_ADVERTISING)
           {
               nrf_gpio_pin_clear(ADVERTISING_LED_PIN_NO);

               // Configure buttons with sense level low as wakeup source.
               nrf_gpio_cfg_sense_input(BUTTON_S1,
                                        BUTTON_PULL,
                                        NRF_GPIO_PIN_SENSE_LOW);

               // Go to system-off mode (this function will not return; wakeup will cause a reset)
               err_code = sd_power_system_off();
               APP_ERROR_CHECK(err_code);
           }

           break;
    */
    default:
        // No implementation needed.
        break;
    }
}

/**@brief Function for dispatching a BLE stack event to all modules with a BLE stack event handler.
 *
 * @details This function is called from the scheduler in the main loop after a BLE stack
 *          event has been received.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 */
static void ble_evt_dispatch(ble_evt_t * p_ble_evt)
{
    on_ble_evt(p_ble_evt);
    ble_conn_params_on_ble_evt(p_ble_evt);
    ble_displays_on_ble_evt(&m_dis, p_ble_evt);
}

/**@brief Function for dispatching a system event to interested modules.
 *
 * @details This function is called from the System event interrupt handler after a system
 *          event has been received.
 *
 * @param[in]   sys_evt   System stack event.
 */
static void sys_evt_dispatch(uint32_t sys_evt)
{
    pstorage_sys_event_handler(sys_evt);
}

void ble_stack_init(void)
{
    uint32_t err_code;
    nrf_clock_lf_cfg_t clock_lf_cfg = NRF_CLOCK_LFCLKSRC;

    SOFTDEVICE_HANDLER_INIT(&clock_lf_cfg, NULL);

    // Enable BLE stack
    ble_enable_params_t ble_enable_params;
    err_code = softdevice_enable_get_default_config(0, 1, &ble_enable_params);
    APP_ERROR_CHECK(err_code);

    err_code = softdevice_enable(&ble_enable_params);
    APP_ERROR_CHECK(err_code);

    err_code = softdevice_ble_evt_handler_set(ble_evt_dispatch);
    APP_ERROR_CHECK(err_code);

    err_code = softdevice_sys_evt_handler_set(sys_evt_dispatch);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for the Event Scheduler initialization.
 */
static void scheduler_init(void)
{
    APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);
}


static void button_event_handler(uint8_t pin_no, uint8_t button_action)
{
    uint32_t err_code;

    switch (pin_no)
    {
    case BUTTON_S0:
        err_code = ble_displays_on_button_change(&m_dis, button_action, &m_dis.button1_handles);
        if (err_code != NRF_SUCCESS &&
                err_code != BLE_ERROR_INVALID_CONN_HANDLE &&
                err_code != NRF_ERROR_INVALID_STATE)
        {
            APP_ERROR_CHECK(err_code);
        }
        break;

    case BUTTON_S1:
        err_code = ble_displays_on_button_change(&m_dis, button_action, &m_dis.button2_handles);
        if (err_code != NRF_SUCCESS &&
                err_code != BLE_ERROR_INVALID_CONN_HANDLE &&
                err_code != NRF_ERROR_INVALID_STATE)
        {
            APP_ERROR_CHECK(err_code);
        }
        break;

    case BUTTON_S2:
        err_code = ble_displays_on_button_change(&m_dis, button_action, &m_dis.button3_handles);
        if (err_code != NRF_SUCCESS &&
                err_code != BLE_ERROR_INVALID_CONN_HANDLE &&
                err_code != NRF_ERROR_INVALID_STATE)
        {
            APP_ERROR_CHECK(err_code);
        }
        break;

    case BUTTON_S3:
        err_code = ble_displays_on_button_change(&m_dis, button_action, &m_dis.button4_handles);
        if (err_code != NRF_SUCCESS &&
                err_code != BLE_ERROR_INVALID_CONN_HANDLE &&
                err_code != NRF_ERROR_INVALID_STATE)
        {
            APP_ERROR_CHECK(err_code);
        }
        break;
    default:
        //APP_ERROR_HANDLER(pin_no);
        break;
    }
}

/**@brief Function for initializing the GPIOTE handler module.
 */
static void gpiote_init(void)
{
    APP_GPIOTE_INIT(APP_GPIOTE_MAX_USERS);
}


/**@brief Function for initializing the button handler module.
 */
static void buttons_init(void)
{
    // Note: Array must be static because a pointer to it will be saved in the Button handler
    //       module.
    static app_button_cfg_t buttons[] =
    {
        {BUTTON_S0, false, BUTTON_PULL, button_event_handler},
        {BUTTON_S1, false, BUTTON_PULL, button_event_handler},
        {BUTTON_S2, false, BUTTON_PULL, button_event_handler},
        {BUTTON_S3, false, BUTTON_PULL, button_event_handler}
    };

    app_button_init(buttons, sizeof(buttons) / sizeof(buttons[0]), BUTTON_DETECTION_DELAY);
}


/**@brief Function for the Power manager.
 */
static void power_manage(void)
{
    uint32_t err_code = sd_app_evt_wait();
    APP_ERROR_CHECK(err_code);
}

/**
 * @brief Function for initializing bsp module.
 */

void bsp_configuration() {

    //uint32_t err_code = NRF_SUCCESS;

    NRF_CLOCK->LFCLKSRC            = (CLOCK_LFCLKSRC_SRC_Xtal << CLOCK_LFCLKSRC_SRC_Pos);
    NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_LFCLKSTART    = 1;

    while (NRF_CLOCK->EVENTS_LFCLKSTARTED == 0) {
        // Do nothing.
    }
}

static void pstorage_cb_handler(pstorage_handle_t  * handle,
                                uint8_t              op_code,
                                uint32_t             result,
                                uint8_t            * p_data,
                                uint32_t             data_len)
{
    switch (op_code)
    {
    case PSTORAGE_LOAD_OP_CODE:
        if (result == NRF_SUCCESS)
        {
            // Store operation successful.
            SEGGER_RTT_printf(0, "\x1B[32mPSTORAGE_LOAD_OP_CODE SUCCESS\x1B[0m\n");
        }
        else
        {
            // Store operation failed.
            SEGGER_RTT_printf(0, "\x1B[32mPSTORAGE_LOAD_OP_CODE FAILURE\x1B[0m\n");
        }
        // Source memory can now be reused or freed.
        break;

    case PSTORAGE_STORE_OP_CODE :
        if (result == NRF_SUCCESS)
        {
            // Store operation successful.
            SEGGER_RTT_printf(0, "\x1B[32mPSTORAGE_STORE_OP_CODE SUCCESS\x1B[0m\n");

            if (final_storage_bitmap_flag == 1) {
                SEGGER_RTT_printf(0, "\x1B[32mPROCESSING\x1B[0m\n");
                //clear flags
                final_storage_bitmap_flag = 0;
                SEGGER_RTT_printf(0, "\x1B[32mPSTORAGE_STORE_OP_CODE SUCCESS received. Processing bitmap ...\x1B[0m\n");
                unpack_file();
                clean_complete();
            }
        }
        else
        {
            // Store operation failed.
            SEGGER_RTT_printf(0, "\x1B[32mPSTORAGE_STORE_OP_CODE FAILURE\x1B[0m\n");

            if (final_storage_bitmap_flag == 1) {
                final_storage_bitmap_flag = 0;
                clean_complete();
            }
        }
        // Source memory can now be reused or freed.
        break;
    case PSTORAGE_UPDATE_OP_CODE:
        if (result == NRF_SUCCESS)
        {
            // Update operation successful.
            SEGGER_RTT_printf(0, "\x1B[32mPSTORAGE_UPDATE_OP_CODE SUCCESS\x1B[0m\n");
        }
        else
        {
            // Update operation failed.
            SEGGER_RTT_printf(0, "\x1B[32mPSTORAGE_UPDATE_OP_CODE FAILURE\x1B[0m\n");
        }
        break;
    case PSTORAGE_CLEAR_OP_CODE:
        if (result == NRF_SUCCESS)
        {
            // Clear operation successful.
            SEGGER_RTT_printf(0, "\x1B[32mPSTORAGE_CLEAR_OP_CODE SUCCESS\x1B[0m\n");
        }
        else
        {
            // Clear operation failed.
            SEGGER_RTT_printf(0, "\x1B[32mPSTORAGE_CLEAR_OP_CODE FAILURE\x1B[0m\n");
        }
        break;
    }
}

uint32_t pstorage_initialize() {

    uint32_t retval;
    pstorage_module_param_t param;

    retval = pstorage_init();

    if (retval == NRF_SUCCESS)
    {
        SEGGER_RTT_printf(0, "\x1B[32m1\x1B[0m\n");
        //param.block_size  = 2304;
        //param.block_count = 18;
        param.block_size  = 1024;
        param.block_count = 40;
        param.cb          = pstorage_cb_handler;

        retval = pstorage_register(&param, &pstorage_handle);

        if ( retval == NRF_SUCCESS) {

            retval = pstorage_clear(&pstorage_handle, 40 * 1024);
        }
    }
    return retval;
}

/**@brief Function for application main entry.
 */
int main(void)
{
    bsp_configuration();
    // Initialize
    timers_init();
    gpiote_init();
    adc_init();
    buttons_init();

    tft_setup();
    fillScreen(ST7735_BLACK);

    ble_stack_init();
    scheduler_init();
    gap_params_init();
    services_init();
    advertising_init();
    conn_params_init();
    sec_params_init();

    leds_init();
    // Start execution
    timers_start();

    uint32_t pstorage_init = pstorage_initialize();

    if (pstorage_init == NRF_SUCCESS) {
        SEGGER_RTT_printf(0, "\x1B[32mPSTORAGE INIT SUCCESS\x1B[0m\n");
    }
    else {
        SEGGER_RTT_printf(0, "\x1B[32mPSTORAGE INIT FAILURE\x1B[0m\n");
    }

    advertising_start();

    // Enter main loop
    for (;;)
    {
        app_sched_execute();
        power_manage();
    }
}

/**
 * @}
 */