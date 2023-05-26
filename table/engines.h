#define MK(a,b,c,d,e,f) {a,b,c,d,((e)<<4)|(f)}
#define MW(a,b,c,d,e,f) {a,b|0x80,c,d,((e)<<4)|(f)}

EngineInfo _engine_info[TOTAL_NUM_ENGINES] = {
	MK(  4809,  20,  15,  30,   0,   1), /* 0 */
	MK( 12784,  20,  22,  30,   0,   6), /* 1 */
	MK(  9497,  20,  20,  50,   0,   8), /* 2 */
	MK( 11688,  20,  20,  30,   0,   8), /* 3 */
	MK( 16802,  20,  20,  30,   0,   8), /* 4 */
	MK( 18993,  20,  20,  30,   0,   8), /* 5 */
	MK( 20820,  20,  20,  30,   0,   8), /* 6 */
	MK(  8766,  20,  20,  30,   0,   6), /* 7 */
	MK(  5114,  20,  21,  30,   0,   1), /* 8 */
	MK(  5479,  20,  20,  30,   0,   1), /* 9 */
	MK( 12419,  20,  23,  25,   0,   1), /* 10 */
	MK( 13149,  20,  12,  30,   0,   1), /* 11 */
	MK( 23376,  20,  15,  35,   0,   1), /* 12 */
	MK( 14976,  20,  18,  28,   0,   1), /* 13 */
	MK( 14245,  20,  20,  30,   0,   1), /* 14 */
	MK( 15341,  20,  22,  33,   0,   1), /* 15 */
	MK( 14976,  20,  20,  25,   0,   6), /* 16 */
	MK( 16437,  20,  20,  30,   0,   6), /* 17 */
	MK( 18993,  20,  22,  30,   0,   6), /* 18 */
	MK( 13880,  20,  22,  30,   0,   6), /* 19 */
	MK( 20454,  20,  22,  30,   0,   6), /* 20 */
	MK( 16071,  20,  22,  30,   0,   6), /* 21 */
	MK( 20820,  20,  20,  25,   0,   1), /* 22 */
	MK( 16437,  20,  23,  30,   0,   1), /* 23 */
	MK( 19359,  20,  23,  80,   0,   1), /* 24 */
	MK( 23376,  20,  25,  30,   0,   1), /* 25 */
	MK( 26298,  20,  25,  50,   0,   1), /* 26 */
	MW(  1827,  20,  20,  50,   0,  15), /* 27 */
	MW(  1827,  20,  20,  50,   0,  15), /* 28 */
	MW(  1827,  20,  20,  50,   0,   3), /* 29 */
	MW(  1827,  20,  20,  50,   0,   7), /* 30 */
	MW(  1827,  20,  20,  50,   0,   3), /* 31 */
	MW(  1827,  20,  20,  50,   0,   7), /* 32 */
	MW(  1827,  20,  20,  50,   0,   7), /* 33 */
	MW(  1827,  20,  20,  50,   0,   7), /* 34 */
	MW(  1827,  20,  20,  50,   0,   1), /* 35 */
	MW(  1827,  20,  20,  50,   0,   1), /* 36 */
	MW(  1827,  20,  20,  50,   0,   7), /* 37 */
	MW(  1827,  20,  20,  50,   0,   6), /* 38 */
	MW(  1827,  20,  20,  50,   0,   2), /* 39 */
	MW(  1827,  20,  20,  50,   0,   4), /* 40 */
	MW(  1827,  20,  20,  50,   0,   4), /* 41 */
	MW(  1827,  20,  20,  50,   0,   4), /* 42 */
	MW(  1827,  20,  20,  50,   0,   4), /* 43 */
	MW(  1827,  20,  20,  50,   0,   8), /* 44 */
	MW(  1827,  20,  20,  50,   0,   8), /* 45 */
	MW(  1827,  20,  20,  50,   0,   8), /* 46 */
	MW(  1827,  20,  20,  50,   0,   8), /* 47 */
	MW(  1827,  20,  20,  50,   0,   8), /* 48 */
	MW(  1827,  20,  20,  50,   0,   8), /* 49 */
	MW(  1827,  20,  20,  50,   0,   8), /* 50 */
	MW(  1827,  20,  20,  50,   0,   8), /* 51 */
	MW(  1827,  20,  20,  50,   0,   8), /* 52 */
	MW(  1827,  20,  20,  50,   0,   8), /* 53 */
	MK( 28490,  20,  20,  50,   1,   7), /* 54 */
	MK( 31047,  20,  20,  50,   1,   7), /* 55 */
	MK( 28855,  20,  20,  50,   1,   8), /* 56 */
	MW(  1827,  20,  20,  50,   1,  15), /* 57 */
	MW(  1827,  20,  20,  50,   1,  15), /* 58 */
	MW(  1827,  20,  20,  50,   1,   3), /* 59 */
	MW(  1827,  20,  20,  50,   1,   7), /* 60 */
	MW(  1827,  20,  20,  50,   1,   3), /* 61 */
	MW(  1827,  20,  20,  50,   1,   7), /* 62 */
	MW(  1827,  20,  20,  50,   1,   7), /* 63 */
	MW(  1827,  20,  20,  50,   1,   7), /* 64 */
	MW(  1827,  20,  20,  50,   1,   1), /* 65 */
	MW(  1827,  20,  20,  50,   1,   1), /* 66 */
	MW(  1827,  20,  20,  50,   1,   7), /* 67 */
	MW(  1827,  20,  20,  50,   1,   6), /* 68 */
	MW(  1827,  20,  20,  50,   1,   2), /* 69 */
	MW(  1827,  20,  20,  50,   1,   4), /* 70 */
	MW(  1827,  20,  20,  50,   1,   4), /* 71 */
	MW(  1827,  20,  20,  50,   1,   4), /* 72 */
	MW(  1827,  20,  20,  50,   1,   4), /* 73 */
	MW(  1827,  20,  20,  50,   1,   8), /* 74 */
	MW(  1827,  20,  20,  50,   1,   8), /* 75 */
	MW(  1827,  20,  20,  50,   1,   8), /* 76 */
	MW(  1827,  20,  20,  50,   1,   8), /* 77 */
	MW(  1827,  20,  20,  50,   1,   8), /* 78 */
	MW(  1827,  20,  20,  50,   1,   8), /* 79 */
	MW(  1827,  20,  20,  50,   1,   8), /* 80 */
	MW(  1827,  20,  20,  50,   1,   8), /* 81 */
	MW(  1827,  20,  20,  50,   1,   8), /* 82 */
	MW(  1827,  20,  20,  50,   1,   8), /* 83 */
	MK( 36525,  20,  20,  50,   2,   7), /* 84 */
	MK( 39447,  20,  20,  50,   2,   7), /* 85 */
	MK( 42004,  20,  20,  50,   2,   7), /* 86 */
	MK( 42735,  20,  20,  50,   2,   7), /* 87 */
	MK( 36891,  20,  20,  60,   2,   8), /* 88 */
	MW(  1827,  20,  20,  50,   2,  15), /* 89 */
	MW(  1827,  20,  20,  50,   2,  15), /* 90 */
	MW(  1827,  20,  20,  50,   2,   3), /* 91 */
	MW(  1827,  20,  20,  50,   2,   7), /* 92 */
	MW(  1827,  20,  20,  50,   2,   3), /* 93 */
	MW(  1827,  20,  20,  50,   2,   7), /* 94 */
	MW(  1827,  20,  20,  50,   2,   7), /* 95 */
	MW(  1827,  20,  20,  50,   2,   7), /* 96 */
	MW(  1827,  20,  20,  50,   2,   1), /* 97 */
	MW(  1827,  20,  20,  50,   2,   1), /* 98 */
	MW(  1827,  20,  20,  50,   2,   7), /* 99 */
	MW(  1827,  20,  20,  50,   2,   6), /* 100 */
	MW(  1827,  20,  20,  50,   2,   2), /* 101 */
	MW(  1827,  20,  20,  50,   2,   4), /* 102 */
	MW(  1827,  20,  20,  50,   2,   4), /* 103 */
	MW(  1827,  20,  20,  50,   2,   4), /* 104 */
	MW(  1827,  20,  20,  50,   2,   4), /* 105 */
	MW(  1827,  20,  20,  50,   2,   8), /* 106 */
	MW(  1827,  20,  20,  50,   2,   8), /* 107 */
	MW(  1827,  20,  20,  50,   2,   8), /* 108 */
	MW(  1827,  20,  20,  50,   2,   8), /* 109 */
	MW(  1827,  20,  20,  50,   2,   8), /* 110 */
	MW(  1827,  20,  20,  50,   2,   8), /* 111 */
	MW(  1827,  20,  20,  50,   2,   8), /* 112 */
	MW(  1827,  20,  20,  50,   2,   8), /* 113 */
	MW(  1827,  20,  20,  50,   2,   8), /* 114 */
	MW(  1827,  20,  20,  50,   2,   8), /* 115 */
	MK(  3378,  20,  12,  40,   0,   7), /* 116 */
	MK( 16071,  20,  15,  30,   0,   7), /* 117 */
	MK( 24107,  20,  15,  40,   0,   7), /* 118 */
	MK( 32142,  20,  15,  80,   0,   7), /* 119 */
	MK(  9132,  20,  15,  40,   0,   8), /* 120 */
	MK( 18993,  20,  15,  40,   0,   8), /* 121 */
	MK( 32873,  20,  15,  80,   0,   8), /* 122 */
	MK(  5479,  20,  15,  55,   0,   3), /* 123 */
	MK( 20089,  20,  15,  55,   0,   3), /* 124 */
	MK( 33969,  20,  15,  85,   0,   3), /* 125 */
	MK(  5479,  20,  15,  55,   0,   7), /* 126 */
	MK( 21550,  20,  15,  55,   0,   7), /* 127 */
	MK( 35795,  20,  15,  85,   0,   7), /* 128 */
	MK(  5479,  20,  15,  55,   0,   8), /* 129 */
	MK( 21550,  20,  15,  55,   0,   8), /* 130 */
	MK( 35795,  20,  15,  85,   0,   8), /* 131 */
	MK(  5479,  20,  15,  55,   0,   7), /* 132 */
	MK( 19359,  20,  15,  55,   0,   7), /* 133 */
	MK( 31047,  20,  15,  85,   0,   7), /* 134 */
	MK(  5479,  20,  15,  55,   0,   3), /* 135 */
	MK( 21915,  20,  15,  55,   0,   3), /* 136 */
	MK( 37256,  20,  15,  85,   0,   3), /* 137 */
	MK(  5479,  20,  15,  55,   0,   7), /* 138 */
	MK( 19724,  20,  15,  55,   0,   7), /* 139 */
	MK( 31047,  20,  15,  85,   0,   7), /* 140 */
	MK(  5479,  20,  15,  55,   0,   7), /* 141 */
	MK( 21185,  20,  15,  55,   0,   7), /* 142 */
	MK( 32873,  20,  15,  85,   0,   7), /* 143 */
	MK(  5479,  20,  15,  55,   0,   7), /* 144 */
	MK( 19724,  20,  15,  55,   0,   7), /* 145 */
	MK( 35430,  20,  15,  85,   0,   7), /* 146 */
	MK(  5479,  20,  15,  55,   0,   1), /* 147 */
	MK( 20820,  20,  15,  55,   0,   1), /* 148 */
	MK( 33238,  20,  15,  85,   0,   1), /* 149 */
	MK(  5479,  20,  15,  55,   0,   1), /* 150 */
	MK( 21185,  20,  15,  55,   0,   1), /* 151 */
	MK( 31777,  20,  15,  85,   0,   1), /* 152 */
	MK(  5479,  20,  15,  55,   0,   7), /* 153 */
	MK( 22281,  20,  15,  55,   0,   7), /* 154 */
	MK( 33603,  20,  15,  85,   0,   7), /* 155 */
	MK(  5479,  20,  15,  55,   0,   6), /* 156 */
	MK( 18628,  20,  15,  55,   0,   6), /* 157 */
	MK( 30681,  20,  15,  85,   0,   6), /* 158 */
	MK(  5479,  20,  15,  55,   0,   2), /* 159 */
	MK( 21185,  20,  15,  55,   0,   2), /* 160 */
	MK( 31777,  20,  15,  85,   0,   2), /* 161 */
	MK(  5479,  20,  15,  55,   0,   4), /* 162 */
	MK( 20820,  20,  15,  55,   0,   4), /* 163 */
	MK( 33238,  20,  15,  85,   0,   4), /* 164 */
	MK(  5479,  20,  15,  55,   0,   4), /* 165 */
	MK( 20970,  20,  15,  55,   0,   4), /* 166 */
	MK( 33388,  20,  15,  85,   0,   4), /* 167 */
	MK(  5479,  20,  15,  55,   0,   4), /* 168 */
	MK( 21335,  20,  15,  55,   0,   4), /* 169 */
	MK( 33753,  20,  15,  85,   0,   4), /* 170 */
	MK(  5479,  20,  15,  55,   0,   4), /* 171 */
	MK( 20604,  20,  15,  55,   0,   4), /* 172 */
	MK( 33023,  20,  15,  85,   0,   4), /* 173 */
	MK(  5479,  20,  15,  55,   0,   8), /* 174 */
	MK( 19724,  20,  15,  55,   0,   8), /* 175 */
	MK( 33238,  20,  15,  85,   0,   8), /* 176 */
	MK(  5479,  20,  15,  55,   0,   8), /* 177 */
	MK( 20089,  20,  15,  55,   0,   8), /* 178 */
	MK( 33603,  20,  15,  85,   0,   8), /* 179 */
	MK(  5479,  20,  15,  55,   0,   8), /* 180 */
	MK( 20454,  20,  15,  55,   0,   8), /* 181 */
	MK( 33969,  20,  15,  85,   0,   8), /* 182 */
	MK(  5479,  20,  15,  55,   0,   8), /* 183 */
	MK( 20820,  20,  15,  55,   0,   8), /* 184 */
	MK( 34334,  20,  15,  85,   0,   8), /* 185 */
	MK(  5479,  20,  15,  55,   0,   8), /* 186 */
	MK( 21185,  20,  15,  55,   0,   8), /* 187 */
	MK( 34699,  20,  15,  85,   0,   8), /* 188 */
	MK(  5479,  20,  15,  55,   0,   8), /* 189 */
	MK( 21550,  20,  15,  55,   0,   8), /* 190 */
	MK( 35064,  20,  15,  85,   0,   8), /* 191 */
	MK(  5479,  20,  15,  55,   0,   8), /* 192 */
	MK( 19874,  20,  15,  55,   0,   8), /* 193 */
	MK( 35430,  20,  15,  85,   0,   8), /* 194 */
	MK(  5479,  20,  15,  55,   0,   8), /* 195 */
	MK( 20239,  20,  15,  55,   0,   8), /* 196 */
	MK( 35795,  20,  15,  85,   0,   8), /* 197 */
	MK(  5479,  20,  15,  55,   0,   8), /* 198 */
	MK( 20604,  20,  15,  55,   0,   8), /* 199 */
	MK( 32873,  20,  15,  85,   0,   8), /* 200 */
	MK(  5479,  20,  15,  55,   0,   8), /* 201 */
	MK( 20970,  20,  15,  55,   0,   8), /* 202 */
	MK( 33023,  20,  15,  85,   0,   8), /* 203 */
	MK(  2922,   5,  30,  50,   0,   7), /* 204 */
	MK( 17167,   5,  30,  90,   0,   7), /* 205 */
	MK(  2192,   5,  30,  55,   0,   7), /* 206 */
	MK( 18628,   5,  30,  90,   0,   7), /* 207 */
	MK( 17257,  10,  25,  90,   0,   7), /* 208 */
	MK(  9587,   5,  30,  40,   0,   8), /* 209 */
	MK( 20544,   5,  30,  90,   0,   8), /* 210 */
	MK(  2557,   5,  30,  55,   0,   7), /* 211 */
	MK( 19724,   5,  30,  98,   0,   7), /* 212 */
	MK(  9587,   5,  30,  45,   0,   8), /* 213 */
	MK( 22371,   5,  30,  90,   0,   8), /* 214 */
	MK(  2922,  20,  20,  20,   0,   7), /* 215 */
	MK(  9922,  20,  24,  20,   0,   7), /* 216 */
	MK( 12659,  20,  18,  20,   0,   7), /* 217 */
	MK( 17652,  20,  25,  35,   0,   7), /* 218 */
	MK(  4929,  20,  30,  30,   0,   7), /* 219 */
	MK( 13695,  20,  23,  25,   0,   7), /* 220 */
	MK( 16341,  20,  26,  30,   0,   7), /* 221 */
	MK( 21395,  20,  25,  30,   0,   7), /* 222 */
	MK( 18263,  20,  20,  30,   0,   7), /* 223 */
	MK( 25233,  20,  25,  30,   0,   7), /* 224 */
	MK( 15371,  20,  22,  25,   0,   7), /* 225 */
	MK( 15461,  20,  25,  25,   0,   7), /* 226 */
	MK( 16952,  20,  22,  25,   0,   7), /* 227 */
	MK( 17227,  20,  25,  30,   0,   7), /* 228 */
	MK( 22371,  20,  25,  35,   0,   7), /* 229 */
	MK( 22341,  20,  25,  30,   0,   7), /* 230 */
	MK( 27209,  20,  25,  30,   0,   7), /* 231 */
	MK( 17988,  20,  20,  30,   0,   7), /* 232 */
	MK( 18993,  20,  24,  35,   0,   7), /* 233 */
	MK( 22401,  20,  24,  30,   0,   7), /* 234 */
	MK( 24472,  20,  24,  30,   0,   7), /* 235 */
	MK( 26724,  20,  24,  30,   0,   7), /* 236 */
	MK( 22005,  20,  25,  30,   0,   7), /* 237 */
	MK( 24107,  20,  20,  35,   0,   7), /* 238 */
	MK( 29310,  20,  25,  60,   0,   7), /* 239 */
	MK( 35520,  20,  22,  30,   0,   7), /* 240 */
	MK( 36981,  20,  22,  30,   0,   7), /* 241 */
	MK( 38807,  20,  22,  50,   0,   7), /* 242 */
	MK( 42094,  20,  25,  30,   0,   7), /* 243 */
	MK( 44651,  20,  23,  30,   0,   7), /* 244 */
	MK( 40268,  20,  25,  30,   0,   7), /* 245 */
	MK( 33693,  20,  25,  50,   0,   7), /* 246 */
	MK( 32963,  20,  20,  60,   0,   7), /* 247 */
	MK(  9222,  20,  20,  35,   0,   8), /* 248 */
	MK( 12874,  20,  20,  35,   0,   8), /* 249 */
	MK( 16892,  20,  20,  35,   0,   8), /* 250 */
	MK( 21275,  20,  20,  99,   0,   8), /* 251 */
	MK( 23832,  20,  20,  99,   0,   8), /* 252 */
	MK( 13575,  20,  20,  40,   0,   7), /* 253 */
	MK( 28215,  20,  20,  30,   0,   7), /* 254 */
	MK( 13575,  20,  20,  99,   0,   8), /* 255 */
};

