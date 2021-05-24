#ifndef EMU_THREAD_H
#define EMU_THREAD_H

#include <stdbool.h>

/* ScummVM doesn't have a top-level main loop that we can use, so instead we run it in its own thread
 * and switch between it and the main thread. Calling this function will block the current thread
 * and unblock the other.
 *
 * This function can be called from either the main or the emulation thread.
 */
void retro_switch_thread(void);

/* Initialize the emulation thread and any related resources.
 *
 * Only call this function from the main thread.
 */
bool retro_init_emu_thread(void);

/* Destroy the emulation thread and any related resources. Only call this after the emulation thread
 * has finished (or canceled) and joined.
 *
 * Only call this function from the main thread.
 */
void retro_deinit_emu_thread(void);

/* Returns true if the emulation thread was initialized successfully.
 *
 * This function can be called from either the main or the emulation thread.
 */
bool retro_is_emu_thread_initialized(void);

/* Join the emulation thread. The thread must have exited naturally or been canceled.
 *
 * Only call this function from the main thread.
 */
void retro_join_emu_thread(void);

/* Cancel the emulation thread.
 *
 * Only call this function from the main thread.
 */
void retro_cancel_emu_thread(void);

/* Returns true if the emulation thread has exited naturally.
 *
 * This function can be called from either the main or the emulation thread.
 */
bool retro_emu_thread_exited(void);

#endif
