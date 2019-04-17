/**************************************************************************//**
 * @file     system_da1469x.c
 * @brief    CMSIS Device System Source File for
 *           DA1469x Device Series
 * @version  V5.00
 * @date     02. November 2016
 ******************************************************************************/
/*
 * Copyright (c) 2009-2016 ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//#include <tinyara/config.h>

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <errno.h>
#include <debug.h>

#include <tinyara/init.h>
#include <arch/board/board.h>

#include "sdk_defs.h"
#include "interrupts.h"

#include "hw_bod.h"
#include "hw_cache.h"
#include "hw_clk.h"
#include "hw_cpm.h"
#include "hw_gpio.h"
#include "hw_memctrl.h"
#include "hw_otpc.h"
#include "hw_pdc.h"
#include "hw_pd.h"
#include "hw_qspi.h"
#include "hw_sys.h"
#include "qspi_automode.h"
#include "sys_tcs.h"
#include "sys_trng.h"

#if dg_configUSE_CLOCK_MGR
#include "sys_clock_mgr.h"
#include "sys_clock_mgr_internal.h"
#endif

extern uint32_t __bss_end__;

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
/* .data is positioned first in the primary RAM followed immediately by .bss.
 * The IDLE thread stack lies just after .bss and has size give by
 * CONFIG_IDLETHREAD_STACKSIZE;  The heap then begins just after the IDLE.
 * ARM EABI requires 64 bit stack alignment.
 */
#define IDLE_STACKSIZE (CONFIG_IDLETHREAD_STACKSIZE & ~7)
#define IDLE_STACK     ((uintptr_t)&__bss_end__ + IDLE_STACKSIZE)
#define HEAP_BASE      ((uintptr_t)&__bss_end__ + IDLE_STACKSIZE)

/* g_idle_topstack: _sbss is the start of the BSS region as defined by the
 * linker script. _ebss lies at the end of the BSS region. The idle task
 * stack starts at the end of BSS and is of size CONFIG_IDLETHREAD_STACKSIZE.
 * The IDLE thread is the thread that the system boots on and, eventually,
 * becomes the IDLE, do nothing task that runs only when there is nothing
 * else to run.  The heap continues from there until the end of memory.
 * g_idle_topstack is a read-only variable the provides this computed
 * address.
 */
const uintptr_t g_idle_topstack = HEAP_BASE;

#ifndef __SYSTEM_CLOCK
# define __SYSTEM_CLOCK                 (SYSCLK_FREQUENCY)
#endif

/*
 * Linker symbols
 *
 * Note: if any of them is missing, please correct your linker script. Please refer to the linker
 * script of pxp_reporter.
 */
extern uint32_t __copy_table_start__;
extern uint32_t __copy_table_end__;
extern uint32_t __zero_table_start__;
extern uint32_t __zero_table_end__;
extern uint8_t end;
extern uint8_t __HeapLimit;

/*
 * Global variables
 */
__RETAINED_RW static uint8_t *heapend = &end;
__RETAINED_UNINIT __UNUSED uint32_t black_orca_chip_version;
__RETAINED_RW uint32_t SystemCoreClock = __SYSTEM_CLOCK;        /*!< System Clock Frequency (Core Clock) */
__RETAINED_RW uint32_t SystemLPClock = dg_configXTAL32K_FREQ;   /*!< System Low Power Clock Frequency (LP Clock) */


/**
 * @brief  Memory safe implementation of newlib's _sbrk().
 *
 */
__LTO_EXT
void *_sbrk(int incr)
{
        uint8_t *newheapstart;

        if (heapend + incr > &__HeapLimit) {
                /* Hitting this, means that the value of _HEAP_SIZE is too small.
                 * The value of incr is in stored_incr at this point. By checking the equation
                 * above, it is straightforward to determine the missing space.
                 */
                volatile int stored_incr __UNUSED;

                stored_incr = incr;
                ASSERT_ERROR(0);

                errno = ENOMEM;
                return (void *)-1;
        }

        newheapstart = heapend;
        heapend += incr;

        return newheapstart;
}

/**
* \brief  SDK implementation of stdlib's rand().
*
int rand(void)
{
        return (int) sys_trng_rand();
}
*/

/**
* \brief  SDK implementation of stdlib's srand().
*
*/
void srand (unsigned __seed)
{
}

