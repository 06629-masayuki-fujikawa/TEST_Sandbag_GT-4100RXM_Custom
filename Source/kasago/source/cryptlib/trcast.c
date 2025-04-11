/*
 * Description: Testing the ESP algorithems --- cast128
 *
 * Filename: trcast.c
 * Author: Jin Zhang
 * Date Created: 08/01/2001
 * $Source: source/cryptlib/trcast.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:35:23JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */



/*  $KAME: cast128.c,v 1.4 2000/11/06 13:58:08 itojun Exp $ */

/*
 * heavily modified by Tomomi Suzuki <suzuki@grelot.elec.ryukoku.ac.jp>
 */
/*
 * The CAST-128 Encryption Algorithm (RFC 2144)
 *
 * original implementation <Hideo "Sir MaNMOS" Morisita>
 * 1997/08/21
 */
/*
 * Copyright (C) 1997 Hideo "Sir MANMOS" Morishita
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY Hideo "Sir MaNMOS" Morishita ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL Hideo "Sir MaNMOS" Morishita BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */


#include <trsocket.h>
#include <trmacro.h>
#include <trtype.h>

#ifdef TM_USE_CAST128
#include <trcrylib.h>


static const tt32Bit TM_CONST_QLF S1[] = {
    TM_UL(0x30fb40d4), TM_UL(0x9fa0ff0b), TM_UL(0x6beccd2f), TM_UL(0x3f258c7a),
    TM_UL(0x1e213f2f), TM_UL(0x9c004dd3), TM_UL(0x6003e540), TM_UL(0xcf9fc949),
    TM_UL(0xbfd4af27), TM_UL(0x88bbbdb5), TM_UL(0xe2034090), TM_UL(0x98d09675),
    TM_UL(0x6e63a0e0), TM_UL(0x15c361d2), TM_UL(0xc2e7661d), TM_UL(0x22d4ff8e),
    TM_UL(0x28683b6f), TM_UL(0xc07fd059), TM_UL(0xff2379c8), TM_UL(0x775f50e2),
    TM_UL(0x43c340d3), TM_UL(0xdf2f8656), TM_UL(0x887ca41a), TM_UL(0xa2d2bd2d),
    TM_UL(0xa1c9e0d6), TM_UL(0x346c4819), TM_UL(0x61b76d87), TM_UL(0x22540f2f),
    TM_UL(0x2abe32e1), TM_UL(0xaa54166b), TM_UL(0x22568e3a), TM_UL(0xa2d341d0),
    TM_UL(0x66db40c8), TM_UL(0xa784392f), TM_UL(0x004dff2f), TM_UL(0x2db9d2de),
    TM_UL(0x97943fac), TM_UL(0x4a97c1d8), TM_UL(0x527644b7), TM_UL(0xb5f437a7),
    TM_UL(0xb82cbaef), TM_UL(0xd751d159), TM_UL(0x6ff7f0ed), TM_UL(0x5a097a1f),
    TM_UL(0x827b68d0), TM_UL(0x90ecf52e), TM_UL(0x22b0c054), TM_UL(0xbc8e5935),
    TM_UL(0x4b6d2f7f), TM_UL(0x50bb64a2), TM_UL(0xd2664910), TM_UL(0xbee5812d),
    TM_UL(0xb7332290), TM_UL(0xe93b159f), TM_UL(0xb48ee411), TM_UL(0x4bff345d),
    TM_UL(0xfd45c240), TM_UL(0xad31973f), TM_UL(0xc4f6d02e), TM_UL(0x55fc8165),
    TM_UL(0xd5b1caad), TM_UL(0xa1ac2dae), TM_UL(0xa2d4b76d), TM_UL(0xc19b0c50),
    TM_UL(0x882240f2), TM_UL(0x0c6e4f38), TM_UL(0xa4e4bfd7), TM_UL(0x4f5ba272),
    TM_UL(0x564c1d2f), TM_UL(0xc59c5319), TM_UL(0xb949e354), TM_UL(0xb04669fe),
    TM_UL(0xb1b6ab8a), TM_UL(0xc71358dd), TM_UL(0x6385c545), TM_UL(0x110f935d),
    TM_UL(0x57538ad5), TM_UL(0x6a390493), TM_UL(0xe63d37e0), TM_UL(0x2a54f6b3),
    TM_UL(0x3a787d5f), TM_UL(0x6276a0b5), TM_UL(0x19a6fcdf), TM_UL(0x7a42206a),
    TM_UL(0x29f9d4d5), TM_UL(0xf61b1891), TM_UL(0xbb72275e), TM_UL(0xaa508167),
    TM_UL(0x38901091), TM_UL(0xc6b505eb), TM_UL(0x84c7cb8c), TM_UL(0x2ad75a0f),
    TM_UL(0x874a1427), TM_UL(0xa2d1936b), TM_UL(0x2ad286af), TM_UL(0xaa56d291),
    TM_UL(0xd7894360), TM_UL(0x425c750d), TM_UL(0x93b39e26), TM_UL(0x187184c9),
    TM_UL(0x6c00b32d), TM_UL(0x73e2bb14), TM_UL(0xa0bebc3c), TM_UL(0x54623779),
    TM_UL(0x64459eab), TM_UL(0x3f328b82), TM_UL(0x7718cf82), TM_UL(0x59a2cea6),
    TM_UL(0x04ee002e), TM_UL(0x89fe78e6), TM_UL(0x3fab0950), TM_UL(0x325ff6c2),
    TM_UL(0x81383f05), TM_UL(0x6963c5c8), TM_UL(0x76cb5ad6), TM_UL(0xd49974c9),
    TM_UL(0xca180dcf), TM_UL(0x380782d5), TM_UL(0xc7fa5cf6), TM_UL(0x8ac31511),
    TM_UL(0x35e79e13), TM_UL(0x47da91d0), TM_UL(0xf40f9086), TM_UL(0xa7e2419e),
    TM_UL(0x31366241), TM_UL(0x051ef495), TM_UL(0xaa573b04), TM_UL(0x4a805d8d),
    TM_UL(0x548300d0), TM_UL(0x00322a3c), TM_UL(0xbf64cddf), TM_UL(0xba57a68e),
    TM_UL(0x75c6372b), TM_UL(0x50afd341), TM_UL(0xa7c13275), TM_UL(0x915a0bf5),
    TM_UL(0x6b54bfab), TM_UL(0x2b0b1426), TM_UL(0xab4cc9d7), TM_UL(0x449ccd82),
    TM_UL(0xf7fbf265), TM_UL(0xab85c5f3), TM_UL(0x1b55db94), TM_UL(0xaad4e324),
    TM_UL(0xcfa4bd3f), TM_UL(0x2deaa3e2), TM_UL(0x9e204d02), TM_UL(0xc8bd25ac),
    TM_UL(0xeadf55b3), TM_UL(0xd5bd9e98), TM_UL(0xe31231b2), TM_UL(0x2ad5ad6c),
    TM_UL(0x954329de), TM_UL(0xadbe4528), TM_UL(0xd8710f69), TM_UL(0xaa51c90f),
    TM_UL(0xaa786bf6), TM_UL(0x22513f1e), TM_UL(0xaa51a79b), TM_UL(0x2ad344cc),
    TM_UL(0x7b5a41f0), TM_UL(0xd37cfbad), TM_UL(0x1b069505), TM_UL(0x41ece491),
    TM_UL(0xb4c332e6), TM_UL(0x032268d4), TM_UL(0xc9600acc), TM_UL(0xce387e6d),
    TM_UL(0xbf6bb16c), TM_UL(0x6a70fb78), TM_UL(0x0d03d9c9), TM_UL(0xd4df39de),
    TM_UL(0xe01063da), TM_UL(0x4736f464), TM_UL(0x5ad328d8), TM_UL(0xb347cc96),
    TM_UL(0x75bb0fc3), TM_UL(0x98511bfb), TM_UL(0x4ffbcc35), TM_UL(0xb58bcf6a),
    TM_UL(0xe11f0abc), TM_UL(0xbfc5fe4a), TM_UL(0xa70aec10), TM_UL(0xac39570a),
    TM_UL(0x3f04442f), TM_UL(0x6188b153), TM_UL(0xe0397a2e), TM_UL(0x5727cb79),
    TM_UL(0x9ceb418f), TM_UL(0x1cacd68d), TM_UL(0x2ad37c96), TM_UL(0x0175cb9d),
    TM_UL(0xc69dff09), TM_UL(0xc75b65f0), TM_UL(0xd9db40d8), TM_UL(0xec0e7779),
    TM_UL(0x4744ead4), TM_UL(0xb11c3274), TM_UL(0xdd24cb9e), TM_UL(0x7e1c54bd),
    TM_UL(0xf01144f9), TM_UL(0xd2240eb1), TM_UL(0x9675b3fd), TM_UL(0xa3ac3755),
    TM_UL(0xd47c27af), TM_UL(0x51c85f4d), TM_UL(0x56907596), TM_UL(0xa5bb15e6),
    TM_UL(0x580304f0), TM_UL(0xca042cf1), TM_UL(0x011a37ea), TM_UL(0x8dbfaadb),
    TM_UL(0x35ba3e4a), TM_UL(0x3526ffa0), TM_UL(0xc37b4d09), TM_UL(0xbc306ed9),
    TM_UL(0x98a52666), TM_UL(0x5648f725), TM_UL(0xff5e569d), TM_UL(0x0ced63d0),
    TM_UL(0x7c63b2cf), TM_UL(0x700b45e1), TM_UL(0xd5ea50f1), TM_UL(0x85a92872),
    TM_UL(0xaf1fbda7), TM_UL(0xd4234870), TM_UL(0xa7870bf3), TM_UL(0x2d3b4d79),
    TM_UL(0x42e04198), TM_UL(0x0cd0ede7), TM_UL(0x26470db8), TM_UL(0xf881814c),
    TM_UL(0x474d6ad7), TM_UL(0x7c0c5e5c), TM_UL(0xd1231959), TM_UL(0x381b7298),
    TM_UL(0xf5d2f4db), TM_UL(0xab838653), TM_UL(0x6e2f1e23), TM_UL(0x83719c9e),
    TM_UL(0xbd91e046), TM_UL(0x9a56456e), TM_UL(0xdc39200c), TM_UL(0x20c8c571),
    TM_UL(0x962bda1c), TM_UL(0xe1e696ff), TM_UL(0xb141ab08), TM_UL(0x7cca89b9),
    TM_UL(0x1a69e783), TM_UL(0x02cc4843), TM_UL(0xa2f7c579), TM_UL(0x429ef47d),
    TM_UL(0x427b169c), TM_UL(0x5ac9f049), TM_UL(0xdd8f0f00), TM_UL(0x5c8165bf)
};

