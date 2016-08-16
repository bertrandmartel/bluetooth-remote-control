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
#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "nrf_assert.h"
#include "app_error.h"
#include "nrf_gpio.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advertising.h"
#include "ble_advdata.h"
#include "ble_hids.h"
#include "ble_bas.h"
#include "ble_dis.h"
#include "ble_conn_params.h"
#include "bsp.h"
#include "sensorsim.h"
#include "app_gpiote.h"
#include "app_button.h"
#include "bsp_btn_ble.h"
#include "app_scheduler.h"
#include "device_manager.h"
#include "softdevice_handler_appsh.h"
#include "app_timer_appsh.h"
#include "pstorage.h"
#include "SEGGER_RTT.h"
#include "adafruit1_8_oled_library.h"
#include "ble_displays.h"
#include "unpack.h"

#define APP_ADV_INTERVAL                64                                          /**< The advertising interval (in units of 0.625 ms. This value corresponds to 40 ms). */
#define APP_ADV_TIMEOUT_IN_SECONDS      180                                         /**< The advertising timeout (in units of seconds). */

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

#define APP_GPIOTE_MAX_USERS            1                                           /**< Maximum number of users of the GPIOTE handler. */

#define BUTTON_S0         BSP_BUTTON_0                                /**< Button used to wake up the application. */
#define BUTTON_S1         BSP_BUTTON_1
#define BUTTON_S2         BSP_BUTTON_2
#define BUTTON_S3         BSP_BUTTON_3

#define IS_SRVC_CHANGED_CHARACT_PRESENT  0                                              /**< Include or not the service_changed characteristic. if not enabled, the server's database cannot be changed for the lifetime of the device*/

#define CENTRAL_LINK_COUNT               0                                              /**< Number of central links used by the application. When changing this number remember to adjust the RAM settings*/
#define PERIPHERAL_LINK_COUNT            1                                              /**< Number of peripheral links used by the application. When changing this number remember to adjust the RAM settings*/

#define UART_TX_BUF_SIZE                 256                                            /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE                 1                                              /**< UART RX buffer size. */

#define KEY_PRESS_BUTTON_ID              0                                              /**< Button used as Keyboard key press. */
#define SHIFT_BUTTON_ID                  1                                              /**< Button used as 'SHIFT' Key. */

#define DEVICE_NAME                      "BleDisplayRemote"                              /**< Name of device. Will be included in the advertising data. */
#define MANUFACTURER_NAME                "NordicSemiconductor"                          /**< Manufacturer. Will be passed to Device Information Service. */

#define APP_TIMER_PRESCALER              0                                              /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_OP_QUEUE_SIZE          4                                              /**< Size of timer operation queues. */

#define BATTERY_LEVEL_MEAS_INTERVAL      APP_TIMER_TICKS(2000, APP_TIMER_PRESCALER)     /**< Battery level measurement interval (ticks). */
#define MIN_BATTERY_LEVEL                81                                             /**< Minimum simulated battery level. */
#define MAX_BATTERY_LEVEL                100                                            /**< Maximum simulated battery level. */
#define BATTERY_LEVEL_INCREMENT          1                                              /**< Increment between each simulated battery level measurement. */

#define PNP_ID_VENDOR_ID_SOURCE          0x02                                           /**< Vendor ID Source. */
#define PNP_ID_VENDOR_ID                 0x1915                                         /**< Vendor ID. */
#define PNP_ID_PRODUCT_ID                0xEEEE                                         /**< Product ID. */
#define PNP_ID_PRODUCT_VERSION           0x0001                                         /**< Product Version. */

#define APP_ADV_FAST_INTERVAL            0x0028                                         /**< Fast advertising interval (in units of 0.625 ms. This value corresponds to 25 ms.). */
#define APP_ADV_SLOW_INTERVAL            0x0C80                                         /**< Slow advertising interval (in units of 0.625 ms. This value corrsponds to 2 seconds). */
#define APP_ADV_FAST_TIMEOUT             30                                             /**< The duration of the fast advertising period (in seconds). */
#define APP_ADV_SLOW_TIMEOUT             180                                            /**< The duration of the slow advertising period (in seconds). */

/*lint -emacro(524, MIN_CONN_INTERVAL) // Loss of precision */
//#define MIN_CONN_INTERVAL                MSEC_TO_UNITS(7.5, UNIT_1_25_MS)               /**< Minimum connection interval (7.5 ms) */
//#define MAX_CONN_INTERVAL                MSEC_TO_UNITS(30, UNIT_1_25_MS)                /**< Maximum connection interval (30 ms). */
//#define SLAVE_LATENCY                    6                                              /**< Slave latency. */
//#define CONN_SUP_TIMEOUT                 MSEC_TO_UNITS(430, UNIT_10_MS)                 /**< Connection supervisory timeout (430 ms). */

#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(30, UNIT_1_25_MS)            /**< Minimum acceptable connection interval (0.5 seconds). */
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(30, UNIT_1_25_MS)            /**< Maximum acceptable connection interval (1 second). */
#define SLAVE_LATENCY                   0                                           /**< Slave latency. */
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)             /**< Connection supervisory timeout (4 seconds). */

