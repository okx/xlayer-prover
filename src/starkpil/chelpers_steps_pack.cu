#include "zklog.hpp"
#include <inttypes.h>

#if defined(__USE_CUDA__) && defined(ENABLE_EXPERIMENTAL_CODE)

#include "chelpers_steps_pack.cuh"
#include "goldilocks_cubic_extension.cuh"
#include "cuda_utils.cuh"
#include "cuda_utils.hpp"
#include "timer.hpp"

#include <iostream>
#include <fstream>
#include <cstdint>

bool writeDataToFile(const std::string& filename, const uint64_t* data, size_t size) {
    // 打开文件
    std::ofstream file(filename, std::ios_base::app);
    if (file.is_open()) {
        // 逐行写入数据
        file << std::hex;
        for (size_t i = 0; i < size; i++) {
            file << (data[i]) << std::endl; //18446744069414584321
        }
        // 关闭文件
        file.close();
        std::cout << "Data written to file successfully!" << std::endl;
        return true;
    } else {
        std::cerr << "Unable to open file." << std::endl;
        return false;
    }
}

bool writeGoldilocksToFile(const std::string& filename, const Goldilocks::Element* data, size_t size) {
    // 打开文件
    std::ofstream file(filename, std::ios_base::app);
    if (file.is_open()) {
        // 逐行写入数据
        for (size_t i = 0; i < size; i++) {
            file << Goldilocks::toU64(data[i]) << std::endl;
        }
        // 关闭文件
        file.close();
        std::cout << "Data8 written to file successfully!" << std::endl;
        return true;
    } else {
        std::cerr << "Unable to open file." << std::endl;
        return false;
    }
}

void check_eq(const std::string& name, const uint64_t *a, const uint64_t *b, size_t n) {
    printf("check u64 %s, n%lu\n", name.c_str(), n);
    for (uint64_t i=0; i<n; i++) {
        if (a[i] != b[i]) {
            printf("name:%s, i:%lu, left:%lu, right:%lu\n", name.c_str(), i, a[i], b[i]);
            assert(0);
        }
    }
}

void check_eq(const std::string& name, const uint16_t *a, const uint16_t *b, uint32_t n) {
    printf("check u16 %s, n:%u\n", name.c_str(), n);
    for (uint64_t i=0; i<n; i++) {
        if (a[i] != b[i]) {
            printf("name:%s, i:%lu, left:%u, right:%u\n", name.c_str(), i, a[i], b[i]);
            assert(0);
        }
    }
}

void check_eq(const std::string& name, const uint8_t *a, const uint8_t *b, uint32_t n) {
    printf("check u8 %s, n:%u\n", name.c_str(), n);
    for (uint64_t i=0; i<n; i++) {
        if (a[i] != b[i]) {
            printf("name:%s, i:%lu, left:%u, right:%u\n", name.c_str(), i, a[i], b[i]);
            assert(0);
        }
    }
}

__global__ void pack_kernel(uint64_t nrowsPack,
                            uint32_t nOps,
                            uint32_t nArgs,
                            uint64_t nBufferT,
                            uint64_t nTemp1,
                            uint64_t nTemp3,
                            gl64_t *tmp1,
                            gl64_t *tmp3,
                            uint64_t *nColsStagesAcc,
                            uint8_t *ops,
                            uint16_t *args,
                            gl64_t *bufferT_,
                            gl64_t *challenges,
                            gl64_t *challenges_ops,
                            gl64_t *numbers,
                            gl64_t *publics,
                            gl64_t *evals);

const int64_t parallel = 1;

void CHelpersStepsPackGPU::prepareGPU(StarkInfo &starkInfo, StepsParams &params, ParserArgs &parserArgs, ParserParams &parserParams) {
    prepare(starkInfo, params, parserArgs, parserParams);
    printf("into cuda prepare...\n");
    cudaInput = (Goldilocks::Element *)malloc(2*nCols*nrowsPack * sizeof(uint64_t)*parallel);
    cudaOutput = (Goldilocks::Element *)malloc(2*nCols*nrowsPack * sizeof(uint64_t)*parallel);
//    writeDataToFile("challenges2.txt", (uint64_t *)challenges, params.challenges.degree()*FIELD_EXTENSION*nrowsPack);
//    writeDataToFile("challenges_ops2.txt", (uint64_t *)challenges_ops, params.challenges.degree()*FIELD_EXTENSION*nrowsPack);
//    writeDataToFile("numbers_2.txt", (uint64_t *)numbers_, parserParams.nNumbers*nrowsPack);
//    writeDataToFile("publics2.txt", (uint64_t *)publics, starkInfo.nPublics*nrowsPack);
//    writeDataToFile("evals2.txt", (uint64_t *)evals, params.evals.degree()*FIELD_EXTENSION*nrowsPack);

    CHECKCUDAERR(cudaMalloc(&nColsStagesAcc_d, nColsStagesAcc.size() * sizeof(uint64_t)));
    CHECKCUDAERR(cudaMemcpy(nColsStagesAcc_d, nColsStagesAcc.data(), nColsStagesAcc.size() * sizeof(uint64_t), cudaMemcpyHostToDevice));

    uint32_t nOps = parserArgs.nOps - parserParams.opsOffset;
    CHECKCUDAERR(cudaMalloc(&ops_d, nOps * sizeof(uint8_t)));
    CHECKCUDAERR(cudaMemcpy(ops_d, &parserArgs.ops[parserParams.opsOffset], nOps * sizeof(uint8_t), cudaMemcpyHostToDevice));

    uint32_t nArgs = parserArgs.nArgs - parserParams.argsOffset;
    printf("cuda nArgs:%u\n", nArgs);
    CHECKCUDAERR(cudaMalloc(&args_d, nArgs * sizeof(uint16_t)));
    CHECKCUDAERR(cudaMemcpy(args_d, &parserArgs.args[parserParams.argsOffset], nArgs * sizeof(uint16_t), cudaMemcpyHostToDevice));

//    writeData8ToFile("ops2.txt", &parserArgs.ops[parserParams.opsOffset], parserArgs.nOps - parserParams.opsOffset);
//    writeData16ToFile("args2.txt", &parserArgs.args[parserParams.argsOffset], parserArgs.nArgs - parserParams.argsOffset);

    CHECKCUDAERR(cudaMalloc(&challenges_d, challenges.size() * sizeof(uint64_t)));
    CHECKCUDAERR(cudaMemcpy(challenges_d, challenges.data(), challenges.size() * sizeof(uint64_t), cudaMemcpyHostToDevice));

    CHECKCUDAERR(cudaMalloc(&challenges_ops_d, challenges_ops.size() * sizeof(uint64_t)));
    CHECKCUDAERR(cudaMemcpy(challenges_ops_d, challenges_ops.data(), challenges_ops.size() * sizeof(uint64_t), cudaMemcpyHostToDevice));

    CHECKCUDAERR(cudaMalloc(&numbers_d, numbers_.size() * sizeof(uint64_t)));
    CHECKCUDAERR(cudaMemcpy(numbers_d, numbers_.data(), numbers_.size() * sizeof(uint64_t), cudaMemcpyHostToDevice));

    CHECKCUDAERR(cudaMalloc(&publics_d, publics.size() * sizeof(uint64_t)));
    CHECKCUDAERR(cudaMemcpy(publics_d, publics.data(), publics.size() * sizeof(uint64_t), cudaMemcpyHostToDevice));

    CHECKCUDAERR(cudaMalloc(&evals_d, evals.size() * sizeof(uint64_t)));
    CHECKCUDAERR(cudaMemcpy(evals_d, evals.data(), evals.size() * sizeof(uint64_t), cudaMemcpyHostToDevice));

    nColsStagesAcc2.resize(nColsStagesAcc.size());
    CHECKCUDAERR(cudaMemcpy(nColsStagesAcc2.data(), nColsStagesAcc_d, nColsStagesAcc2.size() * sizeof(uint64_t), cudaMemcpyDeviceToHost));
    check_eq("nColsStagesAcc", (uint64_t *)nColsStagesAcc.data(), (uint64_t *)nColsStagesAcc2.data(), nColsStagesAcc2.size());

    args2 = (uint16_t *)malloc(nArgs *sizeof(uint16_t));
    CHECKCUDAERR(cudaMemcpy(args2, args_d, nArgs * sizeof(uint16_t), cudaMemcpyDeviceToHost));
    check_eq("args", args, args2, nArgs);

    ops2 = (uint8_t *)malloc(nOps*sizeof(uint8_t));
    CHECKCUDAERR(cudaMemcpy(ops2, ops_d, nOps * sizeof(uint8_t), cudaMemcpyDeviceToHost));
    check_eq("ops", ops, ops2, nOps);

    challenges2.resize(params.challenges.degree()*FIELD_EXTENSION*nrowsPack);
    CHECKCUDAERR(cudaMemcpy(challenges2.data(), challenges_d, challenges2.size() * sizeof(uint64_t), cudaMemcpyDeviceToHost));
    check_eq("challenges", (uint64_t *)challenges.data(), (uint64_t *)challenges2.data(), challenges2.size());

    challenges_ops2.resize(params.challenges.degree()*FIELD_EXTENSION*nrowsPack);
    CHECKCUDAERR(cudaMemcpy(challenges_ops2.data(), challenges_ops_d, challenges_ops2.size() * sizeof(uint64_t), cudaMemcpyDeviceToHost));
    check_eq("challenges_ops", (uint64_t *)challenges_ops.data(), (uint64_t *)challenges_ops2.data(), challenges_ops2.size());

    numbers_2.resize(parserParams.nNumbers*nrowsPack);
    CHECKCUDAERR(cudaMemcpy(numbers_2.data(), numbers_d, numbers_2.size() * sizeof(uint64_t), cudaMemcpyDeviceToHost));
    check_eq("numbers_", (uint64_t *)numbers_.data(), (uint64_t *)numbers_2.data(), numbers_2.size());

    publics2.resize(starkInfo.nPublics*nrowsPack);
    CHECKCUDAERR(cudaMemcpy(publics2.data(), publics_d, publics2.size() * sizeof(uint64_t), cudaMemcpyDeviceToHost));
    check_eq("publics", (uint64_t *)publics.data(), (uint64_t *)publics2.data(), publics2.size());

    evals2.resize(params.evals.degree()*FIELD_EXTENSION*nrowsPack);
    CHECKCUDAERR(cudaMemcpy(evals2.data(), evals_d, evals2.size() * sizeof(uint64_t), cudaMemcpyDeviceToHost));
    check_eq("evals", (uint64_t *)evals.data(), (uint64_t *)evals2.data(), evals2.size());
}

