#include <iostream>
#include "input.hpp"
#include "scalar.hpp"

void preprocessTxs (Context &ctx, json &input)
{
    // Input JSON file must contain a oldStateRoot key at the root level
    if ( !input.contains("oldStateRoot") ||
         !input["oldStateRoot"].is_string() )
    {
        cerr << "Error: oldStateRoot key not found in input JSON file" << endl;
        exit(-1);
    }
    ctx.oldStateRoot = input["oldStateRoot"];
    cout << "preprocessTxs(): oldStateRoot=" << ctx.oldStateRoot << endl;

    // Input JSON file must contain a oldStateRoot key at the root level
    if ( !input.contains("newStateRoot") ||
         !input["newStateRoot"].is_string() )
    {
        cerr << "Error: newStateRoot key not found in input JSON file" << endl;
        exit(-1);
    }
    ctx.newStateRoot = input["newStateRoot"];
    cout << "preprocessTxs(): newStateRoot=" << ctx.newStateRoot << endl;

    // Input JSON file must contain a sequencerAddr key at the root level
    if ( !input.contains("sequencerAddr") ||
         !input["sequencerAddr"].is_string() )
    {
        cerr << "Error: sequencerAddr key not found in input JSON file" << endl;
        exit(-1);
    }
    ctx.sequencerAddr = input["sequencerAddr"];
    cout << "preprocessTxs(): sequencerAddr=" << ctx.sequencerAddr << endl;

    // Input JSON file must contain a chainId key at the root level
    if ( !input.contains("chainId") ||
         !input["chainId"].is_number_unsigned() )
    {
        cerr << "Error: chainId key not found in input JSON file" << endl;
        exit(-1);
    }
    ctx.chainId = input["chainId"];
    cout << "preprocessTxs(): chainId=" << ctx.chainId << endl;

    vector<string> d;
    d.push_back(NormalizeTo0xNFormat(ctx.sequencerAddr,64));
    mpz_class aux(ctx.chainId);
    d.push_back(NormalizeTo0xNFormat(aux.get_str(16),4));
    d.push_back(NormalizeTo0xNFormat(ctx.oldStateRoot,64));

    // Input JSON file must contain a txs string array at the root level
    if ( !input.contains("txs") ||
         !input["txs"].is_array() )
    {
        cerr << "Error: txs key not found in input JSON file" << endl;
        exit(-1);
    }
    for (int i=0; i<input["txs"].size(); i++)
    {
        string tx = input["txs"][i];
        ctx.txs.push_back(tx);
        cout << "preprocessTxs(): tx=" << tx << endl;

        /*
        const rtx = ethers.utils.RLP.decode(ctx.input.txs[i]);
        const chainId = (Number(rtx[6]) - 35) >> 1;
        const sign = Number(rtx[6])  & 1;
        const e =[rtx[0], rtx[1], rtx[2], rtx[3], rtx[4], rtx[5], ethers.utils.hexlify(chainId), "0x", "0x"];
        const signData = ethers.utils.RLP.encode( e );
        ctx.pTxs.push({
            signData: signData,
            signature: {
                r: rtx[7],
                s: rtx[8],
                v: sign + 26
            }
        });
        d.push(signData);
        */

    }

    d.push_back(NormalizeTo0xNFormat(ctx.newStateRoot,64));

    // TODO: Today we are returning a hardcoded globalHadh.  To remove when we migrate the code bellow.
    ctx.globalHash = "0x0dc6cef191fc335eae3d56a871bece8a7b68dc4bab126c6531aaa6d8fc7e77e4";
    //ctx.globalHash = ethers.utils.keccak256(ctx.globalHash = ethers.utils.concat(d));

    // Input JSON file must contain a keys structure at the root level
    if ( !input.contains("keys") ||
         !input["keys"].is_structured() )
    {
        cerr << "Error: keys key not found in input JSON file" << endl;
        exit(-1);
    }
    cout << "keys content:" << endl;
    for (json::iterator it = input["keys"].begin(); it != input["keys"].end(); ++it)
    {
        ctx.keys[it.key()] = it.value();
        cout << "key: " << it.key() << " value: " << it.value() << endl;
    }

    // Input JSON file must contain a db structure at the root level
    if ( !input.contains("db") ||
         !input["db"].is_structured() )
    {
        cerr << "Error: db key not found in input JSON file" << endl;
        exit(-1);
    }
    cout << "db content:" << endl;
    for (json::iterator it = input["db"].begin(); it != input["db"].end(); ++it)
    {
        if (!it.value().is_array() ||
            !it.value().size()==16)
        {
            cerr << "Error: keys value not a 16-elements array in input JSON file: " << it.value() << endl;
            exit(-1);
        }
        vector<RawFr::Element> dbValue;
        for (int i=0; i<16; i++)
        {
            RawFr::Element auxFe;
            ctx.fr.fromString(auxFe, it.value()[i]);
            dbValue.push_back(auxFe);
        }
        RawFr::Element key;
        ctx.fr.fromString(key, it.key());
        ctx.db[key] = dbValue;
        cout << "key: " << it.key() << " value: " << it.value()[0] << " etc." << endl;
    }
}
/* TODO: Migrate this code

function preprocessTxs(ctx) {
    ctx.pTxs = [];
    const d = [];
    d.push(ethers.utils.hexZeroPad(ethers.utils.hexlify(ethers.BigNumber.from(ctx.input.sequencerAddr)), 32));
    d.push(ethers.utils.hexZeroPad(ethers.utils.hexlify(ethers.BigNumber.from(ctx.input.chainId)), 2));
    d.push(ethers.utils.hexZeroPad(ethers.utils.hexlify(ethers.BigNumber.from(ctx.input.oldStateRoot)), 32));
    for (let i=0; i<ctx.input.txs.length; i++) {
        const rtx = ethers.utils.RLP.decode(ctx.input.txs[i]);
        const chainId = (Number(rtx[6]) - 35) >> 1;
        const sign = Number(rtx[6])  & 1;
        const e =[rtx[0], rtx[1], rtx[2], rtx[3], rtx[4], rtx[5], ethers.utils.hexlify(chainId), "0x", "0x"];
        const signData = ethers.utils.RLP.encode( e );
        ctx.pTxs.push({
            signData: signData,
            signature: {
                r: rtx[7],
                s: rtx[8],
                v: sign + 26
            }
        });
        d.push(signData);
    }
    d.push(ethers.utils.hexZeroPad(ethers.utils.hexlify(ethers.BigNumber.from(ctx.input.newStateRoot)), 32));
    ctx.globalHash = ethers.utils.keccak256(ctx.globalHash = ethers.utils.concat(d));
}*/