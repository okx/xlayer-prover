#include "keccak_f_executor.hpp"
#include "utils.hpp"
#include "exit_process.hpp"
#include "zkassert.hpp"
#include "zklog.hpp"

void KeccakFExecutor::loadScript(json j)
{
    if (!j.contains("program") ||
        !j["program"].is_array())
    {
        zklog.error("KeccakFExecutor::loadEvals() found JSON object does not contain not a program array");
        exitProcess();
    }
    for (uint64_t i = 0; i < j["program"].size(); i++)
    {
        if (!j["program"][i].is_object())
        {
            zklog.error("KeccakFExecutor::loadEvals() found JSON array's element is not an object");
            exitProcess();
        }
        if (!j["program"][i].contains("op") ||
            !j["program"][i]["op"].is_string())
        {
            zklog.error("KeccakFExecutor::loadEvals() found JSON array's element does not contain string op field");
            exitProcess();
        }
        if (!j["program"][i].contains("ref") ||
            !j["program"][i]["ref"].is_number_unsigned() ||
            j["program"][i]["ref"] >= KeccakGateConfig.maxRefs)
        {
            zklog.error("KeccakFExecutor::loadEvals() found JSON array's element does not contain unsigned number ref field");
            exitProcess();
        }
        if (!j["program"][i].contains("a") ||
            !j["program"][i]["a"].is_object())
        {
            zklog.error("KeccakFExecutor::loadEvals() found JSON array's element does not contain object a field");
            exitProcess();
        }
        if (!j["program"][i].contains("b") ||
            !j["program"][i]["b"].is_object())
        {
            zklog.error("KeccakFExecutor::loadEvals() found JSON array's element does not contain object b field");
            exitProcess();
        }
        if (!j["program"][i]["a"].contains("type") ||
            !j["program"][i]["a"]["type"].is_string())
        {
            zklog.error("KeccakFExecutor::loadEvals() found JSON array's element does not contain string a type field");
            exitProcess();
        }
        if (!j["program"][i]["b"].contains("type") ||
            !j["program"][i]["b"]["type"].is_string())
        {
            zklog.error("KeccakFExecutor::loadEvals() found JSON array's element does not contain string b type field");
            exitProcess();
        }

        KeccakInstruction instruction;

        // Get gate operation and reference
        if (j["program"][i]["op"] == "xor")
        {
            instruction.op = gop_xor;
        }
        else if (j["program"][i]["op"] == "andp")
        {
            instruction.op = gop_andp;
        }
        else
        {
            string opString = j[i]["op"];
            zklog.error("KeccakFExecutor::loadEvals() found invalid op value: " + opString);
            exitProcess();
        }
        instruction.refr = j["program"][i]["ref"];

        // Get input a pin data
        string typea = j["program"][i]["a"]["type"];
        if (typea == "wired")
        {
            instruction.refa = j["program"][i]["a"]["gate"];
            string pina = j["program"][i]["a"]["pin"];
            instruction.pina = string2pin(pina);
        }
        else if (typea == "input")
        {
            uint64_t bit = j["program"][i]["a"]["bit"];
            instruction.refa = KeccakGateConfig.sinRef0 + bit * 44;
            instruction.pina = PinId::pin_a;
        }
        else
        {
            zklog.error("KeccakFExecutor::loadEvals() found invalid a type value: " + typea);
            exitProcess();
        }

        // Get input b pin data
        string typeb = j["program"][i]["b"]["type"];
        if (typeb == "wired")
        {
            instruction.refb = j["program"][i]["b"]["gate"];
            string pinb = j["program"][i]["b"]["pin"];
            instruction.pinb = string2pin(pinb);
        }
        else if (typeb == "input")
        {
            uint64_t bit = j["program"][i]["b"]["bit"];
            instruction.refb = KeccakGateConfig.sinRef0 + bit * 44;
            instruction.pinb = PinId::pin_a;
        }
        else
        {
            zklog.error("KeccakFExecutor::loadEvals() found invalid b type value: " + typeb);
            exitProcess();
        }

        program.push_back(instruction);
    }

    zkassert(j["maxRef"] == KeccakGateConfig.slotSize);

    bLoaded = true;
}

