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
#include "nrf_assert.h"
#include "app_error.h"
#include "nrf_gpio.h"
#include "nrf51_bitfields.h"
#include "ble_hci.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
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
#include "bsp_btn_ble.h"
#include "SEGGER_RTT.h"
#include "adafruit1_8_oled_library.h"
#include "bsp.h"
#include "nrf_delay.h"
#include "unpack.h"
#include "device_manager.h"
#include "ble_hids.h"

#define APP_ADV_FAST_INTERVAL            0x0028                                         /**< Fast advertising interval (in units of 0.625 ms. This value corresponds to 25 ms.). */
#define APP_ADV_SLOW_INTERVAL            0x0C80                                         /**< Slow advertising interval (in units of 0.625 ms. This value corrsponds to 2 seconds). */
#define APP_ADV_FAST_TIMEOUT             30                                             /**< The duration of the fast advertising period (in seconds). */
#define APP_ADV_SLOW_TIMEOUT             180                                            /**< The duration of the slow advertising period (in seconds). */

#define APP_FEATURE_NOT_SUPPORTED       BLE_GATT_STATUS_ATTERR_APP_BEGIN + 2            /**< Reply when unsupported features are requested. */

#define OUTPUT_REPORT_INDEX              0                                              /**< Index of Output Report. */
#define OUTPUT_REPORT_MAX_LEN            1                                              /**< Maximum length of Output Report. */
#define INPUT_REPORT_KEYS_INDEX          0                                              /**< Index of Input Report. */
#define OUTPUT_REPORT_BIT_MASK_CAPS_LOCK 0x02                                           /**< CAPS LOCK bit in Output Report (based on 'LED Page (0x08)' of the Universal Serial Bus HID Usage Tables). */
#define INPUT_REP_REF_ID                 0                                              /**< Id of reference to Keyboard Input Report. */
#define OUTPUT_REP_REF_ID                0                                              /**< Id of reference to Keyboard Output Report. */
#define INPUT_REPORT_KEYS_MAX_LEN        8                                              /**< Maximum length of the Input Report characteristic. */

static void on_hids_evt(ble_hids_t * p_hids, ble_hids_evt_t * p_evt);

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

#define PAGE_CHUNK 1024
#define PAGE_NUM   39

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

static bool                              m_in_boot_mode = false;                    /**< Current protocol mode. */
static dm_handle_t                       m_bonded_peer_handle;                          /**< Device reference handle to the current bonded central. */
#define BASE_USB_HID_SPEC_VERSION        0x0101                                         /**< Version number of base USB HID Specification implemented by this application. */
#define MAX_BUFFER_ENTRIES               5                                              /**< Number of elements that can be enqueued */
#define MODIFIER_KEY_POS                 0                                              /**< Position of the modifier byte in the Input Report. */
#define SCAN_CODE_POS                    2                                              /**< This macro indicates the start position of the key scan code in a HID Report. As per the document titled 'Device Class Definition for Human Interface Devices (HID) V1.11, each report shall have one modifier byte followed by a reserved constant byte and then the key scan code. */
#define SHIFT_KEY_CODE                   0x02                                           /**< Key code indicating the press of the Shift Key. */
#define SEC_PARAM_LESC                   0                                              /**< LE Secure Connections not enabled. */
#define SEC_PARAM_KEYPRESS               0                                              /**< Keypress notifications not enabled. */

static dm_application_instance_t         m_app_handle;                                  /**< Application identifier allocated by device manager. */
#define MAX_KEYS_IN_ONE_REPORT           (INPUT_REPORT_KEYS_MAX_LEN - SCAN_CODE_POS)    /**< Maximum number of key presses that can be sent in one Input Report. */

#define SCHED_MAX_EVENT_DATA_SIZE       sizeof(app_timer_event_t)                   /**< Maximum size of scheduler events. Note that scheduler BLE stack events do not contain any data, as the events are being pulled from the stack in the event handler. */
#define SCHED_QUEUE_SIZE                10                                          /**< Maximum number of events in the scheduler queue. */

/** Provide status of data list is full or not */
#define BUFFER_LIST_FULL()\
        ((MAX_BUFFER_ENTRIES == buffer_list.count - 1) ? true : false)

APP_TIMER_DEF(m_adc_sampling_timer_id);
APP_TIMER_DEF(m_dpad_timer_id);

static ble_hids_t                        m_hids;                                        /**< Structure used to identify the HID service. */
static bool                              m_caps_on = false;                             /**< Variable to indicate if Caps Lock is turned on. */
static uint8_t m_caps_on_key_scan_str[] =                                                /**< Key pattern to be sent when the output report has been written with the CAPS LOCK bit set. */
{
    0x06, /* Key C */
    0x04, /* Key a */
    0x13, /* Key p */
    0x16, /* Key s */
    0x12, /* Key o */
    0x11, /* Key n */
};

static uint8_t m_caps_off_key_scan_str[] =                                               /**< Key pattern to be sent when the output report has been written with the CAPS LOCK bit cleared. */
{
    0x06, /* Key C */
    0x04, /* Key a */
    0x13, /* Key p */
    0x16, /* Key s */
    0x12, /* Key o */
    0x09, /* Key f */
};

static uint8_t m_sample_key_press_scan_str[] =                                          /**< Key pattern to be sent when the key press button has been pushed. */
{
    0x0b, /* Key h */
    0x08, /* Key e */
    0x0f, /* Key l */
    0x0f, /* Key l */
    0x12, /* Key o */
    0x28  /* Key Return */
};

/** Abstracts buffer element */
typedef struct hid_key_buffer
{
    uint8_t    data_offset;   /**< Max Data that can be buffered for all entries */
    uint8_t    data_len;      /**< Total length of data */
    uint8_t    * p_data;      /**< Scanned key pattern */
    ble_hids_t * p_instance;  /**< Identifies peer and service instance */
} buffer_entry_t;

