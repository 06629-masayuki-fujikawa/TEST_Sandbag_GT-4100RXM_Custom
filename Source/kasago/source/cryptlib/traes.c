/*
 * Description: Testing the ESP algorithem: AES (RJINDAEL)
 *
 * Filename: traes.c
 * Author: Jin Zhang
 * Date Created: 08/01/2001
 * $Source: source/cryptlib/traes.c $
 *
 * Modification History
 * $Revision: 6.0.2.3 $
 * $Date: 2011/11/21 20:15:16GMT $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#include <trsocket.h>
#include <trmacro.h>
#include <trtype.h>

#ifdef TM_USE_AESXCBC
#include <trproto.h> 
#include <trglobal.h>
#endif /* TM_USE_AESXCBC */

#ifdef TM_USE_AES
#include <trcrylib.h>

# define tm_aes_getu32(pt) (((tt32Bit)(pt)[0] << 24) ^ \
                            ((tt32Bit)(pt)[1] << 16) ^ \
                            ((tt32Bit)(pt)[2] <<  8) ^ \
                            ((tt32Bit)(pt)[3]))
# define tm_aes_putu32(ct, st)   { (ct)[0] = tm_8bit((st) >> 24); \
                                   (ct)[1] = tm_8bit((st) >> 16); \
                                   (ct)[2] = tm_8bit((st) >>  8); \
                                   (ct)[3] = tm_8bit(st); }



static const tt32Bit TM_CONST_QLF tlAesRcon[] = 
{
    TM_UL(0x01000000), TM_UL(0x02000000), TM_UL(0x04000000), TM_UL(0x08000000),
    TM_UL(0x10000000), TM_UL(0x20000000), TM_UL(0x40000000), TM_UL(0x80000000),
    TM_UL(0x1B000000), TM_UL(0x36000000)
/* for 128-bit blocks, Rijndael never uses more than 10 tlAesRcon values */
};