/*
 * Dialog default priority configuration table.
 *
 * Content of this table will be applied at system start.
 *
 * \note If interrupt priorities provided by Dialog need to be changed, do not modify this file.
 * Create similar table with SAME name instead somewhere inside code without week attribute,
 * and it will be used instead of table below.
 */
#pragma weak __dialog_interrupt_priorities
INTERRUPT_PRIORITY_CONFIG_START(__dialog_interrupt_priorities)
        PRIORITY_0, /* Start interrupts with priority 0 (highest) */
                /*
                 * Note: Interrupts with priority 0 are not
                 * allowed to perform OS calls.
                 */
        PRIORITY_1, /* Start interrupts with priority 1 */
                CMAC2SYS_IRQn,
                CRYPTO_IRQn,
                RFDIAG_IRQn,
        PRIORITY_2, /* Start interrupts with priority 2 */
                SENSOR_NODE_IRQn,
                DMA_IRQn,
                I2C_IRQn,
                I2C2_IRQn,
                SPI_IRQn,
                SPI2_IRQn,
                ADC_IRQn,
                ADC2_IRQn,
                SRC_IN_IRQn,
                SRC_OUT_IRQn,
                TRNG_IRQn,
        PRIORITY_3, /* Start interrupts with priority 3 */
                SysTick_IRQn,
                UART_IRQn,
                UART2_IRQn,
                UART3_IRQn,
                MRM_IRQn,
                XTAL32M_RDY_IRQn,
                PLL_LOCK_IRQn,
                CHARGER_STATE_IRQn,
                CHARGER_ERROR_IRQn,
                LCD_CONTROLLER_IRQn,
                KEY_WKUP_GPIO_IRQn,
                GPIO_P0_IRQn,
                GPIO_P1_IRQn,
                TIMER_IRQn,
#if !defined(OS_FREERTOS)
                TIMER2_IRQn,
#endif
                TIMER3_IRQn,
                TIMER4_IRQn,
                CAPTIMER1_IRQn,
                RTC_IRQn,
                RTC_EVENT_IRQn,
                MOTOR_CONTROLLER_IRQn,
                LRA_IRQn,
                USB_IRQn,
                PCM_IRQn,
                VBUS_IRQn,
                DCDC_IRQn,
        PRIORITY_4, /* Start interrupts with priority 4 */
        PRIORITY_5, /* Start interrupts with priority 5 */
        PRIORITY_6, /* Start interrupts with priority 6 */
        PRIORITY_7, /* Start interrupts with priority 7 */
        PRIORITY_8, /* Start interrupts with priority 8 */
        PRIORITY_9, /* Start interrupts with priority 9 */
        PRIORITY_10, /* Start interrupts with priority 10 */
        PRIORITY_11, /* Start interrupts with priority 11 */
        PRIORITY_12, /* Start interrupts with priority 12 */
        PRIORITY_13, /* Start interrupts with priority 13 */
        PRIORITY_14, /* Start interrupts with priority 14 */
        PRIORITY_15, /* Start interrupts with priority 15 (lowest) */
#if defined(OS_FREERTOS)
                TIMER2_IRQn,
#endif
INTERRUPT_PRIORITY_CONFIG_END

void set_interrupt_priorities(const int8_t prios[])
{
        uint32_t old_primask, iser, iser2;
        int i = 0;
        uint32_t prio = 0;

        // Assign all bit for preemption to be preempt priority bits.
        // (required by FreeRTOS)
        NVIC_SetPriorityGrouping(0);

        /*
         * We shouldn't change the priority of an enabled interrupt.
         *  1. Globally disable interrupts, saving the global interrupts disable state.
         *  2. Explicitly disable all interrupts, saving the individual interrupt enable state.
         *  3. Set interrupt priorities.
         *  4. Restore individual interrupt enables.
         *  5. Restore global interrupt enable state.
         */
        old_primask = __get_PRIMASK();
        __disable_irq();
        iser  = NVIC->ISER[0];
        iser2 = NVIC->ISER[1];
        NVIC->ICER[0] = iser;
        NVIC->ICER[1] = iser2;

        for (i = 0; prios[i] != PRIORITY_TABLE_END; ++i) {
                switch (prios[i]) {
                case PRIORITY_0:
                case PRIORITY_1:
                case PRIORITY_2:
                case PRIORITY_3:
                case PRIORITY_4:
                case PRIORITY_5:
                case PRIORITY_6:
                case PRIORITY_7:
                case PRIORITY_8:
                case PRIORITY_9:
                case PRIORITY_10:
                case PRIORITY_11:
                case PRIORITY_12:
                case PRIORITY_13:
                case PRIORITY_14:
                case PRIORITY_15:
                        prio = prios[i] - PRIORITY_0;
                        break;
                default:
                        NVIC_SetPriority(prios[i], prio);
                        break;
                }
        }

        NVIC->ISER[0] = iser;
        NVIC->ISER[1] = iser2;
        __set_PRIMASK(old_primask);

        // enable Usage-, Bus-, and MMU Fault
        SCB->SHCSR |= SCB_SHCSR_USGFAULTENA_Msk
                   |  SCB_SHCSR_BUSFAULTENA_Msk
                   |  SCB_SHCSR_MEMFAULTENA_Msk;

		// enablue usage fualt for devide-by-0
		SCB->CCR |= 0x10;
}