static const tt32Bit TM_CONST_QLF S2[] = {
    TM_UL(0x1f201094), TM_UL(0xef0ba75b), TM_UL(0x69e3cf7e), TM_UL(0x393f4380),
    TM_UL(0xfe61cf7a), TM_UL(0xeec5207a), TM_UL(0x55889c94), TM_UL(0x72fc0651),
    TM_UL(0xada7ef79), TM_UL(0x4e1d7235), TM_UL(0xd55a63ce), TM_UL(0xde0436ba),
    TM_UL(0x99c430ef), TM_UL(0x5f0c0794), TM_UL(0x18dcdb7d), TM_UL(0xa1d6eff3),
    TM_UL(0xa0b52f7b), TM_UL(0x59e83605), TM_UL(0xee15b094), TM_UL(0xe9ffd909),
    TM_UL(0xdc440086), TM_UL(0xef944459), TM_UL(0xba83ccb3), TM_UL(0xe0c3cdfb),
    TM_UL(0xd1da4181), TM_UL(0x3b092ab1), TM_UL(0xf997f1c1), TM_UL(0xa5e6cf7b),
    TM_UL(0x01420ddb), TM_UL(0xe4e7ef5b), TM_UL(0x25a1ff41), TM_UL(0xe180f806),
    TM_UL(0x1fc41080), TM_UL(0x179bee7a), TM_UL(0xd37ac6a9), TM_UL(0xfe5830a4),
    TM_UL(0x98de8b7f), TM_UL(0x77e83f4e), TM_UL(0x79929269), TM_UL(0x24fa9f7b),
    TM_UL(0xe113c85b), TM_UL(0xacc40083), TM_UL(0xd7503525), TM_UL(0xf7ea615f),
    TM_UL(0x62143154), TM_UL(0x0d554b63), TM_UL(0x5d681121), TM_UL(0xc866c359),
    TM_UL(0x3d63cf73), TM_UL(0xcee234c0), TM_UL(0xd4d87e87), TM_UL(0x5c672b21),
    TM_UL(0x071f6181), TM_UL(0x39f7627f), TM_UL(0x361e3084), TM_UL(0xe4eb573b),
    TM_UL(0x602f64a4), TM_UL(0xd63acd9c), TM_UL(0x1bbc4635), TM_UL(0x9e81032d),
    TM_UL(0x2701f50c), TM_UL(0x99847ab4), TM_UL(0xa0e3df79), TM_UL(0xba6cf38c),
    TM_UL(0x10843094), TM_UL(0x2537a95e), TM_UL(0xf46f6ffe), TM_UL(0xa1ff3b1f),
    TM_UL(0x208cfb6a), TM_UL(0x8f458c74), TM_UL(0xd9e0a227), TM_UL(0x4ec73a34),
    TM_UL(0xfc884f69), TM_UL(0x3e4de8df), TM_UL(0xef0e0088), TM_UL(0x3559648d),
    TM_UL(0x8a45388c), TM_UL(0x1d804366), TM_UL(0x721d9bfd), TM_UL(0xa58684bb),
    TM_UL(0xe8256333), TM_UL(0x844e8212), TM_UL(0x128d8098), TM_UL(0xfed33fb4),
    TM_UL(0xce280ae1), TM_UL(0x27e19ba5), TM_UL(0xd5a6c252), TM_UL(0xe49754bd),
    TM_UL(0xc5d655dd), TM_UL(0xeb667064), TM_UL(0x77840b4d), TM_UL(0xa1b6a801),
    TM_UL(0x84db26a9), TM_UL(0xe0b56714), TM_UL(0x21f043b7), TM_UL(0xe5d05860),
    TM_UL(0x54f03084), TM_UL(0x066ff472), TM_UL(0xa31aa153), TM_UL(0xdadc4755),
    TM_UL(0xb5625dbf), TM_UL(0x68561be6), TM_UL(0x83ca6b94), TM_UL(0x2d6ed23b),
    TM_UL(0xeccf01db), TM_UL(0xa6d3d0ba), TM_UL(0xb6803d5c), TM_UL(0xaf77a709),
    TM_UL(0x33b4a34c), TM_UL(0x397bc8d6), TM_UL(0x5ee22b95), TM_UL(0x5f0e5304),
    TM_UL(0x81ed6f61), TM_UL(0x20e74364), TM_UL(0xb45e1378), TM_UL(0xde18639b),
    TM_UL(0x881ca122), TM_UL(0xb96726d1), TM_UL(0x8049a7e8), TM_UL(0x22b7da7b),
    TM_UL(0x5e552d25), TM_UL(0x5272d237), TM_UL(0x79d2951c), TM_UL(0xc60d894c),
    TM_UL(0x488cb402), TM_UL(0x1ba4fe5b), TM_UL(0xa4b09f6b), TM_UL(0x1ca815cf),
    TM_UL(0xa20c3005), TM_UL(0x8871df63), TM_UL(0xb9de2fcb), TM_UL(0x0cc6c9e9),
    TM_UL(0x0beeff53), TM_UL(0xe3214517), TM_UL(0xb4542835), TM_UL(0x9f63293c),
    TM_UL(0xee41e729), TM_UL(0x6e1d2d7c), TM_UL(0x50045286), TM_UL(0x1e6685f3),
    TM_UL(0xf33401c6), TM_UL(0x30a22c95), TM_UL(0x31a70850), TM_UL(0x60930f13),
    TM_UL(0x73f98417), TM_UL(0xa1269859), TM_UL(0xec645c44), TM_UL(0x52c877a9),
    TM_UL(0xcdff33a6), TM_UL(0xa02b1741), TM_UL(0x7cbad9a2), TM_UL(0x2180036f),
    TM_UL(0x50d99c08), TM_UL(0xcb3f4861), TM_UL(0xc26bd765), TM_UL(0x64a3f6ab),
    TM_UL(0x80342676), TM_UL(0x25a75e7b), TM_UL(0xe4e6d1fc), TM_UL(0x20c710e6),
    TM_UL(0xcdf0b680), TM_UL(0x17844d3b), TM_UL(0x31eef84d), TM_UL(0x7e0824e4),
    TM_UL(0x2ccb49eb), TM_UL(0x846a3bae), TM_UL(0x8ff77888), TM_UL(0xee5d60f6),
    TM_UL(0x7af75673), TM_UL(0x2fdd5cdb), TM_UL(0xa11631c1), TM_UL(0x30f66f43),
    TM_UL(0xb3faec54), TM_UL(0x157fd7fa), TM_UL(0xef8579cc), TM_UL(0xd152de58),
    TM_UL(0xdb2ffd5e), TM_UL(0x8f32ce19), TM_UL(0x306af97a), TM_UL(0x02f03ef8),
    TM_UL(0x99319ad5), TM_UL(0xc242fa0f), TM_UL(0xa7e3ebb0), TM_UL(0xc68e4906),
    TM_UL(0xb8da230c), TM_UL(0x80823028), TM_UL(0xdcdef3c8), TM_UL(0xd35fb171),
    TM_UL(0x088a1bc8), TM_UL(0xbec0c560), TM_UL(0x61a3c9e8), TM_UL(0xbca8f54d),
    TM_UL(0xc72feffa), TM_UL(0x22822e99), TM_UL(0x82c570b4), TM_UL(0xd8d94e89),
    TM_UL(0x8b1c34bc), TM_UL(0x301e16e6), TM_UL(0x273be979), TM_UL(0xb0ffeaa6),
    TM_UL(0x61d9b8c6), TM_UL(0x00b24869), TM_UL(0xb7ffce3f), TM_UL(0x08dc283b),
    TM_UL(0x43daf65a), TM_UL(0xf7e19798), TM_UL(0x7619b72f), TM_UL(0x8f1c9ba4),
    TM_UL(0xdc8637a0), TM_UL(0x16a7d3b1), TM_UL(0x9fc393b7), TM_UL(0xa7136eeb),
    TM_UL(0xc6bcc63e), TM_UL(0x1a513742), TM_UL(0xef6828bc), TM_UL(0x520365d6),
    TM_UL(0x2d6a77ab), TM_UL(0x3527ed4b), TM_UL(0x821fd216), TM_UL(0x095c6e2e),
    TM_UL(0xdb92f2fb), TM_UL(0x5eea29cb), TM_UL(0x145892f5), TM_UL(0x91584f7f),
    TM_UL(0x5483697b), TM_UL(0x2667a8cc), TM_UL(0x85196048), TM_UL(0x8c4bacea),
    TM_UL(0x833860d4), TM_UL(0x0d23e0f9), TM_UL(0x6c387e8a), TM_UL(0x0ae6d249),
    TM_UL(0xb284600c), TM_UL(0xd835731d), TM_UL(0xdcb1c647), TM_UL(0xac4c56ea),
    TM_UL(0x3ebd81b3), TM_UL(0x230eabb0), TM_UL(0x6438bc87), TM_UL(0xf0b5b1fa),
    TM_UL(0x8f5ea2b3), TM_UL(0xfc184642), TM_UL(0x0a036b7a), TM_UL(0x4fb089bd),
    TM_UL(0x649da589), TM_UL(0xa345415e), TM_UL(0x5c038323), TM_UL(0x3e5d3bb9),
    TM_UL(0x43d79572), TM_UL(0x7e6dd07c), TM_UL(0x06dfdf1e), TM_UL(0x6c6cc4ef),
    TM_UL(0x7160a539), TM_UL(0x73bfbe70), TM_UL(0x83877605), TM_UL(0x4523ecf1)
};

static const tt32Bit TM_CONST_QLF S3[] = {
    TM_UL(0x8defc240), TM_UL(0x25fa5d9f), TM_UL(0xeb903dbf), TM_UL(0xe810c907),
    TM_UL(0x47607fff), TM_UL(0x369fe44b), TM_UL(0x8c1fc644), TM_UL(0xaececa90),
    TM_UL(0xbeb1f9bf), TM_UL(0xeefbcaea), TM_UL(0xe8cf1950), TM_UL(0x51df07ae),
    TM_UL(0x920e8806), TM_UL(0xf0ad0548), TM_UL(0xe13c8d83), TM_UL(0x927010d5),
    TM_UL(0x11107d9f), TM_UL(0x07647db9), TM_UL(0xb2e3e4d4), TM_UL(0x3d4f285e),
    TM_UL(0xb9afa820), TM_UL(0xfade82e0), TM_UL(0xa067268b), TM_UL(0x8272792e),
    TM_UL(0x553fb2c0), TM_UL(0x489ae22b), TM_UL(0xd4ef9794), TM_UL(0x125e3fbc),
    TM_UL(0x21fffcee), TM_UL(0x825b1bfd), TM_UL(0x9255c5ed), TM_UL(0x1257a240),
    TM_UL(0x4e1a8302), TM_UL(0xbae07fff), TM_UL(0x528246e7), TM_UL(0x8e57140e),
    TM_UL(0x3373f7bf), TM_UL(0x8c9f8188), TM_UL(0xa6fc4ee8), TM_UL(0xc982b5a5),
    TM_UL(0xa8c01db7), TM_UL(0x579fc264), TM_UL(0x67094f31), TM_UL(0xf2bd3f5f),
    TM_UL(0x40fff7c1), TM_UL(0x1fb78dfc), TM_UL(0x8e6bd2c1), TM_UL(0x437be59b),
    TM_UL(0x99b03dbf), TM_UL(0xb5dbc64b), TM_UL(0x638dc0e6), TM_UL(0x55819d99),
    TM_UL(0xa197c81c), TM_UL(0x4a012d6e), TM_UL(0xc5884a28), TM_UL(0xccc36f71),
    TM_UL(0xb843c213), TM_UL(0x6c0743f1), TM_UL(0x8309893c), TM_UL(0x0feddd5f),
    TM_UL(0x2f7fe850), TM_UL(0xd7c07f7e), TM_UL(0x02507fbf), TM_UL(0x5afb9a04),
    TM_UL(0xa747d2d0), TM_UL(0x1651192e), TM_UL(0xaf70bf3e), TM_UL(0x58c31380),
    TM_UL(0x5f98302e), TM_UL(0x727cc3c4), TM_UL(0x0a0fb402), TM_UL(0x0f7fef82),
    TM_UL(0x8c96fdad), TM_UL(0x5d2c2aae), TM_UL(0x8ee99a49), TM_UL(0x50da88b8),
    TM_UL(0x8427f4a0), TM_UL(0x1eac5790), TM_UL(0x796fb449), TM_UL(0x8252dc15),
    TM_UL(0xefbd7d9b), TM_UL(0xa672597d), TM_UL(0xada840d8), TM_UL(0x45f54504),
    TM_UL(0xfa5d7403), TM_UL(0xe83ec305), TM_UL(0x4f91751a), TM_UL(0x925669c2),
    TM_UL(0x23efe941), TM_UL(0xa903f12e), TM_UL(0x60270df2), TM_UL(0x0276e4b6),
    TM_UL(0x94fd6574), TM_UL(0x927985b2), TM_UL(0x8276dbcb), TM_UL(0x02778176),
    TM_UL(0xf8af918d), TM_UL(0x4e48f79e), TM_UL(0x8f616ddf), TM_UL(0xe29d840e),
    TM_UL(0x842f7d83), TM_UL(0x340ce5c8), TM_UL(0x96bbb682), TM_UL(0x93b4b148),
    TM_UL(0xef303cab), TM_UL(0x984faf28), TM_UL(0x779faf9b), TM_UL(0x92dc560d),
    TM_UL(0x224d1e20), TM_UL(0x8437aa88), TM_UL(0x7d29dc96), TM_UL(0x2756d3dc),
    TM_UL(0x8b907cee), TM_UL(0xb51fd240), TM_UL(0xe7c07ce3), TM_UL(0xe566b4a1),
    TM_UL(0xc3e9615e), TM_UL(0x3cf8209d), TM_UL(0x6094d1e3), TM_UL(0xcd9ca341),
    TM_UL(0x5c76460e), TM_UL(0x00ea983b), TM_UL(0xd4d67881), TM_UL(0xfd47572c),
    TM_UL(0xf76cedd9), TM_UL(0xbda8229c), TM_UL(0x127dadaa), TM_UL(0x438a074e),
    TM_UL(0x1f97c090), TM_UL(0x081bdb8a), TM_UL(0x93a07ebe), TM_UL(0xb938ca15),
    TM_UL(0x97b03cff), TM_UL(0x3dc2c0f8), TM_UL(0x8d1ab2ec), TM_UL(0x64380e51),
    TM_UL(0x68cc7bfb), TM_UL(0xd90f2788), TM_UL(0x12490181), TM_UL(0x5de5ffd4),
    TM_UL(0xdd7ef86a), TM_UL(0x76a2e214), TM_UL(0xb9a40368), TM_UL(0x925d958f),
    TM_UL(0x4b39fffa), TM_UL(0xba39aee9), TM_UL(0xa4ffd30b), TM_UL(0xfaf7933b),
    TM_UL(0x6d498623), TM_UL(0x193cbcfa), TM_UL(0x27627545), TM_UL(0x825cf47a),
    TM_UL(0x61bd8ba0), TM_UL(0xd11e42d1), TM_UL(0xcead04f4), TM_UL(0x127ea392),
    TM_UL(0x10428db7), TM_UL(0x8272a972), TM_UL(0x9270c4a8), TM_UL(0x127de50b),
    TM_UL(0x285ba1c8), TM_UL(0x3c62f44f), TM_UL(0x35c0eaa5), TM_UL(0xe805d231),
    TM_UL(0x428929fb), TM_UL(0xb4fcdf82), TM_UL(0x4fb66a53), TM_UL(0x0e7dc15b),
    TM_UL(0x1f081fab), TM_UL(0x108618ae), TM_UL(0xfcfd086d), TM_UL(0xf9ff2889),
    TM_UL(0x694bcc11), TM_UL(0x236a5cae), TM_UL(0x12deca4d), TM_UL(0x2c3f8cc5),
    TM_UL(0xd2d02dfe), TM_UL(0xf8ef5896), TM_UL(0xe4cf52da), TM_UL(0x95155b67),
    TM_UL(0x494a488c), TM_UL(0xb9b6a80c), TM_UL(0x5c8f82bc), TM_UL(0x89d36b45),
    TM_UL(0x3a609437), TM_UL(0xec00c9a9), TM_UL(0x44715253), TM_UL(0x0a874b49),
    TM_UL(0xd773bc40), TM_UL(0x7c34671c), TM_UL(0x02717ef6), TM_UL(0x4feb5536),
    TM_UL(0xa2d02fff), TM_UL(0xd2bf60c4), TM_UL(0xd43f03c0), TM_UL(0x50b4ef6d),
    TM_UL(0x07478cd1), TM_UL(0x006e1888), TM_UL(0xa2e53f55), TM_UL(0xb9e6d4bc),
    TM_UL(0xa2048016), TM_UL(0x97573833), TM_UL(0xd7207d67), TM_UL(0xde0f8f3d),
    TM_UL(0x72f87b33), TM_UL(0xabcc4f33), TM_UL(0x7688c55d), TM_UL(0x7b00a6b0),
    TM_UL(0x947b0001), TM_UL(0x570075d2), TM_UL(0xf9bb88f8), TM_UL(0x8942019e),
    TM_UL(0x4264a5ff), TM_UL(0x856302e0), TM_UL(0x72dbd92b), TM_UL(0xee971b69),
    TM_UL(0x6ea22fde), TM_UL(0x5f08ae2b), TM_UL(0xaf7a616d), TM_UL(0xe5c98767),
    TM_UL(0xcf1febd2), TM_UL(0x61efc8c2), TM_UL(0xf1ac2571), TM_UL(0xcc8239c2),
    TM_UL(0x67214cb8), TM_UL(0xb1e583d1), TM_UL(0xb7dc3e62), TM_UL(0x7f10bdce),
    TM_UL(0xf90a5c38), TM_UL(0x0ff0443d), TM_UL(0x606e6dc6), TM_UL(0x60543a49),
    TM_UL(0x5727c148), TM_UL(0x2be98a1d), TM_UL(0x8ab41738), TM_UL(0x20e1be24),
    TM_UL(0xaf96da0f), TM_UL(0x68458425), TM_UL(0x99833be5), TM_UL(0x600d457d),
    TM_UL(0x282f9350), TM_UL(0x8334b362), TM_UL(0xd91d1120), TM_UL(0x2b6d8da0),
    TM_UL(0x642b1e31), TM_UL(0x9c305a00), TM_UL(0x52bce688), TM_UL(0x1b03588a),
    TM_UL(0xf7baefd5), TM_UL(0x4142ed9c), TM_UL(0xa4315c11), TM_UL(0x83323ec5),
    TM_UL(0xdfef4636), TM_UL(0xa133c501), TM_UL(0xe9d3531c), TM_UL(0xee353783)
};