static const tt32Bit TM_CONST_QLF Te0[256] = {
    TM_UL(0xc66363a5), TM_UL(0xf87c7c84), TM_UL(0xee777799), TM_UL(0xf67b7b8d),
    TM_UL(0xfff2f20d), TM_UL(0xd66b6bbd), TM_UL(0xde6f6fb1), TM_UL(0x91c5c554),
    TM_UL(0x60303050), TM_UL(0x02010103), TM_UL(0xce6767a9), TM_UL(0x562b2b7d),
    TM_UL(0xe7fefe19), TM_UL(0xb5d7d762), TM_UL(0x4dababe6), TM_UL(0xec76769a),
    TM_UL(0x8fcaca45), TM_UL(0x1f82829d), TM_UL(0x89c9c940), TM_UL(0xfa7d7d87),
    TM_UL(0xeffafa15), TM_UL(0xb25959eb), TM_UL(0x8e4747c9), TM_UL(0xfbf0f00b),
    TM_UL(0x41adadec), TM_UL(0xb3d4d467), TM_UL(0x5fa2a2fd), TM_UL(0x45afafea),
    TM_UL(0x239c9cbf), TM_UL(0x53a4a4f7), TM_UL(0xe4727296), TM_UL(0x9bc0c05b),
    TM_UL(0x75b7b7c2), TM_UL(0xe1fdfd1c), TM_UL(0x3d9393ae), TM_UL(0x4c26266a),
    TM_UL(0x6c36365a), TM_UL(0x7e3f3f41), TM_UL(0xf5f7f702), TM_UL(0x83cccc4f),
    TM_UL(0x6834345c), TM_UL(0x51a5a5f4), TM_UL(0xd1e5e534), TM_UL(0xf9f1f108),
    TM_UL(0xe2717193), TM_UL(0xabd8d873), TM_UL(0x62313153), TM_UL(0x2a15153f),
    TM_UL(0x0804040c), TM_UL(0x95c7c752), TM_UL(0x46232365), TM_UL(0x9dc3c35e),
    TM_UL(0x30181828), TM_UL(0x379696a1), TM_UL(0x0a05050f), TM_UL(0x2f9a9ab5),
    TM_UL(0x0e070709), TM_UL(0x24121236), TM_UL(0x1b80809b), TM_UL(0xdfe2e23d),
    TM_UL(0xcdebeb26), TM_UL(0x4e272769), TM_UL(0x7fb2b2cd), TM_UL(0xea75759f),
    TM_UL(0x1209091b), TM_UL(0x1d83839e), TM_UL(0x582c2c74), TM_UL(0x341a1a2e),
    TM_UL(0x361b1b2d), TM_UL(0xdc6e6eb2), TM_UL(0xb45a5aee), TM_UL(0x5ba0a0fb),
    TM_UL(0xa45252f6), TM_UL(0x763b3b4d), TM_UL(0xb7d6d661), TM_UL(0x7db3b3ce),
    TM_UL(0x5229297b), TM_UL(0xdde3e33e), TM_UL(0x5e2f2f71), TM_UL(0x13848497),
    TM_UL(0xa65353f5), TM_UL(0xb9d1d168), TM_UL(0x00000000), TM_UL(0xc1eded2c),
    TM_UL(0x40202060), TM_UL(0xe3fcfc1f), TM_UL(0x79b1b1c8), TM_UL(0xb65b5bed),
    TM_UL(0xd46a6abe), TM_UL(0x8dcbcb46), TM_UL(0x67bebed9), TM_UL(0x7239394b),
    TM_UL(0x944a4ade), TM_UL(0x984c4cd4), TM_UL(0xb05858e8), TM_UL(0x85cfcf4a),
    TM_UL(0xbbd0d06b), TM_UL(0xc5efef2a), TM_UL(0x4faaaae5), TM_UL(0xedfbfb16),
    TM_UL(0x864343c5), TM_UL(0x9a4d4dd7), TM_UL(0x66333355), TM_UL(0x11858594),
    TM_UL(0x8a4545cf), TM_UL(0xe9f9f910), TM_UL(0x04020206), TM_UL(0xfe7f7f81),
    TM_UL(0xa05050f0), TM_UL(0x783c3c44), TM_UL(0x259f9fba), TM_UL(0x4ba8a8e3),
    TM_UL(0xa25151f3), TM_UL(0x5da3a3fe), TM_UL(0x804040c0), TM_UL(0x058f8f8a),
    TM_UL(0x3f9292ad), TM_UL(0x219d9dbc), TM_UL(0x70383848), TM_UL(0xf1f5f504),
    TM_UL(0x63bcbcdf), TM_UL(0x77b6b6c1), TM_UL(0xafdada75), TM_UL(0x42212163),
    TM_UL(0x20101030), TM_UL(0xe5ffff1a), TM_UL(0xfdf3f30e), TM_UL(0xbfd2d26d),
    TM_UL(0x81cdcd4c), TM_UL(0x180c0c14), TM_UL(0x26131335), TM_UL(0xc3ecec2f),
    TM_UL(0xbe5f5fe1), TM_UL(0x359797a2), TM_UL(0x884444cc), TM_UL(0x2e171739),
    TM_UL(0x93c4c457), TM_UL(0x55a7a7f2), TM_UL(0xfc7e7e82), TM_UL(0x7a3d3d47),
    TM_UL(0xc86464ac), TM_UL(0xba5d5de7), TM_UL(0x3219192b), TM_UL(0xe6737395),
    TM_UL(0xc06060a0), TM_UL(0x19818198), TM_UL(0x9e4f4fd1), TM_UL(0xa3dcdc7f),
    TM_UL(0x44222266), TM_UL(0x542a2a7e), TM_UL(0x3b9090ab), TM_UL(0x0b888883),
    TM_UL(0x8c4646ca), TM_UL(0xc7eeee29), TM_UL(0x6bb8b8d3), TM_UL(0x2814143c),
    TM_UL(0xa7dede79), TM_UL(0xbc5e5ee2), TM_UL(0x160b0b1d), TM_UL(0xaddbdb76),
    TM_UL(0xdbe0e03b), TM_UL(0x64323256), TM_UL(0x743a3a4e), TM_UL(0x140a0a1e),
    TM_UL(0x924949db), TM_UL(0x0c06060a), TM_UL(0x4824246c), TM_UL(0xb85c5ce4),
    TM_UL(0x9fc2c25d), TM_UL(0xbdd3d36e), TM_UL(0x43acacef), TM_UL(0xc46262a6),
    TM_UL(0x399191a8), TM_UL(0x319595a4), TM_UL(0xd3e4e437), TM_UL(0xf279798b),
    TM_UL(0xd5e7e732), TM_UL(0x8bc8c843), TM_UL(0x6e373759), TM_UL(0xda6d6db7),
    TM_UL(0x018d8d8c), TM_UL(0xb1d5d564), TM_UL(0x9c4e4ed2), TM_UL(0x49a9a9e0),
    TM_UL(0xd86c6cb4), TM_UL(0xac5656fa), TM_UL(0xf3f4f407), TM_UL(0xcfeaea25),
    TM_UL(0xca6565af), TM_UL(0xf47a7a8e), TM_UL(0x47aeaee9), TM_UL(0x10080818),
    TM_UL(0x6fbabad5), TM_UL(0xf0787888), TM_UL(0x4a25256f), TM_UL(0x5c2e2e72),
    TM_UL(0x381c1c24), TM_UL(0x57a6a6f1), TM_UL(0x73b4b4c7), TM_UL(0x97c6c651),
    TM_UL(0xcbe8e823), TM_UL(0xa1dddd7c), TM_UL(0xe874749c), TM_UL(0x3e1f1f21),
    TM_UL(0x964b4bdd), TM_UL(0x61bdbddc), TM_UL(0x0d8b8b86), TM_UL(0x0f8a8a85),
    TM_UL(0xe0707090), TM_UL(0x7c3e3e42), TM_UL(0x71b5b5c4), TM_UL(0xcc6666aa),
    TM_UL(0x904848d8), TM_UL(0x06030305), TM_UL(0xf7f6f601), TM_UL(0x1c0e0e12),
    TM_UL(0xc26161a3), TM_UL(0x6a35355f), TM_UL(0xae5757f9), TM_UL(0x69b9b9d0),
    TM_UL(0x17868691), TM_UL(0x99c1c158), TM_UL(0x3a1d1d27), TM_UL(0x279e9eb9),
    TM_UL(0xd9e1e138), TM_UL(0xebf8f813), TM_UL(0x2b9898b3), TM_UL(0x22111133),
    TM_UL(0xd26969bb), TM_UL(0xa9d9d970), TM_UL(0x078e8e89), TM_UL(0x339494a7),
    TM_UL(0x2d9b9bb6), TM_UL(0x3c1e1e22), TM_UL(0x15878792), TM_UL(0xc9e9e920),
    TM_UL(0x87cece49), TM_UL(0xaa5555ff), TM_UL(0x50282878), TM_UL(0xa5dfdf7a),
    TM_UL(0x038c8c8f), TM_UL(0x59a1a1f8), TM_UL(0x09898980), TM_UL(0x1a0d0d17),
    TM_UL(0x65bfbfda), TM_UL(0xd7e6e631), TM_UL(0x844242c6), TM_UL(0xd06868b8),
    TM_UL(0x824141c3), TM_UL(0x299999b0), TM_UL(0x5a2d2d77), TM_UL(0x1e0f0f11),
    TM_UL(0x7bb0b0cb), TM_UL(0xa85454fc), TM_UL(0x6dbbbbd6), TM_UL(0x2c16163a)
};
static const tt32Bit TM_CONST_QLF Te1[256] = {
    TM_UL(0xa5c66363), TM_UL(0x84f87c7c), TM_UL(0x99ee7777), TM_UL(0x8df67b7b),
    TM_UL(0x0dfff2f2), TM_UL(0xbdd66b6b), TM_UL(0xb1de6f6f), TM_UL(0x5491c5c5),
    TM_UL(0x50603030), TM_UL(0x03020101), TM_UL(0xa9ce6767), TM_UL(0x7d562b2b),
    TM_UL(0x19e7fefe), TM_UL(0x62b5d7d7), TM_UL(0xe64dabab), TM_UL(0x9aec7676),
    TM_UL(0x458fcaca), TM_UL(0x9d1f8282), TM_UL(0x4089c9c9), TM_UL(0x87fa7d7d),
    TM_UL(0x15effafa), TM_UL(0xebb25959), TM_UL(0xc98e4747), TM_UL(0x0bfbf0f0),
    TM_UL(0xec41adad), TM_UL(0x67b3d4d4), TM_UL(0xfd5fa2a2), TM_UL(0xea45afaf),
    TM_UL(0xbf239c9c), TM_UL(0xf753a4a4), TM_UL(0x96e47272), TM_UL(0x5b9bc0c0),
    TM_UL(0xc275b7b7), TM_UL(0x1ce1fdfd), TM_UL(0xae3d9393), TM_UL(0x6a4c2626),
    TM_UL(0x5a6c3636), TM_UL(0x417e3f3f), TM_UL(0x02f5f7f7), TM_UL(0x4f83cccc),
    TM_UL(0x5c683434), TM_UL(0xf451a5a5), TM_UL(0x34d1e5e5), TM_UL(0x08f9f1f1),
    TM_UL(0x93e27171), TM_UL(0x73abd8d8), TM_UL(0x53623131), TM_UL(0x3f2a1515),
    TM_UL(0x0c080404), TM_UL(0x5295c7c7), TM_UL(0x65462323), TM_UL(0x5e9dc3c3),
    TM_UL(0x28301818), TM_UL(0xa1379696), TM_UL(0x0f0a0505), TM_UL(0xb52f9a9a),
    TM_UL(0x090e0707), TM_UL(0x36241212), TM_UL(0x9b1b8080), TM_UL(0x3ddfe2e2),
    TM_UL(0x26cdebeb), TM_UL(0x694e2727), TM_UL(0xcd7fb2b2), TM_UL(0x9fea7575),
    TM_UL(0x1b120909), TM_UL(0x9e1d8383), TM_UL(0x74582c2c), TM_UL(0x2e341a1a),
    TM_UL(0x2d361b1b), TM_UL(0xb2dc6e6e), TM_UL(0xeeb45a5a), TM_UL(0xfb5ba0a0),
    TM_UL(0xf6a45252), TM_UL(0x4d763b3b), TM_UL(0x61b7d6d6), TM_UL(0xce7db3b3),
    TM_UL(0x7b522929), TM_UL(0x3edde3e3), TM_UL(0x715e2f2f), TM_UL(0x97138484),
    TM_UL(0xf5a65353), TM_UL(0x68b9d1d1), TM_UL(0x00000000), TM_UL(0x2cc1eded),
    TM_UL(0x60402020), TM_UL(0x1fe3fcfc), TM_UL(0xc879b1b1), TM_UL(0xedb65b5b),
    TM_UL(0xbed46a6a), TM_UL(0x468dcbcb), TM_UL(0xd967bebe), TM_UL(0x4b723939),
    TM_UL(0xde944a4a), TM_UL(0xd4984c4c), TM_UL(0xe8b05858), TM_UL(0x4a85cfcf),
    TM_UL(0x6bbbd0d0), TM_UL(0x2ac5efef), TM_UL(0xe54faaaa), TM_UL(0x16edfbfb),
    TM_UL(0xc5864343), TM_UL(0xd79a4d4d), TM_UL(0x55663333), TM_UL(0x94118585),
    TM_UL(0xcf8a4545), TM_UL(0x10e9f9f9), TM_UL(0x06040202), TM_UL(0x81fe7f7f),
    TM_UL(0xf0a05050), TM_UL(0x44783c3c), TM_UL(0xba259f9f), TM_UL(0xe34ba8a8),
    TM_UL(0xf3a25151), TM_UL(0xfe5da3a3), TM_UL(0xc0804040), TM_UL(0x8a058f8f),
    TM_UL(0xad3f9292), TM_UL(0xbc219d9d), TM_UL(0x48703838), TM_UL(0x04f1f5f5),
    TM_UL(0xdf63bcbc), TM_UL(0xc177b6b6), TM_UL(0x75afdada), TM_UL(0x63422121),
    TM_UL(0x30201010), TM_UL(0x1ae5ffff), TM_UL(0x0efdf3f3), TM_UL(0x6dbfd2d2),
    TM_UL(0x4c81cdcd), TM_UL(0x14180c0c), TM_UL(0x35261313), TM_UL(0x2fc3ecec),
    TM_UL(0xe1be5f5f), TM_UL(0xa2359797), TM_UL(0xcc884444), TM_UL(0x392e1717),
    TM_UL(0x5793c4c4), TM_UL(0xf255a7a7), TM_UL(0x82fc7e7e), TM_UL(0x477a3d3d),
    TM_UL(0xacc86464), TM_UL(0xe7ba5d5d), TM_UL(0x2b321919), TM_UL(0x95e67373),
    TM_UL(0xa0c06060), TM_UL(0x98198181), TM_UL(0xd19e4f4f), TM_UL(0x7fa3dcdc),
    TM_UL(0x66442222), TM_UL(0x7e542a2a), TM_UL(0xab3b9090), TM_UL(0x830b8888),
    TM_UL(0xca8c4646), TM_UL(0x29c7eeee), TM_UL(0xd36bb8b8), TM_UL(0x3c281414),
    TM_UL(0x79a7dede), TM_UL(0xe2bc5e5e), TM_UL(0x1d160b0b), TM_UL(0x76addbdb),
    TM_UL(0x3bdbe0e0), TM_UL(0x56643232), TM_UL(0x4e743a3a), TM_UL(0x1e140a0a),
    TM_UL(0xdb924949), TM_UL(0x0a0c0606), TM_UL(0x6c482424), TM_UL(0xe4b85c5c),
    TM_UL(0x5d9fc2c2), TM_UL(0x6ebdd3d3), TM_UL(0xef43acac), TM_UL(0xa6c46262),
    TM_UL(0xa8399191), TM_UL(0xa4319595), TM_UL(0x37d3e4e4), TM_UL(0x8bf27979),
    TM_UL(0x32d5e7e7), TM_UL(0x438bc8c8), TM_UL(0x596e3737), TM_UL(0xb7da6d6d),
    TM_UL(0x8c018d8d), TM_UL(0x64b1d5d5), TM_UL(0xd29c4e4e), TM_UL(0xe049a9a9),
    TM_UL(0xb4d86c6c), TM_UL(0xfaac5656), TM_UL(0x07f3f4f4), TM_UL(0x25cfeaea),
    TM_UL(0xafca6565), TM_UL(0x8ef47a7a), TM_UL(0xe947aeae), TM_UL(0x18100808),
    TM_UL(0xd56fbaba), TM_UL(0x88f07878), TM_UL(0x6f4a2525), TM_UL(0x725c2e2e),
    TM_UL(0x24381c1c), TM_UL(0xf157a6a6), TM_UL(0xc773b4b4), TM_UL(0x5197c6c6),
    TM_UL(0x23cbe8e8), TM_UL(0x7ca1dddd), TM_UL(0x9ce87474), TM_UL(0x213e1f1f),
    TM_UL(0xdd964b4b), TM_UL(0xdc61bdbd), TM_UL(0x860d8b8b), TM_UL(0x850f8a8a),
    TM_UL(0x90e07070), TM_UL(0x427c3e3e), TM_UL(0xc471b5b5), TM_UL(0xaacc6666),
    TM_UL(0xd8904848), TM_UL(0x05060303), TM_UL(0x01f7f6f6), TM_UL(0x121c0e0e),
    TM_UL(0xa3c26161), TM_UL(0x5f6a3535), TM_UL(0xf9ae5757), TM_UL(0xd069b9b9),
    TM_UL(0x91178686), TM_UL(0x5899c1c1), TM_UL(0x273a1d1d), TM_UL(0xb9279e9e),
    TM_UL(0x38d9e1e1), TM_UL(0x13ebf8f8), TM_UL(0xb32b9898), TM_UL(0x33221111),
    TM_UL(0xbbd26969), TM_UL(0x70a9d9d9), TM_UL(0x89078e8e), TM_UL(0xa7339494),
    TM_UL(0xb62d9b9b), TM_UL(0x223c1e1e), TM_UL(0x92158787), TM_UL(0x20c9e9e9),
    TM_UL(0x4987cece), TM_UL(0xffaa5555), TM_UL(0x78502828), TM_UL(0x7aa5dfdf),
    TM_UL(0x8f038c8c), TM_UL(0xf859a1a1), TM_UL(0x80098989), TM_UL(0x171a0d0d),
    TM_UL(0xda65bfbf), TM_UL(0x31d7e6e6), TM_UL(0xc6844242), TM_UL(0xb8d06868),
    TM_UL(0xc3824141), TM_UL(0xb0299999), TM_UL(0x775a2d2d), TM_UL(0x111e0f0f),
    TM_UL(0xcb7bb0b0), TM_UL(0xfca85454), TM_UL(0xd66dbbbb), TM_UL(0x3a2c1616)
};
static const tt32Bit TM_CONST_QLF Te2[256] = {
    TM_UL(0x63a5c663), TM_UL(0x7c84f87c), TM_UL(0x7799ee77), TM_UL(0x7b8df67b),
    TM_UL(0xf20dfff2), TM_UL(0x6bbdd66b), TM_UL(0x6fb1de6f), TM_UL(0xc55491c5),
    TM_UL(0x30506030), TM_UL(0x01030201), TM_UL(0x67a9ce67), TM_UL(0x2b7d562b),
    TM_UL(0xfe19e7fe), TM_UL(0xd762b5d7), TM_UL(0xabe64dab), TM_UL(0x769aec76),
    TM_UL(0xca458fca), TM_UL(0x829d1f82), TM_UL(0xc94089c9), TM_UL(0x7d87fa7d),
    TM_UL(0xfa15effa), TM_UL(0x59ebb259), TM_UL(0x47c98e47), TM_UL(0xf00bfbf0),
    TM_UL(0xadec41ad), TM_UL(0xd467b3d4), TM_UL(0xa2fd5fa2), TM_UL(0xafea45af),
    TM_UL(0x9cbf239c), TM_UL(0xa4f753a4), TM_UL(0x7296e472), TM_UL(0xc05b9bc0),
    TM_UL(0xb7c275b7), TM_UL(0xfd1ce1fd), TM_UL(0x93ae3d93), TM_UL(0x266a4c26),
    TM_UL(0x365a6c36), TM_UL(0x3f417e3f), TM_UL(0xf702f5f7), TM_UL(0xcc4f83cc),
    TM_UL(0x345c6834), TM_UL(0xa5f451a5), TM_UL(0xe534d1e5), TM_UL(0xf108f9f1),
    TM_UL(0x7193e271), TM_UL(0xd873abd8), TM_UL(0x31536231), TM_UL(0x153f2a15),
    TM_UL(0x040c0804), TM_UL(0xc75295c7), TM_UL(0x23654623), TM_UL(0xc35e9dc3),
    TM_UL(0x18283018), TM_UL(0x96a13796), TM_UL(0x050f0a05), TM_UL(0x9ab52f9a),
    TM_UL(0x07090e07), TM_UL(0x12362412), TM_UL(0x809b1b80), TM_UL(0xe23ddfe2),
    TM_UL(0xeb26cdeb), TM_UL(0x27694e27), TM_UL(0xb2cd7fb2), TM_UL(0x759fea75),
    TM_UL(0x091b1209), TM_UL(0x839e1d83), TM_UL(0x2c74582c), TM_UL(0x1a2e341a),
    TM_UL(0x1b2d361b), TM_UL(0x6eb2dc6e), TM_UL(0x5aeeb45a), TM_UL(0xa0fb5ba0),
    TM_UL(0x52f6a452), TM_UL(0x3b4d763b), TM_UL(0xd661b7d6), TM_UL(0xb3ce7db3),
    TM_UL(0x297b5229), TM_UL(0xe33edde3), TM_UL(0x2f715e2f), TM_UL(0x84971384),
    TM_UL(0x53f5a653), TM_UL(0xd168b9d1), TM_UL(0x00000000), TM_UL(0xed2cc1ed),
    TM_UL(0x20604020), TM_UL(0xfc1fe3fc), TM_UL(0xb1c879b1), TM_UL(0x5bedb65b),
    TM_UL(0x6abed46a), TM_UL(0xcb468dcb), TM_UL(0xbed967be), TM_UL(0x394b7239),
    TM_UL(0x4ade944a), TM_UL(0x4cd4984c), TM_UL(0x58e8b058), TM_UL(0xcf4a85cf),
    TM_UL(0xd06bbbd0), TM_UL(0xef2ac5ef), TM_UL(0xaae54faa), TM_UL(0xfb16edfb),
    TM_UL(0x43c58643), TM_UL(0x4dd79a4d), TM_UL(0x33556633), TM_UL(0x85941185),
    TM_UL(0x45cf8a45), TM_UL(0xf910e9f9), TM_UL(0x02060402), TM_UL(0x7f81fe7f),
    TM_UL(0x50f0a050), TM_UL(0x3c44783c), TM_UL(0x9fba259f), TM_UL(0xa8e34ba8),
    TM_UL(0x51f3a251), TM_UL(0xa3fe5da3), TM_UL(0x40c08040), TM_UL(0x8f8a058f),
    TM_UL(0x92ad3f92), TM_UL(0x9dbc219d), TM_UL(0x38487038), TM_UL(0xf504f1f5),
    TM_UL(0xbcdf63bc), TM_UL(0xb6c177b6), TM_UL(0xda75afda), TM_UL(0x21634221),
    TM_UL(0x10302010), TM_UL(0xff1ae5ff), TM_UL(0xf30efdf3), TM_UL(0xd26dbfd2),
    TM_UL(0xcd4c81cd), TM_UL(0x0c14180c), TM_UL(0x13352613), TM_UL(0xec2fc3ec),
    TM_UL(0x5fe1be5f), TM_UL(0x97a23597), TM_UL(0x44cc8844), TM_UL(0x17392e17),
    TM_UL(0xc45793c4), TM_UL(0xa7f255a7), TM_UL(0x7e82fc7e), TM_UL(0x3d477a3d),
    TM_UL(0x64acc864), TM_UL(0x5de7ba5d), TM_UL(0x192b3219), TM_UL(0x7395e673),
    TM_UL(0x60a0c060), TM_UL(0x81981981), TM_UL(0x4fd19e4f), TM_UL(0xdc7fa3dc),
    TM_UL(0x22664422), TM_UL(0x2a7e542a), TM_UL(0x90ab3b90), TM_UL(0x88830b88),
    TM_UL(0x46ca8c46), TM_UL(0xee29c7ee), TM_UL(0xb8d36bb8), TM_UL(0x143c2814),
    TM_UL(0xde79a7de), TM_UL(0x5ee2bc5e), TM_UL(0x0b1d160b), TM_UL(0xdb76addb),
    TM_UL(0xe03bdbe0), TM_UL(0x32566432), TM_UL(0x3a4e743a), TM_UL(0x0a1e140a),
    TM_UL(0x49db9249), TM_UL(0x060a0c06), TM_UL(0x246c4824), TM_UL(0x5ce4b85c),
    TM_UL(0xc25d9fc2), TM_UL(0xd36ebdd3), TM_UL(0xacef43ac), TM_UL(0x62a6c462),
    TM_UL(0x91a83991), TM_UL(0x95a43195), TM_UL(0xe437d3e4), TM_UL(0x798bf279),
    TM_UL(0xe732d5e7), TM_UL(0xc8438bc8), TM_UL(0x37596e37), TM_UL(0x6db7da6d),
    TM_UL(0x8d8c018d), TM_UL(0xd564b1d5), TM_UL(0x4ed29c4e), TM_UL(0xa9e049a9),
    TM_UL(0x6cb4d86c), TM_UL(0x56faac56), TM_UL(0xf407f3f4), TM_UL(0xea25cfea),
    TM_UL(0x65afca65), TM_UL(0x7a8ef47a), TM_UL(0xaee947ae), TM_UL(0x08181008),
    TM_UL(0xbad56fba), TM_UL(0x7888f078), TM_UL(0x256f4a25), TM_UL(0x2e725c2e),
    TM_UL(0x1c24381c), TM_UL(0xa6f157a6), TM_UL(0xb4c773b4), TM_UL(0xc65197c6),
    TM_UL(0xe823cbe8), TM_UL(0xdd7ca1dd), TM_UL(0x749ce874), TM_UL(0x1f213e1f),
    TM_UL(0x4bdd964b), TM_UL(0xbddc61bd), TM_UL(0x8b860d8b), TM_UL(0x8a850f8a),
    TM_UL(0x7090e070), TM_UL(0x3e427c3e), TM_UL(0xb5c471b5), TM_UL(0x66aacc66),
    TM_UL(0x48d89048), TM_UL(0x03050603), TM_UL(0xf601f7f6), TM_UL(0x0e121c0e),
    TM_UL(0x61a3c261), TM_UL(0x355f6a35), TM_UL(0x57f9ae57), TM_UL(0xb9d069b9),
    TM_UL(0x86911786), TM_UL(0xc15899c1), TM_UL(0x1d273a1d), TM_UL(0x9eb9279e),
    TM_UL(0xe138d9e1), TM_UL(0xf813ebf8), TM_UL(0x98b32b98), TM_UL(0x11332211),
    TM_UL(0x69bbd269), TM_UL(0xd970a9d9), TM_UL(0x8e89078e), TM_UL(0x94a73394),
    TM_UL(0x9bb62d9b), TM_UL(0x1e223c1e), TM_UL(0x87921587), TM_UL(0xe920c9e9),
    TM_UL(0xce4987ce), TM_UL(0x55ffaa55), TM_UL(0x28785028), TM_UL(0xdf7aa5df),
    TM_UL(0x8c8f038c), TM_UL(0xa1f859a1), TM_UL(0x89800989), TM_UL(0x0d171a0d),
    TM_UL(0xbfda65bf), TM_UL(0xe631d7e6), TM_UL(0x42c68442), TM_UL(0x68b8d068),
    TM_UL(0x41c38241), TM_UL(0x99b02999), TM_UL(0x2d775a2d), TM_UL(0x0f111e0f),
    TM_UL(0xb0cb7bb0), TM_UL(0x54fca854), TM_UL(0xbbd66dbb), TM_UL(0x163a2c16)
};
static const tt32Bit TM_CONST_QLF Te3[256] = {
    TM_UL(0x6363a5c6), TM_UL(0x7c7c84f8), TM_UL(0x777799ee), TM_UL(0x7b7b8df6),
    TM_UL(0xf2f20dff), TM_UL(0x6b6bbdd6), TM_UL(0x6f6fb1de), TM_UL(0xc5c55491),
    TM_UL(0x30305060), TM_UL(0x01010302), TM_UL(0x6767a9ce), TM_UL(0x2b2b7d56),
    TM_UL(0xfefe19e7), TM_UL(0xd7d762b5), TM_UL(0xababe64d), TM_UL(0x76769aec),
    TM_UL(0xcaca458f), TM_UL(0x82829d1f), TM_UL(0xc9c94089), TM_UL(0x7d7d87fa),
    TM_UL(0xfafa15ef), TM_UL(0x5959ebb2), TM_UL(0x4747c98e), TM_UL(0xf0f00bfb),
    TM_UL(0xadadec41), TM_UL(0xd4d467b3), TM_UL(0xa2a2fd5f), TM_UL(0xafafea45),
    TM_UL(0x9c9cbf23), TM_UL(0xa4a4f753), TM_UL(0x727296e4), TM_UL(0xc0c05b9b),
    TM_UL(0xb7b7c275), TM_UL(0xfdfd1ce1), TM_UL(0x9393ae3d), TM_UL(0x26266a4c),
    TM_UL(0x36365a6c), TM_UL(0x3f3f417e), TM_UL(0xf7f702f5), TM_UL(0xcccc4f83),
    TM_UL(0x34345c68), TM_UL(0xa5a5f451), TM_UL(0xe5e534d1), TM_UL(0xf1f108f9),
    TM_UL(0x717193e2), TM_UL(0xd8d873ab), TM_UL(0x31315362), TM_UL(0x15153f2a),
    TM_UL(0x04040c08), TM_UL(0xc7c75295), TM_UL(0x23236546), TM_UL(0xc3c35e9d),
    TM_UL(0x18182830), TM_UL(0x9696a137), TM_UL(0x05050f0a), TM_UL(0x9a9ab52f),
    TM_UL(0x0707090e), TM_UL(0x12123624), TM_UL(0x80809b1b), TM_UL(0xe2e23ddf),
    TM_UL(0xebeb26cd), TM_UL(0x2727694e), TM_UL(0xb2b2cd7f), TM_UL(0x75759fea),
    TM_UL(0x09091b12), TM_UL(0x83839e1d), TM_UL(0x2c2c7458), TM_UL(0x1a1a2e34),
    TM_UL(0x1b1b2d36), TM_UL(0x6e6eb2dc), TM_UL(0x5a5aeeb4), TM_UL(0xa0a0fb5b),
    TM_UL(0x5252f6a4), TM_UL(0x3b3b4d76), TM_UL(0xd6d661b7), TM_UL(0xb3b3ce7d),
    TM_UL(0x29297b52), TM_UL(0xe3e33edd), TM_UL(0x2f2f715e), TM_UL(0x84849713),
    TM_UL(0x5353f5a6), TM_UL(0xd1d168b9), TM_UL(0x00000000), TM_UL(0xeded2cc1),
    TM_UL(0x20206040), TM_UL(0xfcfc1fe3), TM_UL(0xb1b1c879), TM_UL(0x5b5bedb6),
    TM_UL(0x6a6abed4), TM_UL(0xcbcb468d), TM_UL(0xbebed967), TM_UL(0x39394b72),
    TM_UL(0x4a4ade94), TM_UL(0x4c4cd498), TM_UL(0x5858e8b0), TM_UL(0xcfcf4a85),
    TM_UL(0xd0d06bbb), TM_UL(0xefef2ac5), TM_UL(0xaaaae54f), TM_UL(0xfbfb16ed),
    TM_UL(0x4343c586), TM_UL(0x4d4dd79a), TM_UL(0x33335566), TM_UL(0x85859411),
    TM_UL(0x4545cf8a), TM_UL(0xf9f910e9), TM_UL(0x02020604), TM_UL(0x7f7f81fe),
    TM_UL(0x5050f0a0), TM_UL(0x3c3c4478), TM_UL(0x9f9fba25), TM_UL(0xa8a8e34b),
    TM_UL(0x5151f3a2), TM_UL(0xa3a3fe5d), TM_UL(0x4040c080), TM_UL(0x8f8f8a05),
    TM_UL(0x9292ad3f), TM_UL(0x9d9dbc21), TM_UL(0x38384870), TM_UL(0xf5f504f1),
    TM_UL(0xbcbcdf63), TM_UL(0xb6b6c177), TM_UL(0xdada75af), TM_UL(0x21216342),
    TM_UL(0x10103020), TM_UL(0xffff1ae5), TM_UL(0xf3f30efd), TM_UL(0xd2d26dbf),
    TM_UL(0xcdcd4c81), TM_UL(0x0c0c1418), TM_UL(0x13133526), TM_UL(0xecec2fc3),
    TM_UL(0x5f5fe1be), TM_UL(0x9797a235), TM_UL(0x4444cc88), TM_UL(0x1717392e),
    TM_UL(0xc4c45793), TM_UL(0xa7a7f255), TM_UL(0x7e7e82fc), TM_UL(0x3d3d477a),
    TM_UL(0x6464acc8), TM_UL(0x5d5de7ba), TM_UL(0x19192b32), TM_UL(0x737395e6),
    TM_UL(0x6060a0c0), TM_UL(0x81819819), TM_UL(0x4f4fd19e), TM_UL(0xdcdc7fa3),
    TM_UL(0x22226644), TM_UL(0x2a2a7e54), TM_UL(0x9090ab3b), TM_UL(0x8888830b),
    TM_UL(0x4646ca8c), TM_UL(0xeeee29c7), TM_UL(0xb8b8d36b), TM_UL(0x14143c28),
    TM_UL(0xdede79a7), TM_UL(0x5e5ee2bc), TM_UL(0x0b0b1d16), TM_UL(0xdbdb76ad),
    TM_UL(0xe0e03bdb), TM_UL(0x32325664), TM_UL(0x3a3a4e74), TM_UL(0x0a0a1e14),
    TM_UL(0x4949db92), TM_UL(0x06060a0c), TM_UL(0x24246c48), TM_UL(0x5c5ce4b8),
    TM_UL(0xc2c25d9f), TM_UL(0xd3d36ebd), TM_UL(0xacacef43), TM_UL(0x6262a6c4),
    TM_UL(0x9191a839), TM_UL(0x9595a431), TM_UL(0xe4e437d3), TM_UL(0x79798bf2),
    TM_UL(0xe7e732d5), TM_UL(0xc8c8438b), TM_UL(0x3737596e), TM_UL(0x6d6db7da),
    TM_UL(0x8d8d8c01), TM_UL(0xd5d564b1), TM_UL(0x4e4ed29c), TM_UL(0xa9a9e049),
    TM_UL(0x6c6cb4d8), TM_UL(0x5656faac), TM_UL(0xf4f407f3), TM_UL(0xeaea25cf),
    TM_UL(0x6565afca), TM_UL(0x7a7a8ef4), TM_UL(0xaeaee947), TM_UL(0x08081810),
    TM_UL(0xbabad56f), TM_UL(0x787888f0), TM_UL(0x25256f4a), TM_UL(0x2e2e725c),
    TM_UL(0x1c1c2438), TM_UL(0xa6a6f157), TM_UL(0xb4b4c773), TM_UL(0xc6c65197),
    TM_UL(0xe8e823cb), TM_UL(0xdddd7ca1), TM_UL(0x74749ce8), TM_UL(0x1f1f213e),
    TM_UL(0x4b4bdd96), TM_UL(0xbdbddc61), TM_UL(0x8b8b860d), TM_UL(0x8a8a850f),
    TM_UL(0x707090e0), TM_UL(0x3e3e427c), TM_UL(0xb5b5c471), TM_UL(0x6666aacc),
    TM_UL(0x4848d890), TM_UL(0x03030506), TM_UL(0xf6f601f7), TM_UL(0x0e0e121c),
    TM_UL(0x6161a3c2), TM_UL(0x35355f6a), TM_UL(0x5757f9ae), TM_UL(0xb9b9d069),
    TM_UL(0x86869117), TM_UL(0xc1c15899), TM_UL(0x1d1d273a), TM_UL(0x9e9eb927),
    TM_UL(0xe1e138d9), TM_UL(0xf8f813eb), TM_UL(0x9898b32b), TM_UL(0x11113322),
    TM_UL(0x6969bbd2), TM_UL(0xd9d970a9), TM_UL(0x8e8e8907), TM_UL(0x9494a733),
    TM_UL(0x9b9bb62d), TM_UL(0x1e1e223c), TM_UL(0x87879215), TM_UL(0xe9e920c9),
    TM_UL(0xcece4987), TM_UL(0x5555ffaa), TM_UL(0x28287850), TM_UL(0xdfdf7aa5),
    TM_UL(0x8c8c8f03), TM_UL(0xa1a1f859), TM_UL(0x89898009), TM_UL(0x0d0d171a),
    TM_UL(0xbfbfda65), TM_UL(0xe6e631d7), TM_UL(0x4242c684), TM_UL(0x6868b8d0),
    TM_UL(0x4141c382), TM_UL(0x9999b029), TM_UL(0x2d2d775a), TM_UL(0x0f0f111e),
    TM_UL(0xb0b0cb7b), TM_UL(0x5454fca8), TM_UL(0xbbbbd66d), TM_UL(0x16163a2c)
};
static const tt32Bit TM_CONST_QLF Te4[256] = {
    TM_UL(0x63636363), TM_UL(0x7c7c7c7c), TM_UL(0x77777777), TM_UL(0x7b7b7b7b),
    TM_UL(0xf2f2f2f2), TM_UL(0x6b6b6b6b), TM_UL(0x6f6f6f6f), TM_UL(0xc5c5c5c5),
    TM_UL(0x30303030), TM_UL(0x01010101), TM_UL(0x67676767), TM_UL(0x2b2b2b2b),
    TM_UL(0xfefefefe), TM_UL(0xd7d7d7d7), TM_UL(0xabababab), TM_UL(0x76767676),
    TM_UL(0xcacacaca), TM_UL(0x82828282), TM_UL(0xc9c9c9c9), TM_UL(0x7d7d7d7d),
    TM_UL(0xfafafafa), TM_UL(0x59595959), TM_UL(0x47474747), TM_UL(0xf0f0f0f0),
    TM_UL(0xadadadad), TM_UL(0xd4d4d4d4), TM_UL(0xa2a2a2a2), TM_UL(0xafafafaf),
    TM_UL(0x9c9c9c9c), TM_UL(0xa4a4a4a4), TM_UL(0x72727272), TM_UL(0xc0c0c0c0),
    TM_UL(0xb7b7b7b7), TM_UL(0xfdfdfdfd), TM_UL(0x93939393), TM_UL(0x26262626),
    TM_UL(0x36363636), TM_UL(0x3f3f3f3f), TM_UL(0xf7f7f7f7), TM_UL(0xcccccccc),
    TM_UL(0x34343434), TM_UL(0xa5a5a5a5), TM_UL(0xe5e5e5e5), TM_UL(0xf1f1f1f1),
    TM_UL(0x71717171), TM_UL(0xd8d8d8d8), TM_UL(0x31313131), TM_UL(0x15151515),
    TM_UL(0x04040404), TM_UL(0xc7c7c7c7), TM_UL(0x23232323), TM_UL(0xc3c3c3c3),
    TM_UL(0x18181818), TM_UL(0x96969696), TM_UL(0x05050505), TM_UL(0x9a9a9a9a),
    TM_UL(0x07070707), TM_UL(0x12121212), TM_UL(0x80808080), TM_UL(0xe2e2e2e2),
    TM_UL(0xebebebeb), TM_UL(0x27272727), TM_UL(0xb2b2b2b2), TM_UL(0x75757575),
    TM_UL(0x09090909), TM_UL(0x83838383), TM_UL(0x2c2c2c2c), TM_UL(0x1a1a1a1a),
    TM_UL(0x1b1b1b1b), TM_UL(0x6e6e6e6e), TM_UL(0x5a5a5a5a), TM_UL(0xa0a0a0a0),
    TM_UL(0x52525252), TM_UL(0x3b3b3b3b), TM_UL(0xd6d6d6d6), TM_UL(0xb3b3b3b3),
    TM_UL(0x29292929), TM_UL(0xe3e3e3e3), TM_UL(0x2f2f2f2f), TM_UL(0x84848484),
    TM_UL(0x53535353), TM_UL(0xd1d1d1d1), TM_UL(0x00000000), TM_UL(0xedededed),
    TM_UL(0x20202020), TM_UL(0xfcfcfcfc), TM_UL(0xb1b1b1b1), TM_UL(0x5b5b5b5b),
    TM_UL(0x6a6a6a6a), TM_UL(0xcbcbcbcb), TM_UL(0xbebebebe), TM_UL(0x39393939),
    TM_UL(0x4a4a4a4a), TM_UL(0x4c4c4c4c), TM_UL(0x58585858), TM_UL(0xcfcfcfcf),
    TM_UL(0xd0d0d0d0), TM_UL(0xefefefef), TM_UL(0xaaaaaaaa), TM_UL(0xfbfbfbfb),
    TM_UL(0x43434343), TM_UL(0x4d4d4d4d), TM_UL(0x33333333), TM_UL(0x85858585),
    TM_UL(0x45454545), TM_UL(0xf9f9f9f9), TM_UL(0x02020202), TM_UL(0x7f7f7f7f),
    TM_UL(0x50505050), TM_UL(0x3c3c3c3c), TM_UL(0x9f9f9f9f), TM_UL(0xa8a8a8a8),
    TM_UL(0x51515151), TM_UL(0xa3a3a3a3), TM_UL(0x40404040), TM_UL(0x8f8f8f8f),
    TM_UL(0x92929292), TM_UL(0x9d9d9d9d), TM_UL(0x38383838), TM_UL(0xf5f5f5f5),
    TM_UL(0xbcbcbcbc), TM_UL(0xb6b6b6b6), TM_UL(0xdadadada), TM_UL(0x21212121),
    TM_UL(0x10101010), TM_UL(0xffffffff), TM_UL(0xf3f3f3f3), TM_UL(0xd2d2d2d2),
    TM_UL(0xcdcdcdcd), TM_UL(0x0c0c0c0c), TM_UL(0x13131313), TM_UL(0xecececec),
    TM_UL(0x5f5f5f5f), TM_UL(0x97979797), TM_UL(0x44444444), TM_UL(0x17171717),
    TM_UL(0xc4c4c4c4), TM_UL(0xa7a7a7a7), TM_UL(0x7e7e7e7e), TM_UL(0x3d3d3d3d),
    TM_UL(0x64646464), TM_UL(0x5d5d5d5d), TM_UL(0x19191919), TM_UL(0x73737373),
    TM_UL(0x60606060), TM_UL(0x81818181), TM_UL(0x4f4f4f4f), TM_UL(0xdcdcdcdc),
    TM_UL(0x22222222), TM_UL(0x2a2a2a2a), TM_UL(0x90909090), TM_UL(0x88888888),
    TM_UL(0x46464646), TM_UL(0xeeeeeeee), TM_UL(0xb8b8b8b8), TM_UL(0x14141414),
    TM_UL(0xdededede), TM_UL(0x5e5e5e5e), TM_UL(0x0b0b0b0b), TM_UL(0xdbdbdbdb),
    TM_UL(0xe0e0e0e0), TM_UL(0x32323232), TM_UL(0x3a3a3a3a), TM_UL(0x0a0a0a0a),
    TM_UL(0x49494949), TM_UL(0x06060606), TM_UL(0x24242424), TM_UL(0x5c5c5c5c),
    TM_UL(0xc2c2c2c2), TM_UL(0xd3d3d3d3), TM_UL(0xacacacac), TM_UL(0x62626262),
    TM_UL(0x91919191), TM_UL(0x95959595), TM_UL(0xe4e4e4e4), TM_UL(0x79797979),
    TM_UL(0xe7e7e7e7), TM_UL(0xc8c8c8c8), TM_UL(0x37373737), TM_UL(0x6d6d6d6d),
    TM_UL(0x8d8d8d8d), TM_UL(0xd5d5d5d5), TM_UL(0x4e4e4e4e), TM_UL(0xa9a9a9a9),
    TM_UL(0x6c6c6c6c), TM_UL(0x56565656), TM_UL(0xf4f4f4f4), TM_UL(0xeaeaeaea),
    TM_UL(0x65656565), TM_UL(0x7a7a7a7a), TM_UL(0xaeaeaeae), TM_UL(0x08080808),
    TM_UL(0xbabababa), TM_UL(0x78787878), TM_UL(0x25252525), TM_UL(0x2e2e2e2e),
    TM_UL(0x1c1c1c1c), TM_UL(0xa6a6a6a6), TM_UL(0xb4b4b4b4), TM_UL(0xc6c6c6c6),
    TM_UL(0xe8e8e8e8), TM_UL(0xdddddddd), TM_UL(0x74747474), TM_UL(0x1f1f1f1f),
    TM_UL(0x4b4b4b4b), TM_UL(0xbdbdbdbd), TM_UL(0x8b8b8b8b), TM_UL(0x8a8a8a8a),
    TM_UL(0x70707070), TM_UL(0x3e3e3e3e), TM_UL(0xb5b5b5b5), TM_UL(0x66666666),
    TM_UL(0x48484848), TM_UL(0x03030303), TM_UL(0xf6f6f6f6), TM_UL(0x0e0e0e0e),
    TM_UL(0x61616161), TM_UL(0x35353535), TM_UL(0x57575757), TM_UL(0xb9b9b9b9),
    TM_UL(0x86868686), TM_UL(0xc1c1c1c1), TM_UL(0x1d1d1d1d), TM_UL(0x9e9e9e9e),
    TM_UL(0xe1e1e1e1), TM_UL(0xf8f8f8f8), TM_UL(0x98989898), TM_UL(0x11111111),
    TM_UL(0x69696969), TM_UL(0xd9d9d9d9), TM_UL(0x8e8e8e8e), TM_UL(0x94949494),
    TM_UL(0x9b9b9b9b), TM_UL(0x1e1e1e1e), TM_UL(0x87878787), TM_UL(0xe9e9e9e9),
    TM_UL(0xcececece), TM_UL(0x55555555), TM_UL(0x28282828), TM_UL(0xdfdfdfdf),
    TM_UL(0x8c8c8c8c), TM_UL(0xa1a1a1a1), TM_UL(0x89898989), TM_UL(0x0d0d0d0d),
    TM_UL(0xbfbfbfbf), TM_UL(0xe6e6e6e6), TM_UL(0x42424242), TM_UL(0x68686868),
    TM_UL(0x41414141), TM_UL(0x99999999), TM_UL(0x2d2d2d2d), TM_UL(0x0f0f0f0f),
    TM_UL(0xb0b0b0b0), TM_UL(0x54545454), TM_UL(0xbbbbbbbb), TM_UL(0x16161616)
};