/**
 * Check whether the code has been compiled for a chip version that is compatible with the chip
 * it runs on.
 */
static bool is_compatible_chip_version(void)
{
        uint32_t rev = CHIP_VERSION->CHIP_REVISION_REG;
        uint32_t step = CHIP_VERSION->CHIP_TEST1_REG;


#if (dg_configENABLE_DA1469x_AA_SUPPORT) 
        if (rev == 'A' && step == 'A') {
                return true;
        }
#else
        if (rev == 'A' && step == 'B') {
                return true;
        }
#endif

        return false;
}


#if dg_configUSE_CLOCK_MGR == 0

__RETAINED volatile bool nortos_xtal32m_settled;
__RETAINED volatile bool nortos_pll_locked;

void XTAL32M_Ready_Handler(void)
{
        ASSERT_WARNING(hw_clk_is_xtalm_started());
        if (dg_configXTAL32M_SETTLE_TIME_IN_USEC == 0) {
                hw_clk_xtalm_update_rdy_cnt();
        }
        nortos_xtal32m_settled = true;
}

void PLL_Lock_Handler(void)
{
        ASSERT_WARNING(REG_GETF(CRG_XTAL, PLL_SYS_STATUS_REG, PLL_LOCK_FINE));
        nortos_pll_locked = true;
}

/* carry out clock initialization sequence */
static void nortos_clk_setup(void)
{
         hw_clk_enable_lpclk(LP_CLK_IS_RC32K);
         hw_clk_set_lpclk(LP_CLK_IS_RC32K);

         NVIC_ClearPendingIRQ(XTAL32M_RDY_IRQn);
         NVIC_EnableIRQ(XTAL32M_RDY_IRQn);              // Activate XTAL32M Ready IRQ

         hw_clk_enable_sysclk(SYS_CLK_IS_XTAL32M);      // Enable XTAL32M
#if (dg_configENABLE_DA1469x_AA_SUPPORT)
         /* Workaround for bug2522A_075: XTAL32M does not start if V14 is supplied by DCDC */
         if (hw_clk_is_enabled_sysclk(SYS_CLK_IS_XTAL32M) == false) {
                 /* If XTAL32M is still in IDLE state, then the 1V4 rail may be powered by the
                  * DC/DC converter. In this case XTAL32M cannot be enabled if system has been
                  * woken-up by the PDC.
                  *
                  * Toggle 1V4 rail DC/DC converter supply to allow XTAL32M to start.
                  */
                 uint32_t prev_value = DCDC->DCDC_V14_REG;
                 uint32_t value = prev_value;
                 REG_CLR_FIELD(DCDC, DCDC_V14_REG, DCDC_V14_ENABLE_HV, value);
                 REG_CLR_FIELD(DCDC, DCDC_V14_REG, DCDC_V14_ENABLE_LV, value);
                 DCDC->DCDC_V14_REG = value;             // Disable 1V4 rail supply
                 // Wait for LDO to be OK
                 while (REG_GETF(CRG_TOP, ANA_STATUS_REG, LDO_RADIO_OK) == 0);
                 DCDC->DCDC_V14_REG = prev_value;        // Restore previous value

                 // Check if XTAL32M is started
                 ASSERT_WARNING(hw_clk_is_enabled_sysclk(SYS_CLK_IS_XTAL32M));
         }
#endif
         /* Wait for XTAL32M to settle */
         while (!hw_clk_is_xtalm_started());

         nortos_xtal32m_settled = true;

         hw_clk_set_sysclk(SYS_CLK_IS_XTAL32M);

         NVIC_ClearPendingIRQ(PLL_LOCK_IRQn);
         NVIC_EnableIRQ(PLL_LOCK_IRQn);                         // Activate PLL lock IRQ

         nortos_pll_locked = hw_clk_is_pll_locked();
}

