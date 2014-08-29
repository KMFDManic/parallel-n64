/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - exception.c                                             *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2002 Hacktarux                                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "api/m64p_types.h"
#include "api/callbacks.h"
#include "memory/memory.h"

#include "exception.h"
#include "r4300.h"
#include "cp0.h"
#include "recomph.h"
#include "tlb.h"

static void exception_common(void)
{
   if(delay_slot==1 || delay_slot==3)
   {
      g_cp0_regs[CP0_CAUSE_REG] |= 0x80000000;
      g_cp0_regs[CP0_EPC_REG] -= 4;
   }
   else
   {
      g_cp0_regs[CP0_CAUSE_REG] &= 0x7FFFFFFF;
   }
   last_addr = PC->addr;
   if (r4300emu == CORE_DYNAREC)
   {
      dyna_jump();
      if (!dyna_interp) delay_slot = 0;
   }
   if (r4300emu != CORE_DYNAREC || dyna_interp)
   {
      dyna_interp = 0;
      if (delay_slot)
      {
         skip_jump = PC->addr;
#ifdef NEB_DYNAREC
         if (r4300emu == CORE_NEB_DYNAREC)
         {
            // The Neb Dynarec does not require a pseudo-interrupt to
            // enter the exception handler. But it does need this:
            delay_slot = 0;
         }
         else
         {
#endif
            // The interpreters and the Old Dynarec need this.
            next_interupt = 0;
#ifdef NEB_DYNAREC
         }
#endif
      }
   }
}

void TLB_refill_exception(unsigned int address, int w)
{
   int usual_handler = 0, i;

   if (r4300emu != CORE_DYNAREC && w != 2) update_count();
   if (w == 1)
      g_cp0_regs[CP0_CAUSE_REG] = (3 << 2);
   else
      g_cp0_regs[CP0_CAUSE_REG] = (2 << 2);
   g_cp0_regs[CP0_BADVADDR_REG] = address;
   g_cp0_regs[CP0_CONTEXT_REG] = (g_cp0_regs[CP0_CONTEXT_REG] & 0xFF80000F) | ((address >> 9) & 0x007FFFF0);
   g_cp0_regs[CP0_ENTRYHI_REG] = address & 0xFFFFE000;
   if (g_cp0_regs[CP0_STATUS_REG] & 0x2) // Test de EXL
   {
      generic_jump_to(0x80000180);
      if(delay_slot==1 || delay_slot==3) g_cp0_regs[CP0_CAUSE_REG] |= 0x80000000;
      else g_cp0_regs[CP0_CAUSE_REG] &= 0x7FFFFFFF;
   }
   else
   {
      if (r4300emu != CORE_PURE_INTERPRETER) 
      {
         if (w!=2)
            g_cp0_regs[CP0_EPC_REG] = PC->addr;
         else
            g_cp0_regs[CP0_EPC_REG] = address;
      }
      else g_cp0_regs[CP0_EPC_REG] = PC->addr;

      g_cp0_regs[CP0_CAUSE_REG] &= ~0x80000000;
      g_cp0_regs[CP0_STATUS_REG] |= 0x2; //EXL=1

      if (address >= 0x80000000 && address < 0xc0000000)
         usual_handler = 1;
      for (i=0; i<32; i++)
      {
         if (/*tlb_e[i].v_even &&*/ address >= tlb_e[i].start_even &&
               address <= tlb_e[i].end_even)
            usual_handler = 1;
         if (/*tlb_e[i].v_odd &&*/ address >= tlb_e[i].start_odd &&
               address <= tlb_e[i].end_odd)
            usual_handler = 1;
      }
      if (usual_handler)
      {
         generic_jump_to(0x80000180);
      }
      else
      {
         generic_jump_to(0x80000000);
      }
   }
   if(w != 2) g_cp0_regs[CP0_EPC_REG] -= 4;

   exception_common();
}

void exception_general(void)
{
   update_count();
   g_cp0_regs[CP0_STATUS_REG] |= 2;

   g_cp0_regs[CP0_EPC_REG] = PC->addr;

   generic_jump_to(0x80000180);

   exception_common();
}