void CHelpersStepsPackGPU::cleanupGPU() {
    cudaFree(nColsStagesAcc_d);
    cudaFree(ops_d);
    cudaFree(args_d);
    cudaFree(challenges_d);
    cudaFree(challenges_ops_d);
    cudaFree(numbers_d);
    cudaFree(publics_d);
    cudaFree(evals_d);
}

void CHelpersStepsPackGPU::calculateExpressions(StarkInfo &starkInfo, StepsParams &params, ParserArgs &parserArgs, ParserParams &parserParams) {
    printf("into cuda calculateExpressions...\n");
    setBufferTInfo(starkInfo, parserParams.stage);
    prepareGPU(starkInfo, params, parserArgs, parserParams);

    bool domainExtended = parserParams.stage > 3 ? true : false;
    uint64_t domainSize = domainExtended ? 1 << starkInfo.starkStruct.nBitsExt : 1 << starkInfo.starkStruct.nBits;
    calculateExpressionsRowsGPU(starkInfo, params, parserArgs, parserParams, 0, nrowsPack*parallel);
    cleanupGPU();
    for (uint64_t p = 0; p < parallel; p++) {
        calculateExpressionsRows(starkInfo, params, parserArgs, parserParams, nrowsPack*p, nrowsPack*(p+1));
        for (uint64_t i = 0; i<2*nCols*nrowsPack; i++) {
            if (Goldilocks::toU64(input[i]) != Goldilocks::toU64(cudaInput[p*2*nCols*nrowsPack+i])) {
                printf("input not equal, p:%lu, i:%lu, left:%lu, right:%lu\n", p, i, Goldilocks::toU64(input[i]), Goldilocks::toU64(cudaInput[p*2*nCols*nrowsPack+i]));
                assert(0);
            }
            if (Goldilocks::toU64(output[i]) != Goldilocks::toU64(cudaOutput[p*2*nCols*nrowsPack+i])) {
                printf("output not equal, p:%lu, i:%lu, left:%lu, right:%lu\n", p, i, Goldilocks::toU64(output[i]), Goldilocks::toU64(cudaOutput[p*2*nCols*nrowsPack+i]));
                assert(0);
            }
        }
    }
    calculateExpressionsRows(starkInfo, params, parserArgs, parserParams, nrowsPack*parallel, domainSize);

}

#include <iostream>
#include <fstream>
#include <cstdint>