#endif  /* OS_BAREMETAL */


static __RETAINED_CODE void configure_cache(void)
{
#if (dg_configCODE_LOCATION == NON_VOLATILE_IS_FLASH)
        uint32_t product_header_addr;
        uint32_t active_fw_image_addr;
        uint32_t active_fw_size;
        uint32_t cache_len;
        uint32_t scanned_sectors = 0;

        /* Configure cache according to the 'Active FW image address'
         * field of the product header and 'FW Size' field of the active
         * FW image header. */

        /* Product header is located either at start of FLASH or at the sector
         * boundary (0x4000) if a configuration script is used */

        product_header_addr = MEMORY_QSPIF_S_BASE;
        while ((((uint8_t*) product_header_addr)[0] != 0x50) &&
                (((uint8_t*) product_header_addr)[1] != 0x70) &&
                (scanned_sectors < 10)) {
                product_header_addr += 0x1000;
                scanned_sectors++;
        }

        /* Get active_fw_image_addr */
        ASSERT_WARNING(((uint8_t*) product_header_addr)[0] == 0x50);
        ASSERT_WARNING(((uint8_t*) product_header_addr)[1] == 0x70);
        memcpy((uint8_t*) &active_fw_image_addr, &((uint8_t*) product_header_addr)[2], 4);
        active_fw_image_addr += MEMORY_QSPIF_S_BASE;

        /* Get active_fw_size and align it to 64K boundary */
        ASSERT_WARNING(((uint8_t*) active_fw_image_addr)[0] == 0x51);
        ASSERT_WARNING(((uint8_t*) active_fw_image_addr)[1] == 0x71);
        memcpy((uint8_t*) &active_fw_size, &((uint8_t*) active_fw_image_addr)[2], 4);
        active_fw_size += (0x10000 - (active_fw_size % 0x10000)) % 0x10000;

        /* Calculate length of QSPI FLASH cacheable memory
         * (Cached area len will be (cache_len * 64) KBytes, cache_len can be 0 to 512) */
        cache_len = active_fw_size >> 16;

        /* cache_len shouldn't set any bits that do not fit in
         * CACHE_CTRL2_REG.CACHE_LEN */
        ASSERT_WARNING((cache_len & CACHE_CACHE_CTRL2_REG_CACHE_LEN_Msk) == cache_len);

        GLOBAL_INT_DISABLE();

        hw_cache_set_len(cache_len);

        if (dg_configCACHE_ASSOCIATIVITY != CACHE_ASSOC_AS_IS) {
                if (hw_cache_get_assoc() != dg_configCACHE_ASSOCIATIVITY) {
                        /* override the set associativity setting */
                        hw_cache_set_assoc(CACHE->CACHE_ASSOCCFG_REG);
                }
        }

        if (dg_configCACHE_LINESZ != CACHE_LINESZ_AS_IS) {
                if (hw_cache_get_linesz() != dg_configCACHE_LINESZ) {
                        /* override the cache line setting */
                        hw_cache_set_linesz(dg_configCACHE_LINESZ);
                }
        }

        //REG_SETF(CACHE, CACHE_MRM_CTRL_REG, MRM_START, 1);

        /* flush cache */
        hw_cache_flush();

        GLOBAL_INT_RESTORE();
#endif /* (dg_configCODE_LOCATION == NON_VOLATILE_IS_FLASH) */
}