static const tt32Bit TM_CONST_QLF S4[] = {
    TM_UL(0x9db30420), TM_UL(0x1fb6e9de), TM_UL(0xa7be7bef), TM_UL(0xd273a298),
    TM_UL(0x4a4f7bdb), TM_UL(0x64ad8c57), TM_UL(0x85510443), TM_UL(0xfa020ed1),
    TM_UL(0x7e287aff), TM_UL(0xe60fb663), TM_UL(0x095f35a1), TM_UL(0x79ebf120),
    TM_UL(0xfd059d43), TM_UL(0x6497b7b1), TM_UL(0xf3641f63), TM_UL(0x241e4adf),
    TM_UL(0x28147f5f), TM_UL(0x4fa2b8cd), TM_UL(0xc9430040), TM_UL(0x0cc32220),
    TM_UL(0xfdd30b30), TM_UL(0xc0a5374f), TM_UL(0x1d2d00d9), TM_UL(0x24147b15),
    TM_UL(0xee4d111a), TM_UL(0x0fca5167), TM_UL(0x71ff904c), TM_UL(0x2d195ffe),
    TM_UL(0x1a05645f), TM_UL(0x0c13fefe), TM_UL(0x081b08ca), TM_UL(0x05170121),
    TM_UL(0x80530100), TM_UL(0xe83e5efe), TM_UL(0xac9af4f8), TM_UL(0x7fe72701),
    TM_UL(0xd2b8ee5f), TM_UL(0x06df4261), TM_UL(0xbb9e9b8a), TM_UL(0x7293ea25),
    TM_UL(0xce84ffdf), TM_UL(0xf5718801), TM_UL(0x3dd64b04), TM_UL(0xa26f263b),
    TM_UL(0x7ed48400), TM_UL(0x547eebe6), TM_UL(0x446d4ca0), TM_UL(0x6cf3d6f5),
    TM_UL(0x2649abdf), TM_UL(0xaea0c7f5), TM_UL(0x36338cc1), TM_UL(0x503f7e93),
    TM_UL(0xd3772061), TM_UL(0x11b638e1), TM_UL(0x72500e03), TM_UL(0xf80eb2bb),
    TM_UL(0xabe0502e), TM_UL(0xec8d77de), TM_UL(0x57971e81), TM_UL(0xe14f6746),
    TM_UL(0xc9335400), TM_UL(0x6920318f), TM_UL(0x081dbb99), TM_UL(0xffc304a5),
    TM_UL(0x4d351805), TM_UL(0x7f3d5ce3), TM_UL(0xa6c866c6), TM_UL(0x5d5bcca9),
    TM_UL(0xdaec6fea), TM_UL(0x9f926f91), TM_UL(0x9f46222f), TM_UL(0x3991467d),
    TM_UL(0xa5bf6d8e), TM_UL(0x1143c44f), TM_UL(0x43958302), TM_UL(0xd0214eeb),
    TM_UL(0x022083b8), TM_UL(0x3fb6180c), TM_UL(0x18f8931e), TM_UL(0x281658e6),
    TM_UL(0x26486e3e), TM_UL(0x8bd78a70), TM_UL(0x7477e4c1), TM_UL(0xb506e07c),
    TM_UL(0xf32d0a25), TM_UL(0x79098b02), TM_UL(0xe4eabb81), TM_UL(0x28123b23),
    TM_UL(0x69dead38), TM_UL(0x1574ca16), TM_UL(0xdf871b62), TM_UL(0x211c40b7),
    TM_UL(0xa51a9ef9), TM_UL(0x0014377b), TM_UL(0x041e8ac8), TM_UL(0x09114003),
    TM_UL(0xbd59e4d2), TM_UL(0xe3d156d5), TM_UL(0x4fe876d5), TM_UL(0x2f91a340),
    TM_UL(0x557be8de), TM_UL(0x00eae4a7), TM_UL(0x0ce5c2ec), TM_UL(0x4db4bba6),
    TM_UL(0xe756bdff), TM_UL(0xdd3369ac), TM_UL(0xec17b035), TM_UL(0x06572327),
    TM_UL(0x99afc8b0), TM_UL(0x56c8c391), TM_UL(0x6b65811c), TM_UL(0x5e146119),
    TM_UL(0x6e85cb75), TM_UL(0xbe07c002), TM_UL(0xc2325577), TM_UL(0x893ff4ec),
    TM_UL(0x5bbfc92d), TM_UL(0xd0ec3b25), TM_UL(0xb7801ab7), TM_UL(0x8d6d3b24),
    TM_UL(0x20c763ef), TM_UL(0xc366a5fc), TM_UL(0x9c382880), TM_UL(0x0ace3205),
    TM_UL(0xaac9548a), TM_UL(0xeca1d7c7), TM_UL(0x041afa32), TM_UL(0x1d16625a),
    TM_UL(0x6701902c), TM_UL(0x9b757a54), TM_UL(0x31d477f7), TM_UL(0x9126b031),
    TM_UL(0x36cc6fdb), TM_UL(0xc70b8b46), TM_UL(0xd9e66a48), TM_UL(0x56e55a79),
    TM_UL(0x026a4ceb), TM_UL(0x52437eff), TM_UL(0x2f8f76b4), TM_UL(0x0df980a5),
    TM_UL(0x8674cde3), TM_UL(0xedda04eb), TM_UL(0x17a9be04), TM_UL(0x2c18f4df),
    TM_UL(0xb7747f9d), TM_UL(0xab2af7b4), TM_UL(0xefc34d20), TM_UL(0x2e096b7c),
    TM_UL(0x1741a254), TM_UL(0xe5b6a035), TM_UL(0x213d42f6), TM_UL(0x2c1c7c26),
    TM_UL(0x61c2f50f), TM_UL(0x6552daf9), TM_UL(0xd2c231f8), TM_UL(0x25130f69),
    TM_UL(0xd8167fa2), TM_UL(0x0418f2c8), TM_UL(0x001a96a6), TM_UL(0x0d1526ab),
    TM_UL(0x63315c21), TM_UL(0x5e0a72ec), TM_UL(0x49bafefd), TM_UL(0x187908d9),
    TM_UL(0x8d0dbd86), TM_UL(0x311170a7), TM_UL(0x3e9b640c), TM_UL(0xcc3e10d7),
    TM_UL(0xd5cad3b6), TM_UL(0x0caec388), TM_UL(0xf73001e1), TM_UL(0x6c728aff),
    TM_UL(0x71eae2a1), TM_UL(0x1f9af36e), TM_UL(0xcfcbd12f), TM_UL(0xc1de8417),
    TM_UL(0xac07be6b), TM_UL(0xcb44a1d8), TM_UL(0x8b9b0f56), TM_UL(0x013988c3),
    TM_UL(0xb1c52fca), TM_UL(0xb4be31cd), TM_UL(0xd8782806), TM_UL(0x12a3a4e2),
    TM_UL(0x6f7de532), TM_UL(0x58fd7eb6), TM_UL(0xd01ee900), TM_UL(0x24adffc2),
    TM_UL(0xf4990fc5), TM_UL(0x9711aac5), TM_UL(0x001d7b95), TM_UL(0x82e5e7d2),
    TM_UL(0x109873f6), TM_UL(0x00613096), TM_UL(0xc32d9521), TM_UL(0xada121ff),
    TM_UL(0x29908415), TM_UL(0x7fbb977f), TM_UL(0xaf9eb3db), TM_UL(0x29c9ed2a),
    TM_UL(0x5ce2a465), TM_UL(0xa730f32c), TM_UL(0xd0aa3fe8), TM_UL(0x8a5cc091),
    TM_UL(0xd49e2ce7), TM_UL(0x0ce454a9), TM_UL(0xd60acd86), TM_UL(0x015f1919),
    TM_UL(0x77079103), TM_UL(0xdea03af6), TM_UL(0x78a8565e), TM_UL(0xdee356df),
    TM_UL(0x21f05cbe), TM_UL(0x8b75e387), TM_UL(0xb3c50651), TM_UL(0xb8a5c3ef),
    TM_UL(0xd8eeb6d2), TM_UL(0xe523be77), TM_UL(0xc2154529), TM_UL(0x2f69efdf),
    TM_UL(0xafe67afb), TM_UL(0xf470c4b2), TM_UL(0xf3e0eb5b), TM_UL(0xd6cc9876),
    TM_UL(0x39e4460c), TM_UL(0x1fda8538), TM_UL(0x1987832f), TM_UL(0xca007367),
    TM_UL(0xa99144f8), TM_UL(0x296b299e), TM_UL(0x492fc295), TM_UL(0x9266beab),
    TM_UL(0xb5676e69), TM_UL(0x9bd3ddda), TM_UL(0xdf7e052f), TM_UL(0xdb25701c),
    TM_UL(0x1b5e51ee), TM_UL(0xf65324e6), TM_UL(0x6afce36c), TM_UL(0x0316cc04),
    TM_UL(0x8644213e), TM_UL(0xb7dc59d0), TM_UL(0x7965291f), TM_UL(0xccd6fd43),
    TM_UL(0x41823979), TM_UL(0x932bcdf6), TM_UL(0xb657c34d), TM_UL(0x4edfd282),
    TM_UL(0x7ae5290c), TM_UL(0x3cb9536b), TM_UL(0x851e20fe), TM_UL(0x9833557e),
    TM_UL(0x13ecf0b0), TM_UL(0xd3ffb372), TM_UL(0x3f85c5c1), TM_UL(0x0aef7ed2)
};