STATIC_ASSERT(sizeof(buffer_entry_t) % 4 == 0);

/** Circular buffer list */
typedef struct
{
    buffer_entry_t buffer[MAX_BUFFER_ENTRIES]; /**< Maximum number of entries that can enqueued in the list */
    uint8_t        rp;                         /**< Index to the read location */
    uint8_t        wp;                         /**< Index to write location */
    uint8_t        count;                      /**< Number of elements in the list */
} buffer_list_t;

STATIC_ASSERT(sizeof(buffer_list_t) % 4 == 0);

/** List to enqueue not just data to be sent, but also related information like the handle, connection handle etc */
static buffer_list_t buffer_list;

/**Buffer queue access macros
 *
 * @{ */
/** Initialization of buffer list */
#define BUFFER_LIST_INIT()                                                                        \
        do                                                                                        \
        {                                                                                         \
            buffer_list.rp = 0;                                                                   \
            buffer_list.wp = 0;                                                                   \
            buffer_list.count = 0;                                                                \
        } while (0)

/** Provide status of data list is full or not */
#define BUFFER_LIST_FULL()\
        ((MAX_BUFFER_ENTRIES == buffer_list.count - 1) ? true : false)

/** Provides status of buffer list is empty or not */
#define BUFFER_LIST_EMPTY()\
        ((0 == buffer_list.count) ? true : false)

#define BUFFER_ELEMENT_INIT(i)\
        do                                                                                        \
        {                                                                                         \
            buffer_list.buffer[(i)].p_data = NULL;                                                \
        } while (0)

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

bool block_upload = false;

//when this flag is set to 1, it means it is the final bitmap chunk to be stored before being processed
static volatile uint8_t final_storage_bitmap_flag = 0;

uint32_t image_index = 0;
uint16_t frame_offset = 0;

static volatile uint16_t bitmap_stop_iteration = 0;
static volatile uint16_t bitmap_count_iteration = 0;

#define BITMAP_CHUNK_SIZE 128

/**@brief Function for handling Service errors.
 *
 * @details A pointer to this function will be passed to each service which may need to inform the
 *          application about an error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void service_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

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


/**@brief Function for enqueuing key scan patterns that could not be transmitted either completely
 *        or partially.
 *
 * @warning This handler is an example only. You need to analyze how you wish to send the key
 *          release.
 *
 * @param[in]  p_hids         Identifies the service for which Key Notifications are buffered.
 * @param[in]  p_key_pattern  Pointer to key pattern.
 * @param[in]  pattern_len    Length of key pattern.
 * @param[in]  offset         Offset applied to Key Pattern when requesting a transmission on
 *                            dequeue, @ref buffer_dequeue.
 * @return     NRF_SUCCESS on success, else an error code indicating reason for failure.
 */
static uint32_t buffer_enqueue(ble_hids_t *            p_hids,
                               uint8_t *               p_key_pattern,
                               uint16_t                pattern_len,
                               uint16_t                offset)
{
    buffer_entry_t * element;
    uint32_t         err_code = NRF_SUCCESS;

    if (BUFFER_LIST_FULL())
    {
        // Element cannot be buffered.
        err_code = NRF_ERROR_NO_MEM;
    }
    else
    {
        // Make entry of buffer element and copy data.
        element                 = &buffer_list.buffer[(buffer_list.wp)];
        element->p_instance     = p_hids;
        element->p_data         = p_key_pattern;
        element->data_offset    = offset;
        element->data_len       = pattern_len;

        buffer_list.count++;
        buffer_list.wp++;

        if (buffer_list.wp == MAX_BUFFER_ENTRIES)
        {
            buffer_list.wp = 0;
        }
    }

    return err_code;
}


/** @brief   Function for checking if the Shift key is pressed.
 *
 *  @returns true if the SHIFT_BUTTON is pressed. false otherwise.
 */
static bool is_shift_key_pressed(void)
{
    //bool result;
    //uint32_t err_code = bsp_button_is_pressed(SHIFT_BUTTON_ID,&result);
    //APP_ERROR_CHECK(err_code);
    return false;
}

/**@brief   Function for transmitting a key scan Press & Release Notification.
 *
 * @warning This handler is an example only. You need to analyze how you wish to send the key
 *          release.
 *
 * @param[in]  p_instance     Identifies the service for which Key Notifications are requested.
 * @param[in]  p_key_pattern  Pointer to key pattern.
 * @param[in]  pattern_len    Length of key pattern. 0 < pattern_len < 7.
 * @param[in]  pattern_offset Offset applied to Key Pattern for transmission.
 * @param[out] actual_len     Provides actual length of Key Pattern transmitted, making buffering of
 *                            rest possible if needed.
 * @return     NRF_SUCCESS on success, BLE_ERROR_NO_TX_PACKETS in case transmission could not be
 *             completed due to lack of transmission buffer or other error codes indicating reason
 *             for failure.
 *
 * @note       In case of BLE_ERROR_NO_TX_PACKETS, remaining pattern that could not be transmitted
 *             can be enqueued \ref buffer_enqueue function.
 *             In case a pattern of 'cofFEe' is the p_key_pattern, with pattern_len as 6 and
 *             pattern_offset as 0, the notifications as observed on the peer side would be
 *             1>    'c', 'o', 'f', 'F', 'E', 'e'
 *             2>    -  , 'o', 'f', 'F', 'E', 'e'
 *             3>    -  ,   -, 'f', 'F', 'E', 'e'
 *             4>    -  ,   -,   -, 'F', 'E', 'e'
 *             5>    -  ,   -,   -,   -, 'E', 'e'
 *             6>    -  ,   -,   -,   -,   -, 'e'
 *             7>    -  ,   -,   -,   -,   -,  -
 *             Here, '-' refers to release, 'c' refers to the key character being transmitted.
 *             Therefore 7 notifications will be sent.
 *             In case an offset of 4 was provided, the pattern notifications sent will be from 5-7
 *             will be transmitted.
 */
