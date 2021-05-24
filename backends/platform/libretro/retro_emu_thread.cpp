// This is copyrighted software. More information is at the end of this file.
#include "retro_emu_thread.h"

#include <stdio.h>
#include <pthread.h>

#include "base/main.h"
#include "os.h"

static pthread_t main_thread;
static pthread_t emu_thread;
static pthread_mutex_t emu_mutex;
static pthread_mutex_t main_mutex;
static pthread_cond_t emu_cv;
static pthread_cond_t main_cv;
static bool emu_keep_waiting = true;
static bool main_keep_waiting = true;
static bool emu_has_exited = false;
static bool emu_thread_canceled = false;
static bool emu_thread_initialized = false;

static void* retro_run_emulator(void *args)
{
   static const char *argv[20] = {0};
   unsigned i;

   emu_has_exited      = false;
   emu_thread_canceled = false;

   for(i = 0; i < cmd_params_num; i++)
      argv[i] = cmd_params[i];

   scummvm_main(cmd_params_num, argv);
   emu_has_exited = true;

   /* All done - switch back to the main
    * thread for the final time */
   pthread_mutex_lock(&main_mutex);
   main_keep_waiting = false;
   pthread_mutex_unlock(&main_mutex);
   
   pthread_mutex_lock(&emu_mutex);
   pthread_cond_signal(&main_cv);
   pthread_mutex_unlock(&emu_mutex);

   return NULL;
}

static void retro_switch_to_emu_thread()
{
   pthread_mutex_lock(&emu_mutex);
   emu_keep_waiting = false;
   pthread_mutex_unlock(&emu_mutex);
   pthread_mutex_lock(&main_mutex);
   pthread_cond_signal(&emu_cv);

   main_keep_waiting = true;
   while (main_keep_waiting)
   {
      pthread_cond_wait(&main_cv, &main_mutex);
   }
   pthread_mutex_unlock(&main_mutex);
}

static void retro_switch_to_main_thread()
{
   pthread_mutex_lock(&main_mutex);
   main_keep_waiting = false;
   pthread_mutex_unlock(&main_mutex);
   pthread_mutex_lock(&emu_mutex);
   pthread_cond_signal(&main_cv);

   emu_keep_waiting = true;
   while (emu_keep_waiting)
   {
      pthread_cond_wait(&emu_cv, &emu_mutex);
   }
   pthread_mutex_unlock(&emu_mutex);
}

void retro_switch_thread()
{
   if (pthread_self() == main_thread)
      retro_switch_to_emu_thread();
   else
      retro_switch_to_main_thread();
}

bool retro_init_emu_thread(void)
{
   if (emu_thread_initialized)
      return true;

   main_thread = pthread_self();
   if (pthread_mutex_init(&main_mutex, NULL))
      goto main_mutex_error;
   if (pthread_mutex_init(&emu_mutex, NULL))
      goto emu_mutex_error;
   if (pthread_cond_init(&main_cv, NULL))
      goto main_cv_error;
   if (pthread_cond_init(&emu_cv, NULL))
      goto emu_cv_error;
   if (pthread_create(&emu_thread, NULL, retro_run_emulator, NULL))
      goto emu_thread_error;

   emu_thread_initialized = true;
   return true;

emu_thread_error:
   pthread_cond_destroy(&emu_cv);
emu_cv_error:
   pthread_cond_destroy(&main_cv);
main_cv_error:
   pthread_mutex_destroy(&emu_mutex);
emu_mutex_error:
   pthread_mutex_destroy(&main_mutex);
main_mutex_error:
   return false;
}

void retro_deinit_emu_thread()
{
   if (!emu_thread_initialized)
      return;

   pthread_mutex_destroy(&main_mutex);
   pthread_mutex_destroy(&emu_mutex);
   pthread_cond_destroy(&main_cv);
   pthread_cond_destroy(&emu_cv);
   emu_thread_initialized = false;
}

bool retro_is_emu_thread_initialized()
{
   return emu_thread_initialized;
}

void retro_join_emu_thread()
{
   static bool is_joined = false;
   if (is_joined)
      return;

   pthread_join(emu_thread, NULL);
   is_joined = true;
}

void retro_cancel_emu_thread()
{
   if (emu_thread_canceled)
      return;

   pthread_cancel(emu_thread);
   emu_thread_canceled = true;
}

bool retro_emu_thread_exited()
{
   return emu_has_exited;
}

/*

Copyright (C) 2020 Nikos Chantziaras <realnc@gmail.com>

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation, either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program. If not, see <https://www.gnu.org/licenses/>.

*/
