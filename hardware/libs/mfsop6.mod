PCBNEW-LibModule-V1  Fri 22 May 2015 01:29:03 PM CEST
# encoding utf-8
Units mm
$INDEX
mfsop6
$EndINDEX
$MODULE mfsop6
Po 0 0 0 15 555F12B0 00000000 ~~
Li mfsop6
Cd Mini flat package, 4 pin
Sc 0
AR /555DB3D8
Op 0 0 0
T0 0 -5 1 1 0 0.2 N V 21 N "IC3"
T1 0 5.1 1 1 0 0.2 N V 21 N "LTV-352T"
DS -1.905 1.905 -1.905 -1.905 0.25 21
DS -1.905 -1.905 1.905 -1.905 0.25 21
DS 1.905 -1.905 1.905 1.905 0.25 21
DS 1.905 1.905 -1.905 1.905 0.25 21
DC -1.235 1.5 -1.435 1.7 0.25 21
$PAD
Sh "1" R 0.61 1.52 0 0 0
Dr 0 0 0
At SMD N 00888000
Ne 2 "N-000004"
Po -1.27 3.175
$EndPAD
$PAD
Sh "3" R 0.61 1.52 0 0 0
Dr 0 0 0
At SMD N 00888000
Ne 0 ""
Po 1.27 3.175
$EndPAD
$PAD
Sh "4" R 0.61 1.52 0 0 0
Dr 0 0 0
At SMD N 00888000
Ne 3 "TXD"
Po 1.27 -3.175
$EndPAD
$PAD
Sh "6" R 0.61 1.52 0 0 0
Dr 0 0 0
At SMD N 00888000
Ne 1 "BUS_GND"
Po -1.27 -3.175
$EndPAD
$SHAPE3D
Na "walter/smd_dil/mfp-4.wrl"
Sc 1 1 1
Of 0 0 0
Ro 0 0 0
$EndSHAPE3D
$EndMODULE mfsop6
$EndLIBRARY
