        .module noiseplug
;
; This work is licensed under the Creative Commons Attribution-ShareAlike 3.0 Unported License.
; To view a copy of this license, visit http://creativecommons.org/licenses/by-sa/3.0/
; or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
;
; 
; Port of the NOISEPLUG from Joachim Fenkes : https://github.com/dop3j0e/noiseplug
; 
; Connect a speaker to PA3 and GND


; IC SPECIFIC STUB FROM easypdk folder is needed, makefile takes care of it
;.include "easypdk/PMS150C.s"
;.include "easypdk/PFS154.s"
;.include "easypdk/PFS172.s"
;.include "easypdk/PFS173.s"


;RAM usage 38 bytes
        .area DATA (ABS)
        .org 0x00

p:           .ds 2   ;ptr
tp1:         .ds 1   ;tmp1
tp2:         .ds 1   ;tmp2
tp3:         .ds 1   ;tmp3
tp4:         .ds 1   ;tmp4

i:           .ds 3
sample:      .ds 1
osc_arp:     .ds 2
osc_bass:    .ds 2
bassflange:  .ds 2
boost:       .ds 1
lead1:       .ds 4
lead2:       .ds 4
lead3:       .ds 4
int_ctr:     .ds 1

        .even
_stack:      .ds 8   ;4 for isr + 4 for call depth 2 from main

LEADSIZE  = 174
LEADINIT1 = 1601
LEADINIT2 = 3571

        .area CODE (ABS)
        .org 0x0000

        mov a, #(_stack)
        mov.io sp, a
        mov p, a
        clear p+1
        mov a, #0
clear_sram:
        idxm p, a
        dzsn p
        goto clear_sram
        EASY_PDK_INIT_SYSCLOCK_8MHZ
        goto main_cont

        .org 0x0020
isr:
        pushaf
        inc int_ctr
        set0.io __intrq, #6       ;clear INTRQ_TM2
        popaf
        reti
        
main_cont:
        ;tune SYSCLK to 8.192MHz @5.00V
        EASY_PDK_CALIBRATE_IHRC_8192000HZ_AT_5V

        mov a, #0x2a              ;TM2C = TM2C_CLK_IHRC | TM2C_OUT_PA3 | TM2C_MODE_PWM
        mov.io __tm2c, a          ;PWM clock and output mode
        mov a, #0x01              ;TM2S_PWM_RES_8BIT | TM2S_PRESCALE_NONE | TM2S_SCALE_DIV2
        mov.io __tm2s, a          ;PWM speed IHRC(16.384MHz) /1 /2 /256(8bit) = 32000 Hz
        mov a, #0x40
        mov.io __inten, a         ;enable TM2 interrupt
        mov a, #0x00
        mov.io __intrq, a

        mov a, #LEADSIZE          ;setup variables
        mov lead1, a
        mov lead2, a
        mov lead3, a

        mov a, #(LEADINIT1>>8)
        mov lead2+2, a
        mov a, #(LEADINIT1)
        mov lead2+3, a

        mov a, #(LEADINIT2>>8)
        mov lead3+2, a
        mov a, #(LEADINIT2)
        mov lead3+3, a

        engint                    ;global enable interrupt

mainloop:

        t1sn int_ctr, #2          ;update pwm output every: pwm_int_freq / 4 = 32000/4 = 8000
        goto mainloop

        clear int_ctr

        mov a, sample
        mov.io __tm2b, a          ;output the new sample

        ;sample = 0
        clear sample

        ;i++
        inc i+2
        addc i+1
        addc i

        ; if ((i >> 13) == 76) i = 16 << 13;
        mov a, i+1
        sl  a
        mov a, i
        slc a
        ceqsn a, #0x13
        goto norestart
        mov a, #2
        mov i, a
        clear i+1
    
norestart:

; ==== BASS ====
        ; bassptr(r20) = (i >> 13) & 0xF
        mov a, i
        sr a
        mov a, i+1
        src a
        swap a
        and a, #0xF

        ; if (i >> 19) & 1: bassptr |= 0x10
        t0sn i, #3
        or a, #0x10

        ; note = notes[bassline[bassptr]]
        call getbassline
        mov tp1, a
        call getnote_h
        xch tp1
        call getnote_l
        mov tp2, a

        ; if (bassbeat[(i >> 10) & 7]): note <<= 1
        mov a, i+1
        sr a
        sr a
        and a, #7
        call getbassbeat
        add a, #0
        t0sn.io f, z
        goto nobassbeat
        sl tp2
        slc tp1

nobassbeat:
        ; bassosc += note, ret = (bassosc >> 8) & 0x7F
        mov a, tp2
        add osc_bass+1, a
        mov a, tp1
        addc osc_bass, a
        mov a, osc_bass
        and a, #0x7f
        mov tp3, a

        ; bassflange += note + 1, ret += (bassflange >> 8) & 0x7F
        inc tp2
        mov a, tp2
        add bassflange+1, a
        mov a, tp1
        addc bassflange, a
        mov a, bassflange
        and a, #0x7f
        add tp3, a

        ; if ((i >> 6) & 0xF) == 0xF ==> noaddbass
        mov a, i+1
        and a, #3
        ceqsn a, #3
        goto addbass
        mov a, i+2
        and a, #0xc0
        sub a, #0xc0
        t0sn.io f, z
        goto noaddbass
addbass:
        ;sample += (bass >> 2)
        sr tp3
        sr tp3
        mov a, tp3
        add sample, a

noaddbass:

; ==== ARPEGGIO ====

        ; arpptr = arpseq1[arpseq2[i >> 16]][(i >> 14) & 3]
        mov a, i
        call getarpseq2
        mov tp1, a
        mov a, i+1
        swap a
        sr a
        sr a
        and a, #3
        or a, tp1
        call getarpseq1

        ; if (!(i & (1 << 13))): arpptr >>= 4
        t1sn i+1, #5
        swap a

        ; arpptr = arpeggio[arpptr & 0xF][(i >> 8) & 1]
        and a, #0xF
        sl a
        t0sn i+1, #0
        or a, #1
        call getarpeggio

        ; if (!(i & 0x80)): arpptr >>= 4
        t1sn i+2, #7
        swap a

        ; note = arpnotes[arpptr & 0xF]
        and a, #0xF
        mov tp1, a
        call getarpnote_h
        xch tp1
        call getarpnote_l

        ; arp_osc += note
        add osc_arp+1, a
        mov a, tp1
        addc osc_arp, a

        ; if (!(i >> 17)): break arp
        mov a, i
        sr a
        add a, #0
        t0sn.io f, z
        goto noarp

        ; r20 = arptiming[(i >> 12) & 3]
        mov a, i+1
        swap a
        and a, #3
        call getarptiming
        mov tp1, a

        ; if (!((r20 << ((i >> 9) & 7)) & 0x80)): break arp
        mov a, i+1
        sr a
        and a, #7
        t0sn.io f, z
        goto arptiming_noshift

arptiming_shift:
        sl tp1
        dzsn a
        goto arptiming_shift

arptiming_noshift:
        t1sn tp1, #7
        goto noarp

        ; if (arp_osc & (1 << 12)): sample += 35;
        mov a, #35
        t0sn osc_arp, #4
        add sample, a

noarp:

; ==== LEAD ===
        clear tp3
        mov a, #~1
        mov tp4, a
        mov a, #lead1
        call lead_voice
        add sample, a
        sr a
        add sample, a

        mov a, #4
        mov tp3, a
        mov a, #~2
        mov tp4, a
        mov a, #lead2
        call lead_voice
        sr a
        add sample, a
        sr a
        add sample, a

        mov a, #8
        mov tp3, a
        mov a, #~4
        mov tp4, a
        mov a, #lead3
        call lead_voice
        sr a
        add a, sample

        goto mainloop             ;rjmp mainloop


; input: lead_voice
;   dataset pointer in a
;   voice delay (4 * voice_nr) in tp3
;   inverted boost mask in tp4
; returns sample in a