#define FIRST_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(5000, APP_TIMER_PRESCALER)     /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY    APP_TIMER_TICKS(30000, APP_TIMER_PRESCALER)    /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT     3                                              /**< Number of attempts before giving up the connection parameter negotiation. */

#define SEC_PARAM_BOND                   1                                              /**< Perform bonding. */
#define SEC_PARAM_MITM                   0                                              /**< Man In The Middle protection not required. */
#define SEC_PARAM_LESC                   0                                              /**< LE Secure Connections not enabled. */
#define SEC_PARAM_KEYPRESS               0                                              /**< Keypress notifications not enabled. */
#define SEC_PARAM_IO_CAPABILITIES        BLE_GAP_IO_CAPS_NONE                           /**< No I/O capabilities. */
#define SEC_PARAM_OOB                    0                                              /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE           7                                              /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE           16                                             /**< Maximum encryption key size. */

#define OUTPUT_REPORT_INDEX              0                                              /**< Index of Output Report. */
#define OUTPUT_REPORT_MAX_LEN            1                                              /**< Maximum length of Output Report. */

#define INPUT_REPORT_KEYS_INDEX          0                                              /**< Index of Input Report. */
#define INPUT_REPORT_CONSUMER_INDEX      1                                              /**< Index of Consumer Input Report. */

#define OUTPUT_REPORT_BIT_MASK_CAPS_LOCK 0x02                                           /**< CAPS LOCK bit in Output Report (based on 'LED Page (0x08)' of the Universal Serial Bus HID Usage Tables). */
#define INPUT_REP_REF_ID                 0                                              /**< Id of reference to Keyboard Input Report. */
#define INPUT_CONSUMER_REP_REF_ID        1                                              /**< Id of reference to Consumer control Input Report. */
#define OUTPUT_REP_REF_ID                0                                              /**< Id of reference to Keyboard Output Report. */

#define APP_FEATURE_NOT_SUPPORTED       BLE_GATT_STATUS_ATTERR_APP_BEGIN + 2            /**< Reply when unsupported features are requested. */

#define MAX_BUFFER_ENTRIES               5                                              /**< Number of elements that can be enqueued */

#define BASE_USB_HID_SPEC_VERSION        0x0101                                         /**< Version number of base USB HID Specification implemented by this application. */

#define INPUT_REPORT_KEYS_MAX_LEN        8                                              /**< Maximum length of the Input Report characteristic. */
#define INPUT_REPORT_CONSUMER_MAX_LEN    2                                              /**< Maximum length of the Consumer Input Report characteristic. */

#define DEAD_BEEF                        0xDEADBEEF                                     /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

#define SCHED_MAX_EVENT_DATA_SIZE        MAX(APP_TIMER_SCHED_EVT_SIZE,\
                                             BLE_STACK_HANDLER_SCHED_EVT_SIZE)          /**< Maximum size of scheduler events. */
#define SCHED_QUEUE_SIZE                 10                                             /**< Maximum number of events in the scheduler queue. */

#define MODIFIER_KEY_POS                 0                                              /**< Position of the modifier byte in the Input Report. */
#define SCAN_CODE_POS                    2                                              /**< This macro indicates the start position of the key scan code in a HID Report. As per the document titled 'Device Class Definition for Human Interface Devices (HID) V1.11, each report shall have one modifier byte followed by a reserved constant byte and then the key scan code. */
#define SHIFT_KEY_CODE                   0x02                                           /**< Key code indicating the press of the Shift Key. */

#define MAX_KEYS_IN_ONE_REPORT           (INPUT_REPORT_KEYS_MAX_LEN - SCAN_CODE_POS)    /**< Maximum number of key presses that can be sent in one Input Report. */

APP_TIMER_DEF(m_adc_sampling_timer_id);
APP_TIMER_DEF(m_dpad_timer_id);
#define BUTTON_DETECTION_DELAY          APP_TIMER_TICKS(50, APP_TIMER_PRESCALER)    /**< Delay from a GPIOTE event until a button is reported as pushed (in number of timer ticks). */
#define ADC_SAMPLING_INTERVAL           APP_TIMER_TICKS(100, APP_TIMER_PRESCALER) /**< Sampling rate for the ADC */

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

/** @} */

typedef enum
{
    BLE_NO_ADV,               /**< No advertising running. */
    BLE_DIRECTED_ADV,         /**< Direct advertising to the latest central. */
    BLE_FAST_ADV_WHITELIST,   /**< Advertising with whitelist. */
    BLE_FAST_ADV,             /**< Fast advertising running. */
    BLE_SLOW_ADV,             /**< Slow advertising running. */
    BLE_SLEEP,                /**< Go to system-off. */
} ble_advertising_mode_t;

