; number of filter taps
.equ fir_coeffs_0_ntaps, 24

.section xbss, xmemory, eds, data
.align 64

.global _fir_coeffs_0
_fir_coeffs_0:
.hword 0x0096, 0xff84, 0x006d, 0xfe0b, 0xfc1b, 0x040c, 0x026f, 0x02ae, 0x0b15
.hword 0xebd3, 0xe2c3, 0x21fc, 0x21fc, 0xe2c3, 0xebd3, 0x0b15, 0x02ae, 0x026f
.hword 0x040c, 0xfc1b, 0xfe0b, 0x006d, 0xff84, 0x0096


; number of filter taps
.equ fir_coeffs_1_q_ntaps, 23

.section xbss, xmemory, eds, data
.align 32

.global _fir_coeffs_1_q
_fir_coeffs_1_q:
.hword 0xffe8, 0xffa3, 0xff21, 0xfe87, 0xfe4e, 0xff32, 0x01e9, 0x06a7, 0x0ccb
.hword 0x12dc, 0x170a, 0x17f4, 0x154a, 0x0ff3, 0x09a0, 0x040c, 0x004b, 0xfe90
.hword 0xfe54, 0xfed2, 0xff69, 0xffcd, 0xfff7


; number of filter taps
.equ fir_coeffs_1_i_ntaps, 23

.section xbss, xmemory, eds, data
.align 32

.global _fir_coeffs_1_i
_fir_coeffs_1_i:
.hword 0xfff7, 0xffcd, 0xff69, 0xfed2, 0xfe54, 0xfe90, 0x004b, 0x040c, 0x09a0
.hword 0x0ff3, 0x154a, 0x17f4, 0x170a, 0x12dc, 0x0ccb, 0x06a7, 0x01e9, 0xff32
.hword 0xfe4e, 0xfe87, 0xff21, 0xffa3, 0xffe8


; number of filter taps
.equ fir_coeffs_2_ntaps, 96

.section xbss, xmemory, eds, data
.align 256

.global _fir_coeffs_2
_fir_coeffs_2:
.hword 0x000e, 0x0010, 0x0010, 0x001a, 0x001c, 0x0025, 0x0027, 0x002d, 0x002e
.hword 0x0030, 0x002c, 0x0028, 0x001e, 0x0012, 0x0000, 0xffec, 0xffd2, 0xffb7
.hword 0xff99, 0xff7a, 0xff5b, 0xff3e, 0xff24, 0xff10, 0xff03, 0xff00, 0xff09
.hword 0xff1f, 0xff43, 0xff77, 0xffbc, 0x0012, 0x0078, 0x00ee, 0x0171, 0x0201
.hword 0x029a, 0x0339, 0x03db, 0x047b, 0x0517, 0x05aa, 0x0631, 0x06a7, 0x070a
.hword 0x0757, 0x078b, 0x07a5, 0x07a5, 0x078b, 0x0757, 0x070a, 0x06a7, 0x0631
.hword 0x05aa, 0x0517, 0x047b, 0x03db, 0x0339, 0x029a, 0x0201, 0x0171, 0x00ee
.hword 0x0078, 0x0012, 0xffbc, 0xff77, 0xff43, 0xff1f, 0xff09, 0xff00, 0xff03
.hword 0xff10, 0xff24, 0xff3e, 0xff5b, 0xff7a, 0xff99, 0xffb7, 0xffd2, 0xffec
.hword 0x0000, 0x0012, 0x001e, 0x0028, 0x002c, 0x0030, 0x002e, 0x002d, 0x0027
.hword 0x0025, 0x001c, 0x001a, 0x0010, 0x0010, 0x000e


; number of filter taps
.equ fir_coeffs_3_ntaps, 96

.section xbss, xmemory, eds, data
.align 256