void KeccakFExecutor::execute(KeccakFExecuteInput &input, KeccakFExecuteOutput &output)
{
    // Reset polynomials
    memset(output.pol, 0, sizeof(output.pol));

    const uint64_t numberOfSlots = ((KeccakGateConfig.polLength - 1) / KeccakGateConfig.slotSize);
    const uint64_t keccakMask = 0xFFFFFFFFFFF;

    // Set KeccakGateConfig.zeroRef values
    output.pol[pin_a][KeccakGateConfig.zeroRef] = 0;
    output.pol[pin_b][KeccakGateConfig.zeroRef] = keccakMask;

    // Set Sin and Rin values
    for (uint64_t slot = 0; slot < numberOfSlots; slot++)
    {
        for (uint64_t row = 0; row < 9; row++)
        {
            uint64_t mask = uint64_t(1) << row;
            for (uint64_t i = 0; i < 1600; i++)
            {
                if (input.Sin[slot][row][i] == 1)
                {
                    output.pol[pin_a][KeccakGateConfig.relRef2AbsRef(KeccakGateConfig.sinRef0 + i * 44, slot)] |= mask;
                }
            }
        }
    }

    // Execute the program
    KeccakInstruction instruction;
    for (uint64_t slot = 0; slot < numberOfSlots; slot++)
    {
        for (uint64_t i = 0; i < program.size(); i++)
        {
            instruction = program[i];
            uint64_t absRefa = KeccakGateConfig.relRef2AbsRef(instruction.refa, slot);
            uint64_t absRefb = KeccakGateConfig.relRef2AbsRef(instruction.refb, slot);
            uint64_t absRefr = KeccakGateConfig.relRef2AbsRef(instruction.refr, slot);

            output.pol[pin_a][absRefr] = output.pol[instruction.pina][absRefa];
            output.pol[pin_b][absRefr] = output.pol[instruction.pinb][absRefb];

            /*if (instruction.refr==(3200*44+1) || instruction.refr==((3200*44)+2) || instruction.refr==1 || instruction.refr==KeccakGateConfig.slotSize)
            {
                zklog.info("slot=" + to_string(slot) + " i=" + to_string(i) + "/" + to_string(program.size()) + " refa=" + to_string(instruction.refa) + " absRefa=" + to_string(absRefa) + " refb=" + to_string(instruction.refb) + " absRefb=" + to_string(absRefb) + " refr=" + to_string(instruction.refr) + " absRefr=" + to_string(absRefr));
            }*/

            switch (program[i].op)
            {
            case gop_xor:
                output.pol[pin_r][absRefr] = (output.pol[instruction.pina][absRefa] ^ output.pol[instruction.pinb][absRefb]) & keccakMask;
                break;

            case gop_andp:
                output.pol[pin_r][absRefr] = ((~output.pol[instruction.pina][absRefa]) & output.pol[instruction.pinb][absRefb]) & keccakMask;
                break;

            default:
                zklog.error("KeccakFExecutor::execute() found invalid op: " + to_string(program[i].op) + " in evaluation: " + to_string(i));
                exitProcess();
            }
        }
    }
}

/* Input is fe[54][1600], output is KeccakPols */
void KeccakFExecutor::execute(const Goldilocks::Element *input, const uint64_t inputLength, KeccakFCommitPols &pols)
{
    if (inputLength != numberOfSlots * 1600)
    {
        zklog.error("KeccakFExecutor::execute() got input size=" + to_string(inputLength) + " different from numberOfSlots=" + to_string(numberOfSlots) + "x1600");
        exitProcess();
    }
    vector<vector<Goldilocks::Element>> inputVector;
    for (uint64_t slot = 0; slot < numberOfSlots; slot++)
    {
        vector<Goldilocks::Element> aux;
        for (uint64_t i = 0; i < 1600; i++)
        {
            aux.push_back(input[slot * 1600 + i]);
        }
        inputVector.push_back(aux);
    }
    execute(inputVector, pols);
}