/* this function configures the PDC table only the first time it is called */
static void configure_pdc(void)
{
        uint32_t pdc_entry_index __UNUSED;
        bool no_syscpu_pdc_entries = true;
        NVIC_DisableIRQ(PDC_IRQn);
        NVIC_ClearPendingIRQ(PDC_IRQn);

#if defined(CONFIG_USE_BLE) || (dg_configENABLE_DEBUGGER == 1)
        /* Set up PDC entry for CMAC2SYS IRQ or VBUS IRQ or debugger */
        pdc_entry_index = hw_pdc_add_entry(HW_PDC_LUT_ENTRY_VAL(
                                                HW_PDC_TRIG_SELECT_PERIPHERAL,
                                                HW_PDC_PERIPH_TRIG_ID_COMBO,
                                                HW_PDC_MASTER_CM33,
                                                (dg_configENABLE_XTAL32M_ON_WAKEUP ? HW_PDC_LUT_ENTRY_EN_XTAL : 0)));
        hw_pdc_set_pending(pdc_entry_index);
        hw_pdc_acknowledge(pdc_entry_index);
        no_syscpu_pdc_entries = false;
#endif

#if defined(CONFIG_USE_BLE)
        /*
         * Set up PDC entry for CMAC wakeup from MAC timer.
         * This entry is also used for the SYS2CMAC mailbox interrupt.
         */
        pdc_entry_index = hw_pdc_add_entry(HW_PDC_LUT_ENTRY_VAL(
                                                        HW_PDC_TRIG_SELECT_PERIPHERAL,
                                                        HW_PDC_PERIPH_TRIG_ID_MAC_TIMER,
                                                        HW_PDC_MASTER_CMAC,
                                                        HW_PDC_LUT_ENTRY_EN_XTAL));

        hw_pdc_set_pending(pdc_entry_index);
        hw_pdc_acknowledge(pdc_entry_index);
#endif

#if defined(OS_FREERTOS)
        /* FreeRTOS Timer requires PD_TIM to be always on */
        REG_SETF(CRG_TOP, PMU_CTRL_REG, TIM_SLEEP, 0);
        while (REG_GETF(CRG_TOP, SYS_STAT_REG, TIM_IS_UP) == 0) {
        }

        /* Add PDC entry to wakeup from Timer2 */
        pdc_entry_index = hw_pdc_add_entry(HW_PDC_LUT_ENTRY_VAL(
                                                HW_PDC_TRIG_SELECT_PERIPHERAL,
                                                HW_PDC_PERIPH_TRIG_ID_TIMER2,
                                                HW_PDC_MASTER_CM33,
                                                (dg_configENABLE_XTAL32M_ON_WAKEUP ? HW_PDC_LUT_ENTRY_EN_XTAL : 0)));
        hw_pdc_set_pending(pdc_entry_index);
        hw_pdc_acknowledge(pdc_entry_index);
        no_syscpu_pdc_entries = false;
#endif

        /* Let SYSCPU goto sleep when needed */
        if (!no_syscpu_pdc_entries) {
                REG_SETF(CRG_TOP, PMU_CTRL_REG, SYS_SLEEP, 1);
        }

        /* clear the PDC IRQ since it will be pending here */
        NVIC_ClearPendingIRQ(PDC_IRQn);
}

/**
 * Basic system setup.
 *
 * @brief  Setup the AMBA clocks. Ensure proper alignment of copy and zero table entries.
 *
 * @note   No variable initialization should take place here, since copy & zero tables
 *         have not yet been initialized yet and any modifications to variables will
 *         be discarded. For the same reason, functions that initialize or are
 *         using initialized variables should not be called from here.
 */