.global _fir_coeffs_3
_fir_coeffs_3:
.hword 0x000e, 0x0010, 0x0010, 0x001a, 0x001c, 0x0025, 0x0027, 0x002d, 0x002e
.hword 0x0030, 0x002c, 0x0028, 0x001e, 0x0012, 0x0000, 0xffec, 0xffd2, 0xffb7
.hword 0xff99, 0xff7a, 0xff5b, 0xff3e, 0xff24, 0xff10, 0xff03, 0xff00, 0xff09
.hword 0xff1f, 0xff43, 0xff77, 0xffbc, 0x0012, 0x0078, 0x00ee, 0x0171, 0x0201
.hword 0x029a, 0x0339, 0x03db, 0x047b, 0x0517, 0x05aa, 0x0631, 0x06a7, 0x070a
.hword 0x0757, 0x078b, 0x07a5, 0x07a5, 0x078b, 0x0757, 0x070a, 0x06a7, 0x0631
.hword 0x05aa, 0x0517, 0x047b, 0x03db, 0x0339, 0x029a, 0x0201, 0x0171, 0x00ee
.hword 0x0078, 0x0012, 0xffbc, 0xff77, 0xff43, 0xff1f, 0xff09, 0xff00, 0xff03
.hword 0xff10, 0xff24, 0xff3e, 0xff5b, 0xff7a, 0xff99, 0xffb7, 0xffd2, 0xffec
.hword 0x0000, 0x0012, 0x001e, 0x0028, 0x002c, 0x0030, 0x002e, 0x002d, 0x0027
.hword 0x0025, 0x001c, 0x001a, 0x0010, 0x0010, 0x000e


; number of filter taps
.equ fir_coeffs_4_ntaps, 96

.section xbss, xmemory, eds, data
.align 256

.global _fir_coeffs_4
_fir_coeffs_4:
.hword 0x000e, 0x0010, 0x0010, 0x001a, 0x001c, 0x0025, 0x0027, 0x002d, 0x002e
.hword 0x0030, 0x002c, 0x0028, 0x001e, 0x0012, 0x0000, 0xffec, 0xffd2, 0xffb7
.hword 0xff99, 0xff7a, 0xff5b, 0xff3e, 0xff24, 0xff10, 0xff03, 0xff00, 0xff09
.hword 0xff1f, 0xff43, 0xff77, 0xffbc, 0x0012, 0x0078, 0x00ee, 0x0171, 0x0201
.hword 0x029a, 0x0339, 0x03db, 0x047b, 0x0517, 0x05aa, 0x0631, 0x06a7, 0x070a
.hword 0x0757, 0x078b, 0x07a5, 0x07a5, 0x078b, 0x0757, 0x070a, 0x06a7, 0x0631
.hword 0x05aa, 0x0517, 0x047b, 0x03db, 0x0339, 0x029a, 0x0201, 0x0171, 0x00ee
.hword 0x0078, 0x0012, 0xffbc, 0xff77, 0xff43, 0xff1f, 0xff09, 0xff00, 0xff03
.hword 0xff10, 0xff24, 0xff3e, 0xff5b, 0xff7a, 0xff99, 0xffb7, 0xffd2, 0xffec
.hword 0x0000, 0x0012, 0x001e, 0x0028, 0x002c, 0x0030, 0x002e, 0x002d, 0x0027
.hword 0x0025, 0x001c, 0x001a, 0x0010, 0x0010, 0x000e


; number of filter taps
.equ fir_coeffs_5_ntaps, 380

.section xbss, xmemory, eds, data
.align 1024

