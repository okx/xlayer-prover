#include "goldilocks_cubic_extension.hpp"
#include "zhInv.hpp"
#include "starks.hpp"
#include "constant_pols_starks.hpp"
#include "zkevmSteps.hpp"
#include "zkevm.chelpers.step3.parser.hpp"
#include <immintrin.h>

#define NR_ 4

void ZkevmSteps::step3_parser_first_avx(StepsParams &params, uint64_t nrows, uint64_t nrowsBatch)
{
#pragma omp parallel for
     for (uint64_t i = 0; i < nrows; i += nrowsBatch)
     {
          int i_args = 0;
          __m256i *tmp1 = new __m256i[20000];
          Goldilocks3::Element_avx *tmp3 = new Goldilocks3::Element_avx[20000];
          uint64_t offsets1[4], offsets2[4];
          uint64_t numpols = params.pConstPols2ns->numPols();

          for (int kk = 0; kk < NOPS_; ++kk)
          {
               switch (op3[kk])
               {
               case 0:
               {
                    Goldilocks::add_avx(tmp1[(args3[i_args])], tmp1[args3[i_args + 1]], tmp1[args3[i_args + 2]]);
                    i_args += 3;
                    break;
               }
               case 1:
               {
                    Goldilocks::add_avx(tmp1[(args3[i_args])], tmp1[args3[i_args + 1]], &params.pols[args3[i_args + 2] + i * args3[i_args + 3]], args3[i_args + 3]);
                    i_args += 4;
                    break;
               }
               case 2:
               {
                    Goldilocks::add_avx(tmp1[(args3[i_args])], tmp1[args3[i_args + 1]], Goldilocks::fromU64(args3[i_args + 2]));
                    i_args += 3;
                    break;
               }
               case 3:
               {
                    Goldilocks::add_avx(tmp1[(args3[i_args])], tmp1[args3[i_args + 1]], &params.pConstPols2ns->getElement(args3[i_args + 2], i), numpols);
                    i_args += 3;
                    break;
               }
               case 4:
               {
                    Goldilocks::add_avx(tmp1[(args3[i_args])], &params.pols[args3[i_args + 1] + i * args3[i_args + 2]], &params.pols[args3[i_args + 3] + i * args3[i_args + 4]], args3[i_args + 2], args3[i_args + 4]);
                    i_args += 5;
                    break;
               }
               case 5:
               {
                    for (uint64_t j = 0; j < NR_; ++j)
                    {
                         offsets1[j] = args3[i_args + 1] + (((i + j) + args3[i_args + 2]) % args3[i_args + 3]) * args3[i_args + 4];
                         offsets2[j] = args3[i_args + 5] + (((i + j) + args3[i_args + 6]) % args3[i_args + 7]) * args3[i_args + 8];
                    }
                    Goldilocks::add_avx(tmp1[args3[i_args]], &params.pols[0], &params.pols[0], offsets1, offsets2);
                    i_args += 9;
                    break;
               }
               case 6:
               {
                    Goldilocks::add_avx(tmp1[(args3[i_args])], &params.pols[args3[i_args + 1] + i * args3[i_args + 2]], &params.pConstPols2ns->getElement(args3[i_args + 3], i), args3[i_args + 2], numpols);
                    i_args += 4;
                    break;
               }
               case 7:
               {
                    Goldilocks::add_avx(tmp1[(args3[i_args])], &params.pols[args3[i_args + 1] + i * args3[i_args + 2]], Goldilocks::fromU64(args3[i_args + 3]), args3[i_args + 2]);
                    i_args += 4;
                    break;
               }
               case 8:
               {
                    Goldilocks::add_avx(tmp1[(args3[i_args])], &params.pConstPols2ns->getElement(args3[i_args + 1], i), &params.pConstPols2ns->getElement(args3[i_args + 2], i), numpols, numpols);
                    i_args += 3;
                    break;
               }
               case 9:
               {
                    for (uint64_t j = 0; j < NR_; ++j)
                    {
                         offsets1[j] = args3[i_args + 1] + (((i + j) + args3[i_args + 2]) % args3[i_args + 3]) * numpols;
                         offsets2[j] = args3[i_args + 4] + (((i + j) + args3[i_args + 5]) % args3[i_args + 6]) * numpols;
                    }
                    Goldilocks::add_avx(tmp1[args3[i_args]], &params.pConstPols2ns->getElement(0, 0), &params.pConstPols2ns->getElement(0, 0), offsets1, offsets2);
                    i_args += 7;
                    break;
               }
               case 10:
               {
                    Goldilocks::add_avx(tmp1[(args3[i_args])], &params.pConstPols2ns->getElement(args3[i_args + 1], i), Goldilocks::fromU64(args3[i_args + 2]), numpols);
                    i_args += 3;
                    break;
               }
               case 11:
               {
                    for (uint64_t j = 0; j < NR_; ++j)
                    {
                         offsets1[j] = args3[i_args + 1] + (((i + j) + args3[i_args + 2]) % args3[i_args + 3]) * numpols;
                    }
                    Goldilocks::add_avx(tmp1[args3[i_args]], &params.pConstPols2ns->getElement(0, 0), Goldilocks::fromU64(args3[i_args + 4]), offsets1);
                    i_args += 5;
                    break;
               }
               case 12:
               {

                    Goldilocks3::add13_avx(tmp3[args3[i_args]], tmp1[args3[i_args + 1]], tmp3[args3[i_args + 2]]);
                    i_args += 3;
                    break;
               }
               case 13:
               {
                    Goldilocks3::add1c3c_avx(tmp3[args3[i_args]], Goldilocks::fromU64(args3[i_args + 1]), params.challenges[args3[i_args + 2]]);
                    i_args += 3;
                    break;
               }
               case 14:
               {
                    Goldilocks3::add13c_avx(tmp3[args3[i_args]], tmp1[args3[i_args + 1]], params.challenges[args3[i_args + 2]]);
                    i_args += 3;
                    break;
               }
               case 15:
               {

                    Goldilocks3::add13_avx(tmp3[args3[i_args]], &params.pols[args3[i_args + 1] + i * args3[i_args + 2]], tmp3[args3[i_args + 3]], args3[i_args + 2]);
                    i_args += 4;
                    break;
               }
               case 16:
               {
                    Goldilocks3::add13c_avx(tmp3[args3[i_args]], &params.pols[args3[i_args + 1] + i * args3[i_args + 2]], params.challenges[args3[i_args + 3]], args3[i_args + 2]);
                    i_args += 4;
                    break;
               }
               case 17:
               {
                    Goldilocks3::add_avx(tmp3[args3[i_args]], tmp3[args3[i_args + 1]], tmp3[args3[i_args + 2]]);
                    i_args += 3;
                    break;
               }
               case 18:
               {

                    Goldilocks3::add33c_avx(tmp3[args3[i_args]], tmp3[args3[i_args + 1]], params.challenges[args3[i_args + 2]]);
                    i_args += 3;
                    break;
               }
               case 19:
               {
                    Goldilocks3::add_avx(tmp3[args3[i_args]], &params.pols[args3[i_args + 1] + i * args3[i_args + 2]], tmp3[args3[i_args + 3]], args3[i_args + 2]);
                    i_args += 4;
                    break;
               }
               case 20:
               {
                    Goldilocks3::add33c_avx(tmp3[args3[i_args]], &params.pols[args3[i_args + 1] + i * args3[i_args + 2]], params.challenges[args3[i_args + 3]], args3[i_args + 2]);
                    i_args += 4;
                    break;
               }
               case 21:
               {
                    Goldilocks::sub_avx(tmp1[(args3[i_args])], tmp1[args3[i_args + 1]], tmp1[args3[i_args + 2]]);
                    i_args += 3;
                    break;
               }
               case 22:
               {
                    Goldilocks::sub_avx(tmp1[(args3[i_args])], tmp1[args3[i_args + 1]], &params.pols[args3[i_args + 2] + i * args3[i_args + 3]], args3[i_args + 3]);
                    i_args += 4;
                    break;
               }
               case 23:
               {
                    for (uint64_t j = 0; j < NR_; ++j)
                    {
                         offsets2[j] = args3[i_args + 2] + (((i + j) + args3[i_args + 3]) % args3[i_args + 4]) * args3[i_args + 5];
                    }
                    Goldilocks::sub_avx(tmp1[args3[i_args]], tmp1[args3[i_args + 1]], &params.pols[0], offsets2);
                    i_args += 6;
                    break;
               }
               case 24:
               {
                    Goldilocks::sub_avx(tmp1[(args3[i_args])], &params.pols[args3[i_args + 1] + i * args3[i_args + 2]], tmp1[args3[i_args + 3]], args3[i_args + 2]);
                    i_args += 4;
                    break;
               }
               case 25:
               {

                    for (uint64_t j = 0; j < NR_; ++j)
                    {
                         offsets1[j] = args3[i_args + 1] + (((i + j) + args3[i_args + 2]) % args3[i_args + 3]) * args3[i_args + 4];
                    }
                    Goldilocks::sub_avx(tmp1[args3[i_args]], &params.pols[0], tmp1[args3[i_args + 5]], offsets1);
                    i_args += 6;
                    break;
               }
               case 26:
               {
                    Goldilocks::sub_avx(tmp1[(args3[i_args])], tmp1[args3[i_args + 1]], Goldilocks::fromU64(args3[i_args + 2]));
                    i_args += 3;
                    break;
               }
               case 27:
               {
                    Goldilocks::sub_avx(tmp1[(args3[i_args])], Goldilocks::fromU64(args3[i_args + 1]), tmp1[args3[i_args + 2]]);
                    i_args += 3;
                    break;
               }
               case 28:
               {

                    Goldilocks::sub_avx(tmp1[(args3[i_args])], &params.pols[args3[i_args + 1] + i * args3[i_args + 2]], Goldilocks::fromU64(args3[i_args + 3]), args3[i_args + 2]);
                    i_args += 4;
                    break;
               }
               case 29:
               {
                    for (uint64_t j = 0; j < NR_; ++j)
                    {
                         offsets1[j] = args3[i_args + 1] + (((i + j) + args3[i_args + 2]) % args3[i_args + 3]) * args3[i_args + 4];
                    }
                    Goldilocks::sub_avx(tmp1[args3[i_args]], &params.pols[0], Goldilocks::fromU64(args3[i_args + 5]), offsets1);
                    i_args += 6;
                    break;
               }
               case 30:
               {
                    Goldilocks::sub_avx(tmp1[(args3[i_args])], Goldilocks::fromU64(args3[i_args + 1]), &params.pols[args3[i_args + 2] + i * args3[i_args + 3]], args3[i_args + 3]);
                    i_args += 4;
                    break;
               }
               case 31:
               {
                    for (uint64_t j = 0; j < NR_; ++j)
                    {
                         offsets2[j] = args3[i_args + 2] + (((i + j) + args3[i_args + 3]) % args3[i_args + 4]) * args3[i_args + 5];
                    }
                    Goldilocks::sub_avx(tmp1[args3[i_args]], Goldilocks::fromU64(args3[i_args + 1]), &params.pols[0], offsets2);
                    i_args += 6;
                    break;
               }
               case 32:
               {
                    Goldilocks::sub_avx(tmp1[(args3[i_args])], Goldilocks::fromU64(args3[i_args + 1]), &params.pConstPols2ns->getElement(args3[i_args + 2], i), numpols);
                    i_args += 3;
                    break;
               }
               case 33:
               {
                    for (uint64_t j = 0; j < NR_; ++j)
                    {
                         offsets2[j] = args3[i_args + 2] + (((i + j) + args3[i_args + 3]) % args3[i_args + 4]) * numpols;
                    }
                    Goldilocks::sub_avx(tmp1[args3[i_args]], Goldilocks::fromU64(args3[i_args + 1]), &params.pConstPols2ns->getElement(0, 0), offsets2);
                    i_args += 5;
                    break;
               }
               case 34:
               {
                    Goldilocks::sub_avx(tmp1[(args3[i_args])], &params.pols[args3[i_args + 1] + i * args3[i_args + 2]], params.publicInputs[args3[i_args + 3]], args3[i_args + 2]);
                    i_args += 4;
                    break;
               }
               case 35:
               {
                    for (uint64_t j = 0; j < NR_; ++j)
                    {
                         offsets1[j] = args3[i_args + 1] + (((i + j) + args3[i_args + 2]) % args3[i_args + 3]) * args3[i_args + 4];
                         offsets2[j] = args3[i_args + 5] + (i + j) * args3[i_args + 6];
                    }
                    Goldilocks::sub_avx(tmp1[args3[i_args]], &params.pols[0], &params.pols[0], offsets1, offsets2);
                    i_args += 7;
                    break;
               }
               case 36:
               {
                    for (uint64_t j = 0; j < NR_; ++j)
                    {
                         offsets1[j] = args3[i_args + 1] + (i + j) * args3[i_args + 2];
                         offsets2[j] = args3[i_args + 3] + (((i + j) + args3[i_args + 4]) % args3[i_args + 5]) * args3[i_args + 6];
                    }
                    Goldilocks::sub_avx(tmp1[args3[i_args]], &params.pols[0], &params.pols[0], offsets1, offsets2);
                    i_args += 7;
                    break;
               }
               case 37:
               {
                    Goldilocks::sub_avx(tmp1[(args3[i_args])], &params.pols[args3[i_args + 1] + i * args3[i_args + 2]], &params.pols[args3[i_args + 3] + i * args3[i_args + 4]], args3[i_args + 2], args3[i_args + 4]);
                    i_args += 5;
                    break;
               }
               case 38:
               {
                    for (uint64_t j = 0; j < NR_; ++j)
                    {
                         offsets1[j] = args3[i_args + 1] + (((i + j) + args3[i_args + 2]) % args3[i_args + 3]) * args3[i_args + 4];
                         offsets2[j] = args3[i_args + 5] + (((i + j) + args3[i_args + 6]) % args3[i_args + 7]) * args3[i_args + 8];
                    }
                    Goldilocks::sub_avx(tmp1[args3[i_args]], &params.pols[0], &params.pols[0], offsets1, offsets2);
                    i_args += 9;
                    break;
               }
               case 39:
               {
                    Goldilocks::sub_avx(tmp1[(args3[i_args])], &params.pConstPols2ns->getElement(args3[i_args + 1], i), &params.pols[args3[i_args + 2] + i * args3[i_args + 3]], numpols, args3[i_args + 3]);
                    i_args += 4;
                    break;
               }
               case 40:
               {
                    Goldilocks::sub_avx(tmp1[(args3[i_args])], tmp1[args3[i_args + 1]], &params.pConstPols2ns->getElement(args3[i_args + 2], i), numpols);
                    i_args += 3;
                    break;
               }
               case 41:
               {
                    Goldilocks3::sub31c_avx(tmp3[args3[i_args]], &params.pols[args3[i_args + 1] + i * args3[i_args + 2]], Goldilocks::fromU64(args3[i_args + 3]), args3[i_args + 2]);
                    i_args += 4;
                    break;
               }
               case 42:
               {
                    Goldilocks3::sub_avx(tmp3[args3[i_args]], tmp3[args3[i_args + 1]], tmp3[args3[i_args + 2]]);
                    i_args += 3;
                    break;
               }
               case 43:
               {
                    Goldilocks3::sub33c_avx(tmp3[args3[i_args]], tmp3[args3[i_args + 1]], params.challenges[args3[i_args + 2]]);
                    i_args += 3;
                    break;
               }
               case 44:
               {
                    Goldilocks3::sub_avx(tmp3[args3[i_args]], tmp3[args3[i_args + 1]], &params.pols[args3[i_args + 2] + i * args3[i_args + 3]], args3[i_args + 3]);
                    i_args += 4;
                    break;
               }
               case 45:
               {
                    Goldilocks::mult_avx(tmp1[(args3[i_args])], tmp1[args3[i_args + 1]], tmp1[args3[i_args + 2]]);
                    i_args += 3;
                    break;
               }
               case 46:
               {
                    Goldilocks::mul_avx(tmp1[(args3[i_args])], Goldilocks::fromU64(args3[i_args + 1]), tmp1[args3[i_args + 2]]);
                    i_args += 3;
                    break;
               }
               case 47:
               {
                    Goldilocks::mul_avx(tmp1[(args3[i_args])], &params.pols[args3[i_args + 1] + i * args3[i_args + 2]], tmp1[args3[i_args + 3]], args3[i_args + 2]);
                    i_args += 4;
                    break;
               }
               case 48:
               {
                    for (uint64_t j = 0; j < NR_; ++j)
                    {
                         offsets1[j] = args3[i_args + 1] + (((i + j) + args3[i_args + 2]) % args3[i_args + 3]) * args3[i_args + 4];
                    }
                    Goldilocks::mul_avx(tmp1[args3[i_args]], &params.pols[0], tmp1[args3[i_args + 5]], offsets1);
                    i_args += 6;
                    break;
               }
               case 49:
               {
                    Goldilocks::mul_avx(tmp1[(args3[i_args])], tmp1[args3[i_args + 1]], &params.pConstPols2ns->getElement(args3[i_args + 2], i), numpols);
                    i_args += 3;
                    break;
               }
               case 50:
               {
                    Goldilocks::mul_avx(tmp1[(args3[i_args])], &params.pols[args3[i_args + 1] + i * args3[i_args + 2]], &params.pols[args3[i_args + 3] + i * args3[i_args + 4]], args3[i_args + 2], args3[i_args + 4]);
                    i_args += 5;
                    break;
               }
               case 51:
               {
                    for (uint64_t j = 0; j < NR_; ++j)
                    {
                         offsets1[j] = args3[i_args + 1] + (i + j) * args3[i_args + 2];
                         offsets2[j] = args3[i_args + 3] + (((i + j) + args3[i_args + 4]) % args3[i_args + 5]) * args3[i_args + 6];
                    }
                    Goldilocks::mul_avx(tmp1[args3[i_args]], &params.pols[0], &params.pols[0], offsets1, offsets2);
                    i_args += 7;
                    break;
               }
               case 52:
               {
                    for (uint64_t j = 0; j < NR_; ++j)
                    {
                         offsets1[j] = args3[i_args + 1] + (((i + j) + args3[i_args + 2]) % args3[i_args + 3]) * args3[i_args + 4];
                         offsets2[j] = args3[i_args + 5] + (((i + j) + args3[i_args + 6]) % args3[i_args + 7]) * args3[i_args + 8];
                    }
                    Goldilocks::mul_avx(tmp1[args3[i_args]], &params.pols[0], &params.pols[0], offsets1, offsets2);
                    i_args += 9;
                    break;
               }
               case 53:
               {
                    Goldilocks::mul_avx(tmp1[(args3[i_args])], Goldilocks::fromU64(args3[i_args + 1]), &params.pols[args3[i_args + 2] + i * args3[i_args + 3]], args3[i_args + 3]);
                    i_args += 4;
                    break;
               }
               case 54:
               {
                    Goldilocks::mul_avx(tmp1[(args3[i_args])], &params.pols[args3[i_args + 1] + i * args3[i_args + 2]], &params.pConstPols2ns->getElement(args3[i_args + 3], i), args3[i_args + 2], numpols);
                    i_args += 4;
                    break;
               }
               case 55:
               {
                    for (uint64_t j = 0; j < NR_; ++j)
                    {
                         offsets1[j] = args3[i_args + 1] + (((i + j) + args3[i_args + 2]) % args3[i_args + 3]) * args3[i_args + 4];
                         offsets2[j] = args3[i_args + 5] + (i + j) * numpols;
                    }
                    Goldilocks::mul_avx(tmp1[args3[i_args]], &params.pols[0], &params.pConstPols2ns->getElement(0, 0), offsets1, offsets2);
                    i_args += 6;
                    break;
               }
               case 56:
               {
                    Goldilocks::mul_avx(tmp1[(args3[i_args])], tmp1[args3[i_args + 1]], &params.pols[args3[i_args + 2] + i * args3[i_args + 3]], args3[i_args + 3]);
                    i_args += 4;
                    break;
               }
               case 57:
               {
                    for (uint64_t j = 0; j < NR_; ++j)
                    {
                         offsets2[j] = args3[i_args + 2] + (((i + j) + args3[i_args + 3]) % args3[i_args + 4]) * args3[i_args + 5];
                    }
                    Goldilocks::mul_avx(tmp1[args3[i_args]], tmp1[args3[i_args + 1]], &params.pols[0], offsets2);
                    i_args += 6;
                    break;
               }
               case 58:
               {
                    Goldilocks::mul_avx(tmp1[(args3[i_args])], &params.pConstPols2ns->getElement(args3[i_args + 1], i), tmp1[args3[i_args + 2]], numpols);
                    i_args += 3;
                    break;
               }
               case 59:
               {
                    Goldilocks3::mul13c_avx(tmp3[args3[i_args]], tmp1[args3[i_args + 1]], params.challenges[args3[i_args + 2]]);
                    i_args += 3;
                    break;
               }
               case 60:
               {
                    Goldilocks3::mul13_avx(tmp3[args3[i_args]], &params.pConstPols2ns->getElement(args3[i_args + 1], i), tmp3[args3[i_args + 2]], numpols);
                    i_args += 3;
                    break;
               }
               case 61:
               {

                    Goldilocks3::mul13_avx(tmp3[args3[i_args]], tmp1[args3[i_args + 1]], tmp3[args3[i_args + 2]]);
                    i_args += 3;
                    break;
               }
               case 62:
               {
                    Goldilocks3::mul13c_avx(tmp3[args3[i_args]], &params.pols[args3[i_args + 1] + i * args3[i_args + 2]], params.challenges[args3[i_args + 3]], args3[i_args + 2]);
                    i_args += 4;
                    break;
               }
               case 63:
               {
                    for (uint64_t j = 0; j < NR_; ++j)
                    {
                         offsets1[j] = args3[i_args + 1] + (((i + j) + args3[i_args + 2]) % args3[i_args + 3]) * args3[i_args + 4];
                    }
                    Goldilocks3::mul13c_avx(tmp3[args3[i_args]], &params.pols[0], params.challenges[args3[i_args + 5]], offsets1);
                    i_args += 6;
                    break;
               }
               case 64:
               {
                    Goldilocks3::mul13_avx(tmp3[args3[i_args]], &params.pols[args3[i_args + 1] + i * args3[i_args + 2]], tmp3[args3[i_args + 3]], args3[i_args + 2]);
                    i_args += 4;
                    break;
               }
               case 65:
               {
                    for (uint64_t j = 0; j < NR_; ++j)
                    {
                         offsets1[j] = args3[i_args + 1] + (((i + j) + args3[i_args + 2]) % args3[i_args + 3]) * args3[i_args + 4];
                         offsets2[j] = FIELD_EXTENSION * (j + NR_ * args3[i_args + 5]);
                    }
                    Goldilocks3::mul13_avx(tmp3[args3[i_args]], &params.pols[0], tmp3[args3[i_args + 5]], offsets1);
                    i_args += 6;
                    break;
               }
               case 66:
               {
                    Goldilocks3::mul1c3c_avx(tmp3[args3[i_args]], Goldilocks::fromU64(args3[i_args + 1]), (Goldilocks3::Element &)*params.challenges[args3[i_args + 2]]);
                    i_args += 3;
                    break;
               }
               case 67:
               {
                    Goldilocks3::mul13c_avx(tmp3[args3[i_args]], params.x_2ns[i], (Goldilocks3::Element &)*params.challenges[args3[i_args + 1]], params.x_2ns.offset());
                    i_args += 2;
                    break;
               }
               case 68:
               {
                    Goldilocks3::mul13_avx(tmp3[args3[i_args]], params.x_2ns[i], tmp3[args3[i_args + 1]], params.x_2ns.offset());
                    i_args += 2;
                    break;
               }
               case 69:
               {

                    Goldilocks::Element tmp_inv[3];
                    Goldilocks::Element ti0[4];
                    Goldilocks::Element ti1[4];
                    Goldilocks::Element ti2[4];
                    Goldilocks::store(ti0, tmp3[args3[i_args]][0]);
                    Goldilocks::store(ti1, tmp3[args3[i_args]][1]);
                    Goldilocks::store(ti2, tmp3[args3[i_args]][2]);

                    for (uint64_t j = 0; j < NR_; ++j)
                    {
                         tmp_inv[0] = ti0[j];
                         tmp_inv[1] = ti1[j];
                         tmp_inv[2] = ti2[j];
                         Goldilocks3::mul((Goldilocks3::Element &)(params.q_2ns[(i + j) * 3]),
                                          params.zi.zhInv((i + j)),
                                          (Goldilocks3::Element &)tmp_inv);
                    }
                    i_args += 1;
                    break;
               }
               case 70:
               {

                    Goldilocks3::mul33c_avx(tmp3[args3[i_args]], tmp3[args3[i_args + 2]], params.challenges[args3[i_args + 1]]);
                    i_args += 3;
                    break;
               }
               case 71:
               {
                    Goldilocks3::mul_avx(tmp3[args3[i_args]], tmp3[args3[i_args + 1]], tmp3[args3[i_args + 2]]);
                    i_args += 3;
                    break;
               }
               case 72:
               {
                    Goldilocks3::mul_avx(tmp3[args3[i_args]], &params.pols[args3[i_args + 1] + i * args3[i_args + 2]], &params.pols[args3[i_args + 3] + i * args3[i_args + 4]], args3[i_args + 2], args3[i_args + 4]);
                    i_args += 5;
                    break;
               }
               case 73:
               {
                    for (uint64_t j = 0; j < NR_; ++j)
                    {
                         offsets1[j] = args3[i_args + 1] + (((i + j) + args3[i_args + 2]) % args3[i_args + 3]) * args3[i_args + 4];
                    }

                    Goldilocks3::mul33c_avx(tmp3[args3[i_args]], &params.pols[0], params.challenges[args3[i_args + 5]], offsets1);
                    i_args += 6;
                    break;
               }
               case 74:
               {
                    for (uint64_t j = 0; j < NR_; ++j)
                    {
                         offsets1[j] = args3[i_args + 1] + (((i + j) + args3[i_args + 2]) % args3[i_args + 3]) * args3[i_args + 4];
                         offsets2[j] = FIELD_EXTENSION * (j + NR_ * args3[i_args + 5]);
                    }
                    Goldilocks3::mul_avx(tmp3[args3[i_args]], &params.pols[0], tmp3[args3[i_args + 5]], offsets1);
                    i_args += 6;
                    break;
               }
               case 75:
               {
                    Goldilocks3::mul_avx(tmp3[args3[i_args]], &params.pols[args3[i_args + 1] + i * args3[i_args + 2]], tmp3[args3[i_args + 3]], args3[i_args + 2]);
                    i_args += 4;
                    break;
               }
               case 76:
               {
                    Goldilocks3::mul33c_avx(tmp3[args3[i_args]], &params.pols[args3[i_args + 1] + i * args3[i_args + 2]], params.challenges[args3[i_args + 3]], args3[i_args + 2]);
                    i_args += 4;
                    break;
               }
               case 77:
               {
                    for (uint64_t j = 0; j < NR_; ++j)
                    {
                         offsets1[j] = args3[i_args + 1] + (((i + j) + args3[i_args + 2]) % args3[i_args + 3]) * args3[i_args + 4];
                         offsets2[j] = args3[i_args + 5] + (i + j) * args3[i_args + 6];
                    }
                    Goldilocks3::mul_avx(tmp3[args3[i_args]], &params.pols[0], &params.pols[0], offsets1, offsets2);
                    i_args += 7;
                    break;
               }
               case 78:
               {
                    Goldilocks::copy_avx(tmp1[(args3[i_args])], tmp1[args3[i_args + 1]]);
                    i_args += 2;
                    break;
               }
               case 79:
               {
                    Goldilocks::copy_avx(tmp1[(args3[i_args])], &params.pols[args3[i_args + 1] + i * args3[i_args + 2]], args3[i_args + 2]);
                    i_args += 3;
                    break;
               }
               case 80:
               {
                    for (uint64_t j = 0; j < NR_; ++j)
                    {
                         offsets1[j] = args3[i_args + 1] + (((i + j) + args3[i_args + 2]) % args3[i_args + 3]) * args3[i_args + 4];
                    }
                    Goldilocks::copy_avx(tmp1[args3[i_args]], &params.pols[0], offsets1);
                    i_args += 5;
                    break;
               }
               case 81:
               {
                    Goldilocks::copy_avx(tmp1[(args3[i_args])], Goldilocks::fromU64(args3[i_args + 1]));
                    i_args += 2;
                    break;
               }
               case 82:
               {
                    Goldilocks::copy_avx(tmp1[(args3[i_args])], &params.pConstPols2ns->getElement(args3[i_args + 1], i), numpols);
                    i_args += 2;
                    break;
               }
               case 83:
               {
                    for (uint64_t j = 0; j < NR_; ++j)
                    {
                         offsets1[j] = args3[i_args + 1] + (((i + j) + args3[i_args + 2]) % args3[i_args + 3]) * numpols;
                    }
                    Goldilocks::copy_avx(tmp1[(args3[i_args])], &params.pConstPols2ns->getElement(0, 0), offsets1);

                    i_args += 4;
                    break;
               }
               case 84:
               {
                    for (uint64_t j = 0; j < NR_; ++j)
                    {
                         offsets1[j] = args3[i_args + 2] + (((i + j) + args3[i_args + 3]) % args3[i_args + 4]) * args3[i_args + 5];
                    }
                    Goldilocks::add_avx(tmp1[(args3[i_args])], tmp1[args3[i_args + 1]], &params.pols[0], offsets1);
                    i_args += 6;
                    break;
               }
               case 85:
               {
                    for (uint64_t j = 0; j < NR_; ++j)
                    {
                         offsets1[j] = args3[i_args + 1] + (((i + j) + args3[i_args + 2]) % args3[i_args + 3]) * args3[i_args + 4];
                    }
                    Goldilocks::mul_avx(tmp1[args3[i_args]], &params.pols[0], Goldilocks::fromU64(args3[i_args + 5]), offsets1);
                    i_args += 6;
                    break;
               }
               case 86:
               {
                    Goldilocks::add_avx(&params.pols[args3[i_args] + i * args3[i_args + 1]], args3[i_args + 1], tmp1[args3[i_args + 2]], tmp1[args3[i_args + 3]]);
                    i_args += 4;
                    break;
                    break;
               }
               case 87:
               {
                    Goldilocks::add_avx(&params.pols[args3[i_args] + i * args3[i_args + 1]], args3[i_args + 1], tmp1[args3[i_args + 2]], &params.pols[args3[i_args + 3] + i * args3[i_args + 4]], args3[i_args + 4]);
                    i_args += 5;
                    break;
                    break;
               }
               case 88:
               {
                    Goldilocks3::add13_avx(&params.pols[args3[i_args] + i * args3[i_args + 1]], args3[i_args + 1], tmp1[args3[i_args + 2]], tmp3[args3[i_args + 3]]);
                    i_args += 4;
                    break;
               }
               case 89:
               {
                    Goldilocks3::add_avx(&params.pols[args3[i_args] + i * args3[i_args + 1]], args3[i_args + 1], &params.pols[args3[i_args + 2] + i * args3[i_args + 3]], tmp3[args3[i_args + 4]], args3[i_args + 3]);
                    i_args += 5;
                    break;
               }
               case 90:
               {
                    Goldilocks3::add33c_avx(&params.pols[args3[i_args] + i * args3[i_args + 1]], args3[i_args + 1], tmp3[args3[i_args + 2]], params.challenges[args3[i_args + 3]]);
                    i_args += 4;
                    break;
               }
                    /*


                         case 100:
                         {
                              // Goldilocks::copy_avx(&params.pols[args3[i_args] + i * args3[i_args + 1]], args3[i_args + 1], tmp1[(args3[i_args + 2])]);
                              i_args += 3;
                              break;
                         }
                         case 101:
                         {
                              for (uint64_t j = 0; j < NR_; ++j)
                              {
                                   offsets1[j] = args3[i_args] + (((i + j) + args3[i_args + 1]) % args3[i_args + 2]) * args3[i_args + 3];
                              }
                              // Goldilocks::copy_avx(&params.pols[0], offsets1, tmp1[(args3[i_args + 4])]);
                              i_args += 5;
                              break;
                         }

                         case 102:
                         {
                              for (uint64_t j = 0; j < NR_; ++j)
                              {
                                   offsets1[j] = args3[i_args] + (((i + j) + args3[i_args + 1]) % args3[i_args + 2]) * args3[i_args + 3];
                                   offsets2[j] = args3[i_args + 4] + (((i + j) + args3[i_args + 5]) % args3[i_args + 6]) * args3[i_args + 7];
                              }
                              // Goldilocks::mul_avx(&params.pols[0], offsets1, &params.pConstPols2ns->getElement(0, 0), tmp1[(args3[i_args + 8])], offsets2);
                              i_args += 9;
                              break;
                         }*/

               default:
               {
                    // assert(op3[kk] >= 84 && op3[kk] <= 102);
                    assert(op3[kk] <= 102);

                    break;
                    // std::cout << " Wrong operation in step42ns_first!" << std::endl;
                    // exit(1);
               }
               }
          }
          if (i_args != NARGS_)
               std::cout << " " << i_args << " - " << NARGS_ << std::endl;
          assert(i_args == NARGS_);
          delete (tmp1);
          delete (tmp3);
     }
}
void ZkevmSteps::step3_parser_first(StepsParams &params, uint64_t nrows, uint64_t nrowsBatch)
{
}
