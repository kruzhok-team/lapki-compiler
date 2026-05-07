#pragma once

void reset ( void ) {
  /* m1.Reg
   * ext
       Программный сброс по команде извне
   * int
       Чтобы сбросить контроллер, нужно записать константу 0x5fa
       Другие значения игнорируются
   * tek
       Запись маски в дефолтно-нулевой подрегистр
   * refs
       s4.3.5 p91 PM0223
   */
  SCB -> AIRCR = SCB_AIRCR_SYSRESETREQ_Msk
               | ( 0x5fa << SCB_AIRCR_VECTKEY_Pos )
               ;
}

__attribute__((always_inline))
static inline
void reset_bootloader ( void ) {
  /* m1.Reg
   * ext
       Программный сброс по команде извне
   * int
       Чтобы сбросить контроллер, нужно записать константу 0x5fa
       Другие значения игнорируются
   * tek
       Запись маски в дефолтно-нулевой подрегистр
   * refs
       s4.3.5 p91 PM0223
   */
  bool ok = eepromWrite1(erfs.eeprom.loadB1.addr, erfs.eeprom.loadB1.val);
  if (!ok) return;
  rebootAfter = 10;
}