static const tt32Bit TM_CONST_QLF Td0[256] = {
    TM_UL(0x51f4a750), TM_UL(0x7e416553), TM_UL(0x1a17a4c3), TM_UL(0x3a275e96),
    TM_UL(0x3bab6bcb), TM_UL(0x1f9d45f1), TM_UL(0xacfa58ab), TM_UL(0x4be30393),
    TM_UL(0x2030fa55), TM_UL(0xad766df6), TM_UL(0x88cc7691), TM_UL(0xf5024c25),
    TM_UL(0x4fe5d7fc), TM_UL(0xc52acbd7), TM_UL(0x26354480), TM_UL(0xb562a38f),
    TM_UL(0xdeb15a49), TM_UL(0x25ba1b67), TM_UL(0x45ea0e98), TM_UL(0x5dfec0e1),
    TM_UL(0xc32f7502), TM_UL(0x814cf012), TM_UL(0x8d4697a3), TM_UL(0x6bd3f9c6),
    TM_UL(0x038f5fe7), TM_UL(0x15929c95), TM_UL(0xbf6d7aeb), TM_UL(0x955259da),
    TM_UL(0xd4be832d), TM_UL(0x587421d3), TM_UL(0x49e06929), TM_UL(0x8ec9c844),
    TM_UL(0x75c2896a), TM_UL(0xf48e7978), TM_UL(0x99583e6b), TM_UL(0x27b971dd),
    TM_UL(0xbee14fb6), TM_UL(0xf088ad17), TM_UL(0xc920ac66), TM_UL(0x7dce3ab4),
    TM_UL(0x63df4a18), TM_UL(0xe51a3182), TM_UL(0x97513360), TM_UL(0x62537f45),
    TM_UL(0xb16477e0), TM_UL(0xbb6bae84), TM_UL(0xfe81a01c), TM_UL(0xf9082b94),
    TM_UL(0x70486858), TM_UL(0x8f45fd19), TM_UL(0x94de6c87), TM_UL(0x527bf8b7),
    TM_UL(0xab73d323), TM_UL(0x724b02e2), TM_UL(0xe31f8f57), TM_UL(0x6655ab2a),
    TM_UL(0xb2eb2807), TM_UL(0x2fb5c203), TM_UL(0x86c57b9a), TM_UL(0xd33708a5),
    TM_UL(0x302887f2), TM_UL(0x23bfa5b2), TM_UL(0x02036aba), TM_UL(0xed16825c),
    TM_UL(0x8acf1c2b), TM_UL(0xa779b492), TM_UL(0xf307f2f0), TM_UL(0x4e69e2a1),
    TM_UL(0x65daf4cd), TM_UL(0x0605bed5), TM_UL(0xd134621f), TM_UL(0xc4a6fe8a),
    TM_UL(0x342e539d), TM_UL(0xa2f355a0), TM_UL(0x058ae132), TM_UL(0xa4f6eb75),
    TM_UL(0x0b83ec39), TM_UL(0x4060efaa), TM_UL(0x5e719f06), TM_UL(0xbd6e1051),
    TM_UL(0x3e218af9), TM_UL(0x96dd063d), TM_UL(0xdd3e05ae), TM_UL(0x4de6bd46),
    TM_UL(0x91548db5), TM_UL(0x71c45d05), TM_UL(0x0406d46f), TM_UL(0x605015ff),
    TM_UL(0x1998fb24), TM_UL(0xd6bde997), TM_UL(0x894043cc), TM_UL(0x67d99e77),
    TM_UL(0xb0e842bd), TM_UL(0x07898b88), TM_UL(0xe7195b38), TM_UL(0x79c8eedb),
    TM_UL(0xa17c0a47), TM_UL(0x7c420fe9), TM_UL(0xf8841ec9), TM_UL(0x00000000),
    TM_UL(0x09808683), TM_UL(0x322bed48), TM_UL(0x1e1170ac), TM_UL(0x6c5a724e),
    TM_UL(0xfd0efffb), TM_UL(0x0f853856), TM_UL(0x3daed51e), TM_UL(0x362d3927),
    TM_UL(0x0a0fd964), TM_UL(0x685ca621), TM_UL(0x9b5b54d1), TM_UL(0x24362e3a),
    TM_UL(0x0c0a67b1), TM_UL(0x9357e70f), TM_UL(0xb4ee96d2), TM_UL(0x1b9b919e),
    TM_UL(0x80c0c54f), TM_UL(0x61dc20a2), TM_UL(0x5a774b69), TM_UL(0x1c121a16),
    TM_UL(0xe293ba0a), TM_UL(0xc0a02ae5), TM_UL(0x3c22e043), TM_UL(0x121b171d),
    TM_UL(0x0e090d0b), TM_UL(0xf28bc7ad), TM_UL(0x2db6a8b9), TM_UL(0x141ea9c8),
    TM_UL(0x57f11985), TM_UL(0xaf75074c), TM_UL(0xee99ddbb), TM_UL(0xa37f60fd),
    TM_UL(0xf701269f), TM_UL(0x5c72f5bc), TM_UL(0x44663bc5), TM_UL(0x5bfb7e34),
    TM_UL(0x8b432976), TM_UL(0xcb23c6dc), TM_UL(0xb6edfc68), TM_UL(0xb8e4f163),
    TM_UL(0xd731dcca), TM_UL(0x42638510), TM_UL(0x13972240), TM_UL(0x84c61120),
    TM_UL(0x854a247d), TM_UL(0xd2bb3df8), TM_UL(0xaef93211), TM_UL(0xc729a16d),
    TM_UL(0x1d9e2f4b), TM_UL(0xdcb230f3), TM_UL(0x0d8652ec), TM_UL(0x77c1e3d0),
    TM_UL(0x2bb3166c), TM_UL(0xa970b999), TM_UL(0x119448fa), TM_UL(0x47e96422),
    TM_UL(0xa8fc8cc4), TM_UL(0xa0f03f1a), TM_UL(0x567d2cd8), TM_UL(0x223390ef),
    TM_UL(0x87494ec7), TM_UL(0xd938d1c1), TM_UL(0x8ccaa2fe), TM_UL(0x98d40b36),
    TM_UL(0xa6f581cf), TM_UL(0xa57ade28), TM_UL(0xdab78e26), TM_UL(0x3fadbfa4),
    TM_UL(0x2c3a9de4), TM_UL(0x5078920d), TM_UL(0x6a5fcc9b), TM_UL(0x547e4662),
    TM_UL(0xf68d13c2), TM_UL(0x90d8b8e8), TM_UL(0x2e39f75e), TM_UL(0x82c3aff5),
    TM_UL(0x9f5d80be), TM_UL(0x69d0937c), TM_UL(0x6fd52da9), TM_UL(0xcf2512b3),
    TM_UL(0xc8ac993b), TM_UL(0x10187da7), TM_UL(0xe89c636e), TM_UL(0xdb3bbb7b),
    TM_UL(0xcd267809), TM_UL(0x6e5918f4), TM_UL(0xec9ab701), TM_UL(0x834f9aa8),
    TM_UL(0xe6956e65), TM_UL(0xaaffe67e), TM_UL(0x21bccf08), TM_UL(0xef15e8e6),
    TM_UL(0xbae79bd9), TM_UL(0x4a6f36ce), TM_UL(0xea9f09d4), TM_UL(0x29b07cd6),
    TM_UL(0x31a4b2af), TM_UL(0x2a3f2331), TM_UL(0xc6a59430), TM_UL(0x35a266c0),
    TM_UL(0x744ebc37), TM_UL(0xfc82caa6), TM_UL(0xe090d0b0), TM_UL(0x33a7d815),
    TM_UL(0xf104984a), TM_UL(0x41ecdaf7), TM_UL(0x7fcd500e), TM_UL(0x1791f62f),
    TM_UL(0x764dd68d), TM_UL(0x43efb04d), TM_UL(0xccaa4d54), TM_UL(0xe49604df),
    TM_UL(0x9ed1b5e3), TM_UL(0x4c6a881b), TM_UL(0xc12c1fb8), TM_UL(0x4665517f),
    TM_UL(0x9d5eea04), TM_UL(0x018c355d), TM_UL(0xfa877473), TM_UL(0xfb0b412e),
    TM_UL(0xb3671d5a), TM_UL(0x92dbd252), TM_UL(0xe9105633), TM_UL(0x6dd64713),
    TM_UL(0x9ad7618c), TM_UL(0x37a10c7a), TM_UL(0x59f8148e), TM_UL(0xeb133c89),
    TM_UL(0xcea927ee), TM_UL(0xb761c935), TM_UL(0xe11ce5ed), TM_UL(0x7a47b13c),
    TM_UL(0x9cd2df59), TM_UL(0x55f2733f), TM_UL(0x1814ce79), TM_UL(0x73c737bf),
    TM_UL(0x53f7cdea), TM_UL(0x5ffdaa5b), TM_UL(0xdf3d6f14), TM_UL(0x7844db86),
    TM_UL(0xcaaff381), TM_UL(0xb968c43e), TM_UL(0x3824342c), TM_UL(0xc2a3405f),
    TM_UL(0x161dc372), TM_UL(0xbce2250c), TM_UL(0x283c498b), TM_UL(0xff0d9541),
    TM_UL(0x39a80171), TM_UL(0x080cb3de), TM_UL(0xd8b4e49c), TM_UL(0x6456c190),
    TM_UL(0x7bcb8461), TM_UL(0xd532b670), TM_UL(0x486c5c74), TM_UL(0xd0b85742)
};
static const tt32Bit TM_CONST_QLF Td1[256] = {
    TM_UL(0x5051f4a7), TM_UL(0x537e4165), TM_UL(0xc31a17a4), TM_UL(0x963a275e),
    TM_UL(0xcb3bab6b), TM_UL(0xf11f9d45), TM_UL(0xabacfa58), TM_UL(0x934be303),
    TM_UL(0x552030fa), TM_UL(0xf6ad766d), TM_UL(0x9188cc76), TM_UL(0x25f5024c),
    TM_UL(0xfc4fe5d7), TM_UL(0xd7c52acb), TM_UL(0x80263544), TM_UL(0x8fb562a3),
    TM_UL(0x49deb15a), TM_UL(0x6725ba1b), TM_UL(0x9845ea0e), TM_UL(0xe15dfec0),
    TM_UL(0x02c32f75), TM_UL(0x12814cf0), TM_UL(0xa38d4697), TM_UL(0xc66bd3f9),
    TM_UL(0xe7038f5f), TM_UL(0x9515929c), TM_UL(0xebbf6d7a), TM_UL(0xda955259),
    TM_UL(0x2dd4be83), TM_UL(0xd3587421), TM_UL(0x2949e069), TM_UL(0x448ec9c8),
    TM_UL(0x6a75c289), TM_UL(0x78f48e79), TM_UL(0x6b99583e), TM_UL(0xdd27b971),
    TM_UL(0xb6bee14f), TM_UL(0x17f088ad), TM_UL(0x66c920ac), TM_UL(0xb47dce3a),
    TM_UL(0x1863df4a), TM_UL(0x82e51a31), TM_UL(0x60975133), TM_UL(0x4562537f),
    TM_UL(0xe0b16477), TM_UL(0x84bb6bae), TM_UL(0x1cfe81a0), TM_UL(0x94f9082b),
    TM_UL(0x58704868), TM_UL(0x198f45fd), TM_UL(0x8794de6c), TM_UL(0xb7527bf8),
    TM_UL(0x23ab73d3), TM_UL(0xe2724b02), TM_UL(0x57e31f8f), TM_UL(0x2a6655ab),
    TM_UL(0x07b2eb28), TM_UL(0x032fb5c2), TM_UL(0x9a86c57b), TM_UL(0xa5d33708),
    TM_UL(0xf2302887), TM_UL(0xb223bfa5), TM_UL(0xba02036a), TM_UL(0x5ced1682),
    TM_UL(0x2b8acf1c), TM_UL(0x92a779b4), TM_UL(0xf0f307f2), TM_UL(0xa14e69e2),
    TM_UL(0xcd65daf4), TM_UL(0xd50605be), TM_UL(0x1fd13462), TM_UL(0x8ac4a6fe),
    TM_UL(0x9d342e53), TM_UL(0xa0a2f355), TM_UL(0x32058ae1), TM_UL(0x75a4f6eb),
    TM_UL(0x390b83ec), TM_UL(0xaa4060ef), TM_UL(0x065e719f), TM_UL(0x51bd6e10),
    TM_UL(0xf93e218a), TM_UL(0x3d96dd06), TM_UL(0xaedd3e05), TM_UL(0x464de6bd),
    TM_UL(0xb591548d), TM_UL(0x0571c45d), TM_UL(0x6f0406d4), TM_UL(0xff605015),
    TM_UL(0x241998fb), TM_UL(0x97d6bde9), TM_UL(0xcc894043), TM_UL(0x7767d99e),
    TM_UL(0xbdb0e842), TM_UL(0x8807898b), TM_UL(0x38e7195b), TM_UL(0xdb79c8ee),
    TM_UL(0x47a17c0a), TM_UL(0xe97c420f), TM_UL(0xc9f8841e), TM_UL(0x00000000),
    TM_UL(0x83098086), TM_UL(0x48322bed), TM_UL(0xac1e1170), TM_UL(0x4e6c5a72),
    TM_UL(0xfbfd0eff), TM_UL(0x560f8538), TM_UL(0x1e3daed5), TM_UL(0x27362d39),
    TM_UL(0x640a0fd9), TM_UL(0x21685ca6), TM_UL(0xd19b5b54), TM_UL(0x3a24362e),
    TM_UL(0xb10c0a67), TM_UL(0x0f9357e7), TM_UL(0xd2b4ee96), TM_UL(0x9e1b9b91),
    TM_UL(0x4f80c0c5), TM_UL(0xa261dc20), TM_UL(0x695a774b), TM_UL(0x161c121a),
    TM_UL(0x0ae293ba), TM_UL(0xe5c0a02a), TM_UL(0x433c22e0), TM_UL(0x1d121b17),
    TM_UL(0x0b0e090d), TM_UL(0xadf28bc7), TM_UL(0xb92db6a8), TM_UL(0xc8141ea9),
    TM_UL(0x8557f119), TM_UL(0x4caf7507), TM_UL(0xbbee99dd), TM_UL(0xfda37f60),
    TM_UL(0x9ff70126), TM_UL(0xbc5c72f5), TM_UL(0xc544663b), TM_UL(0x345bfb7e),
    TM_UL(0x768b4329), TM_UL(0xdccb23c6), TM_UL(0x68b6edfc), TM_UL(0x63b8e4f1),
    TM_UL(0xcad731dc), TM_UL(0x10426385), TM_UL(0x40139722), TM_UL(0x2084c611),
    TM_UL(0x7d854a24), TM_UL(0xf8d2bb3d), TM_UL(0x11aef932), TM_UL(0x6dc729a1),
    TM_UL(0x4b1d9e2f), TM_UL(0xf3dcb230), TM_UL(0xec0d8652), TM_UL(0xd077c1e3),
    TM_UL(0x6c2bb316), TM_UL(0x99a970b9), TM_UL(0xfa119448), TM_UL(0x2247e964),
    TM_UL(0xc4a8fc8c), TM_UL(0x1aa0f03f), TM_UL(0xd8567d2c), TM_UL(0xef223390),
    TM_UL(0xc787494e), TM_UL(0xc1d938d1), TM_UL(0xfe8ccaa2), TM_UL(0x3698d40b),
    TM_UL(0xcfa6f581), TM_UL(0x28a57ade), TM_UL(0x26dab78e), TM_UL(0xa43fadbf),
    TM_UL(0xe42c3a9d), TM_UL(0x0d507892), TM_UL(0x9b6a5fcc), TM_UL(0x62547e46),
    TM_UL(0xc2f68d13), TM_UL(0xe890d8b8), TM_UL(0x5e2e39f7), TM_UL(0xf582c3af),
    TM_UL(0xbe9f5d80), TM_UL(0x7c69d093), TM_UL(0xa96fd52d), TM_UL(0xb3cf2512),
    TM_UL(0x3bc8ac99), TM_UL(0xa710187d), TM_UL(0x6ee89c63), TM_UL(0x7bdb3bbb),
    TM_UL(0x09cd2678), TM_UL(0xf46e5918), TM_UL(0x01ec9ab7), TM_UL(0xa8834f9a),
    TM_UL(0x65e6956e), TM_UL(0x7eaaffe6), TM_UL(0x0821bccf), TM_UL(0xe6ef15e8),
    TM_UL(0xd9bae79b), TM_UL(0xce4a6f36), TM_UL(0xd4ea9f09), TM_UL(0xd629b07c),
    TM_UL(0xaf31a4b2), TM_UL(0x312a3f23), TM_UL(0x30c6a594), TM_UL(0xc035a266),
    TM_UL(0x37744ebc), TM_UL(0xa6fc82ca), TM_UL(0xb0e090d0), TM_UL(0x1533a7d8),
    TM_UL(0x4af10498), TM_UL(0xf741ecda), TM_UL(0x0e7fcd50), TM_UL(0x2f1791f6),
    TM_UL(0x8d764dd6), TM_UL(0x4d43efb0), TM_UL(0x54ccaa4d), TM_UL(0xdfe49604),
    TM_UL(0xe39ed1b5), TM_UL(0x1b4c6a88), TM_UL(0xb8c12c1f), TM_UL(0x7f466551),
    TM_UL(0x049d5eea), TM_UL(0x5d018c35), TM_UL(0x73fa8774), TM_UL(0x2efb0b41),
    TM_UL(0x5ab3671d), TM_UL(0x5292dbd2), TM_UL(0x33e91056), TM_UL(0x136dd647),
    TM_UL(0x8c9ad761), TM_UL(0x7a37a10c), TM_UL(0x8e59f814), TM_UL(0x89eb133c),
    TM_UL(0xeecea927), TM_UL(0x35b761c9), TM_UL(0xede11ce5), TM_UL(0x3c7a47b1),
    TM_UL(0x599cd2df), TM_UL(0x3f55f273), TM_UL(0x791814ce), TM_UL(0xbf73c737),
    TM_UL(0xea53f7cd), TM_UL(0x5b5ffdaa), TM_UL(0x14df3d6f), TM_UL(0x867844db),
    TM_UL(0x81caaff3), TM_UL(0x3eb968c4), TM_UL(0x2c382434), TM_UL(0x5fc2a340),
    TM_UL(0x72161dc3), TM_UL(0x0cbce225), TM_UL(0x8b283c49), TM_UL(0x41ff0d95),
    TM_UL(0x7139a801), TM_UL(0xde080cb3), TM_UL(0x9cd8b4e4), TM_UL(0x906456c1),
    TM_UL(0x617bcb84), TM_UL(0x70d532b6), TM_UL(0x74486c5c), TM_UL(0x42d0b857)
};
static const tt32Bit TM_CONST_QLF Td2[256] = {
    TM_UL(0xa75051f4), TM_UL(0x65537e41), TM_UL(0xa4c31a17), TM_UL(0x5e963a27),
    TM_UL(0x6bcb3bab), TM_UL(0x45f11f9d), TM_UL(0x58abacfa), TM_UL(0x03934be3),
    TM_UL(0xfa552030), TM_UL(0x6df6ad76), TM_UL(0x769188cc), TM_UL(0x4c25f502),
    TM_UL(0xd7fc4fe5), TM_UL(0xcbd7c52a), TM_UL(0x44802635), TM_UL(0xa38fb562),
    TM_UL(0x5a49deb1), TM_UL(0x1b6725ba), TM_UL(0x0e9845ea), TM_UL(0xc0e15dfe),
    TM_UL(0x7502c32f), TM_UL(0xf012814c), TM_UL(0x97a38d46), TM_UL(0xf9c66bd3),
    TM_UL(0x5fe7038f), TM_UL(0x9c951592), TM_UL(0x7aebbf6d), TM_UL(0x59da9552),
    TM_UL(0x832dd4be), TM_UL(0x21d35874), TM_UL(0x692949e0), TM_UL(0xc8448ec9),
    TM_UL(0x896a75c2), TM_UL(0x7978f48e), TM_UL(0x3e6b9958), TM_UL(0x71dd27b9),
    TM_UL(0x4fb6bee1), TM_UL(0xad17f088), TM_UL(0xac66c920), TM_UL(0x3ab47dce),
    TM_UL(0x4a1863df), TM_UL(0x3182e51a), TM_UL(0x33609751), TM_UL(0x7f456253),
    TM_UL(0x77e0b164), TM_UL(0xae84bb6b), TM_UL(0xa01cfe81), TM_UL(0x2b94f908),
    TM_UL(0x68587048), TM_UL(0xfd198f45), TM_UL(0x6c8794de), TM_UL(0xf8b7527b),
    TM_UL(0xd323ab73), TM_UL(0x02e2724b), TM_UL(0x8f57e31f), TM_UL(0xab2a6655),
    TM_UL(0x2807b2eb), TM_UL(0xc2032fb5), TM_UL(0x7b9a86c5), TM_UL(0x08a5d337),
    TM_UL(0x87f23028), TM_UL(0xa5b223bf), TM_UL(0x6aba0203), TM_UL(0x825ced16),
    TM_UL(0x1c2b8acf), TM_UL(0xb492a779), TM_UL(0xf2f0f307), TM_UL(0xe2a14e69),
    TM_UL(0xf4cd65da), TM_UL(0xbed50605), TM_UL(0x621fd134), TM_UL(0xfe8ac4a6),
    TM_UL(0x539d342e), TM_UL(0x55a0a2f3), TM_UL(0xe132058a), TM_UL(0xeb75a4f6),
    TM_UL(0xec390b83), TM_UL(0xefaa4060), TM_UL(0x9f065e71), TM_UL(0x1051bd6e),
    TM_UL(0x8af93e21), TM_UL(0x063d96dd), TM_UL(0x05aedd3e), TM_UL(0xbd464de6),
    TM_UL(0x8db59154), TM_UL(0x5d0571c4), TM_UL(0xd46f0406), TM_UL(0x15ff6050),
    TM_UL(0xfb241998), TM_UL(0xe997d6bd), TM_UL(0x43cc8940), TM_UL(0x9e7767d9),
    TM_UL(0x42bdb0e8), TM_UL(0x8b880789), TM_UL(0x5b38e719), TM_UL(0xeedb79c8),
    TM_UL(0x0a47a17c), TM_UL(0x0fe97c42), TM_UL(0x1ec9f884), TM_UL(0x00000000),
    TM_UL(0x86830980), TM_UL(0xed48322b), TM_UL(0x70ac1e11), TM_UL(0x724e6c5a),
    TM_UL(0xfffbfd0e), TM_UL(0x38560f85), TM_UL(0xd51e3dae), TM_UL(0x3927362d),
    TM_UL(0xd9640a0f), TM_UL(0xa621685c), TM_UL(0x54d19b5b), TM_UL(0x2e3a2436),
    TM_UL(0x67b10c0a), TM_UL(0xe70f9357), TM_UL(0x96d2b4ee), TM_UL(0x919e1b9b),
    TM_UL(0xc54f80c0), TM_UL(0x20a261dc), TM_UL(0x4b695a77), TM_UL(0x1a161c12),
    TM_UL(0xba0ae293), TM_UL(0x2ae5c0a0), TM_UL(0xe0433c22), TM_UL(0x171d121b),
    TM_UL(0x0d0b0e09), TM_UL(0xc7adf28b), TM_UL(0xa8b92db6), TM_UL(0xa9c8141e),
    TM_UL(0x198557f1), TM_UL(0x074caf75), TM_UL(0xddbbee99), TM_UL(0x60fda37f),
    TM_UL(0x269ff701), TM_UL(0xf5bc5c72), TM_UL(0x3bc54466), TM_UL(0x7e345bfb),
    TM_UL(0x29768b43), TM_UL(0xc6dccb23), TM_UL(0xfc68b6ed), TM_UL(0xf163b8e4),
    TM_UL(0xdccad731), TM_UL(0x85104263), TM_UL(0x22401397), TM_UL(0x112084c6),
    TM_UL(0x247d854a), TM_UL(0x3df8d2bb), TM_UL(0x3211aef9), TM_UL(0xa16dc729),
    TM_UL(0x2f4b1d9e), TM_UL(0x30f3dcb2), TM_UL(0x52ec0d86), TM_UL(0xe3d077c1),
    TM_UL(0x166c2bb3), TM_UL(0xb999a970), TM_UL(0x48fa1194), TM_UL(0x642247e9),
    TM_UL(0x8cc4a8fc), TM_UL(0x3f1aa0f0), TM_UL(0x2cd8567d), TM_UL(0x90ef2233),
    TM_UL(0x4ec78749), TM_UL(0xd1c1d938), TM_UL(0xa2fe8cca), TM_UL(0x0b3698d4),
    TM_UL(0x81cfa6f5), TM_UL(0xde28a57a), TM_UL(0x8e26dab7), TM_UL(0xbfa43fad),
    TM_UL(0x9de42c3a), TM_UL(0x920d5078), TM_UL(0xcc9b6a5f), TM_UL(0x4662547e),
    TM_UL(0x13c2f68d), TM_UL(0xb8e890d8), TM_UL(0xf75e2e39), TM_UL(0xaff582c3),
    TM_UL(0x80be9f5d), TM_UL(0x937c69d0), TM_UL(0x2da96fd5), TM_UL(0x12b3cf25),
    TM_UL(0x993bc8ac), TM_UL(0x7da71018), TM_UL(0x636ee89c), TM_UL(0xbb7bdb3b),
    TM_UL(0x7809cd26), TM_UL(0x18f46e59), TM_UL(0xb701ec9a), TM_UL(0x9aa8834f),
    TM_UL(0x6e65e695), TM_UL(0xe67eaaff), TM_UL(0xcf0821bc), TM_UL(0xe8e6ef15),
    TM_UL(0x9bd9bae7), TM_UL(0x36ce4a6f), TM_UL(0x09d4ea9f), TM_UL(0x7cd629b0),
    TM_UL(0xb2af31a4), TM_UL(0x23312a3f), TM_UL(0x9430c6a5), TM_UL(0x66c035a2),
    TM_UL(0xbc37744e), TM_UL(0xcaa6fc82), TM_UL(0xd0b0e090), TM_UL(0xd81533a7),
    TM_UL(0x984af104), TM_UL(0xdaf741ec), TM_UL(0x500e7fcd), TM_UL(0xf62f1791),
    TM_UL(0xd68d764d), TM_UL(0xb04d43ef), TM_UL(0x4d54ccaa), TM_UL(0x04dfe496),
    TM_UL(0xb5e39ed1), TM_UL(0x881b4c6a), TM_UL(0x1fb8c12c), TM_UL(0x517f4665),
    TM_UL(0xea049d5e), TM_UL(0x355d018c), TM_UL(0x7473fa87), TM_UL(0x412efb0b),
    TM_UL(0x1d5ab367), TM_UL(0xd25292db), TM_UL(0x5633e910), TM_UL(0x47136dd6),
    TM_UL(0x618c9ad7), TM_UL(0x0c7a37a1), TM_UL(0x148e59f8), TM_UL(0x3c89eb13),
    TM_UL(0x27eecea9), TM_UL(0xc935b761), TM_UL(0xe5ede11c), TM_UL(0xb13c7a47),
    TM_UL(0xdf599cd2), TM_UL(0x733f55f2), TM_UL(0xce791814), TM_UL(0x37bf73c7),
    TM_UL(0xcdea53f7), TM_UL(0xaa5b5ffd), TM_UL(0x6f14df3d), TM_UL(0xdb867844),
    TM_UL(0xf381caaf), TM_UL(0xc43eb968), TM_UL(0x342c3824), TM_UL(0x405fc2a3),
    TM_UL(0xc372161d), TM_UL(0x250cbce2), TM_UL(0x498b283c), TM_UL(0x9541ff0d),
    TM_UL(0x017139a8), TM_UL(0xb3de080c), TM_UL(0xe49cd8b4), TM_UL(0xc1906456),
    TM_UL(0x84617bcb), TM_UL(0xb670d532), TM_UL(0x5c74486c), TM_UL(0x5742d0b8)
};
static const tt32Bit TM_CONST_QLF Td3[256] = {
    TM_UL(0xf4a75051), TM_UL(0x4165537e), TM_UL(0x17a4c31a), TM_UL(0x275e963a),
    TM_UL(0xab6bcb3b), TM_UL(0x9d45f11f), TM_UL(0xfa58abac), TM_UL(0xe303934b),
    TM_UL(0x30fa5520), TM_UL(0x766df6ad), TM_UL(0xcc769188), TM_UL(0x024c25f5),
    TM_UL(0xe5d7fc4f), TM_UL(0x2acbd7c5), TM_UL(0x35448026), TM_UL(0x62a38fb5),
    TM_UL(0xb15a49de), TM_UL(0xba1b6725), TM_UL(0xea0e9845), TM_UL(0xfec0e15d),
    TM_UL(0x2f7502c3), TM_UL(0x4cf01281), TM_UL(0x4697a38d), TM_UL(0xd3f9c66b),
    TM_UL(0x8f5fe703), TM_UL(0x929c9515), TM_UL(0x6d7aebbf), TM_UL(0x5259da95),
    TM_UL(0xbe832dd4), TM_UL(0x7421d358), TM_UL(0xe0692949), TM_UL(0xc9c8448e),
    TM_UL(0xc2896a75), TM_UL(0x8e7978f4), TM_UL(0x583e6b99), TM_UL(0xb971dd27),
    TM_UL(0xe14fb6be), TM_UL(0x88ad17f0), TM_UL(0x20ac66c9), TM_UL(0xce3ab47d),
    TM_UL(0xdf4a1863), TM_UL(0x1a3182e5), TM_UL(0x51336097), TM_UL(0x537f4562),
    TM_UL(0x6477e0b1), TM_UL(0x6bae84bb), TM_UL(0x81a01cfe), TM_UL(0x082b94f9),
    TM_UL(0x48685870), TM_UL(0x45fd198f), TM_UL(0xde6c8794), TM_UL(0x7bf8b752),
    TM_UL(0x73d323ab), TM_UL(0x4b02e272), TM_UL(0x1f8f57e3), TM_UL(0x55ab2a66),
    TM_UL(0xeb2807b2), TM_UL(0xb5c2032f), TM_UL(0xc57b9a86), TM_UL(0x3708a5d3),
    TM_UL(0x2887f230), TM_UL(0xbfa5b223), TM_UL(0x036aba02), TM_UL(0x16825ced),
    TM_UL(0xcf1c2b8a), TM_UL(0x79b492a7), TM_UL(0x07f2f0f3), TM_UL(0x69e2a14e),
    TM_UL(0xdaf4cd65), TM_UL(0x05bed506), TM_UL(0x34621fd1), TM_UL(0xa6fe8ac4),
    TM_UL(0x2e539d34), TM_UL(0xf355a0a2), TM_UL(0x8ae13205), TM_UL(0xf6eb75a4),
    TM_UL(0x83ec390b), TM_UL(0x60efaa40), TM_UL(0x719f065e), TM_UL(0x6e1051bd),
    TM_UL(0x218af93e), TM_UL(0xdd063d96), TM_UL(0x3e05aedd), TM_UL(0xe6bd464d),
    TM_UL(0x548db591), TM_UL(0xc45d0571), TM_UL(0x06d46f04), TM_UL(0x5015ff60),
    TM_UL(0x98fb2419), TM_UL(0xbde997d6), TM_UL(0x4043cc89), TM_UL(0xd99e7767),
    TM_UL(0xe842bdb0), TM_UL(0x898b8807), TM_UL(0x195b38e7), TM_UL(0xc8eedb79),
    TM_UL(0x7c0a47a1), TM_UL(0x420fe97c), TM_UL(0x841ec9f8), TM_UL(0x00000000),
    TM_UL(0x80868309), TM_UL(0x2bed4832), TM_UL(0x1170ac1e), TM_UL(0x5a724e6c),
    TM_UL(0x0efffbfd), TM_UL(0x8538560f), TM_UL(0xaed51e3d), TM_UL(0x2d392736),
    TM_UL(0x0fd9640a), TM_UL(0x5ca62168), TM_UL(0x5b54d19b), TM_UL(0x362e3a24),
    TM_UL(0x0a67b10c), TM_UL(0x57e70f93), TM_UL(0xee96d2b4), TM_UL(0x9b919e1b),
    TM_UL(0xc0c54f80), TM_UL(0xdc20a261), TM_UL(0x774b695a), TM_UL(0x121a161c),
    TM_UL(0x93ba0ae2), TM_UL(0xa02ae5c0), TM_UL(0x22e0433c), TM_UL(0x1b171d12),
    TM_UL(0x090d0b0e), TM_UL(0x8bc7adf2), TM_UL(0xb6a8b92d), TM_UL(0x1ea9c814),
    TM_UL(0xf1198557), TM_UL(0x75074caf), TM_UL(0x99ddbbee), TM_UL(0x7f60fda3),
    TM_UL(0x01269ff7), TM_UL(0x72f5bc5c), TM_UL(0x663bc544), TM_UL(0xfb7e345b),
    TM_UL(0x4329768b), TM_UL(0x23c6dccb), TM_UL(0xedfc68b6), TM_UL(0xe4f163b8),
    TM_UL(0x31dccad7), TM_UL(0x63851042), TM_UL(0x97224013), TM_UL(0xc6112084),
    TM_UL(0x4a247d85), TM_UL(0xbb3df8d2), TM_UL(0xf93211ae), TM_UL(0x29a16dc7),
    TM_UL(0x9e2f4b1d), TM_UL(0xb230f3dc), TM_UL(0x8652ec0d), TM_UL(0xc1e3d077),
    TM_UL(0xb3166c2b), TM_UL(0x70b999a9), TM_UL(0x9448fa11), TM_UL(0xe9642247),
    TM_UL(0xfc8cc4a8), TM_UL(0xf03f1aa0), TM_UL(0x7d2cd856), TM_UL(0x3390ef22),
    TM_UL(0x494ec787), TM_UL(0x38d1c1d9), TM_UL(0xcaa2fe8c), TM_UL(0xd40b3698),
    TM_UL(0xf581cfa6), TM_UL(0x7ade28a5), TM_UL(0xb78e26da), TM_UL(0xadbfa43f),
    TM_UL(0x3a9de42c), TM_UL(0x78920d50), TM_UL(0x5fcc9b6a), TM_UL(0x7e466254),
    TM_UL(0x8d13c2f6), TM_UL(0xd8b8e890), TM_UL(0x39f75e2e), TM_UL(0xc3aff582),
    TM_UL(0x5d80be9f), TM_UL(0xd0937c69), TM_UL(0xd52da96f), TM_UL(0x2512b3cf),
    TM_UL(0xac993bc8), TM_UL(0x187da710), TM_UL(0x9c636ee8), TM_UL(0x3bbb7bdb),
    TM_UL(0x267809cd), TM_UL(0x5918f46e), TM_UL(0x9ab701ec), TM_UL(0x4f9aa883),
    TM_UL(0x956e65e6), TM_UL(0xffe67eaa), TM_UL(0xbccf0821), TM_UL(0x15e8e6ef),
    TM_UL(0xe79bd9ba), TM_UL(0x6f36ce4a), TM_UL(0x9f09d4ea), TM_UL(0xb07cd629),
    TM_UL(0xa4b2af31), TM_UL(0x3f23312a), TM_UL(0xa59430c6), TM_UL(0xa266c035),
    TM_UL(0x4ebc3774), TM_UL(0x82caa6fc), TM_UL(0x90d0b0e0), TM_UL(0xa7d81533),
    TM_UL(0x04984af1), TM_UL(0xecdaf741), TM_UL(0xcd500e7f), TM_UL(0x91f62f17),
    TM_UL(0x4dd68d76), TM_UL(0xefb04d43), TM_UL(0xaa4d54cc), TM_UL(0x9604dfe4),
    TM_UL(0xd1b5e39e), TM_UL(0x6a881b4c), TM_UL(0x2c1fb8c1), TM_UL(0x65517f46),
    TM_UL(0x5eea049d), TM_UL(0x8c355d01), TM_UL(0x877473fa), TM_UL(0x0b412efb),
    TM_UL(0x671d5ab3), TM_UL(0xdbd25292), TM_UL(0x105633e9), TM_UL(0xd647136d),
    TM_UL(0xd7618c9a), TM_UL(0xa10c7a37), TM_UL(0xf8148e59), TM_UL(0x133c89eb),
    TM_UL(0xa927eece), TM_UL(0x61c935b7), TM_UL(0x1ce5ede1), TM_UL(0x47b13c7a),
    TM_UL(0xd2df599c), TM_UL(0xf2733f55), TM_UL(0x14ce7918), TM_UL(0xc737bf73),
    TM_UL(0xf7cdea53), TM_UL(0xfdaa5b5f), TM_UL(0x3d6f14df), TM_UL(0x44db8678),
    TM_UL(0xaff381ca), TM_UL(0x68c43eb9), TM_UL(0x24342c38), TM_UL(0xa3405fc2),
    TM_UL(0x1dc37216), TM_UL(0xe2250cbc), TM_UL(0x3c498b28), TM_UL(0x0d9541ff),
    TM_UL(0xa8017139), TM_UL(0x0cb3de08), TM_UL(0xb4e49cd8), TM_UL(0x56c19064),
    TM_UL(0xcb84617b), TM_UL(0x32b670d5), TM_UL(0x6c5c7448), TM_UL(0xb85742d0)
};
static const tt32Bit TM_CONST_QLF Td4[256] = {
    TM_UL(0x52525252), TM_UL(0x09090909), TM_UL(0x6a6a6a6a), TM_UL(0xd5d5d5d5),
    TM_UL(0x30303030), TM_UL(0x36363636), TM_UL(0xa5a5a5a5), TM_UL(0x38383838),
    TM_UL(0xbfbfbfbf), TM_UL(0x40404040), TM_UL(0xa3a3a3a3), TM_UL(0x9e9e9e9e),
    TM_UL(0x81818181), TM_UL(0xf3f3f3f3), TM_UL(0xd7d7d7d7), TM_UL(0xfbfbfbfb),
    TM_UL(0x7c7c7c7c), TM_UL(0xe3e3e3e3), TM_UL(0x39393939), TM_UL(0x82828282),
    TM_UL(0x9b9b9b9b), TM_UL(0x2f2f2f2f), TM_UL(0xffffffff), TM_UL(0x87878787),
    TM_UL(0x34343434), TM_UL(0x8e8e8e8e), TM_UL(0x43434343), TM_UL(0x44444444),
    TM_UL(0xc4c4c4c4), TM_UL(0xdededede), TM_UL(0xe9e9e9e9), TM_UL(0xcbcbcbcb),
    TM_UL(0x54545454), TM_UL(0x7b7b7b7b), TM_UL(0x94949494), TM_UL(0x32323232),
    TM_UL(0xa6a6a6a6), TM_UL(0xc2c2c2c2), TM_UL(0x23232323), TM_UL(0x3d3d3d3d),
    TM_UL(0xeeeeeeee), TM_UL(0x4c4c4c4c), TM_UL(0x95959595), TM_UL(0x0b0b0b0b),
    TM_UL(0x42424242), TM_UL(0xfafafafa), TM_UL(0xc3c3c3c3), TM_UL(0x4e4e4e4e),
    TM_UL(0x08080808), TM_UL(0x2e2e2e2e), TM_UL(0xa1a1a1a1), TM_UL(0x66666666),
    TM_UL(0x28282828), TM_UL(0xd9d9d9d9), TM_UL(0x24242424), TM_UL(0xb2b2b2b2),
    TM_UL(0x76767676), TM_UL(0x5b5b5b5b), TM_UL(0xa2a2a2a2), TM_UL(0x49494949),
    TM_UL(0x6d6d6d6d), TM_UL(0x8b8b8b8b), TM_UL(0xd1d1d1d1), TM_UL(0x25252525),
    TM_UL(0x72727272), TM_UL(0xf8f8f8f8), TM_UL(0xf6f6f6f6), TM_UL(0x64646464),
    TM_UL(0x86868686), TM_UL(0x68686868), TM_UL(0x98989898), TM_UL(0x16161616),
    TM_UL(0xd4d4d4d4), TM_UL(0xa4a4a4a4), TM_UL(0x5c5c5c5c), TM_UL(0xcccccccc),
    TM_UL(0x5d5d5d5d), TM_UL(0x65656565), TM_UL(0xb6b6b6b6), TM_UL(0x92929292),
    TM_UL(0x6c6c6c6c), TM_UL(0x70707070), TM_UL(0x48484848), TM_UL(0x50505050),
    TM_UL(0xfdfdfdfd), TM_UL(0xedededed), TM_UL(0xb9b9b9b9), TM_UL(0xdadadada),
    TM_UL(0x5e5e5e5e), TM_UL(0x15151515), TM_UL(0x46464646), TM_UL(0x57575757),
    TM_UL(0xa7a7a7a7), TM_UL(0x8d8d8d8d), TM_UL(0x9d9d9d9d), TM_UL(0x84848484),
    TM_UL(0x90909090), TM_UL(0xd8d8d8d8), TM_UL(0xabababab), TM_UL(0x00000000),
    TM_UL(0x8c8c8c8c), TM_UL(0xbcbcbcbc), TM_UL(0xd3d3d3d3), TM_UL(0x0a0a0a0a),
    TM_UL(0xf7f7f7f7), TM_UL(0xe4e4e4e4), TM_UL(0x58585858), TM_UL(0x05050505),
    TM_UL(0xb8b8b8b8), TM_UL(0xb3b3b3b3), TM_UL(0x45454545), TM_UL(0x06060606),
    TM_UL(0xd0d0d0d0), TM_UL(0x2c2c2c2c), TM_UL(0x1e1e1e1e), TM_UL(0x8f8f8f8f),
    TM_UL(0xcacacaca), TM_UL(0x3f3f3f3f), TM_UL(0x0f0f0f0f), TM_UL(0x02020202),
    TM_UL(0xc1c1c1c1), TM_UL(0xafafafaf), TM_UL(0xbdbdbdbd), TM_UL(0x03030303),
    TM_UL(0x01010101), TM_UL(0x13131313), TM_UL(0x8a8a8a8a), TM_UL(0x6b6b6b6b),
    TM_UL(0x3a3a3a3a), TM_UL(0x91919191), TM_UL(0x11111111), TM_UL(0x41414141),
    TM_UL(0x4f4f4f4f), TM_UL(0x67676767), TM_UL(0xdcdcdcdc), TM_UL(0xeaeaeaea),
    TM_UL(0x97979797), TM_UL(0xf2f2f2f2), TM_UL(0xcfcfcfcf), TM_UL(0xcececece),
    TM_UL(0xf0f0f0f0), TM_UL(0xb4b4b4b4), TM_UL(0xe6e6e6e6), TM_UL(0x73737373),
    TM_UL(0x96969696), TM_UL(0xacacacac), TM_UL(0x74747474), TM_UL(0x22222222),
    TM_UL(0xe7e7e7e7), TM_UL(0xadadadad), TM_UL(0x35353535), TM_UL(0x85858585),
    TM_UL(0xe2e2e2e2), TM_UL(0xf9f9f9f9), TM_UL(0x37373737), TM_UL(0xe8e8e8e8),
    TM_UL(0x1c1c1c1c), TM_UL(0x75757575), TM_UL(0xdfdfdfdf), TM_UL(0x6e6e6e6e),
    TM_UL(0x47474747), TM_UL(0xf1f1f1f1), TM_UL(0x1a1a1a1a), TM_UL(0x71717171),
    TM_UL(0x1d1d1d1d), TM_UL(0x29292929), TM_UL(0xc5c5c5c5), TM_UL(0x89898989),
    TM_UL(0x6f6f6f6f), TM_UL(0xb7b7b7b7), TM_UL(0x62626262), TM_UL(0x0e0e0e0e),
    TM_UL(0xaaaaaaaa), TM_UL(0x18181818), TM_UL(0xbebebebe), TM_UL(0x1b1b1b1b),
    TM_UL(0xfcfcfcfc), TM_UL(0x56565656), TM_UL(0x3e3e3e3e), TM_UL(0x4b4b4b4b),
    TM_UL(0xc6c6c6c6), TM_UL(0xd2d2d2d2), TM_UL(0x79797979), TM_UL(0x20202020),
    TM_UL(0x9a9a9a9a), TM_UL(0xdbdbdbdb), TM_UL(0xc0c0c0c0), TM_UL(0xfefefefe),
    TM_UL(0x78787878), TM_UL(0xcdcdcdcd), TM_UL(0x5a5a5a5a), TM_UL(0xf4f4f4f4),
    TM_UL(0x1f1f1f1f), TM_UL(0xdddddddd), TM_UL(0xa8a8a8a8), TM_UL(0x33333333),
    TM_UL(0x88888888), TM_UL(0x07070707), TM_UL(0xc7c7c7c7), TM_UL(0x31313131),
    TM_UL(0xb1b1b1b1), TM_UL(0x12121212), TM_UL(0x10101010), TM_UL(0x59595959),
    TM_UL(0x27272727), TM_UL(0x80808080), TM_UL(0xecececec), TM_UL(0x5f5f5f5f),
    TM_UL(0x60606060), TM_UL(0x51515151), TM_UL(0x7f7f7f7f), TM_UL(0xa9a9a9a9),
    TM_UL(0x19191919), TM_UL(0xb5b5b5b5), TM_UL(0x4a4a4a4a), TM_UL(0x0d0d0d0d),
    TM_UL(0x2d2d2d2d), TM_UL(0xe5e5e5e5), TM_UL(0x7a7a7a7a), TM_UL(0x9f9f9f9f),
    TM_UL(0x93939393), TM_UL(0xc9c9c9c9), TM_UL(0x9c9c9c9c), TM_UL(0xefefefef),
    TM_UL(0xa0a0a0a0), TM_UL(0xe0e0e0e0), TM_UL(0x3b3b3b3b), TM_UL(0x4d4d4d4d),
    TM_UL(0xaeaeaeae), TM_UL(0x2a2a2a2a), TM_UL(0xf5f5f5f5), TM_UL(0xb0b0b0b0),
    TM_UL(0xc8c8c8c8), TM_UL(0xebebebeb), TM_UL(0xbbbbbbbb), TM_UL(0x3c3c3c3c),
    TM_UL(0x83838383), TM_UL(0x53535353), TM_UL(0x99999999), TM_UL(0x61616161),
    TM_UL(0x17171717), TM_UL(0x2b2b2b2b), TM_UL(0x04040404), TM_UL(0x7e7e7e7e),
    TM_UL(0xbabababa), TM_UL(0x77777777), TM_UL(0xd6d6d6d6), TM_UL(0x26262626),
    TM_UL(0xe1e1e1e1), TM_UL(0x69696969), TM_UL(0x14141414), TM_UL(0x63636363),
    TM_UL(0x55555555), TM_UL(0x21212121), TM_UL(0x0c0c0c0c), TM_UL(0x7d7d7d7d)
};