void CHelpersStepsPackGPU::calculateExpressionsRowsGPU(StarkInfo &starkInfo, StepsParams &params, ParserArgs &parserArgs, ParserParams &parserParams,
    uint64_t rowIni, uint64_t rowEnd){

    bool domainExtended = parserParams.stage > 3 ? true : false;
    uint64_t domainSize = domainExtended ? 1 << starkInfo.starkStruct.nBitsExt : 1 << starkInfo.starkStruct.nBits;
    uint8_t *storePol = &parserArgs.storePols[parserParams.storePolsOffset];

    if(rowEnd < rowIni || rowEnd > domainSize) {
        zklog.info("Invalid range for rowIni and rowEnd");
        exitProcess();
    }
    if((rowEnd -rowIni) % nrowsPack != 0) {
       nrowsPack = 1;
    }

    printf("nCols:%lu\n", nCols);
    printf("nrowsPack:%lu\n", nrowsPack);
    printf("buffer:%lu\n", 2*nCols*nrowsPack);
    printf("tmp1:%lu\n", parserParams.nTemp1*nrowsPack);
    printf("tmp3:%lu\n", parserParams.nTemp3*FIELD_EXTENSION*nrowsPack);

    printf("params.pConstPols, degree:%lu, nCols:%lu\n", params.pConstPols->degree(), params.pConstPols->numPols());
    printf("params.pConstPols2ns, degree:%lu, nCols:%lu\n", params.pConstPols2ns->degree(), params.pConstPols2ns->numPols());


    CHECKCUDAERR(cudaSetDevice(0));

    Goldilocks::Element *bufferT_ = (Goldilocks::Element *)get_pinned_mem();
    //Goldilocks::Element bufferT_[2*nCols*nrowsPack*parallel];
    gl64_t *bufferT_d;
    CHECKCUDAERR(cudaMalloc(&bufferT_d, 2*nCols*nrowsPack * sizeof(uint64_t)*parallel));

    gl64_t *tmp1_d;
    gl64_t *tmp3_d;
    CHECKCUDAERR(cudaMalloc(&tmp1_d, parserParams.nTemp1*nrowsPack * sizeof(uint64_t) *parallel));
    CHECKCUDAERR(cudaMalloc(&tmp3_d, parserParams.nTemp3*FIELD_EXTENSION*nrowsPack * sizeof(uint64_t)*parallel));

    for (uint64_t i = rowIni; i < rowEnd; i+= nrowsPack*parallel) {
        printf("rows:%lu\n", i);
        memset(bufferT_, 0, 2*nCols*nrowsPack*parallel*sizeof(uint64_t));
#pragma omp parallel for
        for (uint64_t j = 0; j < parallel; j++) {
            loadPolinomials(starkInfo, params, bufferT_ + 2*nCols*nrowsPack*j, i+nrowsPack*j, parserParams.stage, nrowsPack, domainExtended);
        }

        if (i == 0) {
            memcpy(cudaInput, bufferT_, 2*nCols*nrowsPack* sizeof(Goldilocks::Element));
            writeDataToFile("input2.txt", (uint64_t *)bufferT_, 2*nCols*nrowsPack);
        }


        //TimerStart(Memcpy_H_to_D);
        CHECKCUDAERR(cudaMemcpy(bufferT_d, bufferT_, 2*nCols*nrowsPack * sizeof(uint64_t) *parallel, cudaMemcpyHostToDevice));
        //TimerStopAndLog(Memcpy_H_to_D);
        //TimerStart(Kernel_Func);
        pack_kernel<<<(parallel+15)/16,16>>>(nrowsPack, parserParams.nOps, parserParams.nArgs, 2*nCols*nrowsPack, parserParams.nTemp1*nrowsPack, parserParams.nTemp3*FIELD_EXTENSION*nrowsPack, tmp1_d, tmp3_d, nColsStagesAcc_d, ops_d, args_d, bufferT_d, challenges_d, challenges_ops_d, numbers_d, publics_d, evals_d);
        //TimerStopAndLog(Kernel_Func);
        //TimerStart(Memcpy_D_to_H);
        CHECKCUDAERR(cudaMemcpy(bufferT_, bufferT_d, 2*nCols*nrowsPack * sizeof(uint64_t) *parallel, cudaMemcpyDeviceToHost));
        //TimerStopAndLog(Memcpy_D_to_H);

        //writeDataToFile("output2.txt", (uint64 *)bufferT_, 2*nCols*nrowsPack);
        //assert(0);

//        for (uint64_t j = 0; j < parallel; j++) {
//            writeDataToFile("buffer2.txt", (uint64_t *)bufferT_ + 2*nCols*nrowsPack*j, 2*nCols*nrowsPack);
//        }
//
//        if (i == nrowsPack*parallel) {
//            assert(0);
//        }

        if (i == 0) {
            memcpy(cudaOutput, bufferT_, 2*nCols*nrowsPack* sizeof(Goldilocks::Element));
            writeDataToFile("output2.txt", (uint64_t *)bufferT_, 2*nCols*nrowsPack);
        }

#pragma omp parallel for
        for (uint64_t j = 0; j < parallel; j++) {
            storePolinomials(starkInfo, params, bufferT_ + 2*nCols*nrowsPack*j, storePol, i+nrowsPack*j, nrowsPack, domainExtended);
        }
    }

    cudaFree(bufferT_d);
    cudaFree(tmp1_d);
    cudaFree(tmp3_d);
}