static uint32_t send_key_scan_press_release(ble_hids_t *   p_hids,
        uint8_t *      p_key_pattern,
        uint16_t       pattern_len,
        uint16_t       pattern_offset,
        uint16_t *     p_actual_len)
{
    uint32_t err_code;
    uint16_t offset;
    uint16_t data_len;
    uint8_t  data[INPUT_REPORT_KEYS_MAX_LEN];

    // HID Report Descriptor enumerates an array of size 6, the pattern hence shall not be any
    // longer than this.
    STATIC_ASSERT((INPUT_REPORT_KEYS_MAX_LEN - 2) == 6);

    ASSERT(pattern_len <= (INPUT_REPORT_KEYS_MAX_LEN - 2));

    offset   = pattern_offset;
    data_len = pattern_len;

    do
    {
        SEGGER_RTT_printf(0, "\x1B[32mtest %d\x1B[0m\n", sizeof(data));
        SEGGER_RTT_printf(0, "\x1B[32mtest22 %d\x1B[0m\n", sizeof(data));

        // Reset the data buffer.
        memset(data, 0, sizeof(data));
        SEGGER_RTT_printf(0, "\x1B[32mtest2 %d\x1B[0m\n", sizeof(data));
        SEGGER_RTT_printf(0, "\x1B[32mtest : %d %d %d %d\x1B[0m\n", SCAN_CODE_POS, offset, data_len, p_key_pattern[0]);

        // Copy the scan code.
        memcpy(data + SCAN_CODE_POS + offset, p_key_pattern + offset, data_len - offset);

        SEGGER_RTT_printf(0, "\x1B[32mtest1\x1B[0m\n");

        if (is_shift_key_pressed())
        {
            data[MODIFIER_KEY_POS] |= SHIFT_KEY_CODE;
        }

        if (!m_in_boot_mode)
        {

            SEGGER_RTT_printf(0, "\x1B[32mtest2\x1B[0m\n");

            err_code = ble_hids_inp_rep_send(p_hids,
                                             INPUT_REPORT_KEYS_INDEX,
                                             INPUT_REPORT_KEYS_MAX_LEN,
                                             data);
        }
        else
        {
            SEGGER_RTT_printf(0, "\x1B[32mtest3\x1B[0m\n");

#if 0
            err_code = ble_hids_boot_kb_inp_rep_send(p_hids,
                       INPUT_REPORT_KEYS_MAX_LEN,
                       data);
#endif
        }

        /*
        if (err_code != NRF_SUCCESS)
        {
            break;
        }
        */

        offset++;
    } while (offset <= data_len);

    *p_actual_len = offset;

    return err_code;
}

/**@brief Function for sending sample key presses to the peer.
 *
 * @param[in]   key_pattern_len   Pattern length.
 * @param[in]   p_key_pattern     Pattern to be sent.
 */
static void keys_send(uint8_t key_pattern_len, uint8_t * p_key_pattern)
{
    uint32_t err_code;
    uint16_t actual_len;

    err_code = send_key_scan_press_release(&m_hids,
                                           p_key_pattern,
                                           key_pattern_len,
                                           0,
                                           &actual_len);
    // An additional notification is needed for release of all keys, therefore check
    // is for actual_len <= key_pattern_len and not actual_len < key_pattern_len.
    if ((err_code == BLE_ERROR_NO_TX_PACKETS) && (actual_len <= key_pattern_len))
    {
        // Buffer enqueue routine return value is not intentionally checked.
        // Rationale: Its better to have a a few keys missing than have a system
        // reset. Recommendation is to work out most optimal value for
        // MAX_BUFFER_ENTRIES to minimize chances of buffer queue full condition
        UNUSED_VARIABLE(buffer_enqueue(&m_hids, p_key_pattern, key_pattern_len, actual_len));
    }


    if ((err_code != NRF_SUCCESS) &&
            (err_code != NRF_ERROR_INVALID_STATE) &&
            (err_code != BLE_ERROR_NO_TX_PACKETS) &&
            (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING)
       )
    {
        APP_ERROR_HANDLER(err_code);
    }
}