static const tt32Bit TM_CONST_QLF S5[] = {
    TM_UL(0x7ec90c04), TM_UL(0x2c6e74b9), TM_UL(0x9b0e66df), TM_UL(0xa6337911),
    TM_UL(0xb86a7fff), TM_UL(0x1dd358f5), TM_UL(0x44dd9d44), TM_UL(0x1731167f),
    TM_UL(0x08fbf1fa), TM_UL(0xe7f511cc), TM_UL(0xd2051b00), TM_UL(0x735aba00),
    TM_UL(0x2ab722d8), TM_UL(0x386381cb), TM_UL(0xacf6243a), TM_UL(0x69befd7a),
    TM_UL(0xe6a2e77f), TM_UL(0xf0c720cd), TM_UL(0xc4494816), TM_UL(0xccf5c180),
    TM_UL(0x38851640), TM_UL(0x15b0a848), TM_UL(0xe68b18cb), TM_UL(0x4caadeff),
    TM_UL(0x5f480a01), TM_UL(0x0412b2aa), TM_UL(0x259814fc), TM_UL(0x41d0efe2),
    TM_UL(0x4e40b48d), TM_UL(0x248eb6fb), TM_UL(0x8dba1cfe), TM_UL(0x41a99b02),
    TM_UL(0x1a550a04), TM_UL(0xba8f65cb), TM_UL(0x7251f4e7), TM_UL(0x95a51725),
    TM_UL(0xc106ecd7), TM_UL(0x97a5980a), TM_UL(0xc539b9aa), TM_UL(0x4d79fe6a),
    TM_UL(0xf2f3f763), TM_UL(0x68af8040), TM_UL(0xed0c9e56), TM_UL(0x11b4958b),
    TM_UL(0xe1eb5a88), TM_UL(0x8709e6b0), TM_UL(0xd7e07156), TM_UL(0x4e29fea7),
    TM_UL(0x6366e52d), TM_UL(0x02d1c000), TM_UL(0xc4ac8e05), TM_UL(0x9377f571),
    TM_UL(0x0c05372a), TM_UL(0x578535f2), TM_UL(0x2261be02), TM_UL(0xd642a0c9),
    TM_UL(0xdf13a280), TM_UL(0x74b55bd2), TM_UL(0x682199c0), TM_UL(0xd421e5ec),
    TM_UL(0x53fb3ce8), TM_UL(0xc8adedb3), TM_UL(0x28a87fc9), TM_UL(0x3d959981),
    TM_UL(0x5c1ff900), TM_UL(0xfe38d399), TM_UL(0x0c4eff0b), TM_UL(0x062407ea),
    TM_UL(0xaa2f4fb1), TM_UL(0x4fb96976), TM_UL(0x90c79505), TM_UL(0xb0a8a774),
    TM_UL(0xef55a1ff), TM_UL(0xe59ca2c2), TM_UL(0xa6b62d27), TM_UL(0xe66a4263),
    TM_UL(0xdf65001f), TM_UL(0x0ec50966), TM_UL(0xdfdd55bc), TM_UL(0x29de0655),
    TM_UL(0x911e739a), TM_UL(0x17af8975), TM_UL(0x32c7911c), TM_UL(0x89f89468),
    TM_UL(0x0d01e980), TM_UL(0x524755f4), TM_UL(0x03b63cc9), TM_UL(0x0cc844b2),
    TM_UL(0xbcf3f0aa), TM_UL(0x87ac36e9), TM_UL(0xe53a7426), TM_UL(0x01b3d82b),
    TM_UL(0x1a9e7449), TM_UL(0x64ee2d7e), TM_UL(0xcddbb1da), TM_UL(0x01c94910),
    TM_UL(0xb868bf80), TM_UL(0x0d26f3fd), TM_UL(0x9342ede7), TM_UL(0x04a5c284),
    TM_UL(0x636737b6), TM_UL(0x50f5b616), TM_UL(0xf24766e3), TM_UL(0x8eca36c1),
    TM_UL(0x136e05db), TM_UL(0xfef18391), TM_UL(0xfb887a37), TM_UL(0xd6e7f7d4),
    TM_UL(0xc7fb7dc9), TM_UL(0x3063fcdf), TM_UL(0xb6f589de), TM_UL(0xec2941da),
    TM_UL(0x26e46695), TM_UL(0xb7566419), TM_UL(0xf654efc5), TM_UL(0xd08d58b7),
    TM_UL(0x48925401), TM_UL(0xc1bacb7f), TM_UL(0xe5ff550f), TM_UL(0xb6083049),
    TM_UL(0x5bb5d0e8), TM_UL(0x87d72e5a), TM_UL(0xab6a6ee1), TM_UL(0x223a66ce),
    TM_UL(0xc62bf3cd), TM_UL(0x9e0885f9), TM_UL(0x68cb3e47), TM_UL(0x086c010f),
    TM_UL(0xa21de820), TM_UL(0xd18b69de), TM_UL(0xf3f65777), TM_UL(0xfa02c3f6),
    TM_UL(0x407edac3), TM_UL(0xcbb3d550), TM_UL(0x1793084d), TM_UL(0xb0d70eba),
    TM_UL(0x0ab378d5), TM_UL(0xd951fb0c), TM_UL(0xded7da56), TM_UL(0x4124bbe4),
    TM_UL(0x94ca0b56), TM_UL(0x0f5755d1), TM_UL(0xe0e1e56e), TM_UL(0x6184b5be),
    TM_UL(0x580a249f), TM_UL(0x94f74bc0), TM_UL(0xe327888e), TM_UL(0x9f7b5561),
    TM_UL(0xc3dc0280), TM_UL(0x05687715), TM_UL(0x646c6bd7), TM_UL(0x44904db3),
    TM_UL(0x66b4f0a3), TM_UL(0xc0f1648a), TM_UL(0x697ed5af), TM_UL(0x49e92ff6),
    TM_UL(0x309e374f), TM_UL(0x2cb6356a), TM_UL(0x85808573), TM_UL(0x4991f840),
    TM_UL(0x76f0ae02), TM_UL(0x083be84d), TM_UL(0x28421c9a), TM_UL(0x44489406),
    TM_UL(0x736e4cb8), TM_UL(0xc1092910), TM_UL(0x8bc95fc6), TM_UL(0x7d869cf4),
    TM_UL(0x134f616f), TM_UL(0x2e77118d), TM_UL(0xb31b2be1), TM_UL(0xaa90b472),
    TM_UL(0x3ca5d717), TM_UL(0x7d161bba), TM_UL(0x9cad9010), TM_UL(0xaf462ba2),
    TM_UL(0x9fe459d2), TM_UL(0x45d34559), TM_UL(0xd9f2da13), TM_UL(0xdbc65487),
    TM_UL(0xf3e4f94e), TM_UL(0x176d486f), TM_UL(0x097c13ea), TM_UL(0x631da5c7),
    TM_UL(0x445f7382), TM_UL(0x175683f4), TM_UL(0xcdc66a97), TM_UL(0x70be0288),
    TM_UL(0xb3cdcf72), TM_UL(0x6e5dd2f3), TM_UL(0x20936079), TM_UL(0x459b80a5),
    TM_UL(0xbe60e2db), TM_UL(0xa9c23101), TM_UL(0xeba5315c), TM_UL(0x224e42f2),
    TM_UL(0x1c5c1572), TM_UL(0xf6721b2c), TM_UL(0x1ad2fff3), TM_UL(0x8c25404e),
    TM_UL(0x324ed72f), TM_UL(0x4067b7fd), TM_UL(0x0523138e), TM_UL(0x5ca3bc78),
    TM_UL(0xdc0fd66e), TM_UL(0x75922283), TM_UL(0x784d6b17), TM_UL(0x58ebb16e),
    TM_UL(0x44094f85), TM_UL(0x3f481d87), TM_UL(0xfcfeae7b), TM_UL(0x77b5ff76),
    TM_UL(0x8c2302bf), TM_UL(0xaaf47556), TM_UL(0x5f46b02a), TM_UL(0x2b092801),
    TM_UL(0x3d38f5f7), TM_UL(0x0ca81f36), TM_UL(0x52af4a8a), TM_UL(0x66d5e7c0),
    TM_UL(0xdf3b0874), TM_UL(0x95055110), TM_UL(0x1b5ad7a8), TM_UL(0xf61ed5ad),
    TM_UL(0x6cf6e479), TM_UL(0x20758184), TM_UL(0xd0cefa65), TM_UL(0x88f7be58),
    TM_UL(0x4a046826), TM_UL(0x0ff6f8f3), TM_UL(0xa09c7f70), TM_UL(0x5346aba0),
    TM_UL(0x5ce96c28), TM_UL(0xe176eda3), TM_UL(0x6bac307f), TM_UL(0x376829d2),
    TM_UL(0x85360fa9), TM_UL(0x17e3fe2a), TM_UL(0x24b79767), TM_UL(0xf5a96b20),
    TM_UL(0xd6cd2595), TM_UL(0x68ff1ebf), TM_UL(0x7555442c), TM_UL(0xf19f06be),
    TM_UL(0xf9e0659a), TM_UL(0xeeb9491d), TM_UL(0x34010718), TM_UL(0xbb30cab8),
    TM_UL(0xe822fe15), TM_UL(0x88570983), TM_UL(0x750e6249), TM_UL(0xda627e55),
    TM_UL(0x5e76ffa8), TM_UL(0xb1534546), TM_UL(0x6d47de08), TM_UL(0xefe9e7d4)
};

static const tt32Bit TM_CONST_QLF S6[] = {
    TM_UL(0xf6fa8f9d), TM_UL(0x2cac6ce1), TM_UL(0x4ca34867), TM_UL(0xe2337f7c),
    TM_UL(0x95db08e7), TM_UL(0x016843b4), TM_UL(0xeced5cbc), TM_UL(0x325553ac),
    TM_UL(0xbf9f0960), TM_UL(0xdfa1e2ed), TM_UL(0x83f0579d), TM_UL(0x63ed86b9),
    TM_UL(0x1ab6a6b8), TM_UL(0xde5ebe39), TM_UL(0xf38ff732), TM_UL(0x8989b138),
    TM_UL(0x33f14961), TM_UL(0xc01937bd), TM_UL(0xf506c6da), TM_UL(0xe4625e7e),
    TM_UL(0xa308ea99), TM_UL(0x4e23e33c), TM_UL(0x79cbd7cc), TM_UL(0x48a14367),
    TM_UL(0xa3149619), TM_UL(0xfec94bd5), TM_UL(0xa114174a), TM_UL(0xeaa01866),
    TM_UL(0xa084db2d), TM_UL(0x09a8486f), TM_UL(0xa888614a), TM_UL(0x2900af98),
    TM_UL(0x01665991), TM_UL(0xe1992863), TM_UL(0xc8f30c60), TM_UL(0x2e78ef3c),
    TM_UL(0xd0d51932), TM_UL(0xcf0fec14), TM_UL(0xf7ca07d2), TM_UL(0xd0a82072),
    TM_UL(0xfd41197e), TM_UL(0x9305a6b0), TM_UL(0xe86be3da), TM_UL(0x74bed3cd),
    TM_UL(0x372da53c), TM_UL(0x4c7f4448), TM_UL(0xdab5d440), TM_UL(0x6dba0ec3),
    TM_UL(0x083919a7), TM_UL(0x9fbaeed9), TM_UL(0x49dbcfb0), TM_UL(0x4e670c53),
    TM_UL(0x5c3d9c01), TM_UL(0x64bdb941), TM_UL(0x2c0e636a), TM_UL(0xba7dd9cd),
    TM_UL(0xea6f7388), TM_UL(0xe70bc762), TM_UL(0x35f29adb), TM_UL(0x5c4cdd8d),
    TM_UL(0xf0d48d8c), TM_UL(0xb88153e2), TM_UL(0x08a19866), TM_UL(0x1ae2eac8),
    TM_UL(0x284caf89), TM_UL(0xaa928223), TM_UL(0x9334be53), TM_UL(0x3b3a21bf),
    TM_UL(0x16434be3), TM_UL(0x9aea3906), TM_UL(0xefe8c36e), TM_UL(0xf890cdd9),
    TM_UL(0x80226dae), TM_UL(0xc340a4a3), TM_UL(0xdf7e9c09), TM_UL(0xa694a807),
    TM_UL(0x5b7c5ecc), TM_UL(0x221db3a6), TM_UL(0x9a69a02f), TM_UL(0x68818a54),
    TM_UL(0xceb2296f), TM_UL(0x53c0843a), TM_UL(0xfe893655), TM_UL(0x25bfe68a),
    TM_UL(0xb4628abc), TM_UL(0xcf222ebf), TM_UL(0x25ac6f48), TM_UL(0xa9a99387),
    TM_UL(0x53bddb65), TM_UL(0xe76ffbe7), TM_UL(0xe967fd78), TM_UL(0x0ba93563),
    TM_UL(0x8e342bc1), TM_UL(0xe8a11be9), TM_UL(0x4980740d), TM_UL(0xc8087dfc),
    TM_UL(0x8de4bf99), TM_UL(0xa11101a0), TM_UL(0x7fd37975), TM_UL(0xda5a26c0),
    TM_UL(0xe81f994f), TM_UL(0x9528cd89), TM_UL(0xfd339fed), TM_UL(0xb87834bf),
    TM_UL(0x5f04456d), TM_UL(0x22258698), TM_UL(0xc9c4c83b), TM_UL(0x2dc156be),
    TM_UL(0x4f628daa), TM_UL(0x57f55ec5), TM_UL(0xe2220abe), TM_UL(0xd2916ebf),
    TM_UL(0x4ec75b95), TM_UL(0x24f2c3c0), TM_UL(0x42d15d99), TM_UL(0xcd0d7fa0),
    TM_UL(0x7b6e27ff), TM_UL(0xa8dc8af0), TM_UL(0x7345c106), TM_UL(0xf41e232f),
    TM_UL(0x35162386), TM_UL(0xe6ea8926), TM_UL(0x3333b094), TM_UL(0x157ec6f2),
    TM_UL(0x372b74af), TM_UL(0x692573e4), TM_UL(0xe9a9d848), TM_UL(0xf3160289),
    TM_UL(0x3a62ef1d), TM_UL(0xa787e238), TM_UL(0xf3a5f676), TM_UL(0x74364853),
    TM_UL(0x20951063), TM_UL(0x4576698d), TM_UL(0xb6fad407), TM_UL(0x592af950),
    TM_UL(0x36f73523), TM_UL(0x4cfb6e87), TM_UL(0x7da4cec0), TM_UL(0x6c152daa),
    TM_UL(0xcb0396a8), TM_UL(0xc50dfe5d), TM_UL(0xfcd707ab), TM_UL(0x0921c42f),
    TM_UL(0x89dff0bb), TM_UL(0x5fe2be78), TM_UL(0x448f4f33), TM_UL(0x754613c9),
    TM_UL(0x2b05d08d), TM_UL(0x48b9d585), TM_UL(0xdc049441), TM_UL(0xc8098f9b),
    TM_UL(0x7dede786), TM_UL(0xc39a3373), TM_UL(0x42410005), TM_UL(0x6a091751),
    TM_UL(0x0ef3c8a6), TM_UL(0x890072d6), TM_UL(0x28207682), TM_UL(0xa9a9f7be),
    TM_UL(0xbf32679d), TM_UL(0xd45b5b75), TM_UL(0xb353fd00), TM_UL(0xcbb0e358),
    TM_UL(0x830f220a), TM_UL(0x1f8fb214), TM_UL(0xd372cf08), TM_UL(0xcc3c4a13),
    TM_UL(0x8cf63166), TM_UL(0x061c87be), TM_UL(0x88c98f88), TM_UL(0x6062e397),
    TM_UL(0x47cf8e7a), TM_UL(0xb6c85283), TM_UL(0x3cc2acfb), TM_UL(0x3fc06976),
    TM_UL(0x4e8f0252), TM_UL(0x64d8314d), TM_UL(0xda3870e3), TM_UL(0x1e665459),
    TM_UL(0xc10908f0), TM_UL(0x513021a5), TM_UL(0x6c5b68b7), TM_UL(0x822f8aa0),
    TM_UL(0x3007cd3e), TM_UL(0x74719eef), TM_UL(0xdc872681), TM_UL(0x073340d4),
    TM_UL(0x7e432fd9), TM_UL(0x0c5ec241), TM_UL(0x8809286c), TM_UL(0xf592d891),
    TM_UL(0x08a930f6), TM_UL(0x957ef305), TM_UL(0xb7fbffbd), TM_UL(0xc266e96f),
    TM_UL(0x6fe4ac98), TM_UL(0xb173ecc0), TM_UL(0xbc60b42a), TM_UL(0x953498da),
    TM_UL(0xfba1ae12), TM_UL(0x2d4bd736), TM_UL(0x0f25faab), TM_UL(0xa4f3fceb),
    TM_UL(0xe2969123), TM_UL(0x257f0c3d), TM_UL(0x9348af49), TM_UL(0x361400bc),
    TM_UL(0xe8816f4a), TM_UL(0x3814f200), TM_UL(0xa3f94043), TM_UL(0x9c7a54c2),
    TM_UL(0xbc704f57), TM_UL(0xda41e7f9), TM_UL(0xc25ad33a), TM_UL(0x54f4a084),
    TM_UL(0xb17f5505), TM_UL(0x59357cbe), TM_UL(0xedbd15c8), TM_UL(0x7f97c5ab),
    TM_UL(0xba5ac7b5), TM_UL(0xb6f6deaf), TM_UL(0x3a479c3a), TM_UL(0x5302da25),
    TM_UL(0x653d7e6a), TM_UL(0x54268d49), TM_UL(0x51a477ea), TM_UL(0x5017d55b),
    TM_UL(0xd7d25d88), TM_UL(0x44136c76), TM_UL(0x0404a8c8), TM_UL(0xb8e5a121),
    TM_UL(0xb81a928a), TM_UL(0x60ed5869), TM_UL(0x97c55b96), TM_UL(0xeaec991b),
    TM_UL(0x29935913), TM_UL(0x01fdb7f1), TM_UL(0x088e8dfa), TM_UL(0x9ab6f6f5),
    TM_UL(0x3b4cbf9f), TM_UL(0x4a5de3ab), TM_UL(0xe6051d35), TM_UL(0xa0e1d855),
    TM_UL(0xd36b4cf1), TM_UL(0xf544edeb), TM_UL(0xb0e93524), TM_UL(0xbebb8fbd),
    TM_UL(0xa2d762cf), TM_UL(0x49c92f54), TM_UL(0x38b5f331), TM_UL(0x7128a454),
    TM_UL(0x48392905), TM_UL(0xa65b1db8), TM_UL(0x851c97bd), TM_UL(0xd675cf2f)
};