__global__ void pack_kernel(uint64_t nrowsPack,
                            uint32_t nOps,
                            uint32_t nArgs,
                            uint64_t nBufferT,
                            uint64_t nTemp1,
                            uint64_t nTemp3,
                            gl64_t *tmp1,
                            gl64_t *tmp3,
                            uint64_t *nColsStagesAcc,
                            uint8_t *ops,
                            uint16_t *args,
                            gl64_t *bufferT_,
                            gl64_t *challenges,
                            gl64_t *challenges_ops,
                            gl64_t *numbers_,
                            gl64_t *publics,
                            gl64_t *evals)
{
    uint64_t idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx >= parallel) {
        return;
    }

    bufferT_ = bufferT_ + nBufferT*idx;
    tmp1 = tmp1 + nTemp1*idx;
    tmp3 = tmp3 + nTemp3*idx;

    uint64_t i_args = 0;

    for (uint64_t kk = 0; kk < nOps; ++kk) {
        switch (ops[kk]) {
            case 0: {
                // COPY commit1 to commit1
                gl64_t::copy_pack(nrowsPack, &bufferT_[(nColsStagesAcc[args[i_args]] + args[i_args + 1]) * nrowsPack], &bufferT_[(nColsStagesAcc[args[i_args + 2]] + args[i_args + 3]) * nrowsPack]);
                i_args += 4;
                break;
            }
            case 1: {
                // OPERATION WITH DEST: commit1 - SRC0: commit1 - SRC1: commit1
                gl64_t::op_pack(nrowsPack, args[i_args], &bufferT_[(nColsStagesAcc[args[i_args + 1]] + args[i_args + 2]) * nrowsPack], &bufferT_[(nColsStagesAcc[args[i_args + 3]] + args[i_args + 4]) * nrowsPack], &bufferT_[(nColsStagesAcc[args[i_args + 5]] + args[i_args + 6]) * nrowsPack]);
                i_args += 7;
                break;
            }
            case 2: {
                // OPERATION WITH DEST: commit1 - SRC0: commit1 - SRC1: tmp1
                gl64_t::op_pack(nrowsPack, args[i_args], &bufferT_[(nColsStagesAcc[args[i_args + 1]] + args[i_args + 2]) * nrowsPack], &bufferT_[(nColsStagesAcc[args[i_args + 3]] + args[i_args + 4]) * nrowsPack], &tmp1[args[i_args + 5] * nrowsPack]);
                i_args += 6;
                break;
            }
            case 3: {
                // OPERATION WITH DEST: commit1 - SRC0: commit1 - SRC1: public
                gl64_t::op_pack(nrowsPack, args[i_args], &bufferT_[(nColsStagesAcc[args[i_args + 1]] + args[i_args + 2]) * nrowsPack], &bufferT_[(nColsStagesAcc[args[i_args + 3]] + args[i_args + 4]) * nrowsPack], &publics[args[i_args + 5] * nrowsPack]);
                i_args += 6;
                break;
            }
            case 4: {
                // OPERATION WITH DEST: commit1 - SRC0: commit1 - SRC1: number
                gl64_t::op_pack(nrowsPack, args[i_args], &bufferT_[(nColsStagesAcc[args[i_args + 1]] + args[i_args + 2]) * nrowsPack], &bufferT_[(nColsStagesAcc[args[i_args + 3]] + args[i_args + 4]) * nrowsPack], &numbers_[args[i_args + 5]*nrowsPack]);
                i_args += 6;
                break;
            }
            case 5: {
                // COPY tmp1 to commit1
                gl64_t::copy_pack(nrowsPack, &bufferT_[(nColsStagesAcc[args[i_args]] + args[i_args + 1]) * nrowsPack], &tmp1[args[i_args + 2] * nrowsPack]);
                i_args += 3;
                break;
            }
            case 6: {
                // OPERATION WITH DEST: commit1 - SRC0: tmp1 - SRC1: tmp1
                gl64_t::op_pack(nrowsPack, args[i_args], &bufferT_[(nColsStagesAcc[args[i_args + 1]] + args[i_args + 2]) * nrowsPack], &tmp1[args[i_args + 3] * nrowsPack], &tmp1[args[i_args + 4] * nrowsPack]);
                i_args += 5;
                break;
            }
            case 7: {
                // OPERATION WITH DEST: commit1 - SRC0: tmp1 - SRC1: public
                gl64_t::op_pack(nrowsPack, args[i_args], &bufferT_[(nColsStagesAcc[args[i_args + 1]] + args[i_args + 2]) * nrowsPack], &tmp1[args[i_args + 3] * nrowsPack], &publics[args[i_args + 4] * nrowsPack]);
                i_args += 5;
                break;
            }
            case 8: {
                // OPERATION WITH DEST: commit1 - SRC0: tmp1 - SRC1: number
                gl64_t::op_pack(nrowsPack, args[i_args], &bufferT_[(nColsStagesAcc[args[i_args + 1]] + args[i_args + 2]) * nrowsPack], &tmp1[args[i_args + 3] * nrowsPack], &numbers_[args[i_args + 4]*nrowsPack]);
                i_args += 5;
                break;
            }
            case 9: {
                // COPY public to commit1
                gl64_t::copy_pack(nrowsPack, &bufferT_[(nColsStagesAcc[args[i_args]] + args[i_args + 1]) * nrowsPack], &publics[args[i_args + 2] * nrowsPack]);
                i_args += 3;
                break;
            }
            case 10: {
                // OPERATION WITH DEST: commit1 - SRC0: public - SRC1: public
                gl64_t::op_pack(nrowsPack, args[i_args], &bufferT_[(nColsStagesAcc[args[i_args + 1]] + args[i_args + 2]) * nrowsPack], &publics[args[i_args + 3] * nrowsPack], &publics[args[i_args + 4] * nrowsPack]);
                i_args += 5;
                break;
            }
            case 11: {
                // OPERATION WITH DEST: commit1 - SRC0: public - SRC1: number
                gl64_t::op_pack(nrowsPack, args[i_args], &bufferT_[(nColsStagesAcc[args[i_args + 1]] + args[i_args + 2]) * nrowsPack], &publics[args[i_args + 3] * nrowsPack], &numbers_[args[i_args + 4]*nrowsPack]);
                i_args += 5;
                break;
            }
            case 12: {
                // COPY number to commit1
                gl64_t::copy_pack(nrowsPack, &bufferT_[(nColsStagesAcc[args[i_args]] + args[i_args + 1]) * nrowsPack], &numbers_[args[i_args + 2]*nrowsPack]);
                i_args += 3;
                break;
            }
            case 13: {
                // OPERATION WITH DEST: commit1 - SRC0: number - SRC1: number
                gl64_t::op_pack(nrowsPack, args[i_args], &bufferT_[(nColsStagesAcc[args[i_args + 1]] + args[i_args + 2]) * nrowsPack], &numbers_[args[i_args + 3]*nrowsPack], &numbers_[args[i_args + 4]*nrowsPack]);
                i_args += 5;
                break;
            }
            case 14: {
                // COPY commit1 to tmp1
                gl64_t::copy_pack(nrowsPack, &tmp1[args[i_args] * nrowsPack], &bufferT_[(nColsStagesAcc[args[i_args + 1]] + args[i_args + 2]) * nrowsPack]);
                i_args += 3;
                break;
            }
            case 15: {
                // OPERATION WITH DEST: tmp1 - SRC0: commit1 - SRC1: commit1
                gl64_t::op_pack(nrowsPack, args[i_args], &tmp1[args[i_args + 1] * nrowsPack], &bufferT_[(nColsStagesAcc[args[i_args + 2]] + args[i_args + 3]) * nrowsPack], &bufferT_[(nColsStagesAcc[args[i_args + 4]] + args[i_args + 5]) * nrowsPack]);
                i_args += 6;
                break;
            }
            case 16: {
                // OPERATION WITH DEST: tmp1 - SRC0: commit1 - SRC1: tmp1
                gl64_t::op_pack(nrowsPack, args[i_args], &tmp1[args[i_args + 1] * nrowsPack], &bufferT_[(nColsStagesAcc[args[i_args + 2]] + args[i_args + 3]) * nrowsPack], &tmp1[args[i_args + 4] * nrowsPack]);
                i_args += 5;
                break;
            }
            case 17: {
                // OPERATION WITH DEST: tmp1 - SRC0: commit1 - SRC1: public
                gl64_t::op_pack(nrowsPack, args[i_args], &tmp1[args[i_args + 1] * nrowsPack], &bufferT_[(nColsStagesAcc[args[i_args + 2]] + args[i_args + 3]) * nrowsPack], &publics[args[i_args + 4] * nrowsPack]);
                i_args += 5;
                break;
            }
            case 18: {
                // OPERATION WITH DEST: tmp1 - SRC0: commit1 - SRC1: number
                gl64_t::op_pack(nrowsPack, args[i_args], &tmp1[args[i_args + 1] * nrowsPack], &bufferT_[(nColsStagesAcc[args[i_args + 2]] + args[i_args + 3]) * nrowsPack], &numbers_[args[i_args + 4]*nrowsPack]);
                i_args += 5;
                break;
            }
            case 19: {
                // COPY tmp1 to tmp1
                gl64_t::copy_pack(nrowsPack, &tmp1[args[i_args] * nrowsPack], &tmp1[args[i_args + 1] * nrowsPack]);
                i_args += 2;
                break;
            }
            case 20: {
                // OPERATION WITH DEST: tmp1 - SRC0: tmp1 - SRC1: tmp1
                gl64_t::op_pack(nrowsPack, args[i_args], &tmp1[args[i_args + 1] * nrowsPack], &tmp1[args[i_args + 2] * nrowsPack], &tmp1[args[i_args + 3] * nrowsPack]);
                i_args += 4;
                break;
            }
            case 21: {
                // OPERATION WITH DEST: tmp1 - SRC0: tmp1 - SRC1: public
                gl64_t::op_pack(nrowsPack, args[i_args], &tmp1[args[i_args + 1] * nrowsPack], &tmp1[args[i_args + 2] * nrowsPack], &publics[args[i_args + 3] * nrowsPack]);
                i_args += 4;
                break;
            }
            case 22: {
                // OPERATION WITH DEST: tmp1 - SRC0: tmp1 - SRC1: number
                gl64_t::op_pack(nrowsPack, args[i_args], &tmp1[args[i_args + 1] * nrowsPack], &tmp1[args[i_args + 2] * nrowsPack], &numbers_[args[i_args + 3]*nrowsPack]);
                i_args += 4;
                break;
            }
            case 23: {
                // COPY public to tmp1
                gl64_t::copy_pack(nrowsPack, &tmp1[args[i_args] * nrowsPack], &publics[args[i_args + 1] * nrowsPack]);
                i_args += 2;
                break;
            }
            case 24: {
                // OPERATION WITH DEST: tmp1 - SRC0: public - SRC1: public
                gl64_t::op_pack(nrowsPack, args[i_args], &tmp1[args[i_args + 1] * nrowsPack], &publics[args[i_args + 2] * nrowsPack], &publics[args[i_args + 3] * nrowsPack]);
                i_args += 4;
                break;
            }
            case 25: {
                // OPERATION WITH DEST: tmp1 - SRC0: public - SRC1: number
                gl64_t::op_pack(nrowsPack, args[i_args], &tmp1[args[i_args + 1] * nrowsPack], &publics[args[i_args + 2] * nrowsPack], &numbers_[args[i_args + 3]*nrowsPack]);
                i_args += 4;
                break;
            }
            case 26: {
                // COPY number to tmp1
                gl64_t::copy_pack(nrowsPack, &tmp1[args[i_args] * nrowsPack], &numbers_[args[i_args + 1]*nrowsPack]);
                i_args += 2;
                break;
            }
            case 27: {
                // OPERATION WITH DEST: tmp1 - SRC0: number - SRC1: number
                gl64_t::op_pack(nrowsPack, args[i_args], &tmp1[args[i_args + 1] * nrowsPack], &numbers_[args[i_args + 2]*nrowsPack], &numbers_[args[i_args + 3]*nrowsPack]);
                i_args += 4;
                break;
            }
            case 28: {
                // OPERATION WITH DEST: commit3 - SRC0: commit3 - SRC1: commit1
                Goldilocks3GPU::op_31_pack(nrowsPack, args[i_args], &bufferT_[(nColsStagesAcc[args[i_args + 1]] + args[i_args + 2]) * nrowsPack], &bufferT_[(nColsStagesAcc[args[i_args + 3]] + args[i_args + 4]) * nrowsPack], &bufferT_[(nColsStagesAcc[args[i_args + 5]] + args[i_args + 6]) * nrowsPack]);
                i_args += 7;
                break;
            }
            case 29: {
                // OPERATION WITH DEST: commit3 - SRC0: commit3 - SRC1: tmp1
                Goldilocks3GPU::op_31_pack(nrowsPack, args[i_args], &bufferT_[(nColsStagesAcc[args[i_args + 1]] + args[i_args + 2]) * nrowsPack], &bufferT_[(nColsStagesAcc[args[i_args + 3]] + args[i_args + 4]) * nrowsPack], &tmp1[args[i_args + 5] * nrowsPack]);
                i_args += 6;
                break;
            }
            case 30: {
                // OPERATION WITH DEST: commit3 - SRC0: commit3 - SRC1: public
                Goldilocks3GPU::op_31_pack(nrowsPack, args[i_args], &bufferT_[(nColsStagesAcc[args[i_args + 1]] + args[i_args + 2]) * nrowsPack], &bufferT_[(nColsStagesAcc[args[i_args + 3]] + args[i_args + 4]) * nrowsPack], &publics[args[i_args + 5] * nrowsPack]);
                i_args += 6;
                break;
            }
            case 31: {
                // OPERATION WITH DEST: commit3 - SRC0: commit3 - SRC1: number
                Goldilocks3GPU::op_31_pack(nrowsPack, args[i_args], &bufferT_[(nColsStagesAcc[args[i_args + 1]] + args[i_args + 2]) * nrowsPack], &bufferT_[(nColsStagesAcc[args[i_args + 3]] + args[i_args + 4]) * nrowsPack], &numbers_[args[i_args + 5]*nrowsPack]);
                i_args += 6;
                break;
            }
            case 32: {
                // OPERATION WITH DEST: commit3 - SRC0: tmp3 - SRC1: commit1
                Goldilocks3GPU::op_31_pack(nrowsPack, args[i_args], &bufferT_[(nColsStagesAcc[args[i_args + 1]] + args[i_args + 2]) * nrowsPack], &tmp3[args[i_args + 3] * nrowsPack * FIELD_EXTENSION], &bufferT_[(nColsStagesAcc[args[i_args + 4]] + args[i_args + 5]) * nrowsPack]);
                i_args += 6;
                break;
            }
            case 33: {
                // OPERATION WITH DEST: commit3 - SRC0: tmp3 - SRC1: tmp1
                Goldilocks3GPU::op_31_pack(nrowsPack, args[i_args], &bufferT_[(nColsStagesAcc[args[i_args + 1]] + args[i_args + 2]) * nrowsPack], &tmp3[args[i_args + 3] * nrowsPack * FIELD_EXTENSION], &tmp1[args[i_args + 4] * nrowsPack]);
                i_args += 5;
                break;
            }
            case 34: {
                // OPERATION WITH DEST: commit3 - SRC0: tmp3 - SRC1: public
                Goldilocks3GPU::op_31_pack(nrowsPack, args[i_args], &bufferT_[(nColsStagesAcc[args[i_args + 1]] + args[i_args + 2]) * nrowsPack], &tmp3[args[i_args + 3] * nrowsPack * FIELD_EXTENSION], &publics[args[i_args + 4] * nrowsPack]);
                i_args += 5;
                break;
            }
            case 35: {
                // OPERATION WITH DEST: commit3 - SRC0: tmp3 - SRC1: number
                Goldilocks3GPU::op_31_pack(nrowsPack, args[i_args], &bufferT_[(nColsStagesAcc[args[i_args + 1]] + args[i_args + 2]) * nrowsPack], &tmp3[args[i_args + 3] * nrowsPack * FIELD_EXTENSION], &numbers_[args[i_args + 4]*nrowsPack]);
                i_args += 5;
                break;
            }
            case 36: {
                // OPERATION WITH DEST: commit3 - SRC0: challenge - SRC1: commit1
                Goldilocks3GPU::op_31_pack(nrowsPack, args[i_args], &bufferT_[(nColsStagesAcc[args[i_args + 1]] + args[i_args + 2]) * nrowsPack], &challenges[args[i_args + 3]*FIELD_EXTENSION*nrowsPack], &bufferT_[(nColsStagesAcc[args[i_args + 4]] + args[i_args + 5]) * nrowsPack]);
                i_args += 6;
                break;
            }
            case 37: {
                // OPERATION WITH DEST: commit3 - SRC0: challenge - SRC1: tmp1
                Goldilocks3GPU::op_31_pack(nrowsPack, args[i_args], &bufferT_[(nColsStagesAcc[args[i_args + 1]] + args[i_args + 2]) * nrowsPack], &challenges[args[i_args + 3]*FIELD_EXTENSION*nrowsPack], &tmp1[args[i_args + 4] * nrowsPack]);
                i_args += 5;
                break;
            }
            case 38: {
                // OPERATION WITH DEST: commit3 - SRC0: challenge - SRC1: public
                Goldilocks3GPU::op_31_pack(nrowsPack, args[i_args], &bufferT_[(nColsStagesAcc[args[i_args + 1]] + args[i_args + 2]) * nrowsPack], &challenges[args[i_args + 3]*FIELD_EXTENSION*nrowsPack], &publics[args[i_args + 4] * nrowsPack]);
                i_args += 5;
                break;
            }
            case 39: {
                // OPERATION WITH DEST: commit3 - SRC0: challenge - SRC1: number
                Goldilocks3GPU::op_31_pack(nrowsPack, args[i_args], &bufferT_[(nColsStagesAcc[args[i_args + 1]] + args[i_args + 2]) * nrowsPack], &challenges[args[i_args + 3]*FIELD_EXTENSION*nrowsPack], &numbers_[args[i_args + 4]*nrowsPack]);
                i_args += 5;
                break;
            }
            case 40: {
                // COPY commit3 to commit3
                Goldilocks3GPU::copy_pack(nrowsPack, &bufferT_[(nColsStagesAcc[args[i_args]] + args[i_args + 1]) * nrowsPack], &bufferT_[(nColsStagesAcc[args[i_args + 2]] + args[i_args + 3]) * nrowsPack]);
                i_args += 4;
                break;
            }
            case 41: {
                // OPERATION WITH DEST: commit3 - SRC0: commit3 - SRC1: commit3
                Goldilocks3GPU::op_pack(nrowsPack, args[i_args], &bufferT_[(nColsStagesAcc[args[i_args + 1]] + args[i_args + 2]) * nrowsPack], &bufferT_[(nColsStagesAcc[args[i_args + 3]] + args[i_args + 4]) * nrowsPack], &bufferT_[(nColsStagesAcc[args[i_args + 5]] + args[i_args + 6]) * nrowsPack]);
                i_args += 7;
                break;
            }
            case 42: {
                // OPERATION WITH DEST: commit3 - SRC0: commit3 - SRC1: tmp3
                Goldilocks3GPU::op_pack(nrowsPack, args[i_args], &bufferT_[(nColsStagesAcc[args[i_args + 1]] + args[i_args + 2]) * nrowsPack], &bufferT_[(nColsStagesAcc[args[i_args + 3]] + args[i_args + 4]) * nrowsPack], &tmp3[args[i_args + 5] * nrowsPack * FIELD_EXTENSION]);
                i_args += 6;
                break;
            }
            case 43: {
                // MULTIPLICATION WITH DEST: commit3 - SRC0: commit3 - SRC1: challenge
                Goldilocks3GPU::mul_pack(nrowsPack, &bufferT_[(nColsStagesAcc[args[i_args + 1]] + args[i_args + 2]) * nrowsPack], &bufferT_[(nColsStagesAcc[args[i_args + 3]] + args[i_args + 4]) * nrowsPack], &challenges[args[i_args + 5]*FIELD_EXTENSION*nrowsPack], &challenges_ops[args[i_args + 5]*FIELD_EXTENSION*nrowsPack]);
                i_args += 6;
                break;
            }
            case 44: {
                // OPERATION WITH DEST: commit3 - SRC0: commit3 - SRC1: challenge
                Goldilocks3GPU::op_pack(nrowsPack, args[i_args], &bufferT_[(nColsStagesAcc[args[i_args + 1]] + args[i_args + 2]) * nrowsPack], &bufferT_[(nColsStagesAcc[args[i_args + 3]] + args[i_args + 4]) * nrowsPack], &challenges[args[i_args + 5]*FIELD_EXTENSION*nrowsPack]);
                i_args += 6;
                break;
            }
            case 45: {
                // COPY tmp3 to commit3
                Goldilocks3GPU::copy_pack(nrowsPack, &bufferT_[(nColsStagesAcc[args[i_args]] + args[i_args + 1]) * nrowsPack], &tmp3[args[i_args + 2] * nrowsPack * FIELD_EXTENSION]);
                i_args += 3;
                break;
            }
            case 46: {
                // OPERATION WITH DEST: commit3 - SRC0: tmp3 - SRC1: tmp3
                Goldilocks3GPU::op_pack(nrowsPack, args[i_args], &bufferT_[(nColsStagesAcc[args[i_args + 1]] + args[i_args + 2]) * nrowsPack], &tmp3[args[i_args + 3] * nrowsPack * FIELD_EXTENSION], &tmp3[args[i_args + 4] * nrowsPack * FIELD_EXTENSION]);
                i_args += 5;
                break;
            }
            case 47: {
                // MULTIPLICATION WITH DEST: commit3 - SRC0: tmp3 - SRC1: challenge
                Goldilocks3GPU::mul_pack(nrowsPack, &bufferT_[(nColsStagesAcc[args[i_args + 1]] + args[i_args + 2]) * nrowsPack], &tmp3[args[i_args + 3] * nrowsPack * FIELD_EXTENSION], &challenges[args[i_args + 4]*FIELD_EXTENSION*nrowsPack], &challenges_ops[args[i_args + 4]*FIELD_EXTENSION*nrowsPack]);
                i_args += 5;
                break;
            }
            case 48: {
                // OPERATION WITH DEST: commit3 - SRC0: tmp3 - SRC1: challenge
                Goldilocks3GPU::op_pack(nrowsPack, args[i_args], &bufferT_[(nColsStagesAcc[args[i_args + 1]] + args[i_args + 2]) * nrowsPack], &tmp3[args[i_args + 3] * nrowsPack * FIELD_EXTENSION], &challenges[args[i_args + 4]*FIELD_EXTENSION*nrowsPack]);
                i_args += 5;
                break;
            }
            case 49: {
                // MULTIPLICATION WITH DEST: commit3 - SRC0: challenge - SRC1: challenge
                Goldilocks3GPU::mul_pack(nrowsPack, &bufferT_[(nColsStagesAcc[args[i_args + 1]] + args[i_args + 2]) * nrowsPack], &challenges[args[i_args + 3]*FIELD_EXTENSION*nrowsPack], &challenges[args[i_args + 4]*FIELD_EXTENSION*nrowsPack], &challenges_ops[args[i_args + 4]*FIELD_EXTENSION*nrowsPack]);
                i_args += 5;
                break;
            }
            case 50: {
                // OPERATION WITH DEST: commit3 - SRC0: challenge - SRC1: challenge
                Goldilocks3GPU::op_pack(nrowsPack, args[i_args], &bufferT_[(nColsStagesAcc[args[i_args + 1]] + args[i_args + 2]) * nrowsPack], &challenges[args[i_args + 3]*FIELD_EXTENSION*nrowsPack], &challenges[args[i_args + 4]*FIELD_EXTENSION*nrowsPack]);
                i_args += 5;
                break;
            }
            case 51: {
                // OPERATION WITH DEST: tmp3 - SRC0: commit3 - SRC1: commit1
                Goldilocks3GPU::op_31_pack(nrowsPack, args[i_args], &tmp3[args[i_args + 1] * nrowsPack * FIELD_EXTENSION], &bufferT_[(nColsStagesAcc[args[i_args + 2]] + args[i_args + 3]) * nrowsPack], &bufferT_[(nColsStagesAcc[args[i_args + 4]] + args[i_args + 5]) * nrowsPack]);
                i_args += 6;
                break;
            }
            case 52: {
                // OPERATION WITH DEST: tmp3 - SRC0: commit3 - SRC1: tmp1
                Goldilocks3GPU::op_31_pack(nrowsPack, args[i_args], &tmp3[args[i_args + 1] * nrowsPack * FIELD_EXTENSION], &bufferT_[(nColsStagesAcc[args[i_args + 2]] + args[i_args + 3]) * nrowsPack], &tmp1[args[i_args + 4] * nrowsPack]);
                i_args += 5;
                break;
            }
            case 53: {
                // OPERATION WITH DEST: tmp3 - SRC0: commit3 - SRC1: public
                Goldilocks3GPU::op_31_pack(nrowsPack, args[i_args], &tmp3[args[i_args + 1] * nrowsPack * FIELD_EXTENSION], &bufferT_[(nColsStagesAcc[args[i_args + 2]] + args[i_args + 3]) * nrowsPack], &publics[args[i_args + 4] * nrowsPack]);
                i_args += 5;
                break;
            }
            case 54: {
                // OPERATION WITH DEST: tmp3 - SRC0: commit3 - SRC1: number
                Goldilocks3GPU::op_31_pack(nrowsPack, args[i_args], &tmp3[args[i_args + 1] * nrowsPack * FIELD_EXTENSION], &bufferT_[(nColsStagesAcc[args[i_args + 2]] + args[i_args + 3]) * nrowsPack], &numbers_[args[i_args + 4]*nrowsPack]);
                i_args += 5;
                break;
            }
            case 55: {
                // OPERATION WITH DEST: tmp3 - SRC0: tmp3 - SRC1: commit1
                Goldilocks3GPU::op_31_pack(nrowsPack, args[i_args], &tmp3[args[i_args + 1] * nrowsPack * FIELD_EXTENSION], &tmp3[args[i_args + 2] * nrowsPack * FIELD_EXTENSION], &bufferT_[(nColsStagesAcc[args[i_args + 3]] + args[i_args + 4]) * nrowsPack]);
                i_args += 5;
                break;
            }
            case 56: {
                // OPERATION WITH DEST: tmp3 - SRC0: tmp3 - SRC1: tmp1
                Goldilocks3GPU::op_31_pack(nrowsPack, args[i_args], &tmp3[args[i_args + 1] * nrowsPack * FIELD_EXTENSION], &tmp3[args[i_args + 2] * nrowsPack * FIELD_EXTENSION], &tmp1[args[i_args + 3] * nrowsPack]);
                i_args += 4;
                break;
            }
            case 57: {
                // OPERATION WITH DEST: tmp3 - SRC0: tmp3 - SRC1: public
                Goldilocks3GPU::op_31_pack(nrowsPack, args[i_args], &tmp3[args[i_args + 1] * nrowsPack * FIELD_EXTENSION], &tmp3[args[i_args + 2] * nrowsPack * FIELD_EXTENSION], &publics[args[i_args + 3] * nrowsPack]);
                i_args += 4;
                break;
            }
            case 58: {
                // OPERATION WITH DEST: tmp3 - SRC0: tmp3 - SRC1: number
                Goldilocks3GPU::op_31_pack(nrowsPack, args[i_args], &tmp3[args[i_args + 1] * nrowsPack * FIELD_EXTENSION], &tmp3[args[i_args + 2] * nrowsPack * FIELD_EXTENSION], &numbers_[args[i_args + 3]*nrowsPack]);
                i_args += 4;
                break;
            }
            case 59: {
                // OPERATION WITH DEST: tmp3 - SRC0: challenge - SRC1: commit1
                Goldilocks3GPU::op_31_pack(nrowsPack, args[i_args], &tmp3[args[i_args + 1] * nrowsPack * FIELD_EXTENSION], &challenges[args[i_args + 2]*FIELD_EXTENSION*nrowsPack], &bufferT_[(nColsStagesAcc[args[i_args + 3]] + args[i_args + 4]) * nrowsPack]);
                i_args += 5;
                break;
            }
            case 60: {
                // OPERATION WITH DEST: tmp3 - SRC0: challenge - SRC1: tmp1
                Goldilocks3GPU::op_31_pack(nrowsPack, args[i_args], &tmp3[args[i_args + 1] * nrowsPack * FIELD_EXTENSION], &challenges[args[i_args + 2]*FIELD_EXTENSION*nrowsPack], &tmp1[args[i_args + 3] * nrowsPack]);
                i_args += 4;
                break;
            }
            case 61: {
                // OPERATION WITH DEST: tmp3 - SRC0: challenge - SRC1: public
                Goldilocks3GPU::op_31_pack(nrowsPack, args[i_args], &tmp3[args[i_args + 1] * nrowsPack * FIELD_EXTENSION], &challenges[args[i_args + 2]*FIELD_EXTENSION*nrowsPack], &publics[args[i_args + 3] * nrowsPack]);
                i_args += 4;
                break;
            }
            case 62: {
                // OPERATION WITH DEST: tmp3 - SRC0: challenge - SRC1: number
                Goldilocks3GPU::op_31_pack(nrowsPack, args[i_args], &tmp3[args[i_args + 1] * nrowsPack * FIELD_EXTENSION], &challenges[args[i_args + 2]*FIELD_EXTENSION*nrowsPack], &numbers_[args[i_args + 3]*nrowsPack]);
                i_args += 4;
                break;
            }
            case 63: {
                // COPY commit3 to tmp3
                Goldilocks3GPU::copy_pack(nrowsPack, &tmp3[args[i_args] * nrowsPack * FIELD_EXTENSION], &bufferT_[(nColsStagesAcc[args[i_args + 1]] + args[i_args + 2]) * nrowsPack]);
                i_args += 3;
                break;
            }
            case 64: {
                // OPERATION WITH DEST: tmp3 - SRC0: commit3 - SRC1: commit3
                Goldilocks3GPU::op_pack(nrowsPack, args[i_args], &tmp3[args[i_args + 1] * nrowsPack * FIELD_EXTENSION], &bufferT_[(nColsStagesAcc[args[i_args + 2]] + args[i_args + 3]) * nrowsPack], &bufferT_[(nColsStagesAcc[args[i_args + 4]] + args[i_args + 5]) * nrowsPack]);
                i_args += 6;
                break;
            }
            case 65: {
                // OPERATION WITH DEST: tmp3 - SRC0: commit3 - SRC1: tmp3
                Goldilocks3GPU::op_pack(nrowsPack, args[i_args], &tmp3[args[i_args + 1] * nrowsPack * FIELD_EXTENSION], &bufferT_[(nColsStagesAcc[args[i_args + 2]] + args[i_args + 3]) * nrowsPack], &tmp3[args[i_args + 4] * nrowsPack * FIELD_EXTENSION]);
                i_args += 5;
                break;
            }
            case 66: {
                // MULTIPLICATION WITH DEST: tmp3 - SRC0: commit3 - SRC1: challenge
                Goldilocks3GPU::mul_pack(nrowsPack, &tmp3[args[i_args + 1] * nrowsPack * FIELD_EXTENSION], &bufferT_[(nColsStagesAcc[args[i_args + 2]] + args[i_args + 3]) * nrowsPack], &challenges[args[i_args + 4]*FIELD_EXTENSION*nrowsPack], &challenges_ops[args[i_args + 4]*FIELD_EXTENSION*nrowsPack]);
                i_args += 5;
                break;
            }
            case 67: {
                // OPERATION WITH DEST: tmp3 - SRC0: commit3 - SRC1: challenge
                Goldilocks3GPU::op_pack(nrowsPack, args[i_args], &tmp3[args[i_args + 1] * nrowsPack * FIELD_EXTENSION], &bufferT_[(nColsStagesAcc[args[i_args + 2]] + args[i_args + 3]) * nrowsPack], &challenges[args[i_args + 4]*FIELD_EXTENSION*nrowsPack]);
                i_args += 5;
                break;
            }
            case 68: {
                // COPY tmp3 to tmp3
                Goldilocks3GPU::copy_pack(nrowsPack, &tmp3[args[i_args] * nrowsPack * FIELD_EXTENSION], &tmp3[args[i_args + 1] * nrowsPack * FIELD_EXTENSION]);
                i_args += 2;
                break;
            }
            case 69: {
                // OPERATION WITH DEST: tmp3 - SRC0: tmp3 - SRC1: tmp3
                Goldilocks3GPU::op_pack(nrowsPack, args[i_args], &tmp3[args[i_args + 1] * nrowsPack * FIELD_EXTENSION], &tmp3[args[i_args + 2] * nrowsPack * FIELD_EXTENSION], &tmp3[args[i_args + 3] * nrowsPack * FIELD_EXTENSION]);
                i_args += 4;
                break;
            }
            case 70: {
                // MULTIPLICATION WITH DEST: tmp3 - SRC0: tmp3 - SRC1: challenge
                Goldilocks3GPU::mul_pack(nrowsPack, &tmp3[args[i_args + 1] * nrowsPack * FIELD_EXTENSION], &tmp3[args[i_args + 2] * nrowsPack * FIELD_EXTENSION], &challenges[args[i_args + 3]*FIELD_EXTENSION*nrowsPack], &challenges_ops[args[i_args + 3]*FIELD_EXTENSION*nrowsPack]);
                i_args += 4;
                break;
            }
            case 71: {
                // OPERATION WITH DEST: tmp3 - SRC0: tmp3 - SRC1: challenge
                Goldilocks3GPU::op_pack(nrowsPack, args[i_args], &tmp3[args[i_args + 1] * nrowsPack * FIELD_EXTENSION], &tmp3[args[i_args + 2] * nrowsPack * FIELD_EXTENSION], &challenges[args[i_args + 3]*FIELD_EXTENSION*nrowsPack]);
                i_args += 4;
                break;
            }
            case 72: {
                // MULTIPLICATION WITH DEST: tmp3 - SRC0: challenge - SRC1: challenge
                Goldilocks3GPU::mul_pack(nrowsPack, &tmp3[args[i_args + 1] * nrowsPack * FIELD_EXTENSION], &challenges[args[i_args + 2]*FIELD_EXTENSION*nrowsPack], &challenges[args[i_args + 3]*FIELD_EXTENSION*nrowsPack], &challenges_ops[args[i_args + 3]*FIELD_EXTENSION*nrowsPack]);
                i_args += 4;
                break;
            }
            case 73: {
                // OPERATION WITH DEST: tmp3 - SRC0: challenge - SRC1: challenge
                Goldilocks3GPU::op_pack(nrowsPack, args[i_args], &tmp3[args[i_args + 1] * nrowsPack * FIELD_EXTENSION], &challenges[args[i_args + 2]*FIELD_EXTENSION*nrowsPack], &challenges[args[i_args + 3]*FIELD_EXTENSION*nrowsPack]);
                i_args += 4;
                break;
            }
            case 74: {
                // COPY eval to tmp3
                Goldilocks3GPU::copy_pack(nrowsPack, &tmp3[args[i_args] * nrowsPack * FIELD_EXTENSION], &evals[args[i_args + 1]*FIELD_EXTENSION*nrowsPack]);
                i_args += 2;
                break;
            }
            case 75: {
                // MULTIPLICATION WITH DEST: tmp3 - SRC0: eval - SRC1: challenge
                Goldilocks3GPU::mul_pack(nrowsPack, &tmp3[args[i_args + 1] * nrowsPack * FIELD_EXTENSION], &evals[args[i_args + 2]*FIELD_EXTENSION*nrowsPack], &challenges[args[i_args + 3]*FIELD_EXTENSION*nrowsPack], &challenges_ops[args[i_args + 3]*FIELD_EXTENSION*nrowsPack]);
                i_args += 4;
                break;
            }
            case 76: {
                // OPERATION WITH DEST: tmp3 - SRC0: challenge - SRC1: eval
                Goldilocks3GPU::op_pack(nrowsPack, args[i_args], &tmp3[args[i_args + 1] * nrowsPack * FIELD_EXTENSION], &challenges[args[i_args + 2]*FIELD_EXTENSION*nrowsPack], &evals[args[i_args + 3]*FIELD_EXTENSION*nrowsPack]);
                i_args += 4;
                break;
            }
            case 77: {
                // OPERATION WITH DEST: tmp3 - SRC0: tmp3 - SRC1: eval
                Goldilocks3GPU::op_pack(nrowsPack, args[i_args], &tmp3[args[i_args + 1] * nrowsPack * FIELD_EXTENSION], &tmp3[args[i_args + 2] * nrowsPack * FIELD_EXTENSION], &evals[args[i_args + 3]*FIELD_EXTENSION*nrowsPack]);
                i_args += 4;
                break;
            }
            case 78: {
                // OPERATION WITH DEST: tmp3 - SRC0: eval - SRC1: commit1
                Goldilocks3GPU::op_31_pack(nrowsPack, args[i_args], &tmp3[args[i_args + 1] * nrowsPack * FIELD_EXTENSION], &evals[args[i_args + 2]*FIELD_EXTENSION*nrowsPack], &bufferT_[(nColsStagesAcc[args[i_args + 3]] + args[i_args + 4]) * nrowsPack]);
                i_args += 5;
                break;
            }
            case 79: {
                // OPERATION WITH DEST: tmp3 - SRC0: commit3 - SRC1: eval
                Goldilocks3GPU::op_pack(nrowsPack, args[i_args], &tmp3[args[i_args + 1] * nrowsPack * FIELD_EXTENSION], &bufferT_[(nColsStagesAcc[args[i_args + 2]] + args[i_args + 3]) * nrowsPack], &evals[args[i_args + 4]*FIELD_EXTENSION*nrowsPack]);
                i_args += 5;
                break;
            }
            default: {
                assert(false);
            }
        }
    }

    assert(i_args == nArgs);

}

#endif