RailVehicleInfo _rail_vehicle_info[] = {
	// image_index    max_speed (kph)  running_cost_base
	// |  flags       |      power (hp)|    running_cost_class
	// |  |    base_cost     |    weight    |    capacity
	// |  |    |      |      |    |    |    |    |    cargo_type
	// |  |    |      |      |    |    |    |    |    |
	{  2, 0,   7,    64,   300,  47,  50,   0,   0,   0}, /* 0 */
	{ 19, 0,   8,    80,   600,  65,  65,   1,   0,   0}, /* 1 */
	{  2, 0,  10,    72,   400,  85,  90,   0,   0,   0}, /* 2 */
	{  0, 0,  15,    96,   900, 130, 130,   0,   0,   0}, /* 3 */
	{  1, 0,  19,   112,  1000, 140, 145,   0,   0,   0}, /* 4 */
	{ 12, 0,  16,   120,  1400,  95, 125,   1,   0,   0}, /* 5 */
	{ 14, 0,  20,   152,  2000, 120, 135,   1,   0,   0}, /* 6 */
	{  3, 0,  14,    88,  1100, 145, 130,   0,   0,   0}, /* 7 */
	{  0, 0,  13,   112,  1000, 131, 120,   0,   0,   0}, /* 8 */
	{  1, 0,  19,   128,  1200, 162, 140,   0,   0,   0}, /* 9 */
	{  0, 0,  22,   144,  1600, 170, 130,   0,   0,   0}, /* 10 */
	{  8, 1,  11,   112, 600/2,  32,85/2,   1,  38,   CT_PASSENGERS}, /* 11 */
	{ 10, 1,  14,   120, 700/2,  38,70/2,   1,  40,   CT_PASSENGERS}, /* 12 */
	{  4, 0,  15,   128,  1250,  72,  95,   1,   0,   0}, /* 13 */
	{  5, 0,  17,   144,  1750, 101, 120,   1,   0,   0}, /* 14 */
	{  4, 0,  18,   160,  2580, 112, 140,   1,   0,   0}, /* 15 */
	{ 14, 0,  23,    96,  4000, 150, 135,   1,   0,   0}, /* 16 */
	{ 12, 0,  16,   112,  2400, 120, 105,   1,   0,   0}, /* 17 */
	{ 13, 0,  30,   112,  6600, 207, 155,   1,   0,   0}, /* 18 */
	{ 15, 0,  18,   104,  1500, 110, 105,   1,   0,   0}, /* 19 */
	{ 16, 1,  35,   160,3500/2,  95, 205/2, 1,   0,   0}, /* 20 */
	{ 18, 0,  21,   104,  2200, 120, 145,   1,   0,   0}, /* 21 */
	{  6, 1,  20,   200,4500/2,  70, 190/2, 1,   4,   CT_MAIL}, /* 22 */
	{ 20, 0,  26,   160,  3600,  84, 180,   2,   0,   0}, /* 23 */
	{ 20, 0,  30,   176,  5000,  82, 205,   2,   0,   0}, /* 24 */
	{ 21, 1,  40,   240,7000/2,  90, 240/2, 2,   0,   0}, /* 25 */
	{ 23, 1,  43,   264,8000/2,  95, 250/2, 2,   0,   0}, /* 26 */
	{ 33, 2, 247,     0,     0,  25,   0,   0,  40,   CT_PASSENGERS}, /* 27 */
	{ 35, 2, 228,     0,     0,  21,   0,   0,  30,   CT_MAIL}, /* 28 */
	{ 34, 2, 176,     0,     0,  18,   0,   0,  30,   CT_COAL}, /* 29 */
	{ 36, 2, 200,     0,     0,  24,   0,   0,  30,   CT_OIL}, /* 30 */
	{ 37, 2, 192,     0,     0,  20,   0,   0,  25,   CT_LIVESTOCK}, /* 31 */
	{ 38, 2, 190,     0,     0,  21,   0,   0,  25,   CT_GOODS}, /* 32 */
	{ 39, 2, 182,     0,     0,  19,   0,   0,  30,   CT_GRAIN}, /* 33 */
	{ 40, 2, 181,     0,     0,  16,   0,   0,  30,   CT_WOOD}, /* 34 */
	{ 41, 2, 179,     0,     0,  19,   0,   0,  30,   CT_IRON_ORE}, /* 35 */
	{ 42, 2, 196,     0,     0,  18,   0,   0,  20,   CT_STEEL}, /* 36 */
	{ 43, 2, 255,     0,     0,  30,   0,   0,  20,   CT_VALUABLES}, /* 37 */
	{ 44, 2, 191,     0,     0,  22,   0,   0,  25,   CT_FOOD}, /* 38 */
	{ 45, 2, 196,     0,     0,  18,   0,   0,  20,   CT_PAPER}, /* 39 */
	{ 46, 2, 179,     0,     0,  19,   0,   0,  30,   CT_COPPER_ORE}, /* 40 */
	{ 47, 2, 199,     0,     0,  25,   0,   0,  25,   CT_WATER}, /* 41 */
	{ 48, 2, 182,     0,     0,  18,   0,   0,  25,   CT_FRUIT}, /* 42 */
	{ 49, 2, 185,     0,     0,  19,   0,   0,  21,   CT_RUBBER}, /* 43 */
	{ 50, 2, 176,     0,     0,  19,   0,   0,  30,   CT_SUGAR}, /* 44 */
	{ 51, 2, 178,     0,     0,  20,   0,   0,  30,   CT_COTTON_CANDY}, /* 45 */
	{ 52, 2, 192,     0,     0,  20,   0,   0,  30,   CT_TOFFEE}, /* 46 */
	{ 53, 2, 190,     0,     0,  21,   0,   0,  20,   CT_BUBBLES}, /* 47 */
	{ 54, 2, 182,     0,     0,  24,   0,   0,  25,   CT_COLA}, /* 48 */
	{ 55, 2, 181,     0,     0,  21,   0,   0,  25,   CT_CANDY}, /* 49 */
	{ 56, 2, 183,     0,     0,  21,   0,   0,  20,   CT_TOYS}, /* 50 */
	{ 57, 2, 196,     0,     0,  18,   0,   0,  22,   CT_BATTERIES}, /* 51 */
	{ 58, 2, 193,     0,     0,  18,   0,   0,  25,   CT_FIZZY_DRINKS}, /* 52 */
	{ 59, 2, 191,     0,     0,  18,   0,   0,  30,   CT_PLASTIC}, /* 53 */
	{ 25, 0,  52,   304,  9000,  95, 230,   2,   0,   0}, /* 54 */
	{ 26, 1,  60,   336,10000/2, 85, 240/2, 2,  25,   CT_PASSENGERS}, /* 55 */
	{ 26, 0,  53,   320,  5000,  95, 230,   2,   0,   0}, /* 56 */
	{ 60, 2, 247,     0,     0,  25,   0,   0,  45,   CT_PASSENGERS}, /* 57 */
	{ 62, 2, 228,     0,     0,  21,   0,   0,  35,   CT_MAIL}, /* 58 */
	{ 61, 2, 176,     0,     0,  18,   0,   0,  35,   CT_COAL}, /* 59 */
	{ 63, 2, 200,     0,     0,  24,   0,   0,  35,   CT_OIL}, /* 60 */
	{ 64, 2, 192,     0,     0,  20,   0,   0,  30,   CT_LIVESTOCK}, /* 61 */
	{ 65, 2, 190,     0,     0,  21,   0,   0,  30,   CT_GOODS}, /* 62 */
	{ 66, 2, 182,     0,     0,  19,   0,   0,  35,   CT_GRAIN}, /* 63 */
	{ 67, 2, 181,     0,     0,  16,   0,   0,  35,   CT_WOOD}, /* 64 */
	{ 68, 2, 179,     0,     0,  19,   0,   0,  35,   CT_IRON_ORE}, /* 65 */
	{ 69, 2, 196,     0,     0,  18,   0,   0,  25,   CT_STEEL}, /* 66 */
	{ 70, 2, 255,     0,     0,  30,   0,   0,  25,   CT_VALUABLES}, /* 67 */
	{ 71, 2, 191,     0,     0,  22,   0,   0,  30,   CT_FOOD}, /* 68 */
	{ 72, 2, 196,     0,     0,  18,   0,   0,  25,   CT_PAPER}, /* 69 */
	{ 73, 2, 179,     0,     0,  19,   0,   0,  35,   CT_COPPER_ORE}, /* 70 */
	{ 47, 2, 199,     0,     0,  25,   0,   0,  30,   CT_WATER}, /* 71 */
	{ 48, 2, 182,     0,     0,  18,   0,   0,  30,   CT_FRUIT}, /* 72 */
	{ 49, 2, 185,     0,     0,  19,   0,   0,  26,   CT_RUBBER}, /* 73 */
	{ 50, 2, 176,     0,     0,  19,   0,   0,  35,   CT_SUGAR}, /* 74 */
	{ 51, 2, 178,     0,     0,  20,   0,   0,  35,   CT_COTTON_CANDY}, /* 75 */
	{ 52, 2, 192,     0,     0,  20,   0,   0,  35,   CT_TOFFEE}, /* 76 */
	{ 53, 2, 190,     0,     0,  21,   0,   0,  25,   CT_BUBBLES}, /* 77 */
	{ 54, 2, 182,     0,     0,  24,   0,   0,  30,   CT_COLA}, /* 78 */
	{ 55, 2, 181,     0,     0,  21,   0,   0,  30,   CT_CANDY}, /* 79 */
	{ 56, 2, 183,     0,     0,  21,   0,   0,  25,   CT_TOYS}, /* 80 */
	{ 57, 2, 196,     0,     0,  18,   0,   0,  27,   CT_BATTERIES}, /* 81 */
	{ 58, 2, 193,     0,     0,  18,   0,   0,  30,   CT_FIZZY_DRINKS}, /* 82 */
	{ 59, 2, 191,     0,     0,  18,   0,   0,  35,   CT_PLASTIC}, /* 83 */
	{ 28, 0,  70,   400, 10000, 105, 250,   2,   0,   0}, /* 84 */
	{ 29, 0,  74,   448, 12000, 120, 253,   2,   0,   0}, /* 85 */
	{ 30, 0,  82,   480, 15000, 130, 254,   2,   0,   0}, /* 86 */
	{ 31, 1,  95,   640,20000/2,150, 255/2, 2,   0,   0}, /* 87 */
	{ 28, 0,  70,   480, 10000, 120, 250,   2,   0,   0}, /* 88 */
	{ 60, 2, 247,     0,     0,  25,   0,   0,  47,   CT_PASSENGERS}, /* 89 */
	{ 62, 2, 228,     0,     0,  21,   0,   0,  37,   CT_MAIL}, /* 90 */
	{ 61, 2, 176,     0,     0,  18,   0,   0,  37,   CT_COAL}, /* 91 */
	{ 63, 2, 200,     0,     0,  24,   0,   0,  37,   CT_OIL}, /* 92 */
	{ 64, 2, 192,     0,     0,  20,   0,   0,  32,   CT_LIVESTOCK}, /* 93 */
	{ 65, 2, 190,     0,     0,  21,   0,   0,  32,   CT_GOODS}, /* 94 */
	{ 66, 2, 182,     0,     0,  19,   0,   0,  37,   CT_GRAIN}, /* 95 */
	{ 67, 2, 181,     0,     0,  16,   0,   0,  37,   CT_WOOD}, /* 96 */
	{ 68, 2, 179,     0,     0,  19,   0,   0,  37,   CT_IRON_ORE}, /* 97 */
	{ 69, 2, 196,     0,     0,  18,   0,   0,  27,   CT_STEEL}, /* 98 */
	{ 70, 2, 255,     0,     0,  30,   0,   0,  27,   CT_VALUABLES}, /* 99 */
	{ 71, 2, 191,     0,     0,  22,   0,   0,  32,   CT_FOOD}, /* 100 */
	{ 72, 2, 196,     0,     0,  18,   0,   0,  27,   CT_PAPER}, /* 101 */
	{ 73, 2, 179,     0,     0,  19,   0,   0,  37,   CT_COPPER_ORE}, /* 102 */
	{ 47, 2, 199,     0,     0,  25,   0,   0,  32,   CT_WATER}, /* 103 */
	{ 48, 2, 182,     0,     0,  18,   0,   0,  32,   CT_FRUIT}, /* 104 */
	{ 49, 2, 185,     0,     0,  19,   0,   0,  28,   CT_RUBBER}, /* 105 */
	{ 50, 2, 176,     0,     0,  19,   0,   0,  37,   CT_SUGAR}, /* 106 */
	{ 51, 2, 178,     0,     0,  20,   0,   0,  37,   CT_COTTON_CANDY}, /* 107 */
	{ 52, 2, 192,     0,     0,  20,   0,   0,  37,   CT_TOFFEE}, /* 108 */
	{ 53, 2, 190,     0,     0,  21,   0,   0,  27,   CT_BUBBLES}, /* 109 */
	{ 54, 2, 182,     0,     0,  24,   0,   0,  32,   CT_COLA}, /* 110 */
	{ 55, 2, 181,     0,     0,  21,   0,   0,  32,   CT_CANDY}, /* 111 */
	{ 56, 2, 183,     0,     0,  21,   0,   0,  27,   CT_TOYS}, /* 112 */
	{ 57, 2, 196,     0,     0,  18,   0,   0,  29,   CT_BATTERIES}, /* 113 */
	{ 58, 2, 193,     0,     0,  18,   0,   0,  32,   CT_FIZZY_DRINKS}, /* 114 */
	{ 59, 2, 191,     0,     0,  18,   0,   0,  37,   CT_PLASTIC}, /* 115 */
};