static void dpad_timeout_handler(void * p_context) {

    if (button_state_change) {

        button_state_change = false;
        SEGGER_RTT_printf(0, "\x1B[32mbutton_state_change\x1B[0m\n");
        uint16_t err_code = ble_displays_on_button_change(&m_dis, button_state, &m_dis.dpad_handles);

        uint8_t p_key_pattern[1];

        switch (button_state) {
        case BUTTON_DOWN:
        {
            p_key_pattern[0] = 0x51;
            keys_send(1, p_key_pattern);
        }
        case BUTTON_RIGHT:
        {
            p_key_pattern[0] = 0x4F;
            keys_send(1, p_key_pattern);
        }
        case BUTTON_SELECT:
        {
            p_key_pattern[0] = 0x28;
            keys_send(1, p_key_pattern);
        }
        case BUTTON_UP:
        {
            p_key_pattern[0] = 0x52;
            keys_send(1, p_key_pattern);
        }
        case BUTTON_LEFT:
        {
            p_key_pattern[0] = 0x50;
            keys_send(1, p_key_pattern);
        }
        }

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
/*
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
*/
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

    err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_HID_KEYBOARD);
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for putting the chip into sleep mode.
 *
 * @note This function will not return.
 */
static void sleep_mode_enter(void)
{
    uint32_t err_code = bsp_indication_set(BSP_INDICATE_IDLE);
    APP_ERROR_CHECK(err_code);

    // Prepare wakeup buttons.
    err_code = bsp_btn_ble_sleep_mode_prepare();
    APP_ERROR_CHECK(err_code);

    // Go to system-off mode (this function will not return; wakeup will cause a reset).
    err_code = sd_power_system_off();
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling advertising errors.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void ble_advertising_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

/**@brief Function for handling advertising events.
 *
 * @details This function will be called for advertising events which are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    uint32_t err_code;

    switch (ble_adv_evt)
    {
    case BLE_ADV_EVT_DIRECTED:
        err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING_DIRECTED);
        APP_ERROR_CHECK(err_code);
        break;
    case BLE_ADV_EVT_FAST:
        err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
        APP_ERROR_CHECK(err_code);
        break;
    case BLE_ADV_EVT_SLOW:
        err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING_SLOW);
        APP_ERROR_CHECK(err_code);
        break;
    case BLE_ADV_EVT_FAST_WHITELIST:
        err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING_WHITELIST);
        APP_ERROR_CHECK(err_code);
        break;
    case BLE_ADV_EVT_SLOW_WHITELIST:
        err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING_WHITELIST);
        APP_ERROR_CHECK(err_code);
        break;
    case BLE_ADV_EVT_IDLE:
        sleep_mode_enter();
        break;

    case BLE_ADV_EVT_WHITELIST_REQUEST:
    {
        ble_gap_whitelist_t whitelist;
        ble_gap_addr_t    * p_whitelist_addr[BLE_GAP_WHITELIST_ADDR_MAX_COUNT];
        ble_gap_irk_t     * p_whitelist_irk[BLE_GAP_WHITELIST_IRK_MAX_COUNT];

        whitelist.addr_count = BLE_GAP_WHITELIST_ADDR_MAX_COUNT;
        whitelist.irk_count  = BLE_GAP_WHITELIST_IRK_MAX_COUNT;
        whitelist.pp_addrs   = p_whitelist_addr;
        whitelist.pp_irks    = p_whitelist_irk;

        err_code = dm_whitelist_create(&m_app_handle, &whitelist);
        APP_ERROR_CHECK(err_code);

        err_code = ble_advertising_whitelist_reply(&whitelist);
        APP_ERROR_CHECK(err_code);
        break;
    }
    case BLE_ADV_EVT_PEER_ADDR_REQUEST:
    {
        ble_gap_addr_t peer_address;

        // Only Give peer address if we have a handle to the bonded peer.
        if (m_bonded_peer_handle.appl_id != DM_INVALID_ID)
        {

            err_code = dm_peer_addr_get(&m_bonded_peer_handle, &peer_address);
            if (err_code != (NRF_ERROR_NOT_FOUND | DEVICE_MANAGER_ERR_BASE))
            {
                APP_ERROR_CHECK(err_code);

                err_code = ble_advertising_peer_addr_reply(&peer_address);
                APP_ERROR_CHECK(err_code);
            }

        }
        break;
    }
    default:
        break;
    }
}

/**@brief Function for initializing the Advertising functionality.
 *
 * @details Encodes the required advertising data and passes it to the stack.
 *          Also builds a structure to be passed to the stack when starting advertising.
 */
/*
static void advertising_init(void)
{
   ble_advdata_t advdata;

   ble_uuid_t m_adv_uuids[] = {{DISPLAYS_UUID_SERVICE_BUTTON, m_dis.uuid_type}, {BLE_UUID_HUMAN_INTERFACE_DEVICE_SERVICE, BLE_UUID_TYPE_BLE}};

   uint32_t       err_code;
   uint8_t        adv_flags;

   // Build and set advertising data
   memset(&advdata, 0, sizeof(advdata));

   adv_flags                       = BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE;
   advdata.name_type               = BLE_ADVDATA_FULL_NAME;
   advdata.include_appearance      = true;
   advdata.flags                   = adv_flags;
   advdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
   advdata.uuids_complete.p_uuids  = m_adv_uuids;

   ble_adv_modes_config_t options =
   {
       BLE_ADV_WHITELIST_ENABLED,
       BLE_ADV_DIRECTED_ENABLED,
       BLE_ADV_DIRECTED_SLOW_DISABLED, 0, 0,
       BLE_ADV_FAST_ENABLED, APP_ADV_FAST_INTERVAL, APP_ADV_FAST_TIMEOUT,
       BLE_ADV_SLOW_ENABLED, APP_ADV_SLOW_INTERVAL, APP_ADV_SLOW_TIMEOUT
   };

   err_code = ble_advertising_init(&advdata, NULL, &options, on_adv_evt, ble_advertising_error_handler);
   APP_ERROR_CHECK(err_code);
}
*/

static ble_uuid_t m_adv_uuids[] = {{BLE_UUID_HUMAN_INTERFACE_DEVICE_SERVICE, BLE_UUID_TYPE_BLE}};

static void advertising_init(void)
{
    uint32_t       err_code;
    uint8_t        adv_flags;
    ble_advdata_t  advdata;


    //ble_uuid_t m_adv_uuids[] = {{DISPLAYS_UUID_SERVICE_BUTTON, m_dis.uuid_type}, {BLE_UUID_HUMAN_INTERFACE_DEVICE_SERVICE, BLE_UUID_TYPE_BLE}};


    // Build and set advertising data
    memset(&advdata, 0, sizeof(advdata));

    adv_flags                       = BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE;
    advdata.name_type               = BLE_ADVDATA_FULL_NAME;
    advdata.include_appearance      = true;
    advdata.flags                   = adv_flags;
    advdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    advdata.uuids_complete.p_uuids  = m_adv_uuids;

    ble_adv_modes_config_t options =
    {
        BLE_ADV_WHITELIST_ENABLED,
        BLE_ADV_DIRECTED_ENABLED,
        BLE_ADV_DIRECTED_SLOW_DISABLED, 0, 0,
        BLE_ADV_FAST_ENABLED, APP_ADV_FAST_INTERVAL, APP_ADV_FAST_TIMEOUT,
        BLE_ADV_SLOW_ENABLED, APP_ADV_SLOW_INTERVAL, APP_ADV_SLOW_TIMEOUT
    };

    err_code = ble_advertising_init(&advdata, NULL, &options, on_adv_evt, ble_advertising_error_handler);
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
    block_upload = true;
    pstorage_clear(&pstorage_handle, PAGE_NUM * PAGE_CHUNK);
    //send TRANSMIT_COMPLETE
    dispatch_transmit_status(TRANSMIT_COMPLETE);
}

static void bitmap_handler(ble_displays_t * p_dis, ble_gatts_evt_write_t * p_evt_write) {

    SEGGER_RTT_printf(0, "\x1B[32mbitmap handler\x1B[0m\n");

    if (!block_upload && ((transmit_state == TRANSMITTING) || (transmit_state == TRANSMIT_START))) {

        if (!transmit_init) {

            if (p_evt_write->len == 2) {

                transmit_init = true;
                bitmap_length = (p_evt_write->data[0] << 8) + p_evt_write->data[1];
                block_offset = 0;
                bitmap_offset = 0;
                bitmap_count_iteration = 0;

                free(image_part);
                image_part = NULL;
                image_part = (uint8_t*)malloc(sizeof(uint8_t) * PAGE_CHUNK);

                free(image_part2);
                image_part2 = NULL;
                image_part2 = (uint8_t*)malloc(sizeof(uint8_t) * PAGE_CHUNK);

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

                if (frame_offset == PAGE_CHUNK) {

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

/**@brief   Function for initializing the buffer queue used to key events that could not be
 *          transmitted
 *
 * @warning This handler is an example only. You need to analyze how you wish to buffer or buffer at
 *          all.
 *
 * @note    In case of HID keyboard, a temporary buffering could be employed to handle scenarios
 *          where encryption is not yet enabled or there was a momentary link loss or there were no
 *          Transmit buffers.
 */
static void buffer_init(void)
{
    uint32_t buffer_count;

    BUFFER_LIST_INIT();

    for (buffer_count = 0; buffer_count < MAX_BUFFER_ENTRIES; buffer_count++)
    {
        BUFFER_ELEMENT_INIT(buffer_count);
    }
}

/**@brief Function for handling the HID Report Characteristic Write event.
 *
 * @param[in]   p_evt   HID service event.
 */
static void on_hid_rep_char_write(ble_hids_evt_t *p_evt)
{
    if (p_evt->params.char_write.char_id.rep_type == BLE_HIDS_REP_TYPE_OUTPUT)
    {
        uint32_t err_code;
        uint8_t  report_val;
        uint8_t  report_index = p_evt->params.char_write.char_id.rep_index;

        if (report_index == OUTPUT_REPORT_INDEX)
        {
            // This code assumes that the outptu report is one byte long. Hence the following
            // static assert is made.
            STATIC_ASSERT(OUTPUT_REPORT_MAX_LEN == 1);

            err_code = ble_hids_outp_rep_get(&m_hids,
                                             report_index,
                                             OUTPUT_REPORT_MAX_LEN,
                                             0,
                                             &report_val);
            APP_ERROR_CHECK(err_code);

            if (!m_caps_on && ((report_val & OUTPUT_REPORT_BIT_MASK_CAPS_LOCK) != 0))
            {
                // Caps Lock is turned On.
                err_code = bsp_indication_set(BSP_INDICATE_ALERT_3);
                APP_ERROR_CHECK(err_code);

                keys_send(sizeof(m_caps_on_key_scan_str), m_caps_on_key_scan_str);
                m_caps_on = true;
            }
            else if (m_caps_on && ((report_val & OUTPUT_REPORT_BIT_MASK_CAPS_LOCK) == 0))
            {
                // Caps Lock is turned Off .
                err_code = bsp_indication_set(BSP_INDICATE_ALERT_OFF);
                APP_ERROR_CHECK(err_code);

                keys_send(sizeof(m_caps_off_key_scan_str), m_caps_off_key_scan_str);
                m_caps_on = false;
            }
            else
            {
                // The report received is not supported by this application. Do nothing.
            }
        }
    }
}

/**@brief Function for handling HID events.
 *
 * @details This function will be called for all HID events which are passed to the application.
 *
 * @param[in]   p_hids  HID service structure.
 * @param[in]   p_evt   Event received from the HID service.
 */
static void on_hids_evt(ble_hids_t * p_hids, ble_hids_evt_t *p_evt)
{
    switch (p_evt->evt_type)
    {
    case BLE_HIDS_EVT_BOOT_MODE_ENTERED:
        m_in_boot_mode = true;
        break;

    case BLE_HIDS_EVT_REPORT_MODE_ENTERED:
        m_in_boot_mode = false;
        break;

    case BLE_HIDS_EVT_REP_CHAR_WRITE:
        on_hid_rep_char_write(p_evt);
        break;

    case BLE_HIDS_EVT_NOTIF_ENABLED:
    {
        dm_service_context_t   service_context;
        service_context.service_type = DM_PROTOCOL_CNTXT_GATT_SRVR_ID;
        service_context.context_data.len = 0;
        service_context.context_data.p_data = NULL;

        if (m_in_boot_mode)
        {
            // Protocol mode is Boot Protocol mode.
            if (
                p_evt->params.notification.char_id.uuid
                ==
                BLE_UUID_BOOT_KEYBOARD_INPUT_REPORT_CHAR
            )
            {
                // The notification of boot keyboard input report has been enabled.
                // Save the system attribute (CCCD) information into the flash.
                uint32_t err_code;

                err_code = dm_service_context_set(&m_bonded_peer_handle, &service_context);
                if (err_code != NRF_ERROR_INVALID_STATE)
                {
                    APP_ERROR_CHECK(err_code);
                }
                else
                {
                    // The system attributes could not be written to the flash because
                    // the connected central is not a new central. The system attributes
                    // will only be written to flash only when disconnected from this central.
                    // Do nothing now.
                }
            }
            else
            {
                // Do nothing.
            }
        }
        else if (p_evt->params.notification.char_id.rep_type == BLE_HIDS_REP_TYPE_INPUT)
        {
            // The protocol mode is Report Protocol mode. And the CCCD for the input report
            // is changed. It is now time to store all the CCCD information (system
            // attributes) into the flash.
            uint32_t err_code;

            err_code = dm_service_context_set(&m_bonded_peer_handle, &service_context);
            if (err_code != NRF_ERROR_INVALID_STATE)
            {
                APP_ERROR_CHECK(err_code);
            }
            else
            {
                // The system attributes could not be written to the flash because
                // the connected central is not a new central. The system attributes
                // will only be written to flash only when disconnected from this central.
                // Do nothing now.
            }
        }
        else
        {
            // The notification of the report that was enabled by the central is not interesting
            // to this application. So do nothing.
        }
        break;
    }

    default:
        // No implementation needed.
        break;
    }
}

/**@brief Function for initializing HID Service.
 */
static void hids_init()
{
    uint32_t                   err_code;
    ble_hids_init_t            hids_init_obj;
    ble_hids_inp_rep_init_t    input_report_array[1];
    ble_hids_inp_rep_init_t  * p_input_report;
    ble_hids_outp_rep_init_t   output_report_array[1];
    ble_hids_outp_rep_init_t * p_output_report;
    uint8_t                    hid_info_flags;

    memset((void *)input_report_array, 0, sizeof(ble_hids_inp_rep_init_t));
    memset((void *)output_report_array, 0, sizeof(ble_hids_outp_rep_init_t));

    static uint8_t report_map_data[] =
    {
        0x05, 0x01,                 // Usage Page (Generic Desktop)
        0x09, 0x06,                 // Usage (Keyboard)
        0xA1, 0x01,                 // Collection (Application)
        0x05, 0x07,                 //     Usage Page (Key Codes)
        0x19, 0xe0,                 //     Usage Minimum (224)
        0x29, 0xe7,                 //     Usage Maximum (231)
        0x15, 0x00,                 //     Logical Minimum (0)
        0x25, 0x01,                 //     Logical Maximum (1)
        0x75, 0x01,                 //     Report Size (1)
        0x95, 0x08,                 //     Report Count (8)
        0x81, 0x02,                 //     Input (Data, Variable, Absolute)

        0x95, 0x01,                 //     Report Count (1)
        0x75, 0x08,                 //     Report Size (8)
        0x81, 0x01,                 //     Input (Constant) reserved byte(1)

        0x95, 0x05,                 //     Report Count (5)
        0x75, 0x01,                 //     Report Size (1)
        0x05, 0x08,                 //     Usage Page (Page# for LEDs)
        0x19, 0x01,                 //     Usage Minimum (1)
        0x29, 0x05,                 //     Usage Maximum (5)
        0x91, 0x02,                 //     Output (Data, Variable, Absolute), Led report
        0x95, 0x01,                 //     Report Count (1)
        0x75, 0x03,                 //     Report Size (3)
        0x91, 0x01,                 //     Output (Data, Variable, Absolute), Led report padding

        0x95, 0x06,                 //     Report Count (6)
        0x75, 0x08,                 //     Report Size (8)
        0x15, 0x00,                 //     Logical Minimum (0)
        0x25, 0x65,                 //     Logical Maximum (101)
        0x05, 0x07,                 //     Usage Page (Key codes)
        0x19, 0x00,                 //     Usage Minimum (0)
        0x29, 0x65,                 //     Usage Maximum (101)
        0x81, 0x00,                 //     Input (Data, Array) Key array(6 bytes)

        0x09, 0x05,                 //     Usage (Vendor Defined)
        0x15, 0x00,                 //     Logical Minimum (0)
        0x26, 0xFF, 0x00,           //     Logical Maximum (255)
        0x75, 0x08,                 //     Report Count (2)
        0x95, 0x02,                 //     Report Size (8 bit)
        0xB1, 0x02,                 //     Feature (Data, Variable, Absolute)

        0xC0                        // End Collection (Application)
    };

    // Initialize HID Service
    p_input_report                      = &input_report_array[INPUT_REPORT_KEYS_INDEX];
    p_input_report->max_len             = INPUT_REPORT_KEYS_MAX_LEN;
    p_input_report->rep_ref.report_id   = INPUT_REP_REF_ID;
    p_input_report->rep_ref.report_type = BLE_HIDS_REP_TYPE_INPUT;

    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&p_input_report->security_mode.cccd_write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&p_input_report->security_mode.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&p_input_report->security_mode.write_perm);

    p_output_report                      = &output_report_array[OUTPUT_REPORT_INDEX];
    p_output_report->max_len             = OUTPUT_REPORT_MAX_LEN;
    p_output_report->rep_ref.report_id   = OUTPUT_REP_REF_ID;
    p_output_report->rep_ref.report_type = BLE_HIDS_REP_TYPE_OUTPUT;

    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&p_output_report->security_mode.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&p_output_report->security_mode.write_perm);

    hid_info_flags = HID_INFO_FLAG_REMOTE_WAKE_MSK | HID_INFO_FLAG_NORMALLY_CONNECTABLE_MSK;

    memset(&hids_init_obj, 0, sizeof(hids_init_obj));

    hids_init_obj.evt_handler                    = on_hids_evt;
    hids_init_obj.error_handler                  = service_error_handler;
    hids_init_obj.is_kb                          = true;
    hids_init_obj.is_mouse                       = false;
    hids_init_obj.inp_rep_count                  = 1;
    hids_init_obj.p_inp_rep_array                = input_report_array;
    hids_init_obj.outp_rep_count                 = 1;
    hids_init_obj.p_outp_rep_array               = output_report_array;
    hids_init_obj.feature_rep_count              = 0;
    hids_init_obj.p_feature_rep_array            = NULL;
    hids_init_obj.rep_map.data_len               = sizeof(report_map_data);
    hids_init_obj.rep_map.p_data                 = report_map_data;
    hids_init_obj.hid_information.bcd_hid        = BASE_USB_HID_SPEC_VERSION;
    hids_init_obj.hid_information.b_country_code = 0;
    hids_init_obj.hid_information.flags          = hid_info_flags;
    hids_init_obj.included_services_count        = 0;
    hids_init_obj.p_included_services_array      = NULL;

    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&hids_init_obj.rep_map.security_mode.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&hids_init_obj.rep_map.security_mode.write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&hids_init_obj.hid_information.security_mode.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&hids_init_obj.hid_information.security_mode.write_perm);

    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(
        &hids_init_obj.security_mode_boot_kb_inp_rep.cccd_write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&hids_init_obj.security_mode_boot_kb_inp_rep.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&hids_init_obj.security_mode_boot_kb_inp_rep.write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&hids_init_obj.security_mode_boot_kb_outp_rep.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&hids_init_obj.security_mode_boot_kb_outp_rep.write_perm);

    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&hids_init_obj.security_mode_protocol.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&hids_init_obj.security_mode_protocol.write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&hids_init_obj.security_mode_ctrl_point.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&hids_init_obj.security_mode_ctrl_point.write_perm);

    err_code = ble_hids_init(&m_hids, &hids_init_obj);
    APP_ERROR_CHECK(err_code);
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


/**@brief   Function to dequeue key scan patterns that could not be transmitted either completely of
 *          partially.
 *
 * @warning This handler is an example only. You need to analyze how you wish to send the key
 *          release.
 *
 * @param[in]  tx_flag   Indicative of whether the dequeue should result in transmission or not.
 * @note       A typical example when all keys are dequeued with transmission is when link is
 *             disconnected.
 *
 * @return     NRF_SUCCESS on success, else an error code indicating reason for failure.
 */
static uint32_t buffer_dequeue(bool tx_flag)
{
    buffer_entry_t * p_element;
    uint32_t         err_code = NRF_SUCCESS;
    uint16_t         actual_len;

    if (BUFFER_LIST_EMPTY())
    {
        err_code = NRF_ERROR_NOT_FOUND;
    }
    else
    {
        bool remove_element = true;

        p_element = &buffer_list.buffer[(buffer_list.rp)];

        if (tx_flag)
        {
            err_code = send_key_scan_press_release(p_element->p_instance,
                                                   p_element->p_data,
                                                   p_element->data_len,
                                                   p_element->data_offset,
                                                   &actual_len);
            // An additional notification is needed for release of all keys, therefore check
            // is for actual_len <= element->data_len and not actual_len < element->data_len
            if ((err_code == BLE_ERROR_NO_TX_PACKETS) && (actual_len <= p_element->data_len))
            {
                // Transmission could not be completed, do not remove the entry, adjust next data to
                // be transmitted
                p_element->data_offset = actual_len;
                remove_element         = false;
            }
        }

        if (remove_element)
        {
            BUFFER_ELEMENT_INIT(buffer_list.rp);

            buffer_list.rp++;
            buffer_list.count--;

            if (buffer_list.rp == MAX_BUFFER_ENTRIES)
            {
                buffer_list.rp = 0;
            }
        }
    }

    return err_code;
}

/**@brief Function for handling the Application's BLE Stack events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 */
static void on_ble_evt(ble_evt_t * p_ble_evt)
{
    uint32_t                         err_code;
    ble_gatts_rw_authorize_reply_params_t auth_reply;

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

    case BLE_EVT_TX_COMPLETE:
        // Send next key event
        (void) buffer_dequeue(true);
        break;

    case BLE_GAP_EVT_DISCONNECTED:

        (void) buffer_dequeue(false);

        m_conn_handle = BLE_CONN_HANDLE_INVALID;

        err_code = app_button_disable();
        APP_ERROR_CHECK(err_code);

        advertising_start();
        break;

    case BLE_EVT_USER_MEM_REQUEST:
        err_code = sd_ble_user_mem_reply(m_conn_handle, NULL);
        APP_ERROR_CHECK(err_code);
        break;

    case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
        if (p_ble_evt->evt.gatts_evt.params.authorize_request.type
                != BLE_GATTS_AUTHORIZE_TYPE_INVALID)
        {
            if ((p_ble_evt->evt.gatts_evt.params.authorize_request.request.write.op
                    == BLE_GATTS_OP_PREP_WRITE_REQ)
                    || (p_ble_evt->evt.gatts_evt.params.authorize_request.request.write.op
                        == BLE_GATTS_OP_EXEC_WRITE_REQ_NOW)
                    || (p_ble_evt->evt.gatts_evt.params.authorize_request.request.write.op
                        == BLE_GATTS_OP_EXEC_WRITE_REQ_CANCEL))
            {
                if (p_ble_evt->evt.gatts_evt.params.authorize_request.type
                        == BLE_GATTS_AUTHORIZE_TYPE_WRITE)
                {
                    auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_WRITE;
                }
                else
                {
                    auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_READ;
                }
                auth_reply.params.write.gatt_status = APP_FEATURE_NOT_SUPPORTED;
                err_code = sd_ble_gatts_rw_authorize_reply(m_conn_handle, &auth_reply);
                APP_ERROR_CHECK(err_code);
            }
        }
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

    case BLE_GATTC_EVT_TIMEOUT:
    case BLE_GATTS_EVT_TIMEOUT:
        // Disconnect on GATT Server and Client timeout events.
        err_code = sd_ble_gap_disconnect(m_conn_handle,
                                         BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
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
    dm_ble_evt_handler(p_ble_evt);
    on_ble_evt(p_ble_evt);
    ble_advertising_on_ble_evt(p_ble_evt);
    ble_conn_params_on_ble_evt(p_ble_evt);
    ble_displays_on_ble_evt(&m_dis, p_ble_evt);
    ble_hids_on_ble_evt(&m_hids, p_ble_evt);
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
    ble_advertising_on_sys_evt(sys_evt);
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

        block_upload = false;

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

static void init_storage() {

    pstorage_module_param_t param;

    param.block_size  = PAGE_CHUNK;
    param.block_count = PAGE_NUM;
    param.cb          = pstorage_cb_handler;

    uint32_t retval = pstorage_register(&param, &pstorage_handle);

    if ( retval == NRF_SUCCESS) {
        retval = pstorage_clear(&pstorage_handle, PAGE_NUM * PAGE_CHUNK);
    }
    else {
        SEGGER_RTT_printf(0, "\x1B[32mregister failure\x1B[0m\n");
    }
}

/**@brief Function for handling the Device Manager events.
 *
 * @param[in]   p_evt   Data associated to the device manager event.
 */
static uint32_t device_manager_evt_handler(dm_handle_t const    * p_handle,
        dm_event_t const     * p_event,
        ret_code_t           event_result)
{
    APP_ERROR_CHECK(event_result);

    switch (p_event->event_id)
    {
    case DM_EVT_DEVICE_CONTEXT_LOADED: // Fall through.
    case DM_EVT_SECURITY_SETUP_COMPLETE:
        m_bonded_peer_handle = (*p_handle);
        break;
    }

    return NRF_SUCCESS;
}


/**@brief Function for the Device Manager initialization.
 *
 * @param[in] erase_bonds  Indicates whether bonding information should be cleared from
 *                         persistent storage during initialization of the Device Manager.
 */
static void device_manager_init(bool erase_bonds)
{

    uint32_t               err_code;

    dm_init_param_t        init_param = {.clear_persistent_data = erase_bonds};
    dm_application_param_t  register_param;

    // Initialize peer device handle.
    err_code = dm_handle_initialize(&m_bonded_peer_handle);
    APP_ERROR_CHECK(err_code);

    // Initialize persistent storage module.
    err_code = pstorage_init();
    APP_ERROR_CHECK(err_code);

    err_code = dm_init(&init_param);
    APP_ERROR_CHECK(err_code);

    memset(&register_param.sec_param, 0, sizeof(ble_gap_sec_params_t));

    register_param.sec_param.bond         = SEC_PARAM_BOND;
    register_param.sec_param.mitm         = SEC_PARAM_MITM;
    register_param.sec_param.lesc         = SEC_PARAM_LESC;
    register_param.sec_param.keypress     = SEC_PARAM_KEYPRESS;
    register_param.sec_param.io_caps      = SEC_PARAM_IO_CAPABILITIES;
    register_param.sec_param.oob          = SEC_PARAM_OOB;
    register_param.sec_param.min_key_size = SEC_PARAM_MIN_KEY_SIZE;
    register_param.sec_param.max_key_size = SEC_PARAM_MAX_KEY_SIZE;
    register_param.evt_handler            = device_manager_evt_handler;
    register_param.service_type           = DM_PROTOCOL_CNTXT_GATT_SRVR_ID;

    err_code = dm_register(&m_app_handle, &register_param);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling events from the BSP module.
 *
 * @param[in]   event   Event generated by button press.
 */
static void bsp_event_handler(bsp_event_t event)
{
    uint32_t err_code;
    static uint8_t * p_key = m_sample_key_press_scan_str;
    static uint8_t size = 0;

    switch (event)
    {
    case BSP_EVENT_SLEEP:
        sleep_mode_enter();
        break;

    case BSP_EVENT_DISCONNECT:
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
        if (err_code != NRF_ERROR_INVALID_STATE)
        {
            APP_ERROR_CHECK(err_code);
        }
        break;

    case BSP_EVENT_WHITELIST_OFF:
        err_code = ble_advertising_restart_without_whitelist();
        if (err_code != NRF_ERROR_INVALID_STATE)
        {
            APP_ERROR_CHECK(err_code);
        }
        break;

    case BSP_EVENT_KEY_0:
        if (m_conn_handle != BLE_CONN_HANDLE_INVALID)
        {
            keys_send(1, p_key);
            p_key++;
            size++;
            if (size == MAX_KEYS_IN_ONE_REPORT)
            {
                p_key = m_sample_key_press_scan_str;
                size = 0;
            }
        }
        break;

    default:
        break;
    }
}


/**@brief Function for initializing buttons and leds.
 *
 * @param[out] p_erase_bonds  Will be true if the clear bonding button was pressed to wake the application up.
 */
static void buttons_leds_init(bool * p_erase_bonds)
{
    bsp_event_t startup_event;

    uint32_t err_code = bsp_init(BSP_INIT_LED | BSP_INIT_BUTTONS,
                                 APP_TIMER_TICKS(100, APP_TIMER_PRESCALER),
                                 bsp_event_handler);
    APP_ERROR_CHECK(err_code);

    err_code = bsp_btn_ble_init(NULL, &startup_event);
    APP_ERROR_CHECK(err_code);

    *p_erase_bonds = (startup_event == BSP_EVENT_CLEAR_BONDING_DATA);
}

/**@brief Function for application main entry.
 */
int main(void)
{
    bool erase_bonds;

    bsp_configuration();
    // Initialize
    //app_trace_init();
    timers_init();
    gpiote_init();
    adc_init();
    buttons_init();

    tft_setup();
    fillScreen(ST7735_BLACK);

    //buttons_leds_init(&erase_bonds);
    ble_stack_init();
    scheduler_init();

    device_manager_init(false);

    SEGGER_RTT_printf(0, "\x1B[32mOK\x1B[0m\n");

    //gap_params_init();
    //hids_init();
    //services_init();
    //advertising_init();
    SEGGER_RTT_printf(0, "\x1B[32mOKKK\x1B[0m\n");
    conn_params_init();
    sec_params_init();
    buffer_init();
    leds_init();
    // Start execution
    timers_start();

    //advertising_start();
    ble_advertising_start(BLE_ADV_MODE_FAST);

    SEGGER_RTT_printf(0, "\x1B[32mOK2\x1B[0m\n");

    //init_storage();

    SEGGER_RTT_printf(0, "\x1B[32mOK3\x1B[0m\n");

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