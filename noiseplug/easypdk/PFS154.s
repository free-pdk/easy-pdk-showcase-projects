;
;Copyright (C) 2021  freepdk  https://free-pdk.github.io
;
;This program is free software: you can redistribute it and/or modify
;it under the terms of the GNU General Public License as published by
;the Free Software Foundation, either version 3 of the License, or
;(at your option) any later version.
;
;This program is distributed in the hope that it will be useful,
;but WITHOUT ANY WARRANTY; without even the implied warranty of
;MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;GNU General Public License for more details.
;
;You should have received a copy of the GNU General Public License
;along with this program.  If not, see <http://www.gnu.org/licenses/>.


;defines for PFS154

.area RSEG (ABS)
__clkmd   = 0x03

__inten   = 0x04
__intrq   = 0x05

__pa      = 0x10
__pac     = 0x11

__tm2b    = 0x09
__tm2s    = 0x17
__tm2c    = 0x1c
__tm2ct   = 0x1d
__misclvr = 0x1b

.macro EASY_PDK_INIT_SYSCLOCK_8MHZ
        mov a, #0x20
        mov.io __misclvr, a
        mov a, #0x34
        mov.io __clkmd, a
.endm

.macro EASY_PDK_CALIBRATE_IHRC_8192000HZ_AT_5V
        and a, #'R'
        and a, #'C'
        and a, #((1))
        and a, #((8192000))
        and a, #((8192000)>>8)
        and a, #((8192000)>>16)
        and a, #((8192000)>>24)
        and a, #((5000))
        and a, #((5000)>>8)
        and a, #((0x0B))
.endm