static const tt32Bit TM_CONST_QLF S7[] = {
    TM_UL(0x85e04019), TM_UL(0x332bf567), TM_UL(0x662dbfff), TM_UL(0xcfc65693),
    TM_UL(0x2a8d7f6f), TM_UL(0xab9bc912), TM_UL(0xde6008a1), TM_UL(0x2028da1f),
    TM_UL(0x0227bce7), TM_UL(0x4d642916), TM_UL(0x18fac300), TM_UL(0x50f18b82),
    TM_UL(0x2cb2cb11), TM_UL(0xb232e75c), TM_UL(0x4b3695f2), TM_UL(0xb28707de),
    TM_UL(0xa05fbcf6), TM_UL(0xcd4181e9), TM_UL(0xe150210c), TM_UL(0xe24ef1bd),
    TM_UL(0xb168c381), TM_UL(0xfde4e789), TM_UL(0x5c79b0d8), TM_UL(0x1e8bfd43),
    TM_UL(0x4d495001), TM_UL(0x38be4341), TM_UL(0x913cee1d), TM_UL(0x92a79c3f),
    TM_UL(0x089766be), TM_UL(0xbaeeadf4), TM_UL(0x1286becf), TM_UL(0xb6eacb19),
    TM_UL(0x2660c200), TM_UL(0x7565bde4), TM_UL(0x64241f7a), TM_UL(0x8248dca9),
    TM_UL(0xc3b3ad66), TM_UL(0x28136086), TM_UL(0x0bd8dfa8), TM_UL(0x356d1cf2),
    TM_UL(0x107789be), TM_UL(0xb3b2e9ce), TM_UL(0x0502aa8f), TM_UL(0x0bc0351e),
    TM_UL(0x166bf52a), TM_UL(0xeb12ff82), TM_UL(0xe3486911), TM_UL(0xd34d7516),
    TM_UL(0x4e7b3aff), TM_UL(0x5f43671b), TM_UL(0x9cf6e037), TM_UL(0x4981ac83),
    TM_UL(0x334266ce), TM_UL(0x8c9341b7), TM_UL(0xd0d854c0), TM_UL(0xcb3a6c88),
    TM_UL(0x47bc2829), TM_UL(0x4725ba37), TM_UL(0xa66ad22b), TM_UL(0x7ad61f1e),
    TM_UL(0x0c5cbafa), TM_UL(0x4437f107), TM_UL(0xb6e79962), TM_UL(0x42d2d816),
    TM_UL(0x0a961288), TM_UL(0xe1a5c06e), TM_UL(0x13749e67), TM_UL(0x72fc081a),
    TM_UL(0xb1d139f7), TM_UL(0xf9583745), TM_UL(0xcf19df58), TM_UL(0xbec3f756),
    TM_UL(0xc06eba30), TM_UL(0x07211b24), TM_UL(0x45c28829), TM_UL(0xc95e317f),
    TM_UL(0xbc8ec511), TM_UL(0x38bc46e9), TM_UL(0xc6e6fa14), TM_UL(0xbae8584a),
    TM_UL(0xad4ebc46), TM_UL(0x468f508b), TM_UL(0x7829435f), TM_UL(0xf124183b),
    TM_UL(0x821dba9f), TM_UL(0xaff60ff4), TM_UL(0xea2c4e6d), TM_UL(0x16e39264),
    TM_UL(0x92544a8b), TM_UL(0x009b4fc3), TM_UL(0xaba68ced), TM_UL(0x9ac96f78),
    TM_UL(0x06a5b79a), TM_UL(0xb2856e6e), TM_UL(0x1aec3ca9), TM_UL(0xbe838688),
    TM_UL(0x0e0804e9), TM_UL(0x55f1be56), TM_UL(0xe7e5363b), TM_UL(0xb3a1f25d),
    TM_UL(0xf7debb85), TM_UL(0x61fe033c), TM_UL(0x16746233), TM_UL(0x3c034c28),
    TM_UL(0xda6d0c74), TM_UL(0x79aac56c), TM_UL(0x3ce4e1ad), TM_UL(0x51f0c802),
    TM_UL(0x98f8f35a), TM_UL(0x1626a49f), TM_UL(0xeed82b29), TM_UL(0x1d382fe3),
    TM_UL(0x0c4fb99a), TM_UL(0xbb325778), TM_UL(0x3ec6d97b), TM_UL(0x6e77a6a9),
    TM_UL(0xcb658b5c), TM_UL(0xd45230c7), TM_UL(0x2bd1408b), TM_UL(0x60c03eb7),
    TM_UL(0xb9068d78), TM_UL(0xa33754f4), TM_UL(0xf430c87d), TM_UL(0xc8a71302),
    TM_UL(0xb96d8c32), TM_UL(0xebd4e7be), TM_UL(0xbe8b9d2d), TM_UL(0x7979fb06),
    TM_UL(0xe7225308), TM_UL(0x8b75cf77), TM_UL(0x11ef8da4), TM_UL(0xe083c858),
    TM_UL(0x8d6b786f), TM_UL(0x5a6317a6), TM_UL(0xfa5cf7a0), TM_UL(0x5dda0033),
    TM_UL(0xf28ebfb0), TM_UL(0xf5b9c310), TM_UL(0xa0eac280), TM_UL(0x08b9767a),
    TM_UL(0xa3d9d2b0), TM_UL(0x79d34217), TM_UL(0x021a718d), TM_UL(0x9ac6336a),
    TM_UL(0x2711fd60), TM_UL(0x438050e3), TM_UL(0x069908a8), TM_UL(0x3d7fedc4),
    TM_UL(0x826d2bef), TM_UL(0x4eeb8476), TM_UL(0x488dcf25), TM_UL(0x36c9d566),
    TM_UL(0x28e74e41), TM_UL(0xc2610aca), TM_UL(0x3d49a9cf), TM_UL(0xbae3b9df),
    TM_UL(0xb65f8de6), TM_UL(0x92aeaf64), TM_UL(0x3ac7d5e6), TM_UL(0x9ea80509),
    TM_UL(0xf22b017d), TM_UL(0xa4173f70), TM_UL(0xdd1e16c3), TM_UL(0x15e0d7f9),
    TM_UL(0x50b1b887), TM_UL(0x2b9f4fd5), TM_UL(0x625aba82), TM_UL(0x6a017962),
    TM_UL(0x2ec01b9c), TM_UL(0x15488aa9), TM_UL(0xd716e740), TM_UL(0x40055a2c),
    TM_UL(0x93d29a22), TM_UL(0xe32dbf9a), TM_UL(0x058745b9), TM_UL(0x3453dc1e),
    TM_UL(0xd699296e), TM_UL(0x496cff6f), TM_UL(0x1c9f4986), TM_UL(0xdfe2ed07),
    TM_UL(0xb87242d1), TM_UL(0x19de7eae), TM_UL(0x053e561a), TM_UL(0x15ad6f8c),
    TM_UL(0x66626c1c), TM_UL(0x7154c24c), TM_UL(0xea082b2a), TM_UL(0x93eb2939),
    TM_UL(0x17dcb0f0), TM_UL(0x58d4f2ae), TM_UL(0x9ea294fb), TM_UL(0x52cf564c),
    TM_UL(0x9883fe66), TM_UL(0x2ec40581), TM_UL(0x763953c3), TM_UL(0x01d6692e),
    TM_UL(0xd3a0c108), TM_UL(0xa1e7160e), TM_UL(0xe4f2dfa6), TM_UL(0x693ed285),
    TM_UL(0x74904698), TM_UL(0x4c2b0edd), TM_UL(0x4f757656), TM_UL(0x5d393378),
    TM_UL(0xa132234f), TM_UL(0x3d321c5d), TM_UL(0xc3f5e194), TM_UL(0x4b269301),
    TM_UL(0xc79f022f), TM_UL(0x3c997e7e), TM_UL(0x5e4f9504), TM_UL(0x3ffafbbd),
    TM_UL(0x76f7ad0e), TM_UL(0x296693f4), TM_UL(0x3d1fce6f), TM_UL(0xc61e45be),
    TM_UL(0xd3b5ab34), TM_UL(0xf72bf9b7), TM_UL(0x1b0434c0), TM_UL(0x4e72b567),
    TM_UL(0x5592a33d), TM_UL(0xb5229301), TM_UL(0xcfd2a87f), TM_UL(0x60aeb767),
    TM_UL(0x1814386b), TM_UL(0x30bcc33d), TM_UL(0x38a0c07d), TM_UL(0xfd1606f2),
    TM_UL(0xc363519b), TM_UL(0x589dd390), TM_UL(0x5479f8e6), TM_UL(0x1cb8d647),
    TM_UL(0x97fd61a9), TM_UL(0xea7759f4), TM_UL(0x2d57539d), TM_UL(0x569a58cf),
    TM_UL(0xe84e63ad), TM_UL(0x462e1b78), TM_UL(0x6580f87e), TM_UL(0xf3817914),
    TM_UL(0x91da55f4), TM_UL(0x40a230f3), TM_UL(0xd1988f35), TM_UL(0xb6e318d2),
    TM_UL(0x3ffa50bc), TM_UL(0x3d40f021), TM_UL(0xc3c0bdae), TM_UL(0x4958c24c),
    TM_UL(0x518f36b2), TM_UL(0x84b1d370), TM_UL(0x0fedce83), TM_UL(0x878ddada),
    TM_UL(0xf2a279c7), TM_UL(0x94e01be8), TM_UL(0x90716f4b), TM_UL(0x954b8aa3)
};