/** Abstracts buffer element */
typedef struct hid_key_buffer
{
    uint8_t    data_offset;   /**< Max Data that can be buffered for all entries */
    uint8_t    data_len;      /**< Total length of data */
    uint8_t    * p_data;      /**< Scanned key pattern */
    ble_hids_t * p_instance;  /**< Identifies peer and service instance */
    uint8_t modifier;
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

static ble_hids_t                        m_hids;                                        /**< Structure used to identify the HID service. */
static ble_bas_t                         m_bas;                                         /**< Structure used to identify the battery service. */
static bool                              m_in_boot_mode = false;                        /**< Current protocol mode. */
static uint16_t                          m_conn_handle = BLE_CONN_HANDLE_INVALID;       /**< Handle of the current connection. */

static sensorsim_cfg_t                   m_battery_sim_cfg;                             /**< Battery Level sensor simulator configuration. */
static sensorsim_state_t                 m_battery_sim_state;                           /**< Battery Level sensor simulator state. */

APP_TIMER_DEF(m_battery_timer_id);                                                      /**< Battery timer. */

static dm_application_instance_t         m_app_handle;                                  /**< Application identifier allocated by device manager. */
static dm_handle_t                       m_bonded_peer_handle;                          /**< Device reference handle to the current bonded central. */
static bool                              m_caps_on = false;                             /**< Variable to indicate if Caps Lock is turned on. */

static bool                              m_button_pressed = false;
static bool                              m_consumer_pressed = false;

static ble_uuid_t m_adv_uuids[] = {{BLE_UUID_HUMAN_INTERFACE_DEVICE_SERVICE, BLE_UUID_TYPE_BLE}};

static ble_displays_t                        m_dis;


static bool transmit_init = false;
static volatile uint16_t bitmap_length = 0;
static volatile uint32_t expecting_length = 0;

static const uint16_t MAX_SIZE_ARRAY = 10584;

static uint16_t bitmap_offset = 0;

bool block_upload = false;

#define PAGE_CHUNK 1024
#define PAGE_NUM   39

//when this flag is set to 1, it means it is the final bitmap chunk to be stored before being processed
static volatile uint8_t final_storage_bitmap_flag = 0;

static volatile uint16_t bitmap_stop_iteration = 0;
static volatile uint16_t bitmap_count_iteration = 0;

#define BITMAP_CHUNK_SIZE 128

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

static volatile uint8_t button_state = BUTTON_NONE;
static volatile bool button_state_change = false;
static volatile uint8_t transmit_state = TRANSMIT_NONE;

// ADC timer handler to start ADC sampling
static void adc_sampling_timeout_handler(void * p_context)
{
    NRF_ADC->EVENTS_END  = 0;
    NRF_ADC->TASKS_START = 1;            //Start ADC sampling
}

/** List to enqueue not just data to be sent, but also related information like the handle, connection handle etc */
static buffer_list_t buffer_list;

static void on_hids_evt(ble_hids_t * p_hids, ble_hids_evt_t * p_evt);

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


/**@brief Function for handling advertising errors.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void ble_advertising_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for performing a battery measurement, and update the Battery Level characteristic in the Battery Service.
 */
static void battery_level_update(void)
{
    uint32_t err_code;
    uint8_t  battery_level;

    battery_level = (uint8_t)sensorsim_measure(&m_battery_sim_state, &m_battery_sim_cfg);

    err_code = ble_bas_battery_level_update(&m_bas, battery_level);
    if ((err_code != NRF_SUCCESS) &&
            (err_code != NRF_ERROR_INVALID_STATE) &&
            (err_code != BLE_ERROR_NO_TX_PACKETS) &&
            (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING)
       )
    {
        APP_ERROR_HANDLER(err_code);
    }
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

/**@brief Function for handling the Battery measurement timer timeout.
 *
 * @details This function will be called each time the battery level measurement timer expires.
 *
 * @param[in]   p_context   Pointer used for passing some arbitrary information (context) from the
 *                          app_start_timer() call to the timeout handler.
 */
static void battery_level_meas_timeout_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);
    battery_level_update();
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

    err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_GENERIC_REMOTE_CONTROL);
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing Device Information Service.
 */
static void dis_init(void)
{
    uint32_t         err_code;
    ble_dis_init_t   dis_init_obj;
    ble_dis_pnp_id_t pnp_id;

    pnp_id.vendor_id_source = PNP_ID_VENDOR_ID_SOURCE;
    pnp_id.vendor_id        = PNP_ID_VENDOR_ID;
    pnp_id.product_id       = PNP_ID_PRODUCT_ID;
    pnp_id.product_version  = PNP_ID_PRODUCT_VERSION;

    memset(&dis_init_obj, 0, sizeof(dis_init_obj));

    ble_srv_ascii_to_utf8(&dis_init_obj.manufact_name_str, MANUFACTURER_NAME);
    dis_init_obj.p_pnp_id = &pnp_id;

    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&dis_init_obj.dis_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&dis_init_obj.dis_attr_md.write_perm);

    err_code = ble_dis_init(&dis_init_obj);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing Battery Service.
 */
static void bas_init(void)
{
    uint32_t       err_code;
    ble_bas_init_t bas_init_obj;

    memset(&bas_init_obj, 0, sizeof(bas_init_obj));

    bas_init_obj.evt_handler          = NULL;
    bas_init_obj.support_notification = true;
    bas_init_obj.p_report_ref         = NULL;
    bas_init_obj.initial_batt_level   = 100;

    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&bas_init_obj.battery_level_char_attr_md.cccd_write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&bas_init_obj.battery_level_char_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&bas_init_obj.battery_level_char_attr_md.write_perm);

    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&bas_init_obj.battery_level_report_read_perm);

    err_code = ble_bas_init(&m_bas, &bas_init_obj);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing HID Service.
 */