static int tfAesMakeEncryptKey(
    tt8BitPtr           userKey,
    tt32Bit             bits,
    ttAesKeyInstancePtr key)
{
    tt32BitPtr  rk;
    int         i = 0;
    tt32Bit     temp;
    int         errorCode;

    errorCode = TM_ENOERROR;
    if (!userKey || !key)
    {
        errorCode = TM_EINVAL;
        goto aesMakeEncryptKeyExit;
    }

    if (bits != 128 && bits != 192 && bits != 256)
    {
        errorCode = TM_EINVAL;
        goto aesMakeEncryptKeyExit;
    }

    rk = key->rd_key;

    if (bits==128)
    {
        key->rounds = 10;
    }
    else if (bits==192)
    {
        key->rounds = 12;
    }
    else
    {
        key->rounds = 14;
    }

    rk[0] = tm_aes_getu32(userKey     );
    rk[1] = tm_aes_getu32(userKey +  4);
    rk[2] = tm_aes_getu32(userKey +  8);
    rk[3] = tm_aes_getu32(userKey + 12);
    if (bits == 128) 
    {
        for(;;)
        {
            temp  = rk[3];
            rk[4] = rk[0]
                ^ (Te4[tm_8bit(temp >> 16)] & TM_UL(0xff000000))
                ^ (Te4[tm_8bit(temp >>  8)] & TM_UL(0x00ff0000))
                ^ (Te4[tm_8bit(temp)] & TM_UL(0x0000ff00))
                ^ (Te4[tm_8bit(temp >> 24)] & TM_UL(0x000000ff))
                ^ tlAesRcon[i];
            rk[5] = rk[1] ^ rk[4];
            rk[6] = rk[2] ^ rk[5];
            rk[7] = rk[3] ^ rk[6];
            i++;
            if(i == 10)
            {
                goto aesMakeEncryptKeyExit;
            }
            rk += 4;
        }
    }
    rk[4] = tm_aes_getu32(userKey + 16);
    rk[5] = tm_aes_getu32(userKey + 20);
    if (bits == 192) 
    {
        for(;;) 
        {
            temp = rk[ 5];
            rk[ 6] = rk[ 0]
                ^ (Te4[tm_8bit(temp >> 16)] & TM_UL(0xff000000))
                ^ (Te4[tm_8bit(temp >>  8)] & TM_UL(0x00ff0000))
                ^ (Te4[tm_8bit(temp)] & TM_UL(0x0000ff00))
                ^ (Te4[tm_8bit(temp >> 24)] & TM_UL(0x000000ff))
                ^ tlAesRcon[i];
            rk[ 7] = rk[ 1] ^ rk[ 6];
            rk[ 8] = rk[ 2] ^ rk[ 7];
            rk[ 9] = rk[ 3] ^ rk[ 8];
            i++;
            if(i == 8) 
            {
                goto aesMakeEncryptKeyExit;
            }
            rk[10] = rk[ 4] ^ rk[ 9];
            rk[11] = rk[ 5] ^ rk[10];
            rk += 6;
        }
    }
    rk[6] = tm_aes_getu32(userKey + 24);
    rk[7] = tm_aes_getu32(userKey + 28);
    if (bits == 256) 
    {
        for(;;)
        {
            temp = rk[ 7];
            rk[ 8] = rk[ 0]
                ^ (Te4[tm_8bit(temp >> 16)] & TM_UL(0xff000000))
                ^ (Te4[tm_8bit(temp >>  8)] & TM_UL(0x00ff0000))
                ^ (Te4[tm_8bit(temp)] & TM_UL(0x0000ff00))
                ^ (Te4[tm_8bit(temp >> 24)] & TM_UL(0x000000ff))
                ^ tlAesRcon[i];
            rk[ 9] = rk[ 1] ^ rk[ 8];
            rk[10] = rk[ 2] ^ rk[ 9];
            rk[11] = rk[ 3] ^ rk[10];
            i++;
            if (i == 7) 
            {
                goto aesMakeEncryptKeyExit;
            }
            temp = rk[11];
            rk[12] = rk[ 4]
                ^ (Te4[tm_8bit(temp >> 24)] & TM_UL(0xff000000))
                ^ (Te4[tm_8bit(temp >> 16)] & TM_UL(0x00ff0000))
                ^ (Te4[tm_8bit(temp >>  8)] & TM_UL(0x0000ff00))
                ^ (Te4[tm_8bit(temp)] & TM_UL(0x000000ff));
            rk[13] = rk[ 5] ^ rk[12];
            rk[14] = rk[ 6] ^ rk[13];
            rk[15] = rk[ 7] ^ rk[14];

            rk += 8;
        }
    }
aesMakeEncryptKeyExit:
    return errorCode;
}


