/**
 * \addtogroup BSP
 * \{
 * \addtogroup DEVICES
 * \{
 * \addtogroup FEM
 * \{
 * \brief Front End Module for SKYWORKS SKY66112-11
 */

/**
 ****************************************************************************************
 *
 * @file hw_fem_sky66112-11.h
 *
 * @brief FEM Driver for SKYWORKS SKY66112-11 Low Level Driver API.
 *
 * Copyright (C) 2015-2018 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef HW_FEM_H_
#define HW_FEM_H_

#if dg_configFEM == FEM_SKY66112_11
#include <stdbool.h>

#if defined(dg_configFEM_SKY66112_11_FEM_BIAS_V18) && defined(dg_configFEM_SKY66112_11_FEM_BIAS_V18P)
#error Only one of dg_configFEM_SKY66112_11_FEM_BIAS_V18 and dg_configFEM_SKY66112_11_FEM_BIAS_V18P can be set at a time
#endif

/* Configuration/state structure (actually just a byte) */
typedef struct __attribute__((__packed__)) {
#if dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_A
        uint8_t tx_power: 1;
        uint8_t tx_bypass: 1;
        uint8_t rx_bypass: 1;
#else
        uint8_t tx_power_ble: 1;
        uint8_t tx_bypass_ble: 1;
        uint8_t rx_bypass_ble: 1;
        uint8_t tx_power_ftdf: 1;
        uint8_t tx_bypass_ftdf: 1;
        uint8_t rx_bypass_ftdf: 1;
#endif
        uint8_t antsel: 1;
        uint8_t started: 1;
} hw_fem_config;

#if dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_A
/**
 * \brief Configures FEM TX Power
 *
 * \param [in] high Set true to enable high TX power
 *
 */
void hw_fem_set_txpower(bool high);

/**
 * \brief Configures FEM TX bypass mode
 *
 * \param [in] enable false: Use PA, true: bypass
 *
 */
void hw_fem_set_tx_bypass(bool enable);

/**
 * \brief Configures FEM RX bypass mode
 *
 * \param [in] enable false: Use LNA, true: bypass
 *
 */
void hw_fem_set_rx_bypass(bool enable);

/**
 * \brief Gets the TX Power setting
 *
 * \return true, if TX Power is high, false if low
 *
 */
bool hw_fem_get_txpower(void);

/**
 * \brief Gets the TX bypass mode
 *
 * \return false: No TX bypass, true: TX bypass
 *
 */
bool hw_fem_get_tx_bypass(void);

/**
 * \brief Gets the RX bypass mode
 *
 * \return false: No RX bypass, true: RX bypass
 *
 */
bool hw_fem_get_rx_bypass(void);

#else /* dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_B */
#ifdef CONFIG_USE_BLE
/**
 * \brief Configures FEM TX Power for BLE
 *
 * \param [in] high Set true to enable high TX power
 *
 */
void hw_fem_set_txpower_ble(bool high);

/**
 * \brief Configures FEM TX bypass mode for BLE
 *
 * \param [in] enable false: Use PA, true: bypass
 *
 */
void hw_fem_set_tx_bypass_ble(bool enable);

/**
 * \brief Configures FEM RX bypass mode for BLE
 *
 * \param [in] enable false: Use LNA, true: bypass
 *
 */
void hw_fem_set_rx_bypass_ble(bool enable);

/**
 * \brief Gets the TX Power setting for BLE
 *
 * \return true, if TX Power is high, false if low
 *
 */
bool hw_fem_get_txpower_ble(void);

/**
 * \brief Gets the TX bypass mode for BLE
 *
 * \return false: No TX bypass, true: TX bypass
 *
 */
bool hw_fem_get_tx_bypass_ble(void);

/**
 * \brief Gets the RX bypass mode for BLE
 *
 * \return false: No RX bypass, true: RX bypass
 *
 */
bool hw_fem_get_rx_bypass_ble(void);
#endif /* CONFIG_USE_BLE */

#ifdef CONFIG_USE_FTDF
/**
 * \brief Configures FEM TX Power for FTDF
 *
 * \param [in] high Set true to enable high TX power
 *
 */
void hw_fem_set_txpower_ftdf(bool high);