void SystemInitPre(void) __attribute__((section("text_reset")));
void SystemInitPre(void)
{
        /*
         * Bandgap has already been set by the bootloader.
         * Use fast clocks from now on.
         */
        hw_clk_set_hclk_div(0);
        hw_clk_set_pclk_div(0);

        /*
         * Disable pad latches
         */
        hw_gpio_pad_latch_disable_all();

        /*
         * Make sure we are running on a chip version that the code has been built for.
         */
        ASSERT_WARNING_UNINIT(is_compatible_chip_version());

        /*
         * Ensure 4-byte alignment for all elements of each entry in the Copy Table.
         * If any of the assertions below hits, please correct your linker script
         * file accordingly!
         */
        if (dg_configIMAGE_SETUP == DEVELOPMENT_MODE) {
                uint32_t *p;

                for (p = &__copy_table_start__; p < &__copy_table_end__; p += 3) {
                        ASSERT_WARNING_UNINIT( (p[0] & 0x3) == 0 );     // from
                        ASSERT_WARNING_UNINIT( (p[1] & 0x3) == 0 );     // to
                        ASSERT_WARNING_UNINIT( (p[2] & 0x3) == 0 );     // size
                }
        }

        /*
         * Ensure 4-byte alignment for all elements of each entry in the Data Table.
         * If any of the assertions below hits, please correct your linker script
         * file accordingly!
         */
        if (dg_configIMAGE_SETUP == DEVELOPMENT_MODE) {
                uint32_t *p;

                for (p = &__zero_table_start__; p < &__zero_table_end__; p += 2) {
                        ASSERT_WARNING_UNINIT( (p[0] & 0x3) == 0 );    // start at
                        ASSERT_WARNING_UNINIT( (p[1] & 0x3) == 0 );    // size
                }
        }

        /*
         * Clear all PDC entries and make sure SYS_SLEEP is 0.
         */
        REG_SETF(CRG_TOP, PMU_CTRL_REG, SYS_SLEEP, 0);
        hw_pdc_lut_reset();

        /*
         * Reset memory controller.
         */
        hw_memctrl_reset();

        /*
         * Initialize power domains
         */
        GLOBAL_INT_DISABLE();
        REG_SETF(CRG_TOP, PMU_CTRL_REG, RADIO_SLEEP, 1);
        while (!REG_GETF(CRG_TOP, SYS_STAT_REG, RAD_IS_DOWN));
        REG_SETF(CRG_TOP, PMU_CTRL_REG, PERIPH_SLEEP, 1);
        while (!REG_GETF(CRG_TOP, SYS_STAT_REG, PER_IS_DOWN));
        REG_SETF(CRG_TOP, PMU_CTRL_REG, COM_SLEEP, 1);
        while (!REG_GETF(CRG_TOP, SYS_STAT_REG, COM_IS_DOWN));
        /*
         * PD_TIM is kept active so that XTALRDY_CTRL_REG [XTALRDY_CNT]
         * can be programmed properly.
         */
        REG_SETF(CRG_TOP, PMU_CTRL_REG, TIM_SLEEP, 0);
        while (!REG_GETF(CRG_TOP, SYS_STAT_REG, TIM_IS_UP));
        GLOBAL_INT_RESTORE();

        /*
         * Keep CMAC core under reset
         */
        REG_SETF(CRG_TOP, CLK_RADIO_REG, CMAC_CLK_ENABLE, 0);
        REG_SETF(CRG_TOP, CLK_RADIO_REG, CMAC_SYNCH_RESET, 1);

        /*
         * Disable unused peripherals
         */
#if (dg_configCODE_LOCATION == NON_VOLATILE_IS_FLASH)
        /*
         * Booter has already set QSPI_ENABLE to 1 and it must not
         * be disabled since we are executing from flash.
         */
#else
        /*
         * Since we are executing from RAM, QSPI can be disabled.
         */
        REG_SETF(CRG_TOP, CLK_AMBA_REG, QSPI_ENABLE, 0);
#endif

        REG_SETF(CRG_TOP, CLK_AMBA_REG, QSPI2_ENABLE, 0);
        REG_SETF(CRG_TOP, CLK_AMBA_REG, AES_CLK_ENABLE, 0);
        REG_SETF(CRG_TOP, CLK_AMBA_REG, TRNG_CLK_ENABLE, 0);
        REG_SETF(CRG_TOP, CLK_AMBA_REG, OTP_ENABLE, 0);
}

/**
 * Initialize the system
 *
 * @brief  Setup the microcontroller system.
 *         Initialize the System.
 */