lead_voice:
        mov p, a
        idxm a, p
        mov tp2, a
        inc p
        ceqsn a, #LEADSIZE
        goto noleadsetup

        mov a, #4
        ceqsn a, i
        ret #0
        mov a, tp3
        ceqsn a, i+1
        ret #0

        dec p
        mov a, #-1
        mov tp2, a
        idxm p, a
        inc p
        mov a, #1
        idxm p, a

noleadsetup:
        idxm a, p
        mov tp1, a

        ; if (0 == (i & 0xFF)): clear boost
        mov a, #0
        ceqsn a, i+2
        goto checkleadtimer

        mov a, tp4
        and boost, a

        ; if (0 == i & 0x1FF): leadtimer--
        t0sn i+1, #0
        goto checkleadtimer

        dec tp1
        mov a, tp1
        idxm p, a

checkleadtimer:
        ; if (0 == leadtimer): leadptr++
        mov a, #0
        ceqsn a, tp1
        goto leaddata

        ; leadptr++
        dec p
        inc tp2
        mov a, tp2
        idxm p, a
        inc p

leaddata:
        ; data = leaddata[(leadseq[leadptr >> 4] << 4) | (leadptr & 0xF)];
        ; leadptr(4..7) = leadseq[leadptr >> 4];
        mov a, tp2
        and a, #0xF
        mov tp3, a
        mov a, tp2
        swap a
        and a, #0xF
        call getleadseq
        or a, tp3

        ; data = leaddata[...]
        call getleaddata
        mov tp3, a

        ; if (0 == leadtimer) {
        not tp4
        mov a, tp1
        ceqsn a, #0
        goto noleadupdate

        ; leadtimer = leadtimes[data >> 5]
        mov a, tp3
        swap a
        sr a
        and a, #7
        call getleadtimes
        idxm p, a

        ; boosts |= boostmask
        mov a, tp4
        or boost, a

noleadupdate:
        ; data &= 0x1F
        mov a, #0x1F
        and tp3, a

        ; note = notes[data]
        ; leadosc += note
        inc p
        idxm a, p
        mov tp1, a
        inc p
        idxm a, p
        mov tp2, a

        mov a, tp3
        call getnote_l
        add tp2, a
        addc tp1
        mov a, tp2
        idxm p, a
        dec p
        mov a, tp3
        call getnote_h
        add a, tp1
        idxm p, a

        ; sample = ((lead_osc >> 7) & 0x3F) + ((lead_osc >> 7) & 0x1F)
        sl tp2
        slc a
        and a, #0x3F
        mov tp2, a
        and a, #0x1F
        add tp2, a

        ; if (!(boost & boostmask)): take three quarters
;        mov a, tp4
;        and a, boost
;        t0sn.io f, z
;        goto noreduce
;        sr tp2
;        mov a, tp2
;        sr tp2
;        add tp2, a

noreduce:
        sr tp2
        ; if (data == 0) return 0;
        mov a, #0
        ceqsn a, tp3
        mov a, tp2
        ret

;///////////////////////////////////////////////////////////////////////////////

;//note_h: (arg+1) >>3
getarpnote_h:
        add a, #5
getnote_h:
        add a, #1
        sr a
        sr a
        sr a
        ret

getarpnote_l:
        add a, #5
getnote_l:
        add a, #1
        pcadd a
        ret #0xff
        ret #134
        ret #159
        ret #179
        ret #201
        ret #213
        ret #239
        ret #12
        ret #45
        ret #63
        ret #102
        ret #145
        ret #169
        ret #195
        ret #221
        ret #24
        ret #89
        ret #125
        ret #203

getbassline:
        add a, #1
        pcadd a
        ret #7
        ret #7
        ret #9
        ret #6
        ret #7
        ret #7
        ret #10
        ret #6
        ret #7
        ret #7
        ret #9
        ret #4
        ret #5
        ret #5
        ret #2
        ret #4
        ret #5
        ret #5
        ret #6
        ret #6
        ret #7
        ret #7
        ret #3
        ret #3
        ret #5
        ret #5
        ret #6
        ret #6

getbassbeat:
        add a, #1
        pcadd a
        ret #0
        ret #0
        ret #1
        ret #0
        ret #0
        ret #1
        ret #0
        ret #1

getarpseq1:
        add a, #1
        pcadd a
        ret #0x00
        ret #0x12
        ret #0x00
        ret #0x62
        ret #0x00
        ret #0x12
        ret #0x00
        ret #0x17
        ret #0x00
        ret #0x12
        ret #0x00
        ret #0x12
        ret #0x33
        ret #0x22
        ret #0x00
        ret #0x45

getarpseq2:
        add a, #1
        pcadd a
        ret #0
        ret #4
        ret #0
        ret #4
        ret #0
        ret #4
        ret #0
        ret #8
        ret #12
        ret #12

getarptiming:
        add a, #1
        pcadd a
        ret #0x0C
        ret #0x30
        ret #0xFB
        ret #0x0C

getarpeggio:
        add a, #1
        pcadd a
        ret #0x24
        ret #0x6A
        ret #0x46
        ret #0x9C
        ret #0x13
        ret #0x59
        ret #0x02
        ret #0x47
        ret #0x24
        ret #0x59
        ret #0x24
        ret #0x58
        ret #0x57
        ret #0xAD
        ret #0x35
        ret #0x9B

getleadtimes:
        add a, #1
        pcadd a
        ret #1
        ret #2
        ret #3
        ret #4
        ret #5
        ret #6
        ret #28
        ret #14

getleaddata:
        add a, #1
        pcadd a
        ret #0x67
        ret #0x24
        ret #0x20
        ret #0x27
        ret #0x20
        ret #0x28
        ret #0x89
        ret #0x00
        ret #0x28
        ret #0x20
        ret #0x27
        ret #0x20
        ret #0x28
        ret #0x89
        ret #0x00
        ret #0x28
        ret #0x20
        ret #0x27
        ret #0x20
        ret #0x28
        ret #0x86
        ret #0x00
        ret #0x44
        ret #0x00
        ret #0x63
        ret #0x24
        ret #0x62
        ret #0xa1
        ret #0xe0
        ret #0xe0
        ret #0xe0
        ret #0xe0
        ret #0x20
        ret #0x29
        ret #0x20
        ret #0x2a
        ret #0x8b
        ret #0x00
        ret #0x4e
        ret #0x00
        ret #0x6f
        ret #0x30
        ret #0x71
        ret #0xaf
        ret #0xe0
        ret #0xe0
        ret #0xe0
        ret #0xe0
        ret #0x20
        ret #0x29
        ret #0x20
        ret #0x2a
        ret #0x8b
        ret #0x00
        ret #0x4e
        ret #0x00
        ret #0x6f
        ret #0x30
        ret #0x6f
        ret #0xac
        ret #0xe0
        ret #0xe0
        ret #0xe0
        ret #0xe0
        ret #0x65
        ret #0x22
        ret #0x20
        ret #0x65
        ret #0x26
        ret #0x87
        ret #0x00
        ret #0x68
        ret #0x69
        ret #0x2b
        ret #0xaa
        ret #0xc0
        ret #0x67
        ret #0x24
        ret #0x20
        ret #0x67
        ret #0x28
        ret #0x89
        ret #0x00
        ret #0x68
        ret #0x69
        ret #0x2b
        ret #0xaa
        ret #0xc0
        ret #0x65
        ret #0x22
        ret #0x20
        ret #0x65
        ret #0x26
        ret #0xa7
        ret #0x28
        ret #0x20
        ret #0x69
        ret #0x2b
        ret #0xaa
        ret #0x29
        ret #0x20
        ret #0x68
        ret #0x29
        ret #0xaa
        ret #0x2b
        ret #0x20
        ret #0x69
        ret #0x28
        ret #0x69
        ret #0x67
        ret #0xe0

getleadseq:
        add a, #1
        pcadd a
        ret #0x00
        ret #0x10
        ret #0x00
        ret #0x20
        ret #0x00
        ret #0x10
        ret #0x00
        ret #0x30
        ret #0x40
        ret #0x50
        ret #0x60