/* Input is a vector of numberOfSlots*1600 fe, output is KeccakPols */
void KeccakFExecutor::execute(const vector<vector<Goldilocks::Element>> &input, KeccakFCommitPols &pols)
{
    const uint64_t keccakMask = 0xFFFFFFFFFFF;
    
    // Check input size
    if (input.size() != numberOfSlots)
    {
        zklog.error("KeccakFExecutor::execute() got input.size()=" + to_string(input.size()) + " different from numberOfSlots=" + to_string(numberOfSlots));
        exitProcess();
    }

    // Check number of slots, per input
    for (uint64_t i = 0; i < numberOfSlots; i++)
    {
        if (input[i].size() != 1600)
        {
            zklog.error("KeccakFExecutor::execute() got input i=" + to_string(i) + " size=" + to_string(input[i].size()) + " different from 1600");
            exitProcess();
        }
    }

    // Set KeccakGateConfig.zeroRef values
    for (uint64_t i = 0; i < 4; i++)
    {
        pols.a[i][KeccakGateConfig.zeroRef] = fr.zero();
        pols.b[i][KeccakGateConfig.zeroRef] = fr.fromU64(0x7FF);
        pols.c[i][KeccakGateConfig.zeroRef] = fr.fromU64(fr.toU64(pols.a[i][KeccakGateConfig.zeroRef]) ^ fr.toU64(pols.b[i][KeccakGateConfig.zeroRef]));
    }

    // Set Sin values
    for (uint64_t slot = 0; slot < numberOfSlots; slot++)
    {
        for (uint64_t i = 0; i < 1600; i++)
        {
            setPol(pols.a, KeccakGateConfig.relRef2AbsRef(KeccakGateConfig.sinRef0 + i * 44, slot), fr.toU64(input[slot][i]));
        }
    }

    // Execute the program
#pragma omp parallel for
    for (uint64_t slot = 0; slot < numberOfSlots; slot++)
    {
        for (uint64_t i = 0; i < program.size(); i++)
        {
            uint64_t absRefa = KeccakGateConfig.relRef2AbsRef(program[i].refa, slot);
            uint64_t absRefb = KeccakGateConfig.relRef2AbsRef(program[i].refb, slot);
            uint64_t absRefr = KeccakGateConfig.relRef2AbsRef(program[i].refr, slot);

            switch (program[i].pina)
            {
            case pin_a:
                setPol(pols.a, absRefr, getPol(pols.a, absRefa));
                break;
            case pin_b:
                setPol(pols.a, absRefr, getPol(pols.b, absRefa));
                break;
            case pin_r:
                setPol(pols.a, absRefr, getPol(pols.c, absRefa));
                break;
            default:
                zklog.error("KeccakFExecutor() found invalid program[i].pina=" + to_string(program[i].pina));
                exitProcess();
            }
            switch (program[i].pinb)
            {
            case pin_a:
                setPol(pols.b, absRefr, getPol(pols.a, absRefb));
                break;
            case pin_b:
                setPol(pols.b, absRefr, getPol(pols.b, absRefb));
                break;
            case pin_r:
                setPol(pols.b, absRefr, getPol(pols.c, absRefb));
                break;
            default:
                zklog.error("KeccakFExecutor() found invalid program[i].pinb=" + to_string(program[i].pinb));
                exitProcess();
            }

            /*if (program[i].refr==(3200*44+1) || program[i].refr==((3200*44)+2) || program[i].refr==1 || program[i].refr==KeccakGateConfig.slotSize)
            {
                zklog.info("slot=" + to_string(slot) + " i=" + to_string(i) + "/" + to_string(program.size()) + " refa=" + to_string(program[i].refa) + " absRefa=" + to_string(absRefa) + " refb=" + to_string(program[i].refb) + " absRefb=" + to_string(absRefb) + " refr=" + to_string(program[i].refr) + " absRefr=" + to_string(absRefr));
            }*/

            switch (program[i].op)
            {
            case gop_xor:
            {
                setPol(pols.c, absRefr, (getPol(pols.a, absRefr) ^ getPol(pols.b, absRefr)) & keccakMask);
                break;
            }
            case gop_andp:
            {
                setPol(pols.c, absRefr, ((~getPol(pols.a, absRefr)) & getPol(pols.b, absRefr)) & keccakMask);
                break;
            }
            default:
            {
                zklog.error("KeccakFExecutor::execute() found invalid op: " + to_string(program[i].op) + " in evaluation: " + to_string(i));
                exitProcess();
            }
            }
        }
    }

    zklog.info("KeccakFExecutor successfully processed " + to_string(numberOfSlots) + " Keccak-F actions (" + to_string((double(input.size()) * KeccakGateConfig.slotSize * 100) / N) + "%)");
}

void KeccakFExecutor::setPol(CommitPol (&pol)[4], uint64_t index, uint64_t value)
{
    pol[0][index] = fr.fromU64(value & 0x7FF);
    value = value >> 11;
    pol[1][index] = fr.fromU64(value & 0x7FF);
    value = value >> 11;
    pol[2][index] = fr.fromU64(value & 0x7FF);
    value = value >> 11;
    pol[3][index] = fr.fromU64(value & 0x7FF);
}

uint64_t KeccakFExecutor::getPol(CommitPol (&pol)[4], uint64_t index)
{
    return (uint64_t(1) << 33) * fr.toU64(pol[3][index]) + (uint64_t(1) << 22) * fr.toU64(pol[2][index]) + (uint64_t(1) << 11) * fr.toU64(pol[1][index]) + fr.toU64(pol[0][index]);
}
