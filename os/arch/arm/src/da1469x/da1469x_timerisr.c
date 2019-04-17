/****************************************************************************
 *
 * Copyright 2019 Samsung Electronics All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <tinyara/config.h>

#include <stdint.h>
#include <time.h>
#include <debug.h>
#include <tinyara/arch.h>
#include <arch/board/board.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* The desired timer interrupt frequency is provided by the definition
 * CLK_TCK (see include/time.h).  CLK_TCK defines the desired number of
 * system clock ticks per second.  That value is a user configurable setting
 * that defaults to 100 (100 ticks per second = 10 MS interval).
 *
 * The timer counts at the rate SYSCLK_FREQUENCY as defined in the board.h
 * header file.
 */

#define SYSTICK_RELOAD ((SYSCLK_FREQUENCY / CLK_TCK) - 1)

/* The size of the reload field is 24 bits.  Verify taht the reload value
 * will fit in the reload register.
 */

#if SYSTICK_RELOAD > 0x00ffffff
#error SYSTICK_RELOAD exceeds the range of the RELOAD register
#endif

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Global Functions
 ****************************************************************************/

/****************************************************************************
 * Function:  up_timerisr
 *
 * Description:
 *   The timer ISR will perform a variety of services for various portions
 *   of the systems.
 *
 ****************************************************************************/

int up_timerisr(int irq, uint32_t *regs)
{
	/* Process timer interrupt */

	sched_process_timer();
	return 0;
}

/****************************************************************************
 * Function:  up_timer_initialize
 *
 * Description:
 *   This function is called during start-up to initialize
 *   the timer interrupt.
 *
 ****************************************************************************/

void up_timer_initialize(void)
{
#if 0
#ifdef CONFIG_MT7686_USE_CM4_SYSTICK
	uint32_t regval;

	/* Set the SysTick interrupt to the default priority */

	regval = getreg32(NVIC_SYSH12_15_PRIORITY);
	regval &= ~NVIC_SYSH_PRIORITY_PR15_MASK;
	regval |= (NVIC_SYSH_PRIORITY_DEFAULT << NVIC_SYSH_PRIORITY_PR15_SHIFT);
	putreg32(regval, NVIC_SYSH12_15_PRIORITY);

	/* Configure SysTick to interrupt at the requested rate */

	putreg32(SYSTICK_RELOAD, NVIC_SYSTICK_RELOAD);

	/* Attach the timer interrupt vector */

	(void)irq_attach(MT7686_IRQ_SYSTICK, (xcpt_t) up_timerisr, NULL);

	/* Enable SysTick interrupts */

	putreg32((NVIC_SYSTICK_CTRL_CLKSOURCE | NVIC_SYSTICK_CTRL_TICKINT | NVIC_SYSTICK_CTRL_ENABLE), NVIC_SYSTICK_CTRL);

	/* And enable the timer interrupt */

	up_enable_irq(MT7686_IRQ_SYSTICK);

#else
	/*CONFIG_MT7686_USE_GPT*/
	mt7686_os_gpt_init(1000000 / CLOCKS_PER_SEC);
#endif /* MT7686_USE_SYSTICK */
#endif
}