.global _fir_coeffs_5
_fir_coeffs_5:
.hword 0x0002, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
.hword 0x0000, 0x0000, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xfffe
.hword 0xfffe, 0xfffe, 0xfffe, 0xfffe, 0xfffe, 0xfffe, 0xfffe, 0xfffe, 0xfffe
.hword 0xfffe, 0xfffe, 0xfffe, 0xfffe, 0xfffe, 0xffff, 0xffff, 0xffff, 0x0000
.hword 0x0000, 0x0001, 0x0001, 0x0002, 0x0003, 0x0003, 0x0004, 0x0005, 0x0005
.hword 0x0006, 0x0007, 0x0007, 0x0008, 0x0009, 0x0009, 0x000a, 0x000a, 0x000a
.hword 0x000a, 0x000b, 0x000a, 0x000a, 0x000a, 0x0009, 0x0009, 0x0008, 0x0007
.hword 0x0005, 0x0004, 0x0003, 0x0001, 0xffff, 0xfffd, 0xfffb, 0xfff9, 0xfff7
.hword 0xfff4, 0xfff2, 0xfff0, 0xffee, 0xffeb, 0xffe9, 0xffe7, 0xffe6, 0xffe4
.hword 0xffe3, 0xffe1, 0xffe1, 0xffe0, 0xffe0, 0xffe0, 0xffe1, 0xffe2, 0xffe4
.hword 0xffe6, 0xffe8, 0xffeb, 0xffee, 0xfff2, 0xfff6, 0xfffa, 0xffff, 0x0004
.hword 0x0009, 0x000f, 0x0015, 0x001b, 0x0020, 0x0026, 0x002c, 0x0032, 0x0037
.hword 0x003c, 0x0041, 0x0045, 0x0048, 0x004b, 0x004e, 0x004f, 0x0050, 0x004f
.hword 0x004e, 0x004c, 0x0049, 0x0045, 0x0040, 0x0039, 0x0032, 0x002a, 0x0021
.hword 0x0016, 0x000b, 0x0000, 0xfff3, 0xffe6, 0xffd9, 0xffcb, 0xffbd, 0xffaf
.hword 0xffa1, 0xff93, 0xff86, 0xff79, 0xff6d, 0xff62, 0xff59, 0xff50, 0xff49
.hword 0xff44, 0xff40, 0xff3f, 0xff3f, 0xff42, 0xff47, 0xff4e, 0xff59, 0xff65
.hword 0xff75, 0xff87, 0xff9b, 0xffb3, 0xffcd, 0xffe9, 0x0008, 0x002a, 0x004e
.hword 0x0074, 0x009b, 0x00c5, 0x00f0, 0x011d, 0x014a, 0x0179, 0x01a8, 0x01d7
.hword 0x0206, 0x0235, 0x0264, 0x0291, 0x02be, 0x02e9, 0x0312, 0x033a, 0x035f
.hword 0x0381, 0x03a1, 0x03be, 0x03d8, 0x03ee, 0x0401, 0x0411, 0x041c, 0x0424
.hword 0x0428, 0x0428, 0x0424, 0x041c, 0x0411, 0x0401, 0x03ee, 0x03d8, 0x03be
.hword 0x03a1, 0x0381, 0x035f, 0x033a, 0x0312, 0x02e9, 0x02be, 0x0291, 0x0264
.hword 0x0235, 0x0206, 0x01d7, 0x01a8, 0x0179, 0x014a, 0x011d, 0x00f0, 0x00c5
.hword 0x009b, 0x0074, 0x004e, 0x002a, 0x0008, 0xffe9, 0xffcd, 0xffb3, 0xff9b
.hword 0xff87, 0xff75, 0xff65, 0xff59, 0xff4e, 0xff47, 0xff42, 0xff3f, 0xff3f
.hword 0xff40, 0xff44, 0xff49, 0xff50, 0xff59, 0xff62, 0xff6d, 0xff79, 0xff86
.hword 0xff93, 0xffa1, 0xffaf, 0xffbd, 0xffcb, 0xffd9, 0xffe6, 0xfff3, 0x0000
.hword 0x000b, 0x0016, 0x0021, 0x002a, 0x0032, 0x0039, 0x0040, 0x0045, 0x0049
.hword 0x004c, 0x004e, 0x004f, 0x0050, 0x004f, 0x004e, 0x004b, 0x0048, 0x0045
.hword 0x0041, 0x003c, 0x0037, 0x0032, 0x002c, 0x0026, 0x0020, 0x001b, 0x0015
.hword 0x000f, 0x0009, 0x0004, 0xffff, 0xfffa, 0xfff6, 0xfff2, 0xffee, 0xffeb
.hword 0xffe8, 0xffe6, 0xffe4, 0xffe2, 0xffe1, 0xffe0, 0xffe0, 0xffe0, 0xffe1
.hword 0xffe1, 0xffe3, 0xffe4, 0xffe6, 0xffe7, 0xffe9, 0xffeb, 0xffee, 0xfff0
.hword 0xfff2, 0xfff4, 0xfff7, 0xfff9, 0xfffb, 0xfffd, 0xffff, 0x0001, 0x0003
.hword 0x0004, 0x0005, 0x0007, 0x0008, 0x0009, 0x0009, 0x000a, 0x000a, 0x000a
.hword 0x000b, 0x000a, 0x000a, 0x000a, 0x000a, 0x0009, 0x0009, 0x0008, 0x0007
.hword 0x0007, 0x0006, 0x0005, 0x0005, 0x0004, 0x0003, 0x0003, 0x0002, 0x0001
.hword 0x0001, 0x0000, 0x0000, 0xffff, 0xffff, 0xffff, 0xfffe, 0xfffe, 0xfffe
.hword 0xfffe, 0xfffe, 0xfffe, 0xfffe, 0xfffe, 0xfffe, 0xfffe, 0xfffe, 0xfffe
.hword 0xfffe, 0xfffe, 0xfffe, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff
.hword 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
.hword 0x0000, 0x0002