static const tt32Bit TM_CONST_QLF S8[] = {
    TM_UL(0xe216300d), TM_UL(0xbbddfffc), TM_UL(0xa7ebdabd), TM_UL(0x35648095),
    TM_UL(0x7789f8b7), TM_UL(0xe6c1121b), TM_UL(0x0e241600), TM_UL(0x052ce8b5),
    TM_UL(0x11a9cfb0), TM_UL(0xe5952f11), TM_UL(0xece7990a), TM_UL(0x9386d174),
    TM_UL(0x2a42931c), TM_UL(0x76e38111), TM_UL(0xb12def3a), TM_UL(0x37ddddfc),
    TM_UL(0xde9adeb1), TM_UL(0x0a0cc32c), TM_UL(0xbe197029), TM_UL(0x84a00940),
    TM_UL(0xbb243a0f), TM_UL(0xb4d137cf), TM_UL(0xb44e79f0), TM_UL(0x049eedfd),
    TM_UL(0x0b15a15d), TM_UL(0x480d3168), TM_UL(0x8bbbde5a), TM_UL(0x669ded42),
    TM_UL(0xc7ece831), TM_UL(0x3f8f95e7), TM_UL(0x72df191b), TM_UL(0x7580330d),
    TM_UL(0x94074251), TM_UL(0x5c7dcdfa), TM_UL(0xabbe6d63), TM_UL(0xaa402164),
    TM_UL(0xb301d40a), TM_UL(0x02e7d1ca), TM_UL(0x53571dae), TM_UL(0x7a3182a2),
    TM_UL(0x12a8ddec), TM_UL(0xfdaa335d), TM_UL(0x176f43e8), TM_UL(0x71fb46d4),
    TM_UL(0x38129022), TM_UL(0xce949ad4), TM_UL(0xb84769ad), TM_UL(0x965bd862),
    TM_UL(0x82f3d055), TM_UL(0x66fb9767), TM_UL(0x15b80b4e), TM_UL(0x1d5b47a0),
    TM_UL(0x4cfde06f), TM_UL(0xc28ec4b8), TM_UL(0x57e8726e), TM_UL(0x647a78fc),
    TM_UL(0x99865d44), TM_UL(0x608bd593), TM_UL(0x6c200e03), TM_UL(0x39dc5ff6),
    TM_UL(0x5d0b00a3), TM_UL(0xae63aff2), TM_UL(0x7e8bd632), TM_UL(0x70108c0c),
    TM_UL(0xbbd35049), TM_UL(0x2998df04), TM_UL(0x980cf42a), TM_UL(0x9b6df491),
    TM_UL(0x9e7edd53), TM_UL(0x06918548), TM_UL(0x58cb7e07), TM_UL(0x3b74ef2e),
    TM_UL(0x522fffb1), TM_UL(0xd24708cc), TM_UL(0x1c7e27cd), TM_UL(0xa4eb215b),
    TM_UL(0x3cf1d2e2), TM_UL(0x19b47a38), TM_UL(0x424f7618), TM_UL(0x35856039),
    TM_UL(0x9d17dee7), TM_UL(0x27eb35e6), TM_UL(0xc9aff67b), TM_UL(0x36baf5b8),
    TM_UL(0x09c467cd), TM_UL(0xc18910b1), TM_UL(0xe11dbf7b), TM_UL(0x06cd1af8),
    TM_UL(0x7170c608), TM_UL(0x2d5e3354), TM_UL(0xd4de495a), TM_UL(0x64c6d006),
    TM_UL(0xbcc0c62c), TM_UL(0x3dd00db3), TM_UL(0x708f8f34), TM_UL(0x77d51b42),
    TM_UL(0x264f620f), TM_UL(0x24b8d2bf), TM_UL(0x15c1b79e), TM_UL(0x46a52564),
    TM_UL(0xf8d7e54e), TM_UL(0x3e378160), TM_UL(0x7895cda5), TM_UL(0x859c15a5),
    TM_UL(0xe6459788), TM_UL(0xc37bc75f), TM_UL(0xdb07ba0c), TM_UL(0x0676a3ab),
    TM_UL(0x7f229b1e), TM_UL(0x31842e7b), TM_UL(0x24259fd7), TM_UL(0xf8bef472),
    TM_UL(0x835ffcb8), TM_UL(0x6df4c1f2), TM_UL(0x96f5b195), TM_UL(0xfd0af0fc),
    TM_UL(0xb0fe134c), TM_UL(0xe2506d3d), TM_UL(0x4f9b12ea), TM_UL(0xf215f225),
    TM_UL(0xa223736f), TM_UL(0x9fb4c428), TM_UL(0x25d04979), TM_UL(0x34c713f8),
    TM_UL(0xc4618187), TM_UL(0xea7a6e98), TM_UL(0x7cd16efc), TM_UL(0x1436876c),
    TM_UL(0xf1544107), TM_UL(0xbedeee14), TM_UL(0x56e9af27), TM_UL(0xa04aa441),
    TM_UL(0x3cf7c899), TM_UL(0x92ecbae6), TM_UL(0xdd67016d), TM_UL(0x151682eb),
    TM_UL(0xa842eedf), TM_UL(0xfdba60b4), TM_UL(0xf1907b75), TM_UL(0x20e3030f),
    TM_UL(0x24d8c29e), TM_UL(0xe139673b), TM_UL(0xefa63fb8), TM_UL(0x71873054),
    TM_UL(0xb6f2cf3b), TM_UL(0x9f326442), TM_UL(0xcb15a4cc), TM_UL(0xb01a4504),
    TM_UL(0xf1e47d8d), TM_UL(0x844a1be5), TM_UL(0xbae7dfdc), TM_UL(0x42cbda70),
    TM_UL(0xcd7dae0a), TM_UL(0x57e85b7a), TM_UL(0xd53f5af6), TM_UL(0x20cf4d8c),
    TM_UL(0xcea4d428), TM_UL(0x79d130a4), TM_UL(0x3486ebfb), TM_UL(0x33d3cddc),
    TM_UL(0x77853b53), TM_UL(0x37effcb5), TM_UL(0xc5068778), TM_UL(0xe580b3e6),
    TM_UL(0x4e68b8f4), TM_UL(0xc5c8b37e), TM_UL(0x0d809ea2), TM_UL(0x398feb7c),
    TM_UL(0x132a4f94), TM_UL(0x43b7950e), TM_UL(0x2fee7d1c), TM_UL(0x223613bd),
    TM_UL(0xdd06caa2), TM_UL(0x37df932b), TM_UL(0xc4248289), TM_UL(0xacf3ebc3),
    TM_UL(0x5715f6b7), TM_UL(0xef3478dd), TM_UL(0xf267616f), TM_UL(0xc148cbe4),
    TM_UL(0x9052815e), TM_UL(0x5e410fab), TM_UL(0xb48a2465), TM_UL(0x2eda7fa4),
    TM_UL(0xe87b40e4), TM_UL(0xe98ea084), TM_UL(0x5889e9e1), TM_UL(0xefd390fc),
    TM_UL(0xdd07d35b), TM_UL(0xdb485694), TM_UL(0x38d7e5b2), TM_UL(0x57720101),
    TM_UL(0x730edebc), TM_UL(0x5b643113), TM_UL(0x94917e4f), TM_UL(0x503c2fba),
    TM_UL(0x646f1282), TM_UL(0x7523d24a), TM_UL(0xe0779695), TM_UL(0xf9c17a8f),
    TM_UL(0x7a5b2121), TM_UL(0xd187b896), TM_UL(0x29263a4d), TM_UL(0xba510cdf),
    TM_UL(0x81f47c9f), TM_UL(0xad1163ed), TM_UL(0xea7b5965), TM_UL(0x1a00726e),
    TM_UL(0x11403092), TM_UL(0x00da6d77), TM_UL(0x4a0cdd61), TM_UL(0xad1f4603),
    TM_UL(0x605bdfb0), TM_UL(0x9eedc364), TM_UL(0x22ebe6a8), TM_UL(0xcee7d28a),
    TM_UL(0xa0e736a0), TM_UL(0x5564a6b9), TM_UL(0x10853209), TM_UL(0xc7eb8f37),
    TM_UL(0x2de705ca), TM_UL(0x8951570f), TM_UL(0xdf09822b), TM_UL(0xbd691a6c),
    TM_UL(0xaa12e4f2), TM_UL(0x87451c0f), TM_UL(0xe0f6a27a), TM_UL(0x3ada4819),
    TM_UL(0x4cf1764f), TM_UL(0x0d771c2b), TM_UL(0x67cdb156), TM_UL(0x350d8384),
    TM_UL(0x5938fa0f), TM_UL(0x42399ef3), TM_UL(0x36997b07), TM_UL(0x0e84093d),
    TM_UL(0x4aa93e61), TM_UL(0x8360d87b), TM_UL(0x1fa98b0c), TM_UL(0x1149382c),
    TM_UL(0xe97625a5), TM_UL(0x0614d1b7), TM_UL(0x0e25244b), TM_UL(0x0c768347),
    TM_UL(0x589e8d82), TM_UL(0x0d2059d1), TM_UL(0xa466bb1e), TM_UL(0xf8da0a82),
    TM_UL(0x04f19130), TM_UL(0xba6e4ec0), TM_UL(0x99265164), TM_UL(0x1ee7230d),
    TM_UL(0x50b2ad80), TM_UL(0xeaee6801), TM_UL(0x8db2a283), TM_UL(0xea8bf59e)
};



#define x0x1x2x3 buf[0]
#define x4x5x6x7 buf[1]
#define x8x9xAxB buf[2]
#define xCxDxExF buf[3]
#define z0z1z2z3 buf[4]
#define z4z5z6z7 buf[5]
#define z8z9zAzB buf[6]
#define zCzDzEzF buf[7]

#define tm_cast_byte0(x) (tm_8bit((x) >> 24))
#define tm_cast_byte1(x) (tm_8bit((x) >> 16))
#define tm_cast_byte2(x) (tm_8bit((x) >> 8 ))
#define tm_cast_byte3(x) (tm_8bit((x)))

#define x0 tm_cast_byte0(buf[0])
#define x1 tm_cast_byte1(buf[0])
#define x2 tm_cast_byte2(buf[0])
#define x3 tm_cast_byte3(buf[0])
#define x4 tm_cast_byte0(buf[1])
#define x5 tm_cast_byte1(buf[1])
#define x6 tm_cast_byte2(buf[1])
#define x7 tm_cast_byte3(buf[1])
#define x8 tm_cast_byte0(buf[2])
#define x9 tm_cast_byte1(buf[2])
#define xA tm_cast_byte2(buf[2])
#define xB tm_cast_byte3(buf[2])
#define xC tm_cast_byte0(buf[3])
#define xD tm_cast_byte1(buf[3])
#define xE tm_cast_byte2(buf[3])
#define xF tm_cast_byte3(buf[3])
#define z0 tm_cast_byte0(buf[4])
#define z1 tm_cast_byte1(buf[4])
#define z2 tm_cast_byte2(buf[4])
#define z3 tm_cast_byte3(buf[4])
#define z4 tm_cast_byte0(buf[5])
#define z5 tm_cast_byte1(buf[5])
#define z6 tm_cast_byte2(buf[5])
#define z7 tm_cast_byte3(buf[5])
#define z8 tm_cast_byte0(buf[6])
#define z9 tm_cast_byte1(buf[6])
#define zA tm_cast_byte2(buf[6])
#define zB tm_cast_byte3(buf[6])
#define zC tm_cast_byte0(buf[7])
#define zD tm_cast_byte1(buf[7])
#define zE tm_cast_byte2(buf[7])
#define zF tm_cast_byte3(buf[7])

#define tm_cast_circular_leftshift(x, y) \
( ((tt32Bit)(x) <<(unsigned) (y)) | ((tt32Bit)(x) >>(unsigned) (32-(y))) )


/*
 * Step 1
 */
/* WATCOM 16-bit x86 compiler cannot compile tfSetCast128Subkey
 * Soln: Split calculations into multiple sub-functions
 */
static void tfSetCast128Subkey_1(tt32BitPtr subkey, tt32BitPtr buf);
static void tfSetCast128Subkey_2(tt32BitPtr subkey, tt32BitPtr buf);