static int tfAesMakeDecryptKey(
    tt8BitPtr           userKey,
    tt32Bit             bits,
    ttAesKeyInstancePtr key)
{
    tt32BitPtr    rk;
    int           i;
    int           j;
    int           errorCode;
    tt32Bit       temp;

/* first, start with an encryption schedule */
    errorCode = tfAesMakeEncryptKey(userKey, bits, key);
    if (errorCode != TM_ENOERROR)
    {
        goto aesMakeDecryptKeyExit;
    }

    rk = key->rd_key;

/* invert the order of the round keys: */
    for (i = 0, j = 4*(key->rounds); i < j; i += 4, j -= 4) 
    {
        temp = rk[i    ]; rk[i    ] = rk[j    ]; rk[j    ] = temp;
        temp = rk[i + 1]; rk[i + 1] = rk[j + 1]; rk[j + 1] = temp;
        temp = rk[i + 2]; rk[i + 2] = rk[j + 2]; rk[j + 2] = temp;
        temp = rk[i + 3]; rk[i + 3] = rk[j + 3]; rk[j + 3] = temp;
    }
/* apply the inverse MixColumn transform to all round keys but the 
 * first and the last: 
 */
    for (i = 1; i < (key->rounds); i++) 
    {
        rk += 4;
        rk[0] =
            Td0[tm_8bit(Te4[tm_8bit(rk[0] >> 24)])] ^
            Td1[tm_8bit(Te4[tm_8bit(rk[0] >> 16)])] ^
            Td2[tm_8bit(Te4[tm_8bit(rk[0] >>  8)])] ^
            Td3[tm_8bit(Te4[tm_8bit(rk[0])])];
        rk[1] =
            Td0[tm_8bit(Te4[tm_8bit(rk[1] >> 24)])] ^
            Td1[tm_8bit(Te4[tm_8bit(rk[1] >> 16)])] ^
            Td2[tm_8bit(Te4[tm_8bit(rk[1] >>  8)])] ^
            Td3[tm_8bit(Te4[tm_8bit(rk[1])])];
        rk[2] =
            Td0[tm_8bit(Te4[tm_8bit(rk[2] >> 24)])] ^
            Td1[tm_8bit(Te4[tm_8bit(rk[2] >> 16)])] ^
            Td2[tm_8bit(Te4[tm_8bit(rk[2] >>  8)])] ^
            Td3[tm_8bit(Te4[tm_8bit(rk[2])])];
        rk[3] =
            Td0[tm_8bit(Te4[tm_8bit(rk[3] >> 24)])] ^
            Td1[tm_8bit(Te4[tm_8bit(rk[3] >> 16)])] ^
            Td2[tm_8bit(Te4[tm_8bit(rk[3] >>  8)])] ^
            Td3[tm_8bit(Te4[tm_8bit(rk[3])])];
    }
aesMakeDecryptKeyExit:
    return errorCode;
}


