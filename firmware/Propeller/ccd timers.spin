CON
{
openSpectrometer project, using TCD1304AP
Nathan McCorkle - nmz787@gmail.com

2^N = frq ... expsoure times in microseconds when sysClock is 96Mhz
32 0.010416666666667
31 0.020833333333333
30 0.041666666666667
29 0.083333333333333
28 0.16666666666667
27 0.33333333333333
26 0.66666666666667
25 1.33333333333333
24 2.66666666666667
23 5.33333333333333
22 10.6666666666667  i.e. 2^32/2^22*(1/96000000) == 0.00001066666
21 21.3333333333333
20 42.6666666666667
19 85.3333333333333
18 170.666666666667
17 341.333333333333
16 682.666666666667
15 1365.33333333333
14 2730.66666666667
13 5461.33333333333
12 10922.6666666667
11 21845.3333333333
10 43690.6666666667
9  87381.3333333333
8  174762.666666667
7  349525.333333333
6  699050.666666667
5  1398101.33333333
4  2796202.66666667
3  5592405.33333333
2  11184810.6666667
1  22369621.3333333
0  44739242.6666667
}

_clkmode = xtal1 + pll16x
_xinfreq = 6_000_000
VAR     
        long ICGwait
        long exposureJitterFreeValue
        long inExposureVal
PUB go 

  'wait for exposure length, and mode settings from USB (or previous settings file)
  'exposure length is in microseconds, or an integer from 0-31 (using 2^N for frqb setting)
  'mode could be single/#N snapshot(s), or continuous streaming
  'storage mode would be USB or SD card

  'calculate the required ICGwait parameters, based on exposure time, system clock speed, required chip delays

  inExposureVal := 22                              '22 hardcoded temporarily, would normally get this from settings or user
  exposureJitterFreeValue := 1<<inExposureVal      'frqb gets set with this value  (twice t3 in TCD1304AP datasheet, aka Tint)
  ICGwait :=  ((1<<(32-inExposureVal))/2)/4        

  cognew(@entry, @ICGwait) 'start assembly cog at 'entry', pass ICGwait to par
DAT
'assembly cog fetches the value in parameter for PWM percentage
org
entry   add ctraval,  mClkPin
        add ctrbval,  SHpin
        shl mClkBit, mClkPin
        shl mClkSenseBit, mClkSensePin 
        shl SHbit, SHpin
        shl SHsenseBit, SHsensePin              
        shl ICGbit, ICGpin 
        or dira, SHbit                             'set SHpin as output
        or dira, ICGbit                            'set ICGbit as output
        or dira, mClkBit                           'set mClkPin as output
        or dira, testPinBit
        or outa, ICGbit                            'set ICG high to begin

        'rdlong exposureTime, @exposureJitterFreeValue
        mov frqa, masterFreq  
        mov frqb, exposureTime  '

        rdlong localICGwait1, par    'move calculated ICGwait to a local variable, par is the pointer to ICGwait, passed with cognew(@entry, @ICGwait)
        
        mov ctra, ctraval             'establish counter A mode and APIN        

        mov localICGwait2, localICGwait1  '(t1 in TCD1304AP datasheet, says this should typically be 5000ns)  
        sub localICGwait1, #6  'subtract 6 instructions (cpu cycles) from the ICGwait jump counter -- this is due to setup time require to save these 
        sub localICGwait2, #9  'subtract 2 instructions (cpu cycles) from the ICGwait jump counter 
        mov ctrb, ctrbval   'both counters are now going, master clock and SH line
:ccdStart                                       'if first time through this loop, this ensures pixel integration
                                                '(SH/tINT follows SH being high)
        waitpeq     SHbit, SHbit      'wait til SHpin goes high
        mov pixelCounter, numActivePixels
        waitpne     SHbit, SHbit      'wait til SHpin goes low
:ICGwaitLoop1                                   'wait 100ns less than 1/2 SH period ((2^32)/frqb==num sys clock cycles in SH period)
                                                'ICGdelay = clkCountLenSH/2/4 'period/2, 4 cycles per djnz instruction
        djnz localICGwait1, #:ICGwaitLoop1
        andn     outa, ICGbit         'ICGbit was high to begin, set it low here (left edge of t2 in TCD1304AP datasheet)
        waitpeq     SHbit, SHbit      'wait til SHpin goes high     (right edge of t2 in TCD1304AP datasheet)
        waitpne     SHbit, SHbit      'wait til SHpin goes low      (right edge of t3 in TCD1304AP datasheet) (left edge of t1)
:ICGwaitLoop2
        djnz localICGwait2, #:ICGwaitLoop2
        or   outa, ICGbit                       'ICGbit was low, set it high... CCD analog readout begins NOW... (right edge of t1)

:pixelLoop
        waitpne     mClkSenseBit, mClkSenseBit   'wait til mClk pin goes low
        waitpeq     mClkSenseBit, mClkSenseBit   'wait til mClk pin goes high
        waitpne     mClkSenseBit, mClkSenseBit   'wait til mClk pin goes low
        waitpeq     mClkSenseBit, mClkSenseBit   'wait til mClk pin goes high
        'ADC reading happens here/now
        or outa, testPinBit
        waitpne     mClkSenseBit, mClkSenseBit   'wait til mClk pin goes low
        waitpeq     mClkSenseBit, mClkSenseBit   'wait til mClk pin goes high
        waitpne     mClkSenseBit, mClkSenseBit   'wait til mClk pin goes low
        waitpeq     mClkSenseBit, mClkSenseBit   'wait til mClk pin goes high
        andn outa, testPinBit
        djnz pixelCounter, #:pixelLoop

                                                'need to take reading in 2 masterClk cycles
                                                'because each pixel is 4 cycles, catch it in the middle
                                                'FUTURE if the pixel readout period were oversampled, it MIGHT improve read quality
        rdlong localICGwait1, par
        mov localICGwait2, localICGwait1
        sub localICGwait1, #6  'subtract 6 instructions (cpu cycles) from the ICGwait jump counter -- this is due to setup time require to save these 
        sub localICGwait2, #9  'subtract 9 instructions (cpu cycles) from the ICGwait jump counter
        
        jmp #:ccdStart
':pixelLoop
        'waitcnt 17, 20
        'djnz 21, #:pixelLoop

ctraval long %00100 << 26 'NCO/PWM APIN=7, master clock                            
ctrbval long %00100 << 26 'NCO/PWM BPIN=6

numMasterClkCycles long 0

'GPIO pins
mClkPin       long 7    'CCD master clock pin
mClkSensePin  long 6    'loopback from mClkPin, used for sensing transitions
SHpin         long 5    'CCD SH pin                                                                 
SHsensePin    long 4    'loopback from SHpin, used for sensing transitions
ICGpin        long 8    'CCD ICG pin

mClkBit       long 1
mClkSenseBit  long 1
SHbit         long 1
SHsenseBit    long 1
ICGbit        long 1

testPinBit    long 1<<2

masterFreq  long 134217728     'master CCD frequency, ~3Mhz
exposureTime long 4194304      '~10uSecs

tmp1 long 128                  'system
halfExposureTime long 2097152

numActivePixels long 3648   

maxVal long $FFFF


localICGwait1 long 0
localICGwait2 long 0
pixelCounter  long 0
value res 1