void SystemInit(void)
{
        /*
         * Detect chip version (optionally).
         */
        if (dg_configUSE_AUTO_CHIP_DETECTION == 1) {
                black_orca_chip_version = black_orca_get_chip_version();

                //if (!CHIP_IS_AA) {
                        //ASSERT_WARNING_UNINIT(0);
                //}
        }

        REG_SETF(CRG_TOP, POWER_CTRL_REG, LDO_RADIO_ENABLE, 1); // Switch on the RF LDO

        /*
         * Initialize busy status register
         */
        hw_sys_sw_bsr_init();

        /*
         * Apply default priorities to interrupts.
         */
        set_interrupt_priorities(__dialog_interrupt_priorities);

        SystemCoreClock = __SYSTEM_CLOCK;
        SystemLPClock = dg_configXTAL32K_FREQ;


#if (dg_configENABLE_DA1469x_AA_SUPPORT)
        /*
         * SDADC patch
         */
        hw_sys_pd_com_enable();
        REG_SETF(SDADC, SDADC_CTRL_REG, SDADC_EN, 1); //enable SD ADC
        while (REG_GETF(SDADC, SDADC_CTRL_REG,SDADC_LDO_OK) == 0); //wait until LDO is up
        REG_SETF(SDADC, SDADC_CTRL_REG, SDADC_EN, 0); //disable SD ADC
        hw_sys_pd_com_disable();
#endif

        hw_qspi_disable_init(HW_QSPIC);             // Disable QSPI init after wakeup
//        qspi_automode_init();               // The bootloader may have left the Flash in wrong mode...

        /* Already up in SystemInitPre() */
        ASSERT_WARNING(hw_pd_check_tim_status());

#if dg_configUSE_CLOCK_MGR
        cm_clk_init_low_level_internal();
#else
        hw_clk_xtalm_configure();
        if (dg_configXTAL32M_SETTLE_TIME_IN_USEC != 0) {
                hw_clk_set_xtalm_settling_time(XTAL32M_USEC_TO_256K_CYCLES(dg_configXTAL32M_SETTLE_TIME_IN_USEC)/8, false);
        }
#endif

        configure_pdc();

#if dg_configUSE_CLOCK_MGR
        // Always enable the XTAL32M
        cm_enable_xtalm();
        while (!cm_poll_xtalm_ready());                 // Wait for XTAL32M to settle
        hw_clk_set_sysclk(SYS_CLK_IS_XTAL32M);          // Set XTAL32M as sys_clk

#if (dg_configENABLE_DA1469x_AA_SUPPORT)
        /* Workaround for bug2522A_050: SW needed to overrule the XTAL calibration state machine */
        hw_clk_perform_init_rcosc_calibration();        // Perform initial RCOSC calibration
#endif

        /*
         * Note: If the LP clock is the RCX then we have to wait for the XTAL32M to settle
         *       since we need to estimate the frequency of the RCX before continuing
         *       (calibration procedure).
         */
        if ((dg_configLP_CLK_SOURCE == LP_CLK_IS_ANALOG) && (dg_configUSE_LP_CLK == LP_CLK_RCX)) {
                cm_rcx_calibrate();
                hw_clk_set_lpclk(LP_CLK_IS_RCX);        // Set RCX as the LP clock
        }
#else
        /* perform clock initialization here, as there is no clock manager to do it later for us */
        nortos_clk_setup();
#endif
        /* enable OTP to read TCS values */
        hw_otpc_init();
        hw_otpc_set_speed(HW_OTPC_CLK_FREQ_32MHz);
        hw_otpc_enter_mode(HW_OTPC_MODE_READ);
        /* get TCS values */
        sys_tcs_get_trim_values_from_cs();

        /* Close OPT */
        hw_otpc_close();
        configure_cache();

#if defined(CONFIG_RETARGET) || defined(CONFIG_RTT)
        /* This is needed to initialize stdout, so that it can be used by putchar (that doesn't initialize stdout,
         * contrary to printf). Putchar is needed by the Unity test framework
         * This also has the side effect of changing stdout to unbuffered (which seems more reasonable)
         */
        setvbuf(stdout, NULL, _IONBF, 0);
#endif

        /*
         * Keep PD_PER enabled.
         */
        hw_sys_pd_periph_enable();

        /* Default settings to be used if no CS setting is available*/
        CHARGER->CHARGER_TEST_CTRL_REG = DEFAULT_CHARGER_TEST_CTRL_REG;

        /*
         * Apply tcs settings.
         * They need to be re-applied when the blocks they contain are enabled.
         * PD_MEM is by default enabled.
         * PD_AON settings are applied by the booter
         */
        sys_tcs_apply_reg_pairs(SYS_TCS_GROUP_PD_MEM);
        sys_tcs_apply_reg_pairs(SYS_TCS_GROUP_PD_PER);
        /* In non baremetal apps PD_COMM will be opened by the  power manager */
#ifdef OS_BAREMETAL
        hw_sys_pd_com_enable();
        sys_tcs_apply_reg_pairs(SYS_TCS_GROUP_PD_COMM);
#endif
        sys_tcs_apply_reg_pairs(SYS_TCS_GROUP_PD_SYS);
        /* PD_TMR may not be enabled in baremetal configuration */
        if (REG_GETF(CRG_TOP, SYS_STAT_REG, TIM_IS_UP) == 1) {
                sys_tcs_apply_reg_pairs(SYS_TCS_GROUP_PD_TMR);
        }

        /*
         * Apply custom trim settings which don't require the respective block to be enabled
         */
        sys_tcs_apply_custom_values(SYS_TCS_GROUP_GP_ADC_SINGLE_MODE, sys_tcs_custom_values_system_cb, NULL);
        sys_tcs_apply_custom_values(SYS_TCS_GROUP_GP_ADC_DIFF_MODE, sys_tcs_custom_values_system_cb, NULL);

        /*
         * Apply preferred settings on top of tcs settings.
         */
        hw_sys_set_preferred_values();
#if (dg_configENABLE_DA1469x_AA_SUPPORT)
        /*
         * SDADC patch
         */
        hw_sys_pd_com_enable();
        REG_SETF(SDADC, SDADC_CTRL_REG, SDADC_EN, 1); //enable SD ADC
        while (REG_GETF(SDADC, SDADC_CTRL_REG,SDADC_LDO_OK) == 0); //wait until LDO is up
        REG_SETF(SDADC, SDADC_CTRL_REG, SDADC_EN, 0); //disable SD ADC
        hw_sys_pd_com_disable();
#endif
        /*
         * BOD protection
         */
#if (dg_configUSE_BOD == 1)
        /* BOD has already been enabled at this point but it must be reconfigured */
        hw_bod_configure();
#else
        hw_bod_deactivate();
#endif

	os_start();

	while (1) ;
}