static void hids_init(void)
{
    uint32_t                   err_code;
    ble_hids_init_t            hids_init_obj;
    ble_hids_inp_rep_init_t    input_report_array[2];
    ble_hids_inp_rep_init_t  * p_input_report;
    ble_hids_inp_rep_init_t  * p_input_consumer_report;
    ble_hids_outp_rep_init_t   output_report_array[1];
    ble_hids_outp_rep_init_t * p_output_report;
    uint8_t                    hid_info_flags;

    memset((void *)input_report_array, 0, sizeof(ble_hids_inp_rep_init_t));
    memset((void *)output_report_array, 0, sizeof(ble_hids_outp_rep_init_t));

    static uint8_t report_map_data[] =
    {
        0x05, 0x01, // USAGE_PAGE (Generic Desktop)
        0x09, 0x06, // USAGE (Keyboard)
        0xa1, 0x01, // COLLECTION (Application)
        0x05, 0x07, // USAGE_PAGE (Keyboard)
        0x85, 0x01, // REPORT_ID (1)

        0x19, 0xe0, // USAGE_MINIMUM (kbd LeftControl)
        0x29, 0xe7, // USAGE_MAXIMUM (kbd Right GUI)
        0x15, 0x00, // LOGICAL_MINIMUM (0)
        0x25, 0x01, // LOGICAL_MAXIMUM (1)
        0x75, 0x01, // REPORT_SIZE (1)
        0x95, 0x08, // REPORT_COUNT (8)
        0x81, 0x02, // INPUT (Data,Var,Abs)

        0x95, 0x01, // REPORT_COUNT (1)
        0x75, 0x08, // REPORT_SIZE (8)
        0x81, 0x01, // INPUT (Cnst,Ary,Abs)

        0x95, 0x05, // REPORT_COUNT (5)
        0x75, 0x01, // REPORT_SIZE (1)
        0x05, 0x08, // USAGE_PAGE (LEDs)
        0x85, 0x01, // REPORT_ID (1)
        0x19, 0x01, // USAGE_MINIMUM (Num Lock)
        0x29, 0x05, // USAGE_MAXIMUM (Kana)
        0x91, 0x02, // OUTPUT (Data,Var,Abs)

        0x95, 0x01, // REPORT_COUNT (1)
        0x75, 0x03, // REPORT_SIZE (3)
        0x91, 0x03, // OUTPUT (Cnst,Var,Abs)

        0x95, 0x06, // REPORT_COUNT (6)
        0x75, 0x08, // REPORT_SIZE (8)
        0x15, 0x00, // LOGICAL_MINIMUM (0)
        0x25, 0x65, // LOGICAL_MAXIMUM (101)
        0x05, 0x07, // USAGE_PAGE (Keyboard)
        0x19, 0x00, // USAGE_MINIMUM (Reserved (no event indicated))
        0x29, 0x65, // USAGE_MAXIMUM (Keyboard Application)
        0x81, 0x00, // INPUT (Data,Ary,Abs)
        0xc0,       // END_COLLECTION

        0x05, 0x0C,  // USAGE_PAGE (Consumer Devices)
        0x09, 0x01,  // USAGE (Consumer Control)
        0xA1, 0x01,  // COLLECTION (Application)
        0x85, 0x02,  //   REPORT_ID (2)
        0x75, 0x10,  // REPORT_SIZE (16)
        0x95, 0x01,  // REPORT_COUNT (1)
        0x15, 0x01,  // LOGICAL_MINIMUM (0)
        0x26, 0xFF, 0x02, // LOGICAL_MAXIMUM (1)
        0x19, 0x01,  // USAGE_MINIMUM (Consumer Control)
        0x2A, 0xFF, 0x02, // USAGE_MAXIMUM (Consumer Control)
        0x81, 0x60,  // INPUT (Data,Ary,Abs,NPrf,Null)


        0xc0,        // End Collection (Application)
    };


    // Initialize HID Service
    p_input_report                      = &input_report_array[INPUT_REPORT_KEYS_INDEX];
    p_input_report->max_len             = INPUT_REPORT_KEYS_MAX_LEN;
    p_input_report->rep_ref.report_id   = 1;
    p_input_report->rep_ref.report_type = BLE_HIDS_REP_TYPE_INPUT;

    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&p_input_report->security_mode.cccd_write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&p_input_report->security_mode.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&p_input_report->security_mode.write_perm);

    p_input_consumer_report                      = &input_report_array[INPUT_REPORT_CONSUMER_INDEX];
    p_input_consumer_report->max_len             = INPUT_REPORT_CONSUMER_MAX_LEN;
    p_input_consumer_report->rep_ref.report_id   = 2;
    p_input_consumer_report->rep_ref.report_type = BLE_HIDS_REP_TYPE_INPUT;

    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&p_input_consumer_report->security_mode.cccd_write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&p_input_consumer_report->security_mode.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&p_input_consumer_report->security_mode.write_perm);

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
    hids_init_obj.is_kb                          = false;
    hids_init_obj.is_mouse                       = false;
    hids_init_obj.inp_rep_count                  = 2;
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
                block_max = 0;
                last_value = 0;

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

                    if (!image_part_select) {
                        last_value = image_part[frame_offset - 1];
                    }
                    else {
                        last_value = image_part2[frame_offset - 1];
                    }

                    block_max = block_offset;

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
static void display_service_init(void)
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