; number of filter taps
.equ fir_coeffs_6_ntaps, 200

.section xbss, xmemory, eds, data
.align 512

.global _fir_coeffs_6
_fir_coeffs_6:
.hword 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0000, 0x0000
.hword 0xffff, 0xffff, 0xfffe, 0xfffd, 0xfffc, 0xfffb, 0xfffb, 0xfffa, 0xfffa
.hword 0xfffa, 0xfffa, 0xfffb, 0xfffc, 0xfffe, 0x0000, 0x0003, 0x0006, 0x0009
.hword 0x000d, 0x0010, 0x0013, 0x0016, 0x0017, 0x0018, 0x0018, 0x0016, 0x0013
.hword 0x000f, 0x0009, 0x0002, 0xfff9, 0xfff0, 0xffe6, 0xffdc, 0xffd2, 0xffca
.hword 0xffc3, 0xffbe, 0xffbc, 0xffbc, 0xffc1, 0xffc8, 0xffd4, 0xffe3, 0xfff6
.hword 0x000b, 0x0022, 0x003b, 0x0053, 0x006b, 0x0080, 0x0091, 0x009e, 0x00a4
.hword 0x00a4, 0x009b, 0x008b, 0x0071, 0x0050, 0x0027, 0xfff7, 0xffc2, 0xff8a
.hword 0xff51, 0xff19, 0xfee5, 0xfeb8, 0xfe96, 0xfe7f, 0xfe78, 0xfe83, 0xfea1
.hword 0xfed3, 0xff1b, 0xff79, 0xffec, 0x0072, 0x010b, 0x01b2, 0x0264, 0x031f
.hword 0x03dd, 0x049a, 0x0551, 0x05fe, 0x069c, 0x0727, 0x079b, 0x07f5, 0x0833
.hword 0x0852, 0x0852, 0x0833, 0x07f5, 0x079b, 0x0727, 0x069c, 0x05fe, 0x0551
.hword 0x049a, 0x03dd, 0x031f, 0x0264, 0x01b2, 0x010b, 0x0072, 0xffec, 0xff79
.hword 0xff1b, 0xfed3, 0xfea1, 0xfe83, 0xfe78, 0xfe7f, 0xfe96, 0xfeb8, 0xfee5
.hword 0xff19, 0xff51, 0xff8a, 0xffc2, 0xfff7, 0x0027, 0x0050, 0x0071, 0x008b
.hword 0x009b, 0x00a4, 0x00a4, 0x009e, 0x0091, 0x0080, 0x006b, 0x0053, 0x003b
.hword 0x0022, 0x000b, 0xfff6, 0xffe3, 0xffd4, 0xffc8, 0xffc1, 0xffbc, 0xffbc
.hword 0xffbe, 0xffc3, 0xffca, 0xffd2, 0xffdc, 0xffe6, 0xfff0, 0xfff9, 0x0002
.hword 0x0009, 0x000f, 0x0013, 0x0016, 0x0018, 0x0018, 0x0017, 0x0016, 0x0013
.hword 0x0010, 0x000d, 0x0009, 0x0006, 0x0003, 0x0000, 0xfffe, 0xfffc, 0xfffb
.hword 0xfffa, 0xfffa, 0xfffa, 0xfffa, 0xfffb, 0xfffb, 0xfffc, 0xfffd, 0xfffe
.hword 0xffff, 0xffff, 0x0000, 0x0000, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001
.hword 0x0001, 0x0001


; number of filter taps
.equ fir_coeffs_7_ntaps, 80

.section xbss, xmemory, eds, data
.align 256

.global _fir_coeffs_7
_fir_coeffs_7:
.hword 0x0002, 0x0002, 0x0002, 0x0000, 0xfffb, 0xfff6, 0xfff2, 0xfff1, 0xfff5
.hword 0x0002, 0x0015, 0x0029, 0x0039, 0x003a, 0x0028, 0xffff, 0xffc5, 0xff89
.hword 0xff5e, 0xff5a, 0xff8b, 0xfff3, 0x0083, 0x0117, 0x0182, 0x0194, 0x012c
.hword 0x0044, 0xfefd, 0xfd9e, 0xfc8a, 0xfc30, 0xfcec, 0xfef2, 0x0237, 0x066e
.hword 0x0b0b, 0x0f62, 0x12c1, 0x1499, 0x1499, 0x12c1, 0x0f62, 0x0b0b, 0x066e
.hword 0x0237, 0xfef2, 0xfcec, 0xfc30, 0xfc8a, 0xfd9e, 0xfefd, 0x0044, 0x012c
.hword 0x0194, 0x0182, 0x0117, 0x0083, 0xfff3, 0xff8b, 0xff5a, 0xff5e, 0xff89
.hword 0xffc5, 0xffff, 0x0028, 0x003a, 0x0039, 0x0029, 0x0015, 0x0002, 0xfff5
.hword 0xfff1, 0xfff2, 0xfff6, 0xfffb, 0x0000, 0x0002, 0x0002, 0x0002


; number of filter taps
.equ fir_coeffs_8_ntaps, 40

.section xbss, xmemory, eds, data
.align 128

.global _fir_coeffs_8
_fir_coeffs_8:
.hword 0x0004, 0x0002, 0xfff2, 0xffe2, 0xfff5, 0x003d, 0x0076, 0x002c, 0xff4e
.hword 0xfeae, 0xff72, 0x019d, 0x0330, 0x018e, 0xfc97, 0xf881, 0xfb8b, 0x0877
.hword 0x1a95, 0x27ca, 0x27ca, 0x1a95, 0x0877, 0xfb8b, 0xf881, 0xfc97, 0x018e
.hword 0x0330, 0x019d, 0xff72, 0xfeae, 0xff4e, 0x002c, 0x0076, 0x003d, 0xfff5
.hword 0xffe2, 0xfff2, 0x0002, 0x0004


; number of filter taps
.equ fir_coeffs_9_ntaps, 160

.section xbss, xmemory, eds, data
.align 512

.global _fir_coeffs_9
_fir_coeffs_9:
.hword 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0000, 0xffff, 0xfffe
.hword 0xfffd, 0xfffc, 0xfffa, 0xfff9, 0xfff8, 0xfff8, 0xfff8, 0xfffa, 0xfffc
.hword 0xffff, 0x0003, 0x0008, 0x000d, 0x0012, 0x0017, 0x001b, 0x001e, 0x001e
.hword 0x001c, 0x0018, 0x0010, 0x0006, 0xfff9, 0xffea, 0xffdb, 0xffcb, 0xffbd
.hword 0xffb2, 0xffac, 0xffab, 0xffb0, 0xffbc, 0xffd0, 0xffea, 0x000a, 0x002f
.hword 0x0055, 0x007b, 0x009d, 0x00b8, 0x00c9, 0x00ce, 0x00c4, 0x00aa, 0x007f
.hword 0x0045, 0xfffd, 0xffaa, 0xff52, 0xfef9, 0xfea6, 0xfe60, 0xfe2e, 0xfe17
.hword 0xfe20, 0xfe4f, 0xfea6, 0xff28, 0xffd4, 0x00a6, 0x019a, 0x02a9, 0x03ca
.hword 0x04f3, 0x0619, 0x0730, 0x082e, 0x0907, 0x09b3, 0x0a2a, 0x0a66, 0x0a66
.hword 0x0a2a, 0x09b3, 0x0907, 0x082e, 0x0730, 0x0619, 0x04f3, 0x03ca, 0x02a9
.hword 0x019a, 0x00a6, 0xffd4, 0xff28, 0xfea6, 0xfe4f, 0xfe20, 0xfe17, 0xfe2e
.hword 0xfe60, 0xfea6, 0xfef9, 0xff52, 0xffaa, 0xfffd, 0x0045, 0x007f, 0x00aa
.hword 0x00c4, 0x00ce, 0x00c9, 0x00b8, 0x009d, 0x007b, 0x0055, 0x002f, 0x000a
.hword 0xffea, 0xffd0, 0xffbc, 0xffb0, 0xffab, 0xffac, 0xffb2, 0xffbd, 0xffcb
.hword 0xffdb, 0xffea, 0xfff9, 0x0006, 0x0010, 0x0018, 0x001c, 0x001e, 0x001e
.hword 0x001b, 0x0017, 0x0012, 0x000d, 0x0008, 0x0003, 0xffff, 0xfffc, 0xfffa
.hword 0xfff8, 0xfff8, 0xfff8, 0xfff9, 0xfffa, 0xfffc, 0xfffd, 0xfffe, 0xffff
.hword 0x0000, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001


