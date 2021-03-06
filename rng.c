/*
rng.c
random number generator

Copyright (C) 2009 Evan Rinehart

This software comes with no warranty.
1. This software can be used for any purpose, good or evil.
2. Modifications must retain this license, at least in spirit.


Theory of Operation
Lag-1024 complimentary multiply-with-carry (CMWC) generator

This algorithm performs the following operation
	ax + c
where a, x, and c are numbers less than b (the base, 2^32)
using the rth last products (complimented) mod b as x and the 
last associated carry as c. a is a constant and r is the size 
of the rng state.

Another way to explain the above is
   z = a*x[i-r] + c
x[i] = ~(z % b)
   c =   z / b

r, a, and b must be chosen carefully or the period will be
very much less than the maximum 2^(32*(r+1)). For cmwc we choose
them so that p = ab^r + 1 is prime.

The first r x's and the initial c serve as the generator seed. 
For the generator to produce high quality random numbers as soon 
as possible, x should be prepopulated with r random numbers.

If the following parameters are used
b = 2^32
r = 1024
a = 109111
the resulting rng has a period of 109111*2^327652

The period of this rng is so large that is will effectively
never repeat. It will generate high quality random numbers for
the forseeable future. The state of the generator can be
saved and restored in case of program interruption. Besides
the initial seed the generator will never need to be reseeded.

When the generator is initialized for the very first time there 
are three options. You can reset it, which loads the state with
static data stored in the module. This data was generated with
a high quality RNG and should be the default initial state of the
generator. You may also initialize the generator with output of
the c library rand function using your desired seed. You may also
initialize the generator using /dev/urandom to put it into an
unpredictable state. Besides these three initialization methods you
may load whatever data you wish into the state.

zrand - return the next number in the current sequence
zseed - set the current sequence using 1024+1 numbers
zsrand - set the current sequence using 1 number and C srand
zsrand_u - put the generator into an unpredictable state using urandom
zread - return the generator state which can later be passed to zseed

refs:
http://tbf.coe.wayne.edu/jmasm/vol2_no1.pdf
http://en.wikipedia.org/wiki/Multiply-with-carry
google multiply with carry generator

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define A 109111ULL
#define R 1024

static unsigned x[R];
static unsigned c = 0;
static unsigned long long z;
static int i = 0;
static unsigned n;

static const unsigned seed[R];

unsigned zrand(){
	z = A*x[i] + c;
	n = ~z;
	x[i] = n;
	c = z >> 32;
	i = (i+1) & (R-1);
	return n;
}

void zseed(unsigned data[], unsigned carry){
	memcpy(x, data, R*sizeof(unsigned));
	c = carry;
}

void zread(unsigned data[], unsigned* carry){
	memcpy(data, x, R*sizeof(unsigned));
	*carry = c;
}

void zreset(){
	memcpy(x, seed, 4*R);
	c = 0;
}

void zsrand(unsigned s){
	int j;
	srand(s);
	for(j=0; j<R; j++){
		x[j] = rand();
	}
	c = 0;
}

void zsrand_u(){
	FILE* f = fopen("/dev/urandom", "r");
	if(fread(x, 4, R, f) < 4*R){
		/* do nothing */
	}
	fclose(f);
	c = 0;
}