void tfSetCast128Subkey(tt32BitPtr subkey, tt8BitPtr key)
{
    tt32Bit buf[8]; /* for x0x1x2x3, x4x5x6x7 ..., z0z1z2z3, ... */

    buf[0] = (tt32Bit)
        (((tt32Bit) key[ 0] <<(unsigned) 24)
         | ((tt32Bit) key[ 1] <<(unsigned) 16) 
         | ((tt32Bit) key[ 2] <<(unsigned) 8)
         | (tt32Bit) key[ 3]);
    buf[1] = (tt32Bit)
        (((tt32Bit) key[ 4] <<(unsigned) 24)
         | ((tt32Bit) key[ 5] <<(unsigned) 16) 
         | ((tt32Bit) key[ 6] <<(unsigned) 8)
         | (tt32Bit) key[ 7]);
    buf[2] = (tt32Bit)
        (((tt32Bit) key[ 8] <<(unsigned) 24)
         | ((tt32Bit) key[ 9] <<(unsigned) 16) 
         | ((tt32Bit) key[10] <<(unsigned) 8)
         | (tt32Bit) key[11]);
    buf[3] = (tt32Bit)
        (((tt32Bit) key[12] <<(unsigned) 24)
         | ((tt32Bit) key[13] <<(unsigned) 16) 
         | ((tt32Bit) key[14] <<(unsigned) 8)
         | (tt32Bit) key[15]);

    tfSetCast128Subkey_1(subkey, buf);
    tfSetCast128Subkey_2(subkey, buf);
}

static void tfSetCast128Subkey_1(tt32BitPtr subkey, tt32BitPtr buf)
{
    /* masking subkey */
    z0z1z2z3 = x0x1x2x3 ^ S5[xD] ^ S6[xF] ^ S7[xC] ^ S8[xE] ^ S7[x8];
    z4z5z6z7 = x8x9xAxB ^ S5[z0] ^ S6[z2] ^ S7[z1] ^ S8[z3] ^ S8[xA];
    z8z9zAzB = xCxDxExF ^ S5[z7] ^ S6[z6] ^ S7[z5] ^ S8[z4] ^ S5[x9];
    zCzDzEzF = x4x5x6x7 ^ S5[zA] ^ S6[z9] ^ S7[zB] ^ S8[z8] ^ S6[xB];
    subkey[0]  = S5[z8] ^ S6[z9] ^ S7[z7] ^ S8[z6] ^ S5[z2];
    subkey[1]  = S5[zA] ^ S6[zB] ^ S7[z5] ^ S8[z4] ^ S6[z6];
    subkey[2]  = S5[zC] ^ S6[zD] ^ S7[z3] ^ S8[z2] ^ S7[z9];
    subkey[3]  = S5[zE] ^ S6[zF] ^ S7[z1] ^ S8[z0] ^ S8[zC];

    x0x1x2x3 = z8z9zAzB ^ S5[z5] ^ S6[z7] ^ S7[z4] ^ S8[z6] ^ S7[z0];
    x4x5x6x7 = z0z1z2z3 ^ S5[x0] ^ S6[x2] ^ S7[x1] ^ S8[x3] ^ S8[z2];
    x8x9xAxB = z4z5z6z7 ^ S5[x7] ^ S6[x6] ^ S7[x5] ^ S8[x4] ^ S5[z1];
    xCxDxExF = zCzDzEzF ^ S5[xA] ^ S6[x9] ^ S7[xB] ^ S8[x8] ^ S6[z3];
    subkey[4]  = S5[x3] ^ S6[x2] ^ S7[xC] ^ S8[xD] ^ S5[x8];
    subkey[5]  = S5[x1] ^ S6[x0] ^ S7[xE] ^ S8[xF] ^ S6[xD];
    subkey[6]  = S5[x7] ^ S6[x6] ^ S7[x8] ^ S8[x9] ^ S7[x3];
    subkey[7]  = S5[x5] ^ S6[x4] ^ S7[xA] ^ S8[xB] ^ S8[x7];

    z0z1z2z3 = x0x1x2x3 ^ S5[xD] ^ S6[xF] ^ S7[xC] ^ S8[xE] ^ S7[x8];
    z4z5z6z7 = x8x9xAxB ^ S5[z0] ^ S6[z2] ^ S7[z1] ^ S8[z3] ^ S8[xA];
    z8z9zAzB = xCxDxExF ^ S5[z7] ^ S6[z6] ^ S7[z5] ^ S8[z4] ^ S5[x9];
    zCzDzEzF = x4x5x6x7 ^ S5[zA] ^ S6[z9] ^ S7[zB] ^ S8[z8] ^ S6[xB];
    subkey[8]  = S5[z3] ^ S6[z2] ^ S7[zC] ^ S8[zD] ^ S5[z9];
    subkey[9]  = S5[z1] ^ S6[z0] ^ S7[zE] ^ S8[zF] ^ S6[zC];
    subkey[10] = S5[z7] ^ S6[z6] ^ S7[z8] ^ S8[z9] ^ S7[z2];
    subkey[11] = S5[z5] ^ S6[z4] ^ S7[zA] ^ S8[zB] ^ S8[z6];

    x0x1x2x3 = z8z9zAzB ^ S5[z5] ^ S6[z7] ^ S7[z4] ^ S8[z6] ^ S7[z0];
    x4x5x6x7 = z0z1z2z3 ^ S5[x0] ^ S6[x2] ^ S7[x1] ^ S8[x3] ^ S8[z2];
    x8x9xAxB = z4z5z6z7 ^ S5[x7] ^ S6[x6] ^ S7[x5] ^ S8[x4] ^ S5[z1];
    xCxDxExF = zCzDzEzF ^ S5[xA] ^ S6[x9] ^ S7[xB] ^ S8[x8] ^ S6[z3];
    subkey[12] = S5[x8] ^ S6[x9] ^ S7[x7] ^ S8[x6] ^ S5[x3];
    subkey[13] = S5[xA] ^ S6[xB] ^ S7[x5] ^ S8[x4] ^ S6[x7];
    subkey[14] = S5[xC] ^ S6[xD] ^ S7[x3] ^ S8[x2] ^ S7[x8];
    subkey[15] = S5[xE] ^ S6[xF] ^ S7[x1] ^ S8[x0] ^ S8[xD];
}

static void tfSetCast128Subkey_2(tt32BitPtr subkey, tt32BitPtr buf)
{
    /* rotate subkey (least significast 5 bits) */
    z0z1z2z3 = x0x1x2x3 ^ S5[xD] ^ S6[xF] ^ S7[xC] ^ S8[xE] ^ S7[x8];
    z4z5z6z7 = x8x9xAxB ^ S5[z0] ^ S6[z2] ^ S7[z1] ^ S8[z3] ^ S8[xA];
    z8z9zAzB = xCxDxExF ^ S5[z7] ^ S6[z6] ^ S7[z5] ^ S8[z4] ^ S5[x9];
    zCzDzEzF = x4x5x6x7 ^ S5[zA] ^ S6[z9] ^ S7[zB] ^ S8[z8] ^ S6[xB];
    subkey[16] = (S5[z8] ^ S6[z9] ^ S7[z7] ^ S8[z6] ^ S5[z2]) & 0x1f;
    subkey[17] = (S5[zA] ^ S6[zB] ^ S7[z5] ^ S8[z4] ^ S6[z6]) & 0x1f;
    subkey[18] = (S5[zC] ^ S6[zD] ^ S7[z3] ^ S8[z2] ^ S7[z9]) & 0x1f;
    subkey[19] = (S5[zE] ^ S6[zF] ^ S7[z1] ^ S8[z0] ^ S8[zC]) & 0x1f;

    x0x1x2x3 = z8z9zAzB ^ S5[z5] ^ S6[z7] ^ S7[z4] ^ S8[z6] ^ S7[z0];
    x4x5x6x7 = z0z1z2z3 ^ S5[x0] ^ S6[x2] ^ S7[x1] ^ S8[x3] ^ S8[z2];
    x8x9xAxB = z4z5z6z7 ^ S5[x7] ^ S6[x6] ^ S7[x5] ^ S8[x4] ^ S5[z1];
    xCxDxExF = zCzDzEzF ^ S5[xA] ^ S6[x9] ^ S7[xB] ^ S8[x8] ^ S6[z3];
    subkey[20] = (S5[x3] ^ S6[x2] ^ S7[xC] ^ S8[xD] ^ S5[x8]) & 0x1f;
    subkey[21] = (S5[x1] ^ S6[x0] ^ S7[xE] ^ S8[xF] ^ S6[xD]) & 0x1f;
    subkey[22] = (S5[x7] ^ S6[x6] ^ S7[x8] ^ S8[x9] ^ S7[x3]) & 0x1f;
    subkey[23] = (S5[x5] ^ S6[x4] ^ S7[xA] ^ S8[xB] ^ S8[x7]) & 0x1f;

    z0z1z2z3 = x0x1x2x3 ^ S5[xD] ^ S6[xF] ^ S7[xC] ^ S8[xE] ^ S7[x8];
    z4z5z6z7 = x8x9xAxB ^ S5[z0] ^ S6[z2] ^ S7[z1] ^ S8[z3] ^ S8[xA];
    z8z9zAzB = xCxDxExF ^ S5[z7] ^ S6[z6] ^ S7[z5] ^ S8[z4] ^ S5[x9];
    zCzDzEzF = x4x5x6x7 ^ S5[zA] ^ S6[z9] ^ S7[zB] ^ S8[z8] ^ S6[xB];
    subkey[24] = (S5[z3] ^ S6[z2] ^ S7[zC] ^ S8[zD] ^ S5[z9]) & 0x1f;
    subkey[25] = (S5[z1] ^ S6[z0] ^ S7[zE] ^ S8[zF] ^ S6[zC]) & 0x1f;
    subkey[26] = (S5[z7] ^ S6[z6] ^ S7[z8] ^ S8[z9] ^ S7[z2]) & 0x1f;
    subkey[27] = (S5[z5] ^ S6[z4] ^ S7[zA] ^ S8[zB] ^ S8[z6]) & 0x1f;

    x0x1x2x3 = z8z9zAzB ^ S5[z5] ^ S6[z7] ^ S7[z4] ^ S8[z6] ^ S7[z0];
    x4x5x6x7 = z0z1z2z3 ^ S5[x0] ^ S6[x2] ^ S7[x1] ^ S8[x3] ^ S8[z2];
    x8x9xAxB = z4z5z6z7 ^ S5[x7] ^ S6[x6] ^ S7[x5] ^ S8[x4] ^ S5[z1];
    xCxDxExF = zCzDzEzF ^ S5[xA] ^ S6[x9] ^ S7[xB] ^ S8[x8] ^ S6[z3];
    subkey[28] = (S5[x8] ^ S6[x9] ^ S7[x7] ^ S8[x6] ^ S5[x3]) & 0x1f;
    subkey[29] = (S5[xA] ^ S6[xB] ^ S7[x5] ^ S8[x4] ^ S6[x7]) & 0x1f;
    subkey[30] = (S5[xC] ^ S6[xD] ^ S7[x3] ^ S8[x2] ^ S7[x8]) & 0x1f;
    subkey[31] = (S5[xE] ^ S6[xF] ^ S7[x1] ^ S8[x0] ^ S8[xD]) & 0x1f;
}


#define    tm_cast128_type1(rc, d, km, kr) { \
    tt32Bit x = tm_cast_circular_leftshift(((km)+(d)), (kr)); \
    (rc) = ((S1[tm_cast_byte0(x)] ^ S2[tm_cast_byte1(x)]) - \
    S3[tm_cast_byte2(x)]) + S4[tm_cast_byte3(x)]; \
}

#define    tm_cast128_type2(rc, d, km, kr) { \
    tt32Bit x = tm_cast_circular_leftshift(((km)^(d)), (kr)); \
    (rc) = ((S1[tm_cast_byte0(x)] - S2[tm_cast_byte1(x)]) + \
    S3[tm_cast_byte2(x)]) ^ S4[tm_cast_byte3(x)]; \
}

#define    tm_cast128_type3(rc, d, km, kr) { \
    tt32Bit x = tm_cast_circular_leftshift(((km)-(d)), (kr)); \
    (rc) = ((S1[tm_cast_byte0(x)] + S2[tm_cast_byte1(x)]) ^ \
    S3[tm_cast_byte2(x)]) - S4[tm_cast_byte3(x)]; \
}


void tfCast128EncryptRound16(tt8BitPtr c, ttConst8BitPtr m, tt32BitPtr subkey)
{
    tt32Bit l;    /* left 32bit */
    tt32Bit r;    /* right 32bit */
    tt32Bit br;    /* backup right 32bit */
    tt32Bit rc;    /* result code of CAST128_TYPE?() */
    tt32BitPtr km, kr;

    /* Step 2 */
    l = (tt32Bit)
        (((tt32Bit) m[0] <<(unsigned) 24)
         | ((tt32Bit) m[1] <<(unsigned) 16) 
         | ((tt32Bit) m[2] <<(unsigned) 8)
         | (tt32Bit) m[3]);
    r = (tt32Bit)
        (((tt32Bit) m[4] <<(unsigned) 24)
         | ((tt32Bit) m[5] <<(unsigned) 16) 
         | ((tt32Bit) m[6] <<(unsigned) 8)
         | (tt32Bit) m[7]);

    /* Step 3 */
    km = subkey;
    kr = subkey + 16;

    br = r; tm_cast128_type1(rc, r, *km, *kr); r = l ^ rc; l = br; km++; kr++;
    br = r; tm_cast128_type2(rc, r, *km, *kr); r = l ^ rc; l = br; km++; kr++;
    br = r; tm_cast128_type3(rc, r, *km, *kr); r = l ^ rc; l = br; km++; kr++;
    br = r; tm_cast128_type1(rc, r, *km, *kr); r = l ^ rc; l = br; km++; kr++;
    br = r; tm_cast128_type2(rc, r, *km, *kr); r = l ^ rc; l = br; km++; kr++;
    br = r; tm_cast128_type3(rc, r, *km, *kr); r = l ^ rc; l = br; km++; kr++;
    br = r; tm_cast128_type1(rc, r, *km, *kr); r = l ^ rc; l = br; km++; kr++;
    br = r; tm_cast128_type2(rc, r, *km, *kr); r = l ^ rc; l = br; km++; kr++;
    br = r; tm_cast128_type3(rc, r, *km, *kr); r = l ^ rc; l = br; km++; kr++;
    br = r; tm_cast128_type1(rc, r, *km, *kr); r = l ^ rc; l = br; km++; kr++;
    br = r; tm_cast128_type2(rc, r, *km, *kr); r = l ^ rc; l = br; km++; kr++;
    br = r; tm_cast128_type3(rc, r, *km, *kr); r = l ^ rc; l = br; km++; kr++;
    br = r; tm_cast128_type1(rc, r, *km, *kr); r = l ^ rc; l = br; km++; kr++;
    br = r; tm_cast128_type2(rc, r, *km, *kr); r = l ^ rc; l = br; km++; kr++;
    br = r; tm_cast128_type3(rc, r, *km, *kr); r = l ^ rc; l = br; km++; kr++;
    br = r; tm_cast128_type1(rc, r, *km, *kr); r = l ^ rc; l = br;

    /* Step 4 */
    c[0] = tm_8bit((tt32Bit)r >> 24);
    c[1] = tm_8bit((tt32Bit)r >> 16);
    c[2] = tm_8bit((tt32Bit)r >> 8);
    c[3] = tm_8bit(r);
    c[4] = tm_8bit((tt32Bit)l >> 24);
    c[5] = tm_8bit((tt32Bit)l >> 16);
    c[6] = tm_8bit((tt32Bit)l >>  8);
    c[7] = tm_8bit(l);
}