; number of filter taps
.equ fir_coeffs_10_ntaps, 200

.section xbss, xmemory, eds, data
.align 512

.global _fir_coeffs_10
_fir_coeffs_10:
.hword 0xfffe, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0x0000
.hword 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0007, 0x0007
.hword 0x0006, 0x0005, 0x0003, 0x0000, 0xfffd, 0xfff9, 0xfff5, 0xfff2, 0xffef
.hword 0xffec, 0xffeb, 0xffea, 0xffeb, 0xffee, 0xfff3, 0xfff9, 0x0000, 0x0008
.hword 0x0012, 0x001b, 0x0024, 0x002b, 0x0031, 0x0034, 0x0035, 0x0032, 0x002b
.hword 0x0021, 0x0013, 0x0002, 0xffef, 0xffdb, 0xffc6, 0xffb3, 0xffa3, 0xff97
.hword 0xff90, 0xff8f, 0xff96, 0xffa4, 0xffba, 0xffd6, 0xfff9, 0x001f, 0x0048
.hword 0x0071, 0x0098, 0x00b9, 0x00d2, 0x00e1, 0x00e3, 0x00d7, 0x00bc, 0x0092
.hword 0x005a, 0x0016, 0xffc9, 0xff75, 0xff21, 0xfed0, 0xfe88, 0xfe4f, 0xfe29
.hword 0xfe1c, 0xfe2c, 0xfe5c, 0xfeae, 0xff24, 0xffbb, 0x0072, 0x0145, 0x022f
.hword 0x032a, 0x042d, 0x0531, 0x062d, 0x0718, 0x07eb, 0x089e, 0x092a, 0x098b
.hword 0x09bc, 0x09bc, 0x098b, 0x092a, 0x089e, 0x07eb, 0x0718, 0x062d, 0x0531
.hword 0x042d, 0x032a, 0x022f, 0x0145, 0x0072, 0xffbb, 0xff24, 0xfeae, 0xfe5c
.hword 0xfe2c, 0xfe1c, 0xfe29, 0xfe4f, 0xfe88, 0xfed0, 0xff21, 0xff75, 0xffc9
.hword 0x0016, 0x005a, 0x0092, 0x00bc, 0x00d7, 0x00e3, 0x00e1, 0x00d2, 0x00b9
.hword 0x0098, 0x0071, 0x0048, 0x001f, 0xfff9, 0xffd6, 0xffba, 0xffa4, 0xff96
.hword 0xff8f, 0xff90, 0xff97, 0xffa3, 0xffb3, 0xffc6, 0xffdb, 0xffef, 0x0002
.hword 0x0013, 0x0021, 0x002b, 0x0032, 0x0035, 0x0034, 0x0031, 0x002b, 0x0024
.hword 0x001b, 0x0012, 0x0008, 0x0000, 0xfff9, 0xfff3, 0xffee, 0xffeb, 0xffea
.hword 0xffeb, 0xffec, 0xffef, 0xfff2, 0xfff5, 0xfff9, 0xfffd, 0x0000, 0x0003
.hword 0x0005, 0x0006, 0x0007, 0x0007, 0x0007, 0x0006, 0x0005, 0x0004, 0x0003
.hword 0x0002, 0x0001, 0x0000, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff
.hword 0xffff, 0xfffe
    
    
; number of filter taps
.equ fir_coeffs_11_i_ntaps, 80

.section xbss, xmemory, eds, data
.align 128