uint32_t black_orca_phy_addr(uint32_t addr)
{
        uint32_t phy_addr;
        uint32_t flash_region_base_offset;
        uint32_t flash_region_size;
        uint8_t remap_addr0;
        static const uint32 remap[] = {
                MEMORY_ROM_BASE,
                MEMORY_OTP_BASE,
                MEMORY_QSPIF_BASE,
                MEMORY_SYSRAM_BASE,
                MEMORY_QSPIF_S_BASE,
                MEMORY_OTP_BASE,
                MEMORY_CACHERAM_BASE,
                0
        };

        static const uint32 flash_region_sizes[] = {
             32 * 1024 * 1024,
             16 * 1024 * 1024,
             8 * 1024 * 1024,
             4 * 1024 * 1024,
             2 * 1024 * 1024,
             1 * 1024 * 1024,
             512 * 1024,
             256 * 1024,
        };

        remap_addr0 = REG_GETF(CRG_TOP, SYS_CTRL_REG, REMAP_ADR0);

        if (remap_addr0 != 2) {
                if (addr >= MEMORY_REMAPPED_END) {
                        phy_addr = addr;
                } else {
                        phy_addr = addr + remap[remap_addr0];
                }
        } else {
                /* Take into account flash region base, offset and size */
                flash_region_base_offset = REG_GETF(CACHE, CACHE_FLASH_REG, FLASH_REGION_BASE) << CACHE_CACHE_FLASH_REG_FLASH_REGION_BASE_Pos;
                flash_region_base_offset += REG_GETF(CACHE, CACHE_FLASH_REG, FLASH_REGION_OFFSET) << 2;
                flash_region_size = flash_region_sizes[REG_GETF(CACHE, CACHE_FLASH_REG, FLASH_REGION_SIZE)];
                if (addr < MEMORY_REMAPPED_END) {
                        /*
                         * In the remapped region, accesses are only allowed when
                         * 0 <= addr < flash_region_size.
                         */
                        ASSERT_ERROR(addr < flash_region_size);

                        phy_addr = flash_region_base_offset + addr;
                } else if (IS_QSPIF_ADDRESS(addr)) {
                        /*
                         * In QSPI AHB-C bus, accesses are only allowed when
                         * flash_region_base_offset <= addr
                         *   AND
                         * addr < flash_region_base_offset + flash_region_base_offset
                         */
                        ASSERT_ERROR(addr >= flash_region_base_offset);
                        ASSERT_ERROR(addr < flash_region_base_offset + flash_region_size);
                        phy_addr = addr;
                } else {
                        phy_addr = addr;
                }
        }

        return phy_addr;
}