static const unsigned seed[R] = {
0x78ccb683,0x3481a375,0x5f9e6898,0xf9800e83,
0x572d893b,0x70fff9bb,0xf70ac0ed,0x6141c9e5,
0x56952454,0xe390e62b,0x506ab7f2,0xba359e85,
0x9651ba89,0x2db34285,0xb9e4e900,0xc2c441bf,
0xee4c1b28,0xe7b8dbb8,0x1d503c0c,0xe7b898a0,
0x8973343d,0x83edcea2,0xc4594719,0x1278dfd4,
0x19284537,0x93dce563,0x0f5dcba7,0x10326602,
0x50622b42,0x52fd6472,0x0eab0e1c,0x86726558,
0xd0e942c6,0x1b8b7f7f,0xacfdda30,0xbf546229,
0x86c73055,0x1c4e38d6,0x7c9b6485,0xf1255a28,
0xbf7d2776,0x245ec55a,0xa11b17da,0x5790130f,
0x8aa4c429,0x7b42ce04,0x05de0f3b,0x4e5fcf14,
0xbe728848,0x66309ddd,0xfdde7c99,0x7506dd9c,
0x3d3dc7fe,0xd764811b,0x0fb1d105,0xf8f1f0b4,
0x6b551bbb,0x59dba288,0xa2057ef2,0xb5cec6c3,
0x325edf2a,0x738f018a,0xa0a1595f,0xe07e75b2,
0x2ed4e01e,0xe49e9cf8,0xd81169d0,0xc33d4959,
0xd3c292ef,0xa2b44c50,0x872477c9,0xbd17555a,
0xca1553d9,0xbe07313f,0x98a5b066,0x0352aff0,
0x26b1e8cb,0x33a579e9,0x385bc3c8,0xf0e159b5,
0x7b328475,0x5e4e2476,0x826e3993,0xd2ecff8f,
0x7cce1681,0x45211af5,0x465b7c0a,0xc371aeb3,
0xf1955cf4,0x40ff8fda,0x4d7bf519,0xeaf544d6,
0xff059143,0x97f801d1,0x9ba5d97d,0x3481e4e6,
0x0a7adf1a,0x683d6419,0x1b0c23cc,0x81e96d4d,
0x2d696bb8,0x5501ab1d,0xdc4d40dc,0x0a94689a,
0x0b2e9d49,0x5a54142e,0xc5f53ec4,0x05b99e81,
0xf6d3ad93,0xfa3a35c0,0xd824ca5a,0x9cc48f0f,
0x0824427d,0x68f9ff01,0x2935fccd,0x54b3c756,
0x49577f74,0x2ce4d756,0x13f28c99,0xa1f8eb08,
0x25e5b692,0x146c87f7,0x2a52b5f6,0x38a167e2,
0xf85e0cc5,0x0f1dcfca,0xd587d7d9,0xe670a74f,
0x293cfc4f,0x441598a2,0xc33802d6,0x77611818,
0x324f28ac,0xfc5e95a8,0x8d565d14,0x364cc747,
0xe893601f,0x4a239063,0x4bfe992c,0xa11cccf4,
0x5076fb6a,0x9f82534f,0xd55c4556,0xe67e5278,
0x2713682d,0x3f67bfef,0x5e340226,0xc3827e00,
0x7cb65e4d,0x96ca1fbc,0xa634377c,0xbcd99761,
0xcbf9f020,0x1fe1c5f7,0x11b6c429,0xc50582a8,
0x9a3d4dd6,0x08e33e56,0xc4062d25,0xc86bf8e3,
0x3492f08a,0xf5a3ce2c,0x76a37a2c,0x7a03e627,
0xbd08a762,0xafd09713,0x8dee4875,0xecad968c,
0x1a503997,0x9079d84a,0xd47bf00d,0x8993e4b3,
0x0cc997a7,0xd3692b89,0x71e6bc52,0x43f1083c,
0x43ca213d,0xf55fca42,0xb8b3abff,0x9f2f07f3,
0x450eb777,0x69ab46a0,0x468ef3c9,0x9a3dfded,
0xb12a4324,0xbfa78f40,0x4974cd9d,0x3bd6065a,
0xf22c2b23,0x9d61aab0,0x6ec63c14,0xbdb74178,
0x1e79ec34,0x1ca968ce,0x46f53c5c,0xa3e7a02d,
0x89116891,0xdb212284,0xf76788b0,0xc3e9b8bb,
0xbe634032,0x6d4d79cc,0x19ab0387,0xca0375c7,
0x6694d1a0,0xd19e9ccc,0xde2942b5,0x8eebab87,
0x563942e0,0x63ab7b3a,0x3ef3b2e7,0x6fc8f783,
0x76219787,0xa13e7dcd,0x4f303de9,0xc29bded4,
0x1b2698d6,0x6a69d2dc,0x1a8b8baa,0x85c70b2f,
0x97ca278f,0x5f688eb8,0x56ba8212,0x5bc13245,
0xaaf7059b,0x4b9630bc,0x6a6cf57d,0xa6fff589,
0x2c3bc80d,0x8aa32824,0xbd31bcc7,0xaba8239f,
0xef1f58a5,0x85f26400,0x69f18d3c,0x4920b415,
0xe43439a3,0xceadc396,0x949b2e24,0xc99ba482,
0x1b94752e,0x3f0a9520,0xa5e241c3,0x7433b591,
0xd84ef68e,0x24cac645,0x019b6ceb,0xb87fbbec,
0x5d4b3bb0,0x6e4dc037,0xb99945c9,0xac400e3b,
0x1ba21e36,0x2827b514,0x9a519e3f,0x08113a64,
0x8284e8a9,0x85269c90,0xc8002367,0xf68917be,
0x35ea12f1,0x20010bc0,0xef70eaa4,0xb58d113b,
0xf9c1b996,0xfb851f82,0x47d38c93,0x3f9ecf44,
0x676571e8,0x4da20222,0xfb3e90d4,0x637238d5,
0x133493f6,0xe6e7b50b,0x6782a425,0x89a898b6,
0xe75a0349,0x07acd03f,0xe318ac70,0x68c9c705,
0x24a1cfa1,0x994e441f,0xd9d7deed,0x2945eaeb,
0xaaf00caf,0x9187d425,0x66ad8ade,0xc886ffa3,
0xeaec9646,0x70cffcf4,0x5857f917,0x11cf60f5,
0x237d5194,0x028aec76,0x2f71ef65,0x0e3dc017,
0x5b57ad5b,0x2c7f8c2d,0x240c370c,0xee756fdb,
0x14c42734,0xd7e72c62,0x77e74166,0x4efd0b01,
0xc9db1cd2,0x44be9730,0x63f18a83,0x4f8b4338,
0xa6b8b99d,0x5624b383,0xcf005b92,0xdad5b8e6,
0xaf7d8e31,0xb103ee60,0x5c2d0688,0x458454ad,
0x488e7e33,0x603ccf6b,0x51e2de47,0x4c47cfc0,
0xa26f91bf,0xade37619,0x3f00bff2,0xab40e158,
0x0255cfcb,0x86f2b7ff,0x6759c53c,0xfbef40e2,
0xdd7a83ed,0xcd5347e0,0x0e106d40,0x0640608c,
0x2c9ef901,0x78b76d8b,0x5e7f4eef,0xd216b10d,
0x886c2275,0x8b1d0f79,0xa4b35dac,0x457abe80,
0x404102d0,0x155f7659,0x94d22808,0xb44ce43f,
0xfc90ca4c,0xd8632508,0xca4ae466,0x70b057b3,
0x000636e1,0x43f4348b,0x2683eddd,0x06617c3f,
0xa271f9a0,0x5b8fe19c,0x7f005a2e,0xa7f38cc5,
0x0e35248c,0xb0b924ca,0xb4c240a2,0xc8b90031,
0x146b51c8,0x763e0b09,0xc03a9944,0x9935bf09,
0xfd5c6d44,0x624d91ff,0xcc165723,0xa0112178,
0x56976668,0xebcd0f8b,0x5d93b3d3,0x085bd6f0,
0x84543c6a,0x0ce05ee6,0x57ac79db,0xdc06d8de,
0x90cfe3b1,0x8aa9a759,0xc7828fe4,0x20cf002e,
0x274ec237,0x2903502c,0x9f29fa6d,0x677f044e,
0xc99ed8eb,0xdbe2a291,0x6a6de59b,0x0586cf4a,
0x97bdb15b,0x8f9d7a7f,0xa86f079f,0x1936774d,
0xaf99474b,0x09d34689,0xcdc33564,0x1832e6bf,
0x86c10bf5,0x8748f0bd,0xd261c0c4,0x9a500ba6,
0xe09dbde3,0x1de20a01,0x0c31bfa7,0xef1315a4,
0x4960b046,0x22247379,0xf74eb94d,0x33caf43a,
0xe723c02a,0x7dfa339b,0xa3771fc2,0x60a12776,
0x4fd24a1d,0x79c7180e,0xa29d4d14,0x5d3a4258,
0xabd02a93,0xc74407ac,0x6de3e324,0xeb3cc2e0,
0x40f67f0e,0x510aeb60,0x7e6aa493,0x8b9711f2,
0x0adedf8f,0xc6d4641a,0x590f31fe,0x72944abd,
0xecafecf3,0xe4a560d2,0x8c7cf2d2,0x72252c0c,
0xe21d7b60,0xd12084a5,0x3cd70790,0xe06927d4,
0x52cbda5b,0xaa5a4d46,0xbd0d334b,0x70079756,
0x6444eff8,0x7e9aff56,0x660a5e3e,0xa231cacb,
0xf287ee23,0x5e286f4f,0x37b2d779,0xe8618016,
0x2322f60b,0x0235ab4b,0xc788e32d,0x23788080,
0xe6b7ce71,0xbdf2c30b,0xf89c28ce,0xbaa4dd64,
0x7fb78637,0xf80054f2,0xda8a954b,0x53b2c79c,
0x472181a0,0x55fa9f39,0x4d5a46e6,0x471ca40f,
0xbb7ce1c4,0x80bdda5d,0xd6e43a9a,0xa62569b5,
0x1b1ba751,0x3de2c78a,0xebcca99a,0x756ec1f5,
0xb23615da,0x9bbca3c9,0x855639b6,0x13573a5a,
0x90cabd4d,0xe22ac854,0xa80d9bb3,0xb93886ed,
0x391930c0,0x1ab7dd5f,0x1e64bb37,0xe547b420,
0x0a2e934b,0x6f017028,0xadf47a8b,0x1a3c0002,
0xb56e47b0,0x7e7b953b,0x0c463808,0xd15f880b,
0xf8b0f3d4,0xcd829c10,0xd7f3bd5d,0x8f773cc4,
0x5aa36693,0xd08bc8b1,0xae1af81a,0x1fd7b980,
0xb8666bd0,0xe3742d04,0x4c7dde81,0x5af48b8b,
0x41039866,0x1e080901,0x12aafdf6,0x2b9a488f,
0xccee6671,0xfe0f3664,0x1613867d,0x1925442a,
0xa3d0ff13,0xc75bec5a,0x4b385188,0x5afc94c2,
0x80c17d0f,0x91901bb5,0x650855c3,0xa8dace10,
0xfacbb9e3,0x69d2de39,0xdd42f20a,0xf147960c,
0x5a587906,0xc6e725b4,0x27d0886d,0xf462065b,
0xe21bef3c,0x1fd233aa,0x99ed9b31,0xe6ec751e,
0x6cb2994a,0x8131886a,0x3145e5a8,0xf558a152,
0x34cb4ae8,0xa3652156,0x741e3dd5,0x5e3cb33a,
0xec72dc96,0x2ebcf3d9,0x4befb0e2,0xfca149a2,
0x74eee63c,0x4570a0e4,0xf9e7efd0,0x7f3d3dc4,
0x211a4623,0x7fe53c8f,0x0114126b,0xc2ab46b1,
0xf6fbbc22,0x07e6d02e,0x88ad479b,0x95679e16,
0xb3fc5046,0xa576a264,0x471ec40c,0x4a2d62eb,
0x4438e83d,0x854d650e,0xe224e45e,0x16bc6a50,
0xbab37736,0x43e3ef2b,0xaf92772e,0x275da723,
0xe0aea5d6,0x41e9e4fa,0x1ae44657,0x6a3a894b,
0x9ae2d49f,0xe4b09222,0x2439e401,0x3c7e0cea,
0x0ebd9895,0xcf5632f6,0x6030543e,0x429d9edb,
0x2ff98a12,0x96c33e76,0xda74ba0f,0x2bd84112,
0x20ded070,0xfadb4f71,0x70950006,0x87a5d4fb,
0xc3df2020,0xa0dab835,0x4b4f4c0a,0x3358a676,
0xfa02770e,0x5ba5cc28,0x2cd45d8c,0x63d0e809,
0x542aa98f,0x7e024718,0xf05544fd,0x1d6483d7,
0xf9955a3e,0xe063b004,0x18a8d0cb,0x83ca36c3,
0x87f8af41,0x24356b37,0x87381c1e,0x2e7f8766,
0x1d38e26a,0x1345efcf,0x700c8bbe,0x351d6ed1,
0xbd061dec,0xa08f572d,0xd50f1cfd,0x3aa4120d,
0xf751b9c8,0xc7df7503,0x54f2dacb,0x93e834bf,
0x7687e44d,0x2c919332,0xf847bf8e,0x6e18cdb2,
0x4ea165b1,0xb64fc741,0x655a252d,0x2be1e10c,
0x81622059,0xe3efe002,0xfbd0003b,0x7f3fb5de,
0x0213539a,0x19c16ab7,0xc95f0da3,0x16a3354b,
0x591b7289,0xa8762dcf,0x07669c50,0x4bdbcbda,
0xa3f833d2,0xb0e453d3,0x62ad9e76,0x96d63c01,
0x2dc71a74,0xc2700652,0xaf812ed8,0x58bdb8ee,
0x31194cbd,0x6cc94a8c,0xdbd6721b,0x0b47b413,
0x032f62d4,0x662b8d44,0xe22709fb,0xf391ab6a,
0x75205ed0,0xae56f40d,0xe4a38906,0xde446c0a,
0xcbe7633f,0x15eb029b,0xe483fd69,0x53169fca,
0xf11c2dc7,0x50923fee,0x17091a3e,0x618dc7c0,
0x33494049,0x56a7c2d6,0x3ee64506,0x7b38446f,
0xc16e318f,0xda219fab,0xb8220def,0xdb6e5cff,
0x193dd0f1,0xa3205a84,0xa12a6c53,0x8c802cf5,
0x600f80ff,0xe91425ec,0xfca8dbfa,0x3a284a25,
0xd3ebda03,0x32d8ec03,0x63ef3e64,0x1611e012,
0x5dd8ef17,0x3e9b0525,0x23802863,0xabe1bbbb,
0xc062c84e,0x22191bef,0x6678f71b,0xced7fa52,
0x7d16ab68,0x46b53255,0xb6b4b754,0x585ee57f,
0xad004ebd,0x8d0eb7ba,0x496a4358,0x003a76da,
0x45d8a5f5,0xc7d457c6,0x38f47353,0xe6cf9032,
0x0e22692e,0x617c53d2,0x9d8d4905,0x7b3c48fc,
0x20179377,0xeee8a0a8,0xa1505f61,0x5579ee38,
0xc70f8dab,0x1030ff8f,0x0235bdc7,0xa2435abd,
0x42be7ad0,0x95340af8,0x2c5eecc9,0xc6377722,
0x2df31134,0xe27e7de0,0x11e053f7,0x7395aa54,
0x15ca6f6a,0xc6f999b5,0x3c4e4ba5,0xea34d68e,
0xf5427a59,0xce5c4d06,0xe16d2494,0x3ae219ee,
0x80c2ef1b,0x3259e75d,0x67127bfb,0x0f3163c5,
0xdc6d34a1,0xaa4b95ed,0x7487ac64,0x413df8a8,
0xdebc5b26,0xb5011148,0xd899ec53,0xb093c566,
0x889ad5e7,0x5ddfa97d,0xa1cb7749,0xc3e31de8,
0xaaec178a,0x4ff11de5,0x8942cc38,0x52dd5db6,
0x4789b3c7,0x7e52610b,0xa41fbcc3,0xf8081823,
0x54420030,0x46c27d0e,0x1a67966d,0xa66e320a,
0xd3be6dee,0x0ec03c4e,0xc9bd6e7b,0xe87e5b7b,
0x58c22e12,0x9fd0a92a,0x5095127e,0xf9434224,
0x0a4a9fc6,0x50b37439,0x11451b9e,0x69a59028,
0x00366203,0xf816c308,0x620173d0,0x9fc4dd84,
0x497a7f1f,0xaaba033b,0x8fd4c439,0x1868ee3d,
0xb222ba46,0xa0c3a09a,0x5fa25e0b,0x5a21f132,
0xc12dba9b,0x855f8b4d,0x67009272,0x11f2e885,
0xc455b065,0x44aa4ae4,0xc61b6fb4,0x84cd83b0,
0x0dadacef,0xcf01c1f4,0x992d49f3,0x6f19c165,
0xf8f819ee,0x0094daae,0x7f123269,0xa83ef1fb,
0x38b5fb9b,0xa1094c7f,0x78b880cd,0x4c8c8d12,
0xe6b7b0de,0x1bcc790e,0x2c43ae95,0xa8921cd1,
0x2d97286e,0x844dcfd0,0xf2825de7,0x0d357b5c,
0x63259e87,0x46cbb174,0x7bf4ea7f,0xb16f9c2d,
0x55c138cb,0x33a02270,0xa2b84686,0x19cbe730,
0x37bb8813,0x00e4e6c8,0xde365a1a,0x5e90e5d3,
0xc206f6cd,0x26a56e98,0x6f6de8b3,0x4aed90ca,
0xdedcc216,0xedcb8b04,0x4cd2e06b,0x135d765b,
0xcbdab903,0x1e8623bc,0xf4dd5959,0x58f72256,
0x0cc68c76,0x9385e695,0x0df07923,0x25780fa6,
0xf9aa17d3,0x79ca3daa,0x7de341f3,0x858ebe32,
0x113164a5,0x39bd5133,0xd4e5ae8a,0xe3b00fc0,
0x7b07df0d,0xd3a3b026,0xd576a9a1,0x8bfac055,
0xfa4ee836,0xfa708691,0x5b46911b,0x44e1f903,
0x0eb679e1,0xbc945613,0x977d7ed7,0x1a305f22,
0x0c9bc435,0xada73b62,0x7bb53db9,0x3e7dfc20,
0xb36c4434,0x37075fa0,0x6a5cfc72,0x497abcca,
0x4922d0ab,0xd6cc674b,0xdfaaa1c7,0x09f7e0b0,
0xe9150218,0x53c661d9,0xe24d095b,0xbf9b03c4,
0x9aaeee7e,0x335b206a,0xf60b8e4a,0x996b7933,
0xc4adfbcb,0x588a2ccb,0x80757000,0x1e895e88,
0x4d73fb03,0x65ed62f0,0x03448eb0,0xf349d162,
0xe849f8b7,0xebe423b1,0xc81187a7,0x201e4c9a,
0x1afaa893,0x7803d25b,0xb4f1e9e7,0x6b6eadec,
0xcaef703e,0x3361407d,0x7014c8b6,0x9edeb022,
0xd4b9a3ba,0xbc74f0ad,0x3d00f7f5,0x6e556e31,
0x4ab88abc,0xeae0f853,0xb2d4039d,0xf38f1441,
0xa1ce980e,0x99f874e3,0x894d41be,0xc474fb9f,
0x05af1e82,0xde824b66,0xfc86f0f5,0x93d9f6f3,
0xcd4f6d72,0x9d52f00b,0x87418c3d,0x661689a5,
0x6abd7ce1,0x79890d4f,0x9ddb73b8,0xd19d251c,
0xe2527680,0xef92f5d7,0xd3c94750,0xe0d5a177,
0xbbab5e1e,0xd24b75d2,0x08a46507,0x9e4ecbb5,
0x9e3d0c0f,0x8258ec58,0x6a289d70,0xf58bfe7e,
0x1db04252,0x1e27c93e,0x7f269ba3,0x031e994a,
0x61d00429,0x563751b4,0x0a89cdc6,0x6dfece7a,
0x889611b0,0x86a215e5,0xbcbca365,0x4191770a,
0x60dcc19e,0x607f5121,0x49611e62,0x30a04b14,
0xad538ef1,0x0a48c1ae,0xdc2fd80d,0xf48e5375,
0xc8f7d9fb,0x964c0d8e,0x252641be,0xe552ca6a,
0x3a806a22,0x78c6788f,0x0eb9a73c,0x5cf8eba4,
0xee4ce76e,0xca24d902,0xd81d4257,0xba4f4373,
0xa4e6d2fc,0xcdff8c5d,0xf04452a0,0xd24ed156,
0xe3135ea0,0xfdb8846e,0x54cdd95c,0x3285f5f0,
0x8a5aefe3,0x6c68d56b,0x1e9b5109,0xf34f2aef,
0x84e3071b,0x46b82e1d,0xd970b8ba,0x08bee27d,
0x7408b4ef,0x2a9e6edc,0x3cbf57bb,0xfe3c8cf9,
0x44c2dbfe,0x45206d53,0x5ace5ff5,0x099bf422,
0xbf99926c,0x5aaaf1b7,0xbecee19b,0x878d8917,
0xa4e6b59c,0x181d373d,0x9926d5d1,0x9c79a6b9,
0x21f94776,0xa8fce4be,0x9eb14285,0x5709faa5,
0x9a72dec5,0x2dcbef6c,0xb00e107c,0x635c25d3,
0xf9c6a9c5,0x94fb2617,0x99bf9c4a,0x36b717cd,
0x1e5af761,0x224336ab,0x3d928cdb,0x8104788b,
0x3caa7ea5,0x31eec558,0xc0cf133e,0x787100a9
};