.global _fir_coeffs_11_i
_fir_coeffs_11_i:
.hword 0x0003, 0x0002, 0x0002, 0x0000, 0xfffc, 0xfff7, 0xfff2, 0xfff0, 0xfff3
.hword 0xfffe, 0x0010, 0x0025, 0x0037, 0x003d, 0x002f, 0x000b, 0xffd4, 0xff96
.hword 0xff65, 0xff55, 0xff78, 0xffd4, 0x005d, 0x00f5, 0x0170, 0x019c, 0x0154
.hword 0x008a, 0xff54, 0xfdf1, 0xfcc0, 0xfc2e, 0xfc9e, 0xfe50, 0x014c, 0x0552
.hword 0x09e6, 0x0e61, 0x120e, 0x1453, 0x14cd, 0x1366, 0x105c, 0x0c32, 0x0794
.hword 0x0334, 0xffa7, 0xfd4d, 0xfc40, 0xfc5c, 0xfd4c, 0xfea3, 0xfff9, 0x00ff
.hword 0x0188, 0x0193, 0x0139, 0x00aa, 0x0015, 0xffa0, 0xff60, 0xff58, 0xff7b
.hword 0xffb5, 0xfff1, 0x0020, 0x0039, 0x003c, 0x002f, 0x001a, 0x0006, 0xfff8
.hword 0xfff1, 0xfff0, 0xfff5, 0xfffa, 0xffff, 0x0001, 0x0002, 0x0002


; number of filter taps
.equ fir_coeffs_11_q_ntaps, 80

.section xbss, xmemory, eds, data
.align 128

.global _fir_coeffs_11_q
_fir_coeffs_11_q:
.hword 0x0002, 0x0002, 0x0001, 0xffff, 0xfffa, 0xfff5, 0xfff0, 0xfff1, 0xfff8
.hword 0x0006, 0x001a, 0x002f, 0x003c, 0x0039, 0x0020, 0xfff1, 0xffb5, 0xff7b
.hword 0xff58, 0xff60, 0xffa0, 0x0015, 0x00aa, 0x0139, 0x0193, 0x0188, 0x00ff
.hword 0xfff9, 0xfea3, 0xfd4c, 0xfc5c, 0xfc40, 0xfd4d, 0xffa7, 0x0334, 0x0794
.hword 0x0c32, 0x105c, 0x1366, 0x14cd, 0x1453, 0x120e, 0x0e61, 0x09e6, 0x0552
.hword 0x014c, 0xfe50, 0xfc9e, 0xfc2e, 0xfcc0, 0xfdf1, 0xff54, 0x008a, 0x0154
.hword 0x019c, 0x0170, 0x00f5, 0x005d, 0xffd4, 0xff78, 0xff55, 0xff65, 0xff96
.hword 0xffd4, 0x000b, 0x002f, 0x003d, 0x0037, 0x0025, 0x0010, 0xfffe, 0xfff3
.hword 0xfff0, 0xfff2, 0xfff7, 0xfffc, 0x0000, 0x0002, 0x0002, 0x0003


; number of filter taps
.equ fir_coeffs_12_i_ntaps, 40

.section xbss, xmemory, eds, data
.align 64

.global _fir_coeffs_12_i
_fir_coeffs_12_i:
.hword 0x0004, 0x0004, 0xfff7, 0xffe3, 0xffeb, 0x0029, 0x0071, 0x0050, 0xff8b
.hword 0xfebd, 0xff16, 0x0105, 0x0305, 0x0258, 0xfdfa, 0xf915, 0xf9d9, 0x046f
.hword 0x1616, 0x2583, 0x2933, 0x1ec4, 0x0cdb, 0xfde4, 0xf860, 0xfb3b, 0x0089
.hword 0x0329, 0x022e, 0xffe7, 0xfeb4, 0xff12, 0xfffe, 0x0075, 0x0053, 0x0004
.hword 0xffe1, 0xffec, 0xffff, 0x0004


; number of filter taps
.equ fir_coeffs_12_q_ntaps, 40

.section xbss, xmemory, eds, data
.align 64

.global _fir_coeffs_12_q
_fir_coeffs_12_q:
.hword 0x0004, 0xffff, 0xffec, 0xffe1, 0x0004, 0x0053, 0x0075, 0xfffe, 0xff12
.hword 0xfeb4, 0xffe7, 0x022e, 0x0329, 0x0089, 0xfb3b, 0xf860, 0xfde4, 0x0cdb
.hword 0x1ec4, 0x2933, 0x2583, 0x1616, 0x046f, 0xf9d9, 0xf915, 0xfdfa, 0x0258
.hword 0x0305, 0x0105, 0xff16, 0xfebd, 0xff8b, 0x0050, 0x0071, 0x0029, 0xffeb
.hword 0xffe3, 0xfff7, 0x0004, 0x0004