/**@brief Function for initializing services that will be used by the application.
 */
static void services_init(void)
{
    dis_init();
    bas_init();
    hids_init();
    display_service_init();
}


/**@brief Function for initializing the battery sensor simulator.
 */
static void sensor_simulator_init(void)
{
    m_battery_sim_cfg.min          = MIN_BATTERY_LEVEL;
    m_battery_sim_cfg.max          = MAX_BATTERY_LEVEL;
    m_battery_sim_cfg.incr         = BATTERY_LEVEL_INCREMENT;
    m_battery_sim_cfg.start_at_max = true;

    sensorsim_init(&m_battery_sim_state, &m_battery_sim_cfg);
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
    cp_init.evt_handler                    = NULL;
    cp_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for starting timers.
 */
static void timers_start(void)
{
    uint32_t err_code;

    err_code = app_timer_start(m_battery_timer_id, BATTERY_LEVEL_MEAS_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);

    //ADC timer start
    err_code = app_timer_start(m_adc_sampling_timer_id, ADC_SAMPLING_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);

    //dpad timer start
    err_code = app_timer_start(m_dpad_timer_id, BUTTON_DETECTION_DELAY, NULL);
    APP_ERROR_CHECK(err_code);
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
        uint16_t *     p_actual_len,
        uint8_t modifier)
{
    uint32_t err_code;
    uint16_t offset;
    uint16_t data_len;
    uint8_t  data[INPUT_REPORT_KEYS_MAX_LEN];

    // HID Report Descriptor enumerates an array of size 7, the pattern hence shall not be any
    // longer than this.
    STATIC_ASSERT((INPUT_REPORT_KEYS_MAX_LEN - 2) == 6);

    ASSERT(pattern_len <= (INPUT_REPORT_KEYS_MAX_LEN - 2));

    offset   = pattern_offset;
    data_len = pattern_len;

    SEGGER_RTT_printf(0, "\x1B[32msend_key_scan_press_release\x1B[0m\n");

    do
    {
        SEGGER_RTT_printf(0, "\x1B[32mloop\x1B[0m\n");

        // Reset the data buffer.
        memset(data, 0, sizeof(data));

        // Copy the scan code.
        memcpy(data + SCAN_CODE_POS + offset, p_key_pattern + offset, data_len - offset);

        data[MODIFIER_KEY_POS] = modifier;

        if (!m_in_boot_mode)
        {
            SEGGER_RTT_printf(0, "\x1B[32msend\x1B[0m\n");
            err_code = ble_hids_inp_rep_send(p_hids,
                                             INPUT_REPORT_KEYS_INDEX,
                                             INPUT_REPORT_KEYS_MAX_LEN,
                                             data);
        }
        else
        {
            err_code = ble_hids_boot_kb_inp_rep_send(p_hids,
                       INPUT_REPORT_KEYS_MAX_LEN,
                       data);
        }

        if (err_code != NRF_SUCCESS)
        {
            break;
        }

        offset++;
    } while (offset < data_len);

    *p_actual_len = offset;

    return err_code;
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

/**@brief Function for sending sample key presses to the peer.
 *
 * @param[in]   key_pattern_len   Pattern length.
 * @param[in]   p_key_pattern     Pattern to be sent.
 */
static void keys_send(uint8_t key_pattern_len, uint8_t * p_key_pattern, uint8_t modifier)
{
    uint32_t err_code;
    uint16_t actual_len;

    SEGGER_RTT_printf(0, "\x1B[32mkeys_send\x1B[0m\n");

    err_code = send_key_scan_press_release(&m_hids,
                                           p_key_pattern,
                                           key_pattern_len,
                                           0,
                                           &actual_len,
                                           modifier);

    /*
    // An additional notification is needed for release of all keys, therefore check
    // is for actual_len <= key_pattern_len and not actual_len < key_pattern_len.
    if ((err_code == BLE_ERROR_NO_TX_PACKETS) && (actual_len <= key_pattern_len))
    {
        // Buffer enqueue routine return value is not intentionally checked.
        // Rationale: Its better to have a a few keys missing than have a system
        // reset. Recommendation is to work out most optimal value for
        // MAX_BUFFER_ENTRIES to minimize chances of buffer queue full condition
        UNUSED_VARIABLE(buffer_enqueue(&m_hids, p_key_pattern, key_pattern_len, actual_len, modifier));
    }

    */
    if ((err_code != NRF_SUCCESS) &&
            (err_code != NRF_ERROR_INVALID_STATE) &&
            (err_code != BLE_ERROR_NO_TX_PACKETS) &&
            (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING)
       )
    {
        APP_ERROR_HANDLER(err_code);
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

            SEGGER_RTT_printf(0, "\x1B[32mon_hid_rep_char_write\x1B[0m\n");

            if (!m_caps_on && ((report_val & OUTPUT_REPORT_BIT_MASK_CAPS_LOCK) != 0))
            {
                // Caps Lock is turned On.
                err_code = bsp_indication_set(BSP_INDICATE_ALERT_3);
                APP_ERROR_CHECK(err_code);

                keys_send(sizeof(m_caps_on_key_scan_str), m_caps_on_key_scan_str, 0);
                m_caps_on = true;
            }
            else if (m_caps_on && ((report_val & OUTPUT_REPORT_BIT_MASK_CAPS_LOCK) == 0))
            {
                // Caps Lock is turned Off .
                err_code = bsp_indication_set(BSP_INDICATE_ALERT_OFF);
                APP_ERROR_CHECK(err_code);

                keys_send(sizeof(m_caps_off_key_scan_str), m_caps_off_key_scan_str, 0);
                m_caps_on = false;
            }
            else
            {
                // The report received is not supported by this application. Do nothing.
            }
        }
    }
}

static void send_consumer_key(uint8_t *data) {

    uint32_t err_code = ble_hids_inp_rep_send(&m_hids,
                        INPUT_REPORT_CONSUMER_INDEX,
                        INPUT_REPORT_CONSUMER_MAX_LEN,
                        data);

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

        if (m_conn_handle != BLE_CONN_HANDLE_INVALID)
        {
            uint8_t p_key_pattern[1];

            switch (button_state) {
            case BUTTON_DOWN:
            {
                p_key_pattern[0] = 0x51;
                keys_send(1, p_key_pattern, 0);
                break;
            }
            case BUTTON_RIGHT:
            {
                p_key_pattern[0] = 0x4F;
                keys_send(1, p_key_pattern, 0);
                break;
            }
            case BUTTON_SELECT:
            {
                p_key_pattern[0] = 0x28;
                keys_send(1, p_key_pattern, 0);
                break;
            }
            case BUTTON_UP:
            {
                p_key_pattern[0] = 0x52;
                keys_send(1, p_key_pattern, 0);
                break;
            }
            case BUTTON_LEFT:
            {
                p_key_pattern[0] = 0x50;
                keys_send(1, p_key_pattern, 0);
                break;
            }
            case BUTTON_PLAY_PAUSE:
            {
                SEGGER_RTT_printf(0, "\x1B[32mPLAY_PAUSE\x1B[0m\n");

                m_consumer_pressed = true;

                uint8_t  data[2];
                data[0] = 0xcd;
                data[1] = 0x00;

                send_consumer_key(data);

                break;
            }
            case BUTTON_HOME:
            {
                SEGGER_RTT_printf(0, "\x1B[32mHOME\x1B[0m\n");

                m_consumer_pressed = true;

                uint8_t  data[2];
                data[0] = 0x23;
                data[1] = 0x02;

                send_consumer_key(data);

                break;
            }
            case BUTTON_VOICE:
            {
                m_consumer_pressed = true;

                uint8_t  data[2];
                data[0] = 0x21;
                data[1] = 0x02;

                send_consumer_key(data);

                break;
            }
            case BUTTON_BACK:
            {
                p_key_pattern[0] = 0x29;
                keys_send(1, p_key_pattern, 0x00);
                break;
            }
            case BUTTON_NONE:
            {
                if (!m_consumer_pressed) {
                    SEGGER_RTT_printf(0, "\x1B[32msend non consumer release\x1B[0m\n");
                    p_key_pattern[0] = 0x00;
                    keys_send(1, p_key_pattern, 0);
                }
                else
                {
                    SEGGER_RTT_printf(0, "\x1B[32msend consumer release\x1B[0m\n");

                    uint8_t  data[2];
                    data[0] = 0x00;
                    data[1] = 0x00;

                    send_consumer_key(data);
                }
                m_consumer_pressed = false;
                m_button_pressed = false;
                break;
            }
            }
        }
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
    uint32_t err_code;

    // Initialize timer module, making it use the scheduler.
    APP_TIMER_APPSH_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, true);

    // Create battery timer.
    err_code = app_timer_create(&m_battery_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                battery_level_meas_timeout_handler);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_create(&m_adc_sampling_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                adc_sampling_timeout_handler);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_create(&m_dpad_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                dpad_timeout_handler);
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
        SEGGER_RTT_printf(0, "\x1B[32mBLE_ADV_EVT_DIRECTED\x1B[0m\n");
        err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING_DIRECTED);
        APP_ERROR_CHECK(err_code);
        break;
    case BLE_ADV_EVT_FAST:
        SEGGER_RTT_printf(0, "\x1B[32mBLE_ADV_EVT_FAST\x1B[0m\n");
        err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
        APP_ERROR_CHECK(err_code);
        break;
    case BLE_ADV_EVT_SLOW:
        SEGGER_RTT_printf(0, "\x1B[32mBLE_ADV_EVT_SLOW\x1B[0m\n");
        err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING_SLOW);
        APP_ERROR_CHECK(err_code);
        break;
    case BLE_ADV_EVT_FAST_WHITELIST:
        SEGGER_RTT_printf(0, "\x1B[32mBLE_ADV_EVT_FAST_WHITELIST\x1B[0m\n");
        err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING_WHITELIST);
        APP_ERROR_CHECK(err_code);
        break;
    case BLE_ADV_EVT_SLOW_WHITELIST:
        SEGGER_RTT_printf(0, "\x1B[32mBLE_ADV_EVT_SLOW_WHITELIST\x1B[0m\n");
        err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING_WHITELIST);
        APP_ERROR_CHECK(err_code);
        break;
    case BLE_ADV_EVT_IDLE:
        SEGGER_RTT_printf(0, "\x1B[32mBLE_ADV_EVT_IDLE\x1B[0m\n");
        sleep_mode_enter();
        break;

    case BLE_ADV_EVT_WHITELIST_REQUEST:
    {
        SEGGER_RTT_printf(0, "\x1B[32mBLE_ADV_EVT_WHITELIST_REQUEST\x1B[0m\n");
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
        SEGGER_RTT_printf(0, "\x1B[32mBLE_ADV_EVT_PEER_ADDR_REQUEST\x1B[0m\n");
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


/**@brief Function for handling the Application's BLE Stack events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 */
static void on_ble_evt(ble_evt_t * p_ble_evt)
{
    uint32_t                              err_code;
    ble_gatts_rw_authorize_reply_params_t auth_reply;

    switch (p_ble_evt->header.evt_id)
    {
    case BLE_GAP_EVT_CONNECTED:
        SEGGER_RTT_printf(0, "\x1B[32mBLE_GAP_EVT_CONNECTED\x1B[0m\n");
        err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
        APP_ERROR_CHECK(err_code);
        err_code = app_button_enable();
        APP_ERROR_CHECK(err_code);
        m_conn_handle      = p_ble_evt->evt.gap_evt.conn_handle;
        break;

    case BLE_EVT_TX_COMPLETE:
        SEGGER_RTT_printf(0, "\x1B[32mBLE_EVT_TX_COMPLETE\x1B[0m\n");
        break;

    case BLE_GAP_EVT_DISCONNECTED:
        SEGGER_RTT_printf(0, "\x1B[32mBLE_GAP_EVT_DISCONNECTED\x1B[0m\n");

        m_conn_handle = BLE_CONN_HANDLE_INVALID;
        err_code = app_button_disable();
        APP_ERROR_CHECK(err_code);
        // Reset m_caps_on variable. Upon reconnect, the HID host will re-send the Output
        // report containing the Caps lock state.
        m_caps_on = false;
        // disabling alert 3. signal - used for capslock ON
        err_code = bsp_indication_set(BSP_INDICATE_ALERT_OFF);
        APP_ERROR_CHECK(err_code);
        break;

    case BLE_EVT_USER_MEM_REQUEST:
        SEGGER_RTT_printf(0, "\x1B[32mBLE_EVT_USER_MEM_REQUEST\x1B[0m\n");
        err_code = sd_ble_user_mem_reply(m_conn_handle, NULL);
        APP_ERROR_CHECK(err_code);
        break;

    case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
        SEGGER_RTT_printf(0, "\x1B[32mBLE_GATTS_EVT_RW_AUTHORIZE_REQUEST\x1B[0m\n");
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

    case BLE_GATTC_EVT_TIMEOUT:
    case BLE_GATTS_EVT_TIMEOUT:
        SEGGER_RTT_printf(0, "\x1B[32mBLE_GATTS_EVT_TIMEOUT\x1B[0m\n");
        // Disconnect on GATT Server and Client timeout events.
        err_code = sd_ble_gap_disconnect(m_conn_handle,
                                         BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
        APP_ERROR_CHECK(err_code);
        break;

    default:
        // No implementation needed.
        break;
    }
}


/**@brief   Function for dispatching a BLE stack event to all modules with a BLE stack event handler.
 *
 * @details This function is called from the scheduler in the main loop after a BLE stack
 *          event has been received.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 */
static void ble_evt_dispatch(ble_evt_t * p_ble_evt)
{
    dm_ble_evt_handler(p_ble_evt);
    bsp_btn_ble_on_ble_evt(p_ble_evt);
    on_ble_evt(p_ble_evt);
    ble_advertising_on_ble_evt(p_ble_evt);
    ble_conn_params_on_ble_evt(p_ble_evt);
    ble_hids_on_ble_evt(&m_hids, p_ble_evt);
    ble_bas_on_ble_evt(&m_bas, p_ble_evt);
    ble_displays_on_ble_evt(&m_dis, p_ble_evt);
}


/**@brief   Function for dispatching a system event to interested modules.
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
    SEGGER_RTT_printf(0, "\x1B[32mbutton_event_handler\x1B[0m\n");

    uint8_t old_state = button_state;

    if (button_action == 0) {
        SEGGER_RTT_printf(0, "\x1B[32mold_state : %s : release received for : %d\x1B[0m\n", BUTTON_STATE_STRING_ENUM[button_state], pin_no);

        button_state = BUTTON_NONE;
    }
    else {

        switch (pin_no)
        {
        case BUTTON_S0:

            SEGGER_RTT_printf(0, "\x1B[32mbutton BUTTON_S0 : VOICE\x1B[0m\n");
            button_state = BUTTON_VOICE;
            m_button_pressed = true;
            break;
        case BUTTON_S1:

            SEGGER_RTT_printf(0, "\x1B[32mbutton BUTTON_S1 : BACK\x1B[0m\n");
            button_state = BUTTON_BACK;
            m_button_pressed = true;
            break;

        case BUTTON_S2:
            button_state = BUTTON_PLAY_PAUSE;
            m_button_pressed = true;
            SEGGER_RTT_printf(0, "\x1B[32mbutton BUTTON_S2 : BUTTON_PLAY_PAUSE\x1B[0m\n");
            break;

        case BUTTON_S3:

            SEGGER_RTT_printf(0, "\x1B[32mbutton BUTTON_S3 : HOME\x1B[0m\n");
            button_state = BUTTON_HOME;
            m_button_pressed = true;
            break;
        default:
            //APP_ERROR_HANDLER(pin_no);
            break;
        }
    }

    if (old_state != button_state) {
        SEGGER_RTT_printf(0, "\x1B[32mbutton state2 : %s\x1B[0m\n", BUTTON_STATE_STRING_ENUM[button_state]);
        button_state_change = true;
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

/**@brief Function for handling events from the BSP module.
 *
 * @param[in]   event   Event generated by button press.
 */
static void bsp_event_handler(bsp_event_t event)
{
    uint32_t err_code;

    switch (event)
    {
    case BSP_EVENT_SLEEP:
        SEGGER_RTT_printf(0, "\x1B[32mBSP_EVENT_SLEEP\x1B[0m\n");
        sleep_mode_enter();
        break;

    case BSP_EVENT_DISCONNECT:
        SEGGER_RTT_printf(0, "\x1B[32mBSP_EVENT_DISCONNECT\x1B[0m\n");
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
        if (err_code != NRF_ERROR_INVALID_STATE)
        {
            APP_ERROR_CHECK(err_code);
        }
        break;

    case BSP_EVENT_WHITELIST_OFF:
        SEGGER_RTT_printf(0, "\x1B[32mBSP_EVENT_WHITELIST_OFF\x1B[0m\n");
        err_code = ble_advertising_restart_without_whitelist();
        if (err_code != NRF_ERROR_INVALID_STATE)
        {
            APP_ERROR_CHECK(err_code);
        }
        break;
    case BSP_EVENT_KEY_0:
        SEGGER_RTT_printf(0, "\x1B[32mBSP_EVENT_KEY_0\x1B[0m\n");
        err_code = ble_advertising_restart_without_whitelist();
        if (err_code != NRF_ERROR_INVALID_STATE)
        {
            APP_ERROR_CHECK(err_code);
        }

        break;
    default:
        break;
    }
}

/**@brief Function for initializing the Advertising functionality.
 */
static void advertising_init(void)
{
    uint32_t       err_code;
    uint8_t        adv_flags;
    ble_advdata_t  advdata;

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
        BLE_ADV_WHITELIST_DISABLED,
        BLE_ADV_DIRECTED_ENABLED,
        BLE_ADV_DIRECTED_SLOW_DISABLED, 0, 0,
        BLE_ADV_FAST_ENABLED, APP_ADV_FAST_INTERVAL, APP_ADV_FAST_TIMEOUT,
        BLE_ADV_SLOW_ENABLED, APP_ADV_SLOW_INTERVAL, APP_ADV_SLOW_TIMEOUT
    };

    err_code = ble_advertising_init(&advdata, NULL, &options, on_adv_evt, ble_advertising_error_handler);
    APP_ERROR_CHECK(err_code);
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


/**@brief ADC interrupt handler.
 * @details  This function will fetch the conversion result from the ADC, convert the value into
 *           percentage and send it to peer.
 */
void ADC_IRQHandler(void)
{
    //SEGGER_RTT_printf(0, "\x1B[32mADC_IRQHandler\x1B[0m\n");

    NRF_ADC->EVENTS_END = 0;
    uint16_t adc_result = NRF_ADC->RESULT;
    NRF_ADC->TASKS_STOP = 1;

    if (!m_button_pressed)
    {
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

            SEGGER_RTT_printf(0, "\x1B[32mbutton state : %s for %d\x1B[0m\n", BUTTON_STATE_STRING_ENUM[button_state], adc_result);
            button_state_change = true;
        }
    }
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

/**@brief Function for application main entry.
 */
int main(void)
{
    bool erase_bonds;
    uint32_t err_code;

    bsp_configuration();

    // Initialize.
    timers_init();
    gpiote_init();
    adc_init();
    buttons_leds_init(&erase_bonds);

    buttons_init();

    tft_setup();
    fillScreen(ST7735_BLACK);

    ble_stack_init();
    scheduler_init();

    device_manager_init(erase_bonds);

    gap_params_init();
    advertising_init();
    services_init();

    sensor_simulator_init();
    conn_params_init();
    buffer_init();

    // Start execution.
    timers_start();
    err_code = ble_advertising_start(BLE_ADV_MODE_FAST);
    APP_ERROR_CHECK(err_code);

    // Initialize persistent storage module.
    err_code = pstorage_init();
    APP_ERROR_CHECK(err_code);

    init_storage();

    //advertising_start();

    // Enter main loop.
    for (;;)
    {
        app_sched_execute();
        power_manage();
    }
}

/**
 * @}
 */