/**
 * \brief Configures FEM TX bypass mode for FTDF
 *
 * \param [in] enable false: Use PA, true: bypass
 *
 */
void hw_fem_set_tx_bypass_ftdf(bool enable);

/**
 * \brief Configures FEM RX bypass mode for FTDF
 *
 * \param [in] enable false: Use LNA, true: bypass
 *
 */
void hw_fem_set_rx_bypass_ftdf(bool enable);

/**
 * \brief Gets the TX Power setting for FTDF
 *
 * \return true, if TX Power is high, false if low
 *
 */
bool hw_fem_get_txpower_ftdf(void);

/**
 * \brief Gets the TX bypass mode for FTDF
 *
 * \return false: No TX bypass, true: TX bypass
 *
 */
bool hw_fem_get_tx_bypass_ftdf(void);

/**
 * \brief Gets the RX bypass mode for FTDF
 *
 * \return false: No RX bypass, true: RX bypass
 *
 */
bool hw_fem_get_rx_bypass_ftdf(void);
#endif /* CONFIG_USE_FTDF */

/**
 * \brief Configures FEM TX Power
 *
 * \param [in] high Set true to enable high TX power
 *
 * \deprecated Use the BLE/FTDF specific functions instead
 */
void hw_fem_set_txpower(bool high) DEPRECATED;

/**
 * \brief Configures FEM TX bypass mode
 *
 * \param [in] enable false: Use PA, true: bypass
 *
 * \deprecated Use the BLE/FTDF specific functions instead
 */
void hw_fem_set_tx_bypass(bool enable) DEPRECATED;

/**
 * \brief Configures FEM RX bypass mode
 *
 * \param [in] enable false: Use LNA, true: bypass
 *
 * \deprecated Use the BLE/FTDF specific functions instead
 */
void hw_fem_set_rx_bypass(bool enable) DEPRECATED;

/**
 * \brief Gets the TX Power setting
 *
 * \return true, if TX Power is high, false if low
 *
 * \deprecated Use the BLE/FTDF specific functions instead
  */
bool hw_fem_get_txpower(void) DEPRECATED;

/**
 * \brief Gets the TX bypass mode
 *
 * \return false: No TX bypass, true: TX bypass
 *
 * \deprecated Use the BLE/FTDF specific functions instead
 */
bool hw_fem_get_tx_bypass(void) DEPRECATED;

/**
 * \brief Gets the RX bypass mode
 *
 * \return false: No RX bypass, true: RX bypass
 *
 * \deprecated Use the BLE/FTDF specific functions instead
 */
bool hw_fem_get_rx_bypass(void) DEPRECATED;
#endif /* dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_B */


/**
 * \brief Configures FEM Antenna to use
 *
 * \param [in] one false: antenna 0, true: antenna 1
 *
 */
void hw_fem_set_antenna(bool one);

/**
 * \brief Gets the selected antenna
 *
 * \return false: antenna 1, true: antenna 2
 *
 */
bool hw_fem_get_antenna(void);

/**
 * \brief Sets the FEM Bias
 *
 * \param [in] voltage The voltage to set the FEM Bias to (mV)
 *
 * \return 0: success, -1: Out of range value entered, -2: FEM BIAS not supported on this board
 *
 */
int hw_fem_set_bias(uint16_t voltage);

/**
 * \brief Sets the 2nd FEM Bias
 *
 * \param [in] voltage The voltage to set the 2nd FEM Bias to (mV)
 *
 * \return 0: success, -1: Out of range value entered, -2: 2nd FEM BIAS not supported on this board
 *
 */
int hw_fem_set_bias2(uint16_t voltage);

/**
 * \brief Starts and configures FEM.
 *
 * Configures and sets GPIOs according to configuration set by hw_fem_set_config(). Also configures
 * DCF Timers.
 *
 *
 * \note To be called by the RF driver when RF is powered on
 *
 */
void hw_fem_start(void);

/**
 * \brief Stops FEM.
 *
 * Stops DCF timers and sets all FEM control signals to 0 to achieve the lowest possible power
 *
 * \note To be called by the RF driver when RF is powered off
 *
 */
void hw_fem_stop(void);

#endif /* dg_configFEM == FEM_SKY66112_11 */
#endif /* HW_FEM_H_ */

/**
 * \}
 * \}
 * \}
 */