int tfAesBlockEncrypt(ttAesKeyInstancePtr key,
                      tt8BitPtr           in, 
                      tt8BitPtr           out) 
{
    const tt32Bit TM_FAR *  rk;
    tt32Bit                 s0, s1, s2, s3, t0, t1, t2, t3;
    int                     errorCode;
#if (!defined(TM_OPTIMIZE_SPEED) || defined(TM_OPTIMIZE_SIZE))
    int                     r;
#endif /*!TM_OPTIMIZE_SPEED || TM_OPTIMIZE_SIZE */
    errorCode = TM_ENOERROR;
    
    if(!in || !out || !key)
    {
        errorCode = TM_EINVAL;
        goto aesBlockEncryptExit;
    }

    rk = key->rd_key;

/*
 * map byte array block to cipher state
 * and add initial round key:
 */
    s0 = tm_aes_getu32(in     ) ^ rk[0];
    s1 = tm_aes_getu32(in +  4) ^ rk[1];
    s2 = tm_aes_getu32(in +  8) ^ rk[2];
    s3 = tm_aes_getu32(in + 12) ^ rk[3];
#if (defined(TM_OPTIMIZE_SPEED) && !defined(TM_OPTIMIZE_SIZE))
/* round 1: */
    t0 = Te0[tm_8bit(s0 >> 24)] ^ Te1[tm_8bit(s1 >> 16)] ^ 
         Te2[tm_8bit(s2 >>  8)] ^ Te3[tm_8bit(s3)] ^ rk[ 4];
    t1 = Te0[tm_8bit(s1 >> 24)] ^ Te1[tm_8bit(s2 >> 16)] ^ 
         Te2[tm_8bit(s3 >>  8)] ^ Te3[tm_8bit(s0)] ^ rk[ 5];
    t2 = Te0[tm_8bit(s2 >> 24)] ^ Te1[tm_8bit(s3 >> 16)] ^ 
         Te2[tm_8bit(s0 >>  8)] ^ Te3[tm_8bit(s1)] ^ rk[ 6];
    t3 = Te0[tm_8bit(s3 >> 24)] ^ Te1[tm_8bit(s0 >> 16)] ^ 
         Te2[tm_8bit(s1 >>  8)] ^ Te3[tm_8bit(s2)] ^ rk[ 7];
/* round 2: */
    s0 = Te0[tm_8bit(t0 >> 24)] ^ Te1[tm_8bit(t1 >> 16)] ^ 
         Te2[tm_8bit(t2 >>  8)] ^ Te3[tm_8bit(t3)] ^ rk[ 8];
    s1 = Te0[tm_8bit(t1 >> 24)] ^ Te1[tm_8bit(t2 >> 16)] ^ 
         Te2[tm_8bit(t3 >>  8)] ^ Te3[tm_8bit(t0)] ^ rk[ 9];
    s2 = Te0[tm_8bit(t2 >> 24)] ^ Te1[tm_8bit(t3 >> 16)] ^ 
         Te2[tm_8bit(t0 >>  8)] ^ Te3[tm_8bit(t1)] ^ rk[10];
    s3 = Te0[tm_8bit(t3 >> 24)] ^ Te1[tm_8bit(t0 >> 16)] ^ 
         Te2[tm_8bit(t1 >>  8)] ^ Te3[tm_8bit(t2)] ^ rk[11];
/* round 3: */
    t0 = Te0[tm_8bit(s0 >> 24)] ^ Te1[tm_8bit(s1 >> 16)] ^ 
         Te2[tm_8bit(s2 >>  8)] ^ Te3[tm_8bit(s3)] ^ rk[12];
    t1 = Te0[tm_8bit(s1 >> 24)] ^ Te1[tm_8bit(s2 >> 16)] ^ 
         Te2[tm_8bit(s3 >>  8)] ^ Te3[tm_8bit(s0)] ^ rk[13];
    t2 = Te0[tm_8bit(s2 >> 24)] ^ Te1[tm_8bit(s3 >> 16)] ^ 
         Te2[tm_8bit(s0 >>  8)] ^ Te3[tm_8bit(s1)] ^ rk[14];
    t3 = Te0[tm_8bit(s3 >> 24)] ^ Te1[tm_8bit(s0 >> 16)] ^ 
         Te2[tm_8bit(s1 >>  8)] ^ Te3[tm_8bit(s2)] ^ rk[15];
/* round 4: */
    s0 = Te0[tm_8bit(t0 >> 24)] ^ Te1[tm_8bit(t1 >> 16)] ^ 
         Te2[tm_8bit(t2 >>  8)] ^ Te3[tm_8bit(t3)] ^ rk[16];
    s1 = Te0[tm_8bit(t1 >> 24)] ^ Te1[tm_8bit(t2 >> 16)] ^ 
         Te2[tm_8bit(t3 >>  8)] ^ Te3[tm_8bit(t0)] ^ rk[17];
    s2 = Te0[tm_8bit(t2 >> 24)] ^ Te1[tm_8bit(t3 >> 16)] ^ 
         Te2[tm_8bit(t0 >>  8)] ^ Te3[tm_8bit(t1)] ^ rk[18];
    s3 = Te0[tm_8bit(t3 >> 24)] ^ Te1[tm_8bit(t0 >> 16)] ^ 
         Te2[tm_8bit(t1 >>  8)] ^ Te3[tm_8bit(t2)] ^ rk[19];
/* round 5: */
    t0 = Te0[tm_8bit(s0 >> 24)] ^ Te1[tm_8bit(s1 >> 16)] ^ 
         Te2[tm_8bit(s2 >>  8)] ^ Te3[tm_8bit(s3)] ^ rk[20];
    t1 = Te0[tm_8bit(s1 >> 24)] ^ Te1[tm_8bit(s2 >> 16)] ^ 
         Te2[tm_8bit(s3 >>  8)] ^ Te3[tm_8bit(s0)] ^ rk[21];
    t2 = Te0[tm_8bit(s2 >> 24)] ^ Te1[tm_8bit(s3 >> 16)] ^ 
         Te2[tm_8bit(s0 >>  8)] ^ Te3[tm_8bit(s1)] ^ rk[22];
    t3 = Te0[tm_8bit(s3 >> 24)] ^ Te1[tm_8bit(s0 >> 16)] ^ 
         Te2[tm_8bit(s1 >>  8)] ^ Te3[tm_8bit(s2)] ^ rk[23];
/* round 6: */
    s0 = Te0[tm_8bit(t0 >> 24)] ^ Te1[tm_8bit(t1 >> 16)] ^ 
         Te2[tm_8bit(t2 >>  8)] ^ Te3[tm_8bit(t3)] ^ rk[24];
    s1 = Te0[tm_8bit(t1 >> 24)] ^ Te1[tm_8bit(t2 >> 16)] ^ 
         Te2[tm_8bit(t3 >>  8)] ^ Te3[tm_8bit(t0)] ^ rk[25];
    s2 = Te0[tm_8bit(t2 >> 24)] ^ Te1[tm_8bit(t3 >> 16)] ^ 
         Te2[tm_8bit(t0 >>  8)] ^ Te3[tm_8bit(t1)] ^ rk[26];
    s3 = Te0[tm_8bit(t3 >> 24)] ^ Te1[tm_8bit(t0 >> 16)] ^ 
         Te2[tm_8bit(t1 >>  8)] ^ Te3[tm_8bit(t2)] ^ rk[27];
/* round 7: */
    t0 = Te0[tm_8bit(s0 >> 24)] ^ Te1[tm_8bit(s1 >> 16)] ^ 
         Te2[tm_8bit(s2 >>  8)] ^ Te3[tm_8bit(s3)] ^ rk[28];
    t1 = Te0[tm_8bit(s1 >> 24)] ^ Te1[tm_8bit(s2 >> 16)] ^ 
         Te2[tm_8bit(s3 >>  8)] ^ Te3[tm_8bit(s0)] ^ rk[29];
    t2 = Te0[tm_8bit(s2 >> 24)] ^ Te1[tm_8bit(s3 >> 16)] ^ 
         Te2[tm_8bit(s0 >>  8)] ^ Te3[tm_8bit(s1)] ^ rk[30];
    t3 = Te0[tm_8bit(s3 >> 24)] ^ Te1[tm_8bit(s0 >> 16)] ^ 
         Te2[tm_8bit(s1 >>  8)] ^ Te3[tm_8bit(s2)] ^ rk[31];
/* round 8: */
    s0 = Te0[tm_8bit(t0 >> 24)] ^ Te1[tm_8bit(t1 >> 16)] ^ 
         Te2[tm_8bit(t2 >>  8)] ^ Te3[tm_8bit(t3)] ^ rk[32];
    s1 = Te0[tm_8bit(t1 >> 24)] ^ Te1[tm_8bit(t2 >> 16)] ^ 
         Te2[tm_8bit(t3 >>  8)] ^ Te3[tm_8bit(t0)] ^ rk[33];
    s2 = Te0[tm_8bit(t2 >> 24)] ^ Te1[tm_8bit(t3 >> 16)] ^ 
         Te2[tm_8bit(t0 >>  8)] ^ Te3[tm_8bit(t1)] ^ rk[34];
    s3 = Te0[tm_8bit(t3 >> 24)] ^ Te1[tm_8bit(t0 >> 16)] ^ 
         Te2[tm_8bit(t1 >>  8)] ^ Te3[tm_8bit(t2)] ^ rk[35];
/* round 9: */
    t0 = Te0[tm_8bit(s0 >> 24)] ^ Te1[tm_8bit(s1 >> 16)] ^ 
         Te2[tm_8bit(s2 >>  8)] ^ Te3[tm_8bit(s3)] ^ rk[36];
    t1 = Te0[tm_8bit(s1 >> 24)] ^ Te1[tm_8bit(s2 >> 16)] ^ 
         Te2[tm_8bit(s3 >>  8)] ^ Te3[tm_8bit(s0)] ^ rk[37];
    t2 = Te0[tm_8bit(s2 >> 24)] ^ Te1[tm_8bit(s3 >> 16)] ^ 
         Te2[tm_8bit(s0 >>  8)] ^ Te3[tm_8bit(s1)] ^ rk[38];
    t3 = Te0[tm_8bit(s3 >> 24)] ^ Te1[tm_8bit(s0 >> 16)] ^ 
         Te2[tm_8bit(s1 >>  8)] ^ Te3[tm_8bit(s2)] ^ rk[39];
    if (key->rounds > 10) 
    {
/* round 10: */
        s0 = Te0[tm_8bit(t0 >> 24)] ^ Te1[tm_8bit(t1 >> 16)] ^ 
             Te2[tm_8bit(t2 >>  8)] ^ Te3[tm_8bit(t3)] ^ rk[40];
        s1 = Te0[tm_8bit(t1 >> 24)] ^ Te1[tm_8bit(t2 >> 16)] ^ 
             Te2[tm_8bit(t3 >>  8)] ^ Te3[tm_8bit(t0)] ^ rk[41];
        s2 = Te0[tm_8bit(t2 >> 24)] ^ Te1[tm_8bit(t3 >> 16)] ^ 
             Te2[tm_8bit(t0 >>  8)] ^ Te3[tm_8bit(t1)] ^ rk[42];
        s3 = Te0[tm_8bit(t3 >> 24)] ^ Te1[tm_8bit(t0 >> 16)] ^ 
             Te2[tm_8bit(t1 >>  8)] ^ Te3[tm_8bit(t2)] ^ rk[43];
/* round 11: */
        t0 = Te0[tm_8bit(s0 >> 24)] ^ Te1[tm_8bit(s1 >> 16)] ^ 
             Te2[tm_8bit(s2 >>  8)] ^ Te3[tm_8bit(s3)] ^ rk[44];
        t1 = Te0[tm_8bit(s1 >> 24)] ^ Te1[tm_8bit(s2 >> 16)] ^ 
             Te2[tm_8bit(s3 >>  8)] ^ Te3[tm_8bit(s0)] ^ rk[45];
        t2 = Te0[tm_8bit(s2 >> 24)] ^ Te1[tm_8bit(s3 >> 16)] ^ 
             Te2[tm_8bit(s0 >>  8)] ^ Te3[tm_8bit(s1)] ^ rk[46];
        t3 = Te0[tm_8bit(s3 >> 24)] ^ Te1[tm_8bit(s0 >> 16)] ^ 
             Te2[tm_8bit(s1 >>  8)] ^ Te3[tm_8bit(s2)] ^ rk[47];
        if (key->rounds > 12) 
        {
/* round 12: */
            s0 = Te0[tm_8bit(t0 >> 24)] ^ Te1[tm_8bit(t1 >> 16)] ^ 
                 Te2[tm_8bit(t2 >>  8)] ^ Te3[tm_8bit(t3)] ^ rk[48];
            s1 = Te0[tm_8bit(t1 >> 24)] ^ Te1[tm_8bit(t2 >> 16)] ^ 
                 Te2[tm_8bit(t3 >>  8)] ^ Te3[tm_8bit(t0)] ^ rk[49];
            s2 = Te0[tm_8bit(t2 >> 24)] ^ Te1[tm_8bit(t3 >> 16)] ^  
                 Te2[tm_8bit(t0 >>  8)] ^ Te3[tm_8bit(t1)] ^ rk[50];
            s3 = Te0[tm_8bit(t3 >> 24)] ^ Te1[tm_8bit(t0 >> 16)] ^ 
                 Te2[tm_8bit(t1 >>  8)] ^ Te3[tm_8bit(t2)] ^ rk[51];
/* round 13: */
            t0 = Te0[tm_8bit(s0 >> 24)] ^ Te1[tm_8bit(s1 >> 16)] ^ 
                 Te2[tm_8bit(s2 >>  8)] ^ Te3[tm_8bit(s3)] ^ rk[52];
            t1 = Te0[tm_8bit(s1 >> 24)] ^ Te1[tm_8bit(s2 >> 16)] ^ 
                 Te2[tm_8bit(s3 >>  8)] ^ Te3[tm_8bit(s0)] ^ rk[53];
            t2 = Te0[tm_8bit(s2 >> 24)] ^ Te1[tm_8bit(s3 >> 16)] ^ 
                 Te2[tm_8bit(s0 >>  8)] ^ Te3[tm_8bit(s1)] ^ rk[54];
            t3 = Te0[tm_8bit(s3 >> 24)] ^ Te1[tm_8bit(s0 >> 16)] ^ 
                 Te2[tm_8bit(s1 >>  8)] ^ Te3[tm_8bit(s2)] ^ rk[55];
        }
    }
    rk += key->rounds << 2;
#else  /* !TM_OPTIMIZE_SPEED || TM_OPTIMIZE_SIZE*/
/*
 * Nr - 1 full rounds:
 */
    r = key->rounds >> 1;
    for (;;) 
    {
        t0 =
            Te0[tm_8bit(s0 >> 24)] ^
            Te1[tm_8bit(s1 >> 16)] ^
            Te2[tm_8bit(s2 >>  8)] ^
            Te3[tm_8bit(s3)] ^
            rk[4];
        t1 =
            Te0[tm_8bit(s1 >> 24)] ^
            Te1[tm_8bit(s2 >> 16)] ^
            Te2[tm_8bit(s3 >>  8)] ^
            Te3[tm_8bit(s0)] ^
            rk[5];
        t2 =
            Te0[tm_8bit(s2 >> 24)] ^
            Te1[tm_8bit(s3 >> 16)] ^
            Te2[tm_8bit(s0 >>  8)] ^
            Te3[tm_8bit(s1)] ^
            rk[6];
        t3 =
            Te0[tm_8bit(s3 >> 24)] ^
            Te1[tm_8bit(s0 >> 16)] ^
            Te2[tm_8bit(s1 >>  8)] ^
            Te3[tm_8bit(s2)] ^
            rk[7];

        rk += 8;
        r--;
        if (r == 0) 
        {
            break;
        }

        s0 =
            Te0[tm_8bit(t0 >> 24)       ] ^
            Te1[tm_8bit(t1 >> 16)] ^
            Te2[tm_8bit(t2 >>  8)] ^
            Te3[tm_8bit(t3)] ^
            rk[0];
        s1 =
            Te0[tm_8bit(t1 >> 24)       ] ^
            Te1[tm_8bit(t2 >> 16)] ^
            Te2[tm_8bit(t3 >>  8)] ^
            Te3[tm_8bit(t0)] ^
            rk[1];
        s2 =
            Te0[tm_8bit(t2 >> 24)       ] ^
            Te1[tm_8bit(t3 >> 16)] ^
            Te2[tm_8bit(t0 >>  8)] ^
            Te3[tm_8bit(t1)] ^
            rk[2];
        s3 =
            Te0[tm_8bit(t3 >> 24)       ] ^
            Te1[tm_8bit(t0 >> 16)] ^
            Te2[tm_8bit(t1 >>  8)] ^
            Te3[tm_8bit(t2)] ^
            rk[3];
    }
#endif /* !TM_OPTIMIZE_SPEED || TM_OPTIMIZE_SIZE*/
/*
 * apply last round and
 * map cipher state to byte array block:
 */
    s0 = (Te4[tm_8bit(t0 >> 24)] & TM_UL(0xff000000))
        ^ (Te4[tm_8bit(t1 >> 16)] & TM_UL(0x00ff0000))
        ^ (Te4[tm_8bit(t2 >>  8)] & TM_UL(0x0000ff00))
        ^ (Te4[tm_8bit(t3)] & TM_UL(0x000000ff))
        ^ rk[0];
    tm_aes_putu32(out, s0);
    s1 = (Te4[tm_8bit(t1 >> 24)] & TM_UL(0xff000000))
        ^ (Te4[tm_8bit(t2 >> 16)] & TM_UL(0x00ff0000))
        ^ (Te4[tm_8bit(t3 >>  8)] & TM_UL(0x0000ff00))
        ^ (Te4[tm_8bit(t0)] & TM_UL(0x000000ff))
        ^ rk[1];
    tm_aes_putu32(out +  4, s1);
    s2 = (Te4[tm_8bit(t2 >> 24)] & TM_UL(0xff000000))
        ^ (Te4[tm_8bit(t3 >> 16)] & TM_UL(0x00ff0000))
        ^ (Te4[tm_8bit(t0 >>  8)] & TM_UL(0x0000ff00))
        ^ (Te4[tm_8bit(t1)] & TM_UL(0x000000ff))
        ^ rk[2];
    tm_aes_putu32(out +  8, s2);
    s3 = (Te4[tm_8bit(t3 >> 24)] & TM_UL(0xff000000))
        ^ (Te4[tm_8bit(t0 >> 16)] & TM_UL(0x00ff0000))
        ^ (Te4[tm_8bit(t1 >>  8)] & TM_UL(0x0000ff00))
        ^ (Te4[tm_8bit(t2)] & TM_UL(0x000000ff))
        ^ rk[3];
    tm_aes_putu32(out + 12, s3);
aesBlockEncryptExit:
    return errorCode;

}