ShipVehicleInfo _ship_vehicle_info[] = {
	// image_index  cargo_type     cargo_amount  refittable
	// |  base_cost |              |    running_cost
	// |  |    max_speed           |    |    sfx |
	// |  |    |    |              |    |    |   |
	{  1, 160, 48,  CT_OIL,        220, 140, 4,  0 }, /* 0 */
	{  1, 176, 80,  CT_OIL,        350, 125, 4, 0 }, /* 1 */
	{  2, 96,  64,  CT_PASSENGERS, 100, 90,  5, 0 }, /* 2 */
	{  2, 112, 128, CT_PASSENGERS, 130, 80,  5, 0 }, /* 3 */
	{  3, 148, 224, CT_PASSENGERS, 100, 190, 5, 0 }, /* 4 */
	{  2, 96,  64,  CT_PASSENGERS, 100, 90,  5, 0 }, /* 5 */
	{  2, 112, 128, CT_PASSENGERS, 130, 80,  5, 0 }, /* 6 */
	{  0, 128, 48,  CT_GOODS,      160, 150, 4, 1 }, /* 7 */
	{  0, 144, 80,  CT_GOODS,      190, 113, 4, 1 }, /* 8 */
	{  0, 128, 48,  CT_GOODS,      160, 150, 4, 1 }, /* 9 */
	{  0, 144, 80,  CT_GOODS,      190, 113, 4, 1 }, /* 10 */
};