void tfCast128DecryptRound16(tt8BitPtr m, ttConst8BitPtr c, tt32BitPtr subkey)
{
    tt32Bit l;    /* left 32bit */
    tt32Bit r;    /* right 32bit */
    tt32Bit bl;    /* backup left 32bit */
    tt32Bit rc;    /* result code of CAST128_TYPE?() */
    tt32BitPtr km, kr;

    /* Step 2 */
    r = (tt32Bit)
        (((tt32Bit) c[0] <<(unsigned) 24)
         | ((tt32Bit) c[1] <<(unsigned) 16) 
         | ((tt32Bit) c[2] <<(unsigned) 8)
         | (tt32Bit) c[3]);
    l = (tt32Bit)
        (((tt32Bit) c[4] <<(unsigned) 24)
         | ((tt32Bit) c[5] <<(unsigned) 16) 
         | ((tt32Bit) c[6] <<(unsigned) 8)
         | (tt32Bit) c[7]);

    /* Step 3 */
    km = subkey + 15;
    kr = subkey + 31;

    bl = l; tm_cast128_type1(rc, l, *km, *kr); l = r ^ rc; r = bl; km--; kr--;
    bl = l; tm_cast128_type3(rc, l, *km, *kr); l = r ^ rc; r = bl; km--; kr--;
    bl = l; tm_cast128_type2(rc, l, *km, *kr); l = r ^ rc; r = bl; km--; kr--;
    bl = l; tm_cast128_type1(rc, l, *km, *kr); l = r ^ rc; r = bl; km--; kr--;
    bl = l; tm_cast128_type3(rc, l, *km, *kr); l = r ^ rc; r = bl; km--; kr--;
    bl = l; tm_cast128_type2(rc, l, *km, *kr); l = r ^ rc; r = bl; km--; kr--;
    bl = l; tm_cast128_type1(rc, l, *km, *kr); l = r ^ rc; r = bl; km--; kr--;
    bl = l; tm_cast128_type3(rc, l, *km, *kr); l = r ^ rc; r = bl; km--; kr--;
    bl = l; tm_cast128_type2(rc, l, *km, *kr); l = r ^ rc; r = bl; km--; kr--;
    bl = l; tm_cast128_type1(rc, l, *km, *kr); l = r ^ rc; r = bl; km--; kr--;
    bl = l; tm_cast128_type3(rc, l, *km, *kr); l = r ^ rc; r = bl; km--; kr--;
    bl = l; tm_cast128_type2(rc, l, *km, *kr); l = r ^ rc; r = bl; km--; kr--;
    bl = l; tm_cast128_type1(rc, l, *km, *kr); l = r ^ rc; r = bl; km--; kr--;
    bl = l; tm_cast128_type3(rc, l, *km, *kr); l = r ^ rc; r = bl; km--; kr--;
    bl = l; tm_cast128_type2(rc, l, *km, *kr); l = r ^ rc; r = bl; km--; kr--;
    bl = l; tm_cast128_type1(rc, l, *km, *kr); l = r ^ rc; r = bl;

    /* Step 4 */
    m[0] = tm_8bit((tt32Bit)l >> 24);
    m[1] = tm_8bit((tt32Bit)l >> 16);
    m[2] = tm_8bit((tt32Bit)l >>  8);
    m[3] = tm_8bit(l);
    m[4] = tm_8bit((tt32Bit)r >> 24);
    m[5] = tm_8bit((tt32Bit)r >> 16);
    m[6] = tm_8bit((tt32Bit)r >>  8);
    m[7] = tm_8bit(r);
}


void tfCast128EncryptRound12(tt8BitPtr c, ttConst8BitPtr m, tt32BitPtr subkey)
{
    tt32Bit l;    /* left 32bit */
    tt32Bit r;    /* right 32bit */
    tt32Bit br;    /* backup right 32bit */
    tt32Bit rc;    /* result code of CAST128_TYPE?() */
    tt32BitPtr km, kr;

    /* Step 2 */
    l = (tt32Bit)
        (((tt32Bit) m[0] <<(unsigned) 24)
         | ((tt32Bit) m[1] <<(unsigned) 16) 
         | ((tt32Bit) m[2] <<(unsigned) 8)
         | (tt32Bit) m[3]);
    r = (tt32Bit)
        (((tt32Bit) m[4] <<(unsigned) 24)
         | ((tt32Bit) m[5] <<(unsigned) 16) 
         | ((tt32Bit) m[6] <<(unsigned) 8)
         | (tt32Bit) m[7]);

    /* Step 3 */
    km = subkey;
    kr = subkey + 16;

    br = r; tm_cast128_type1(rc, r, *km, *kr); r = l ^ rc; l = br; km++; kr++;
    br = r; tm_cast128_type2(rc, r, *km, *kr); r = l ^ rc; l = br; km++; kr++;
    br = r; tm_cast128_type3(rc, r, *km, *kr); r = l ^ rc; l = br; km++; kr++;
    br = r; tm_cast128_type1(rc, r, *km, *kr); r = l ^ rc; l = br; km++; kr++;
    br = r; tm_cast128_type2(rc, r, *km, *kr); r = l ^ rc; l = br; km++; kr++;
    br = r; tm_cast128_type3(rc, r, *km, *kr); r = l ^ rc; l = br; km++; kr++;
    br = r; tm_cast128_type1(rc, r, *km, *kr); r = l ^ rc; l = br; km++; kr++;
    br = r; tm_cast128_type2(rc, r, *km, *kr); r = l ^ rc; l = br; km++; kr++;
    br = r; tm_cast128_type3(rc, r, *km, *kr); r = l ^ rc; l = br; km++; kr++;
    br = r; tm_cast128_type1(rc, r, *km, *kr); r = l ^ rc; l = br; km++; kr++;
    br = r; tm_cast128_type2(rc, r, *km, *kr); r = l ^ rc; l = br; km++; kr++;
    br = r; tm_cast128_type3(rc, r, *km, *kr); r = l ^ rc; l = br;

    /* Step 4 */
    c[0] =tm_8bit((tt32Bit)r >> 24);
    c[1] =tm_8bit((tt32Bit)r >> 16);
    c[2] =tm_8bit((tt32Bit)r >>  8);
    c[3] =tm_8bit(r);
    c[4] =tm_8bit((tt32Bit)l >> 24);
    c[5] =tm_8bit((tt32Bit)l >> 16);
    c[6] =tm_8bit((tt32Bit)l >>  8);
    c[7] =tm_8bit(l);
}


void tfCast128DecryptRound12(tt8BitPtr m, ttConst8BitPtr c, tt32BitPtr subkey)
{
    tt32Bit l;    /* left 32bit */
    tt32Bit r;    /* right 32bit */
    tt32Bit bl;    /* backup left 32bit */
    tt32Bit rc;    /* result code of CAST128_TYPE?() */
    tt32BitPtr km, kr;

    /* Step 2 */
    r = (tt32Bit)
        (((tt32Bit) c[0] <<(unsigned) 24)
         | ((tt32Bit) c[1] <<(unsigned) 16) 
         | ((tt32Bit) c[2] <<(unsigned) 8)
         | (tt32Bit) c[3]);
    l = (tt32Bit)
        (((tt32Bit) c[4] <<(unsigned) 24)
         | ((tt32Bit) c[5] <<(unsigned) 16) 
         | ((tt32Bit) c[6] <<(unsigned) 8)
         | (tt32Bit) c[7]);

    /* Step 3 */
    km = subkey + 11;
    kr = subkey + 27;

    bl = l; tm_cast128_type3(rc, l, *km, *kr); l = r ^ rc; r = bl; km--; kr--;
    bl = l; tm_cast128_type2(rc, l, *km, *kr); l = r ^ rc; r = bl; km--; kr--;
    bl = l; tm_cast128_type1(rc, l, *km, *kr); l = r ^ rc; r = bl; km--; kr--;
    bl = l; tm_cast128_type3(rc, l, *km, *kr); l = r ^ rc; r = bl; km--; kr--;
    bl = l; tm_cast128_type2(rc, l, *km, *kr); l = r ^ rc; r = bl; km--; kr--;
    bl = l; tm_cast128_type1(rc, l, *km, *kr); l = r ^ rc; r = bl; km--; kr--;
    bl = l; tm_cast128_type3(rc, l, *km, *kr); l = r ^ rc; r = bl; km--; kr--;
    bl = l; tm_cast128_type2(rc, l, *km, *kr); l = r ^ rc; r = bl; km--; kr--;
    bl = l; tm_cast128_type1(rc, l, *km, *kr); l = r ^ rc; r = bl; km--; kr--;
    bl = l; tm_cast128_type3(rc, l, *km, *kr); l = r ^ rc; r = bl; km--; kr--;
    bl = l; tm_cast128_type2(rc, l, *km, *kr); l = r ^ rc; r = bl; km--; kr--;
    bl = l; tm_cast128_type1(rc, l, *km, *kr); l = r ^ rc; r = bl;

    /* Step 4 */
    m[0] =tm_8bit((tt32Bit)l >> 24);
    m[1] =tm_8bit((tt32Bit)l >> 16);
    m[2] =tm_8bit((tt32Bit)l >>  8);
    m[3] =tm_8bit(l);
    m[4] =tm_8bit((tt32Bit)r >> 24);
    m[5] =tm_8bit((tt32Bit)r >> 16);
    m[6] =tm_8bit((tt32Bit)r >>  8);
    m[7] =tm_8bit(r);
}

/*-----------------------  esp-cast128 ----------------------- */


int tfEspCast128SchedLen(void)
{

    return sizeof(tt32Bit) * 32;
}

int tfEspCast128Schedule(
            ttEspAlgorithmPtr       algoPtr,
            ttGenericKeyPtr         gkeyPtr)
{

    tt8Bit key128[16];
    int keyLen;

    TM_UNREF_IN_ARG(algoPtr);
    keyLen = (int)(gkeyPtr->keyBits >>3);
    tm_bcopy(gkeyPtr->keyDataPtr, key128, keyLen);
/* if the key size is less than 128 bits, must be padded with zero*/
    if(keyLen < 16)
    {
        tm_bzero(key128+keyLen, 16 - keyLen);
    }
    tfSetCast128Subkey((tt32BitPtr)gkeyPtr->keySchedulePtr, key128);
    return 0;
}


int tfEspCast128BlockDecrypt( ttEspAlgorithmPtr algoPtr,
                                     ttGenericKeyPtr   gkeyPtr,
                                     tt8BitPtr         s,
                                     tt8BitPtr         d)
{

    TM_UNREF_IN_ARG(algoPtr);
    if (gkeyPtr->keyBits <= 80)
    {
        tfCast128DecryptRound12(d, s, (tt32BitPtr)gkeyPtr->keySchedulePtr);
    }
    else
    {
        tfCast128DecryptRound16(d, s, (tt32BitPtr)gkeyPtr->keySchedulePtr);
    }
    return TM_ENOERROR;
}

int tfEspCast128BlockEncrypt(ttEspAlgorithmPtr algoPtr,
                                    ttGenericKeyPtr   gkeyPtr,
                                    tt8BitPtr         s,
                                    tt8BitPtr         d)
{

    TM_UNREF_IN_ARG(algoPtr);
    if (gkeyPtr->keyBits <= 80)
    {
        tfCast128EncryptRound12(d, s, (tt32BitPtr)gkeyPtr->keySchedulePtr);
    }
    else
    {
        tfCast128EncryptRound16(d, s, (tt32BitPtr)gkeyPtr->keySchedulePtr);
    }
    return TM_ENOERROR;
}

#else /* ! TM_USE_CAST128 */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */

/* To allow link for builds when TM_USE_CAST128 is not defined */
int tvCastDummy = 0;

#endif /* TM_USE_CAST128 */