int tfAesBlockDecrypt(ttAesKeyInstancePtr key,
                      tt8BitPtr           in, 
                      tt8BitPtr           out) 
{

    const tt32Bit TM_FAR * rk;
    tt32Bit                s0, s1, s2, s3, t0, t1, t2, t3;
    int                    errorCode;
#if (!defined(TM_OPTIMIZE_SPEED) || defined(TM_OPTIMIZE_SIZE))
    int                    r;
#endif /*!TM_OPTIMIZE_SPEED || TM_OPTIMIZE_SIZE */

    errorCode = TM_ENOERROR;
    if(!in || !out || !key)
    {
        errorCode = TM_EINVAL;
        goto aesBlockDecryptExit;
    }
    rk = key->rd_key;

/*
 * map byte array block to cipher state
 * and add initial round key:
 */
    s0 = tm_aes_getu32(in     ) ^ rk[0];
    s1 = tm_aes_getu32(in +  4) ^ rk[1];
    s2 = tm_aes_getu32(in +  8) ^ rk[2];
    s3 = tm_aes_getu32(in + 12) ^ rk[3];
#if (defined(TM_OPTIMIZE_SPEED) && !defined(TM_OPTIMIZE_SIZE))
/* round 1: */
    t0 = Td0[tm_8bit(s0 >> 24)] ^ Td1[tm_8bit(s3 >> 16)] ^ 
         Td2[tm_8bit(s2 >>  8)] ^ Td3[tm_8bit(s1)] ^ rk[ 4];
    t1 = Td0[tm_8bit(s1 >> 24)] ^ Td1[tm_8bit(s0 >> 16)] ^ 
        Td2[tm_8bit(s3 >>  8)] ^ Td3[tm_8bit(s2)] ^ rk[ 5];
    t2 = Td0[tm_8bit(s2 >> 24)] ^ Td1[tm_8bit(s1 >> 16)] ^ 
        Td2[tm_8bit(s0 >>  8)] ^ Td3[tm_8bit(s3)] ^ rk[ 6];
    t3 = Td0[tm_8bit(s3 >> 24)] ^ Td1[tm_8bit(s2 >> 16)] ^ 
        Td2[tm_8bit(s1 >>  8)] ^ Td3[tm_8bit(s0)] ^ rk[ 7];
/* round 2: */
    s0 = Td0[tm_8bit(t0 >> 24)] ^ Td1[tm_8bit(t3 >> 16)] ^ 
        Td2[tm_8bit(t2 >>  8)] ^ Td3[tm_8bit(t1)] ^ rk[ 8];
    s1 = Td0[tm_8bit(t1 >> 24)] ^ Td1[tm_8bit(t0 >> 16)] ^ 
        Td2[tm_8bit(t3 >>  8)] ^ Td3[tm_8bit(t2)] ^ rk[ 9];
    s2 = Td0[tm_8bit(t2 >> 24)] ^ Td1[tm_8bit(t1 >> 16)] ^ 
        Td2[tm_8bit(t0 >>  8)] ^ Td3[tm_8bit(t3)] ^ rk[10];
    s3 = Td0[tm_8bit(t3 >> 24)] ^ Td1[tm_8bit(t2 >> 16)] ^ 
        Td2[tm_8bit(t1 >>  8)] ^ Td3[tm_8bit(t0)] ^ rk[11];
/* round 3: */
    t0 = Td0[tm_8bit(s0 >> 24)] ^ Td1[tm_8bit(s3 >> 16)] ^ 
        Td2[tm_8bit(s2 >>  8)] ^ Td3[tm_8bit(s1)] ^ rk[12];
    t1 = Td0[tm_8bit(s1 >> 24)] ^ Td1[tm_8bit(s0 >> 16)] ^ 
        Td2[tm_8bit(s3 >>  8)] ^ Td3[tm_8bit(s2)] ^ rk[13];
    t2 = Td0[tm_8bit(s2 >> 24)] ^ Td1[tm_8bit(s1 >> 16)] ^ 
        Td2[tm_8bit(s0 >>  8)] ^ Td3[tm_8bit(s3)] ^ rk[14];
    t3 = Td0[tm_8bit(s3 >> 24)] ^ Td1[tm_8bit(s2 >> 16)] ^ 
        Td2[tm_8bit(s1 >>  8)] ^ Td3[tm_8bit(s0)] ^ rk[15];
/* round 4: */
    s0 = Td0[tm_8bit(t0 >> 24)] ^ Td1[tm_8bit(t3 >> 16)] ^ 
        Td2[tm_8bit(t2 >>  8)] ^ Td3[tm_8bit(t1)] ^ rk[16];
    s1 = Td0[tm_8bit(t1 >> 24)] ^ Td1[tm_8bit(t0 >> 16)] ^ 
        Td2[tm_8bit(t3 >>  8)] ^ Td3[tm_8bit(t2)] ^ rk[17];
    s2 = Td0[tm_8bit(t2 >> 24)] ^ Td1[tm_8bit(t1 >> 16)] ^ 
        Td2[tm_8bit(t0 >>  8)] ^ Td3[tm_8bit(t3)] ^ rk[18];
    s3 = Td0[tm_8bit(t3 >> 24)] ^ Td1[tm_8bit(t2 >> 16)] ^ 
        Td2[tm_8bit(t1 >>  8)] ^ Td3[tm_8bit(t0)] ^ rk[19];
/* round 5: */
    t0 = Td0[tm_8bit(s0 >> 24)] ^ Td1[tm_8bit(s3 >> 16)] ^ 
        Td2[tm_8bit(s2 >>  8)] ^ Td3[tm_8bit(s1)] ^ rk[20];
    t1 = Td0[tm_8bit(s1 >> 24)] ^ Td1[tm_8bit(s0 >> 16)] ^ 
        Td2[tm_8bit(s3 >>  8)] ^ Td3[tm_8bit(s2)] ^ rk[21];
    t2 = Td0[tm_8bit(s2 >> 24)] ^ Td1[tm_8bit(s1 >> 16)] ^ 
        Td2[tm_8bit(s0 >>  8)] ^ Td3[tm_8bit(s3)] ^ rk[22];
    t3 = Td0[tm_8bit(s3 >> 24)] ^ Td1[tm_8bit(s2 >> 16)] ^ 
        Td2[tm_8bit(s1 >>  8)] ^ Td3[tm_8bit(s0)] ^ rk[23];
/* round 6: */
    s0 = Td0[tm_8bit(t0 >> 24)] ^ Td1[tm_8bit(t3 >> 16)] ^ 
        Td2[tm_8bit(t2 >>  8)] ^ Td3[tm_8bit(t1)] ^ rk[24];
    s1 = Td0[tm_8bit(t1 >> 24)] ^ Td1[tm_8bit(t0 >> 16)] ^ 
        Td2[tm_8bit(t3 >>  8)] ^ Td3[tm_8bit(t2)] ^ rk[25];
    s2 = Td0[tm_8bit(t2 >> 24)] ^ Td1[tm_8bit(t1 >> 16)] ^ 
        Td2[tm_8bit(t0 >>  8)] ^ Td3[tm_8bit(t3)] ^ rk[26];
    s3 = Td0[tm_8bit(t3 >> 24)] ^ Td1[tm_8bit(t2 >> 16)] ^ 
        Td2[tm_8bit(t1 >>  8)] ^ Td3[tm_8bit(t0)] ^ rk[27];
/* round 7: */
    t0 = Td0[tm_8bit(s0 >> 24)] ^ Td1[tm_8bit(s3 >> 16)] ^ 
        Td2[tm_8bit(s2 >>  8)] ^ Td3[tm_8bit(s1)] ^ rk[28];
    t1 = Td0[tm_8bit(s1 >> 24)] ^ Td1[tm_8bit(s0 >> 16)] ^ 
        Td2[tm_8bit(s3 >>  8)] ^ Td3[tm_8bit(s2)] ^ rk[29];
    t2 = Td0[tm_8bit(s2 >> 24)] ^ Td1[tm_8bit(s1 >> 16)] ^ 
        Td2[tm_8bit(s0 >>  8)] ^ Td3[tm_8bit(s3)] ^ rk[30];
    t3 = Td0[tm_8bit(s3 >> 24)] ^ Td1[tm_8bit(s2 >> 16)] ^ 
        Td2[tm_8bit(s1 >>  8)] ^ Td3[tm_8bit(s0)] ^ rk[31];
/* round 8: */
    s0 = Td0[tm_8bit(t0 >> 24)] ^ Td1[tm_8bit(t3 >> 16)] ^ 
        Td2[tm_8bit(t2 >>  8)] ^ Td3[tm_8bit(t1)] ^ rk[32];
    s1 = Td0[tm_8bit(t1 >> 24)] ^ Td1[tm_8bit(t0 >> 16)] ^ 
        Td2[tm_8bit(t3 >>  8)] ^ Td3[tm_8bit(t2)] ^ rk[33];
    s2 = Td0[tm_8bit(t2 >> 24)] ^ Td1[tm_8bit(t1 >> 16)] ^ 
        Td2[tm_8bit(t0 >>  8)] ^ Td3[tm_8bit(t3)] ^ rk[34];
    s3 = Td0[tm_8bit(t3 >> 24)] ^ Td1[tm_8bit(t2 >> 16)] ^ 
        Td2[tm_8bit(t1 >>  8)] ^ Td3[tm_8bit(t0)] ^ rk[35];
/* round 9: */
    t0 = Td0[tm_8bit(s0 >> 24)] ^ Td1[tm_8bit(s3 >> 16)] ^ 
        Td2[tm_8bit(s2 >>  8)] ^ Td3[tm_8bit(s1)] ^ rk[36];
    t1 = Td0[tm_8bit(s1 >> 24)] ^ Td1[tm_8bit(s0 >> 16)] ^ 
        Td2[tm_8bit(s3 >>  8)] ^ Td3[tm_8bit(s2)] ^ rk[37];
    t2 = Td0[tm_8bit(s2 >> 24)] ^ Td1[tm_8bit(s1 >> 16)] ^ 
        Td2[tm_8bit(s0 >>  8)] ^ Td3[tm_8bit(s3)] ^ rk[38];
    t3 = Td0[tm_8bit(s3 >> 24)] ^ Td1[tm_8bit(s2 >> 16)] ^ 
        Td2[tm_8bit(s1 >>  8)] ^ Td3[tm_8bit(s0)] ^ rk[39];
    if (key->rounds > 10) 
    {
/* round 10: */
        s0 = Td0[tm_8bit(t0 >> 24)] ^ Td1[tm_8bit(t3 >> 16)] ^ 
            Td2[tm_8bit(t2 >>  8)] ^ Td3[tm_8bit(t1)] ^ rk[40];
        s1 = Td0[tm_8bit(t1 >> 24)] ^ Td1[tm_8bit(t0 >> 16)] ^ 
            Td2[tm_8bit(t3 >>  8)] ^ Td3[tm_8bit(t2)] ^ rk[41];
        s2 = Td0[tm_8bit(t2 >> 24)] ^ Td1[tm_8bit(t1 >> 16)] ^ 
            Td2[tm_8bit(t0 >>  8)] ^ Td3[tm_8bit(t3)] ^ rk[42];
        s3 = Td0[tm_8bit(t3 >> 24)] ^ Td1[tm_8bit(t2 >> 16)] ^ 
            Td2[tm_8bit(t1 >>  8)] ^ Td3[tm_8bit(t0)] ^ rk[43];
/* round 11: */
        t0 = Td0[tm_8bit(s0 >> 24)] ^ Td1[tm_8bit(s3 >> 16)] ^ 
            Td2[tm_8bit(s2 >>  8)] ^ Td3[tm_8bit(s1)] ^ rk[44];
        t1 = Td0[tm_8bit(s1 >> 24)] ^ Td1[tm_8bit(s0 >> 16)] ^ 
            Td2[tm_8bit(s3 >>  8)] ^ Td3[tm_8bit(s2)] ^ rk[45];
        t2 = Td0[tm_8bit(s2 >> 24)] ^ Td1[tm_8bit(s1 >> 16)] ^ 
            Td2[tm_8bit(s0 >>  8)] ^ Td3[tm_8bit(s3)] ^ rk[46];
        t3 = Td0[tm_8bit(s3 >> 24)] ^ Td1[tm_8bit(s2 >> 16)] ^ 
            Td2[tm_8bit(s1 >>  8)] ^ Td3[tm_8bit(s0)] ^ rk[47];
        if (key->rounds > 12) 
        {
/* round 12: */
            s0 = Td0[tm_8bit(t0 >> 24)] ^ Td1[tm_8bit(t3 >> 16)] ^ 
                Td2[tm_8bit(t2 >>  8)] ^ Td3[tm_8bit(t1)] ^ rk[48];
            s1 = Td0[tm_8bit(t1 >> 24)] ^ Td1[tm_8bit(t0 >> 16)] ^ 
                Td2[tm_8bit(t3 >>  8)] ^ Td3[tm_8bit(t2)] ^ rk[49];
            s2 = Td0[tm_8bit(t2 >> 24)] ^ Td1[tm_8bit(t1 >> 16)] ^ 
                Td2[tm_8bit(t0 >>  8)] ^ Td3[tm_8bit(t3)] ^ rk[50];
            s3 = Td0[tm_8bit(t3 >> 24)] ^ Td1[tm_8bit(t2 >> 16)] ^ 
                Td2[tm_8bit(t1 >>  8)] ^ Td3[tm_8bit(t0)] ^ rk[51];
/* round 13: */
            t0 = Td0[tm_8bit(s0 >> 24)] ^ Td1[tm_8bit(s3 >> 16)] ^ 
                Td2[tm_8bit(s2 >>  8)] ^ Td3[tm_8bit(s1)] ^ rk[52];
            t1 = Td0[tm_8bit(s1 >> 24)] ^ Td1[tm_8bit(s0 >> 16)] ^ 
                Td2[tm_8bit(s3 >>  8)] ^ Td3[tm_8bit(s2)] ^ rk[53];
            t2 = Td0[tm_8bit(s2 >> 24)] ^ Td1[tm_8bit(s1 >> 16)] ^ 
                Td2[tm_8bit(s0 >>  8)] ^ Td3[tm_8bit(s3)] ^ rk[54];
            t3 = Td0[tm_8bit(s3 >> 24)] ^ Td1[tm_8bit(s2 >> 16)] ^ 
                Td2[tm_8bit(s1 >>  8)] ^ Td3[tm_8bit(s0)] ^ rk[55];
        }
    }
    rk += key->rounds << 2;
#else  /* !TM_OPTIMIZE_SPEED || TM_OPTIMIZE_SIZE*/
/*
 * Nr - 1 full rounds:
 */
    r = key->rounds >> 1;
    for (;;) 
    {
        t0 =
            Td0[tm_8bit(s0 >> 24)] ^
            Td1[tm_8bit(s3 >> 16)] ^
            Td2[tm_8bit(s2 >>  8)] ^
            Td3[tm_8bit(s1)] ^
            rk[4];
        t1 =
            Td0[tm_8bit(s1 >> 24)] ^
            Td1[tm_8bit(s0 >> 16)] ^
            Td2[tm_8bit(s3 >>  8)] ^
            Td3[tm_8bit(s2)] ^
            rk[5];
        t2 =
            Td0[tm_8bit(s2 >> 24)] ^
            Td1[tm_8bit(s1 >> 16)] ^
            Td2[tm_8bit(s0 >>  8)] ^
            Td3[tm_8bit(s3)] ^
            rk[6];
        t3 =
            Td0[tm_8bit(s3 >> 24)] ^
            Td1[tm_8bit(s2 >> 16)] ^
            Td2[tm_8bit(s1 >>  8)] ^
            Td3[tm_8bit(s0)] ^
            rk[7];

        rk += 8;
        r--;
        if (r == 0) 
        {
            break;
        }

        s0 =
            Td0[tm_8bit(t0 >> 24)] ^
            Td1[tm_8bit(t3 >> 16)] ^
            Td2[tm_8bit(t2 >>  8)] ^
            Td3[tm_8bit(t1)] ^
            rk[0];
        s1 =
            Td0[tm_8bit(t1 >> 24)] ^
            Td1[tm_8bit(t0 >> 16)] ^
            Td2[tm_8bit(t3 >>  8)] ^
            Td3[tm_8bit(t2)] ^
            rk[1];
        s2 =
            Td0[tm_8bit(t2 >> 24)] ^
            Td1[tm_8bit(t1 >> 16)] ^
            Td2[tm_8bit(t0 >>  8)] ^
            Td3[tm_8bit(t3)] ^
            rk[2];
        s3 =
            Td0[tm_8bit(t3 >> 24)] ^
            Td1[tm_8bit(t2 >> 16)] ^
            Td2[tm_8bit(t1 >>  8)] ^
            Td3[tm_8bit(t0)] ^
            rk[3];
    }
#endif /* !TM_OPTIMIZE_SPEED || TM_OPTIMIZE_SIZE*/
/*
 * apply last round and
 * map cipher state to byte array block:
 */
    s0 = (Td4[tm_8bit(t0 >> 24)] & TM_UL(0xff000000))
        ^ (Td4[tm_8bit(t3 >> 16)] & TM_UL(0x00ff0000))
        ^ (Td4[tm_8bit(t2 >>  8)] & TM_UL(0x0000ff00))
        ^ (Td4[tm_8bit(t1)] & TM_UL(0x000000ff))
        ^ rk[0];
    tm_aes_putu32(out     , s0);
    s1 = (Td4[tm_8bit(t1 >> 24)] & TM_UL(0xff000000))
        ^ (Td4[tm_8bit(t0 >> 16)] & TM_UL(0x00ff0000))
        ^ (Td4[tm_8bit(t3 >>  8)] & TM_UL(0x0000ff00))
        ^ (Td4[tm_8bit(t2)] & TM_UL(0x000000ff))
        ^ rk[1];
    tm_aes_putu32(out +  4, s1);
    s2 = (Td4[tm_8bit(t2 >> 24)] & TM_UL(0xff000000))
        ^ (Td4[tm_8bit(t1 >> 16)] & TM_UL(0x00ff0000))
        ^ (Td4[tm_8bit(t0 >>  8)] & TM_UL(0x0000ff00))
        ^ (Td4[tm_8bit(t3)] & TM_UL(0x000000ff))
        ^ rk[2];
    tm_aes_putu32(out +  8, s2);
    s3 = (Td4[tm_8bit(t3 >> 24)] & TM_UL(0xff000000))
        ^ (Td4[tm_8bit(t2 >> 16)] & TM_UL(0x00ff0000))
        ^ (Td4[tm_8bit(t1 >>  8)] & TM_UL(0x0000ff00))
        ^ (Td4[tm_8bit(t0)] & TM_UL(0x000000ff))
        ^ rk[3];
    tm_aes_putu32(out + 12, s3);

aesBlockDecryptExit:
    return errorCode;

}

/*-----------------------  esp-aes ----------------------- */

/* as AES uses assymetric scheduled keys, we need to do it twice. */
int tfEspAesSchedLen(void)
{

    return sizeof(ttAesKeyInstance) * 2;
}

int tfEspAesSchedule(ttEspAlgorithmPtr algoPtr,
                     ttGenericKeyPtr   gkeyPtr)
{
    ttAesKeyInstancePtr   k;
    int                   errorCode;

    TM_UNREF_IN_ARG(algoPtr);
    k = (ttAesKeyInstancePtr)gkeyPtr->keySchedulePtr;
    errorCode = tfAesMakeEncryptKey(gkeyPtr->keyDataPtr, 
                                    gkeyPtr->keyBits,
                                    &k[0]);
    if(errorCode != TM_ENOERROR)
    {
        goto espAesScheduleExit;
    }

    errorCode = tfAesMakeDecryptKey(gkeyPtr->keyDataPtr, 
                                    gkeyPtr->keyBits,
                                    &k[1]);
espAesScheduleExit:
    return errorCode;
}



int tfEspAesBlockEncrypt(ttEspAlgorithmPtr algoPtr,
                         ttGenericKeyPtr   gkeyPtr,
                         tt8BitPtr         s,
                         tt8BitPtr         d)
{
    ttAesKeyInstancePtr    p;
    int errorCode;

    TM_UNREF_IN_ARG(algoPtr);
    p = (ttAesKeyInstancePtr)gkeyPtr->keySchedulePtr;

    errorCode = tfAesBlockEncrypt( &p[0], s,  d );
    if( errorCode != TM_ENOERROR )
    {
        return TM_EINVAL;
    }
    return TM_ENOERROR;
}

int tfEspAesBlockDecrypt( ttEspAlgorithmPtr algoPtr,
                          ttGenericKeyPtr   gkeyPtr,
                          tt8BitPtr         s,
                          tt8BitPtr         d)
{
    ttAesKeyInstancePtr    p;
    int errorCode;

    TM_UNREF_IN_ARG(algoPtr);
    p = (ttAesKeyInstancePtr)gkeyPtr->keySchedulePtr;
    
    errorCode = tfAesBlockDecrypt( &p[1], s,  d );
    if (errorCode != TM_ENOERROR)
    {
        return TM_EINVAL;
    }
    return TM_ENOERROR;
}

#ifdef TM_USE_AESXCBC
void tfAesXcbcInit(ttVoidPtr genCmacCtxPtr)
{
    ttAesXcbcCtxPtr     ctxPtr;
    tt8BitPtr           k1;
    
    ctxPtr = ((ttAesXcbcCtxPtr)
                (((ttGenericCmacCtxPtr)genCmacCtxPtr)->contextPtr));
    ctxPtr->keyPtr = ((tt8BitPtr)
                (((ttGenericCmacCtxPtr)genCmacCtxPtr)->keyPtr));
    k1 = (tt8BitPtr)0;
    
/* zero IV and size of buffer. The key is already set,
 * so don't zero the whole struct
 */
    tm_memset(ctxPtr->iv, 0, sizeof(ctxPtr->iv));
    ctxPtr->size = 0;
    ctxPtr->algPtr = (ttEspAlgorithmPtr)0;
    
    ctxPtr->algPtr = tfEspAlgorithmLookup(SADB_EALG_AESCBC);
    if (ctxPtr->algPtr == (ttEspAlgorithmPtr)0)
    {
        goto INIT_RETURN;
    }
    k1 = (tt8BitPtr)tm_get_raw_buffer(ctxPtr->algPtr->ealgBlockSize);
    if (k1 == (tt8BitPtr)0)
    {
        goto INIT_RETURN;
    }   

/* Initialize buffer, to static value specified in the RFC */
    tm_memset(k1, 0x01, ctxPtr->algPtr->ealgBlockSize);

/* Encrypt k1 with ctxPtr's key to find the new key value */
/* Only size supported by this alg */
    ctxPtr->genKey.keyBits = ctxPtr->algPtr->ealgBlockSize<<3;
    ctxPtr->genKey.keyDataPtr = ctxPtr->keyPtr;
    ctxPtr->genKey.keySchedulePtr = 0;
    tfEspSchedule(ctxPtr->algPtr, &ctxPtr->genKey);
/* Encrypt 0x01... with key K to key k1 */
    tfEspDataCbcEncrypt(ctxPtr->iv,
                        k1,
                        k1,
                        ctxPtr->algPtr->ealgBlockSize,
                        &ctxPtr->genKey,
                        ctxPtr->algPtr);

/* k1 has been found, so put it in the schedule */
    tfEspClearSchedule(ctxPtr->genKey.keySchedulePtr);
    ctxPtr->genKey.keyDataPtr = k1;
    ctxPtr->genKey.keyBits = ctxPtr->algPtr->ealgBlockSize<<3;
    ctxPtr->genKey.keySchedulePtr = 0;
    tfEspSchedule(ctxPtr->algPtr, &ctxPtr->genKey);

INIT_RETURN:
    if (k1 != (tt8BitPtr)0)
    {
        tm_free_raw_buffer(k1);
    }
    return;
}

void tfAesXcbcUpdate(ttVoidPtr  xcbcContextPtr,
                     tt8BitPtr  input,
                     ttPktLen   count,
                     ttPktLen   offset)
{
    ttAesXcbcCtxPtr ctxPtr;
    tt8Bit          copyLen;
    tt8Bit          blockSize;

    ctxPtr = (ttAesXcbcCtxPtr)xcbcContextPtr;
    copyLen = 0;
    blockSize = (tt8Bit)ctxPtr->algPtr->ealgBlockSize;
    TM_UNREF_IN_ARG(offset); /* for 16Bit or 32Bit DSP usage */

    while (count)
    {
        if (count > (tt8Bit)(blockSize - ctxPtr->size))
        {
/* set copyLen to be the size remaining in the buffer */
            copyLen = (tt8Bit)(blockSize - ctxPtr->size);
        }
        else
        {
            copyLen = (tt8Bit)count;
        }
/* start copying where we left off */
        tm_memcpy((ctxPtr->buffer+ctxPtr->size), input, copyLen);
        input += copyLen;
        ctxPtr->size = (tt8Bit)(ctxPtr->size + copyLen);
        count -= copyLen;

/* we can only encrypt this block if we know there is more data coming */
        if (count)
        {
            tfEspDataCbcEncrypt(ctxPtr->iv,
                                ctxPtr->buffer,
                                ctxPtr->iv,
                                blockSize,
                                &ctxPtr->genKey,
                                ctxPtr->algPtr);
            ctxPtr->size = 0;
        }
    }
}

void tfAesXcbcFinal(tt8BitPtr outputPtr,
                    ttVoidPtr xcbcContextPtr)
{
    ttAesXcbcCtxPtr ctxPtr;
/* This key may end up being k2 or k3 as described in RFC 3566 */
    tt8BitPtr       k2;
    tt8BitPtr       tempIV;
    tt8Bit          i;
    
    ctxPtr = (ttAesXcbcCtxPtr)xcbcContextPtr;
    k2 =     (tt8BitPtr)tm_get_raw_buffer(ctxPtr->algPtr->ealgBlockSize);
    tempIV = (tt8BitPtr)tm_get_raw_buffer(ctxPtr->algPtr->ealgBlockSize);

    if (k2 == (tt8BitPtr)0 || tempIV == (tt8BitPtr)0)
    {
        goto AESXCBC_FINAL_RETURN;
    }
/* The IV for creating K2/K3 is 16 bytes of 0x00 */   
    tm_memset(tempIV, 0, ctxPtr->algPtr->ealgBlockSize);

    if (ctxPtr->size < ctxPtr->algPtr->ealgBlockSize)
    {
/* In the case the message is not a multiple of the block size, we use 'K3'
 * from the RFC
 */
        tm_memset(k2, 0x03, ctxPtr->algPtr->ealgBlockSize);
    }
    else
    {
/* The message is a multiple of the block size, so we use 'K2' */
        tm_memset(k2, 0x02, ctxPtr->algPtr->ealgBlockSize);
    }

/* create the second key from the initial key, not k1*/
    tfEspClearSchedule(ctxPtr->genKey.keySchedulePtr);
    ctxPtr->genKey.keyDataPtr = ctxPtr->keyPtr;
    ctxPtr->genKey.keyBits = ctxPtr->algPtr->ealgBlockSize<<3;
    ctxPtr->genKey.keySchedulePtr = 0;
/* place K back in the schedule */
    tfEspSchedule(ctxPtr->algPtr, &ctxPtr->genKey);

/* find k2 using K */
    tfEspDataCbcEncrypt(tempIV,
                        k2,
                        k2,
                        ctxPtr->algPtr->ealgBlockSize,
                        &ctxPtr->genKey,
                        ctxPtr->algPtr);

    if (ctxPtr->size < ctxPtr->algPtr->ealgBlockSize)
    {
/* add padding */
        ctxPtr->buffer[ctxPtr->size] = 0x80;
/* set the rest of buffer to 0, don't overwrite anything 
 * important (ctxPtr->size) or the byte 0x80 that was added (+1)
 */
        tm_memset(ctxPtr->buffer + ctxPtr->size + 1,
                  0,
                  ctxPtr->algPtr->ealgBlockSize - ctxPtr->size - 1);
    }

/* buffer is now padded or full */    
    for (i = 0; i < ctxPtr->algPtr->ealgBlockSize; i++)
    {
        ctxPtr->buffer[i] ^= k2[i];
    }

/* now we are done with k2, but we need to encrypt the buffer
 * using k1 again, so rebuild it using k2 space
 */
    tm_memset(k2, 0x01, ctxPtr->algPtr->ealgBlockSize);

/* K is still in the schedule, find k1 using K */
    tfEspDataCbcEncrypt(tempIV,
                        k2,
                        k2,
                        ctxPtr->algPtr->ealgBlockSize,
                        &ctxPtr->genKey,
                        ctxPtr->algPtr);
    
    tfEspClearSchedule(ctxPtr->genKey.keySchedulePtr);
    ctxPtr->genKey.keyDataPtr = k2;
    ctxPtr->genKey.keyBits = ctxPtr->algPtr->ealgBlockSize<<3;
    ctxPtr->genKey.keySchedulePtr = 0;
/* put k1 back in the schedule */
    tfEspSchedule(ctxPtr->algPtr, &ctxPtr->genKey);

/* Encrypt */
    tfEspDataCbcEncrypt(ctxPtr->iv,
                        ctxPtr->buffer,
                        ctxPtr->iv,
                        ctxPtr->algPtr->ealgBlockSize,
                        &ctxPtr->genKey,
                        ctxPtr->algPtr);   

/* outputPtr needs to be 16 bytes */
    tm_memcpy(outputPtr,
              ctxPtr->iv,
              ctxPtr->algPtr->ealgBlockSize);

AESXCBC_FINAL_RETURN:
/* Zeroise sensitive stuff */
    tfEspClearSchedule(ctxPtr->genKey.keySchedulePtr);
    tm_memset((tt8BitPtr)ctxPtr, 0, sizeof(ttAesXcbcCtx));
    if (k2 != (tt8BitPtr)0)
    {
        tm_free_raw_buffer(k2);
    }
    if (tempIV != (tt8BitPtr)0)
    {
        tm_free_raw_buffer(tempIV);
    }
    return;
}
#endif /* TM_USE_AESXCBC */

#else /* ! TM_USE_AES */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */

/* To allow link for builds when TM_USE_AES is not defined */
int tvAesDummy = 0;

#endif /* TM_USE_AES*/
