#include "database_cache_test.hpp"
#include "hashdb/database.hpp"
#include "timer.hpp"
#include "scalar.hpp"

#define NUMBER_OF_DB_CACHE_ADDS 1000
#define NUMBER_OF_DB_CACHE_BENCHS 20000000 

uint64_t DatabaseCacheTest (void)
{
    TimerStart(DATABASE_CACHE_TEST);

    
    Database::dbMTCache.clear();
    
    uint64_t numberOfFailed = 0;

    Database::dbMTCache.setMaxSize(2000000);

    Goldilocks fr;
    mpz_class keyScalar;
    string keyString;
    vector<Goldilocks::Element> value;
    bool bResult;
    for (uint64_t i=0; i<NUMBER_OF_DB_CACHE_ADDS; i++)
    {
        keyScalar = i;
        keyString = PrependZeros(keyScalar.get_str(16), 64);
        value.clear();
        for (uint64_t j=0; j<12; j++)
        {
            value.push_back(fr.fromU64(j));
        }
        bool update = false;
        Database::dbMTCache.add(keyString, value, update);
    }

    //Database::dbMTCache.print(true);

    for (uint64_t i=0; i<NUMBER_OF_DB_CACHE_ADDS; i++)
    {
        keyScalar = i;
        keyString = PrependZeros(keyScalar.get_str(16), 64);
        bResult = Database::dbMTCache.find(keyString, value);
        if (!bResult)
        {
            zklog.error("DatabaseCacheTest() failed calling Database::dbMTCache.find() of key=" + keyString);
            numberOfFailed++;
        }
    }
    
    Database::dbMTCache.clear();

    TimerStopAndLog(DATABASE_CACHE_TEST);

    TimerStart(DATABASE_ASSOCIATIVE_CACHE_TEST);

    DatabaseMTAssociativeCache cache1(20, 17, "cache1");
    uint64_t NUMBER_OF_DB_ASSOCIATIVE_CACHE_ADDS = cache1.getCacheSize();
    //test addKeyValue addKeyValue(Goldilocks::Element (&key)[4], const vector<Goldilocks::Element> &value, bool update);

    //generate a set of random keys
    PoseidonGoldilocks poseidon_test;
    for (uint64_t i=0; i<NUMBER_OF_DB_ASSOCIATIVE_CACHE_ADDS; i++)
    {
        Goldilocks::Element key[4];
        vector<Goldilocks::Element> InputValue;
        for(int k=0; k<12; ++k){
            InputValue.push_back(fr.fromU64(i));
        } 
        poseidon_test.hash(key,(Goldilocks::Element(&)[12])(InputValue.data()[0]));
        cache1.addKeyValue(key, InputValue, false);
    }
    //test findKey
    for (uint64_t i=0; i<NUMBER_OF_DB_ASSOCIATIVE_CACHE_ADDS; i++)
    {
        Goldilocks::Element key[4];
        vector<Goldilocks::Element> value;
        for(int k=0; k<12; ++k){
            value.push_back(fr.fromU64(i));
        } 
        poseidon_test.hash(key,(Goldilocks::Element(&)[12])(value.data()[0]));
        value.clear();
        bResult = cache1.findKey(key, value);
        if (!bResult)
        {
            zklog.error("DatabaseCacheTest() failed calling cache1.findKey() of key=" + keyString);
            numberOfFailed++;
        }
        if(bResult){
            //check the value is correct
            for(int k=0; k<12; ++k){
                if(value[k].fe != fr.fromU64(i).fe){
                    zklog.error("DatabaseCacheTest() failed calling cache1.findKey() of key=" + keyString);
                    numberOfFailed++;
                }
            }
        }

    }
    // test update
    for (uint64_t i=0; i<NUMBER_OF_DB_ASSOCIATIVE_CACHE_ADDS; i++)
    {
        Goldilocks::Element key[4];
        vector<Goldilocks::Element> value;
        for(int k=0; k<12; ++k){
            value.push_back(fr.fromU64(i));
        } 
        poseidon_test.hash(key,(Goldilocks::Element(&)[12])(value.data()[0]));
        // modify the value
        for(int k=0; k<12; ++k){
            value[k] = fr.fromU64(i+1);
        }
        cache1.addKeyValue(key, value, true);
    }
    //test findKey with updated values
     for (uint64_t i=0; i<NUMBER_OF_DB_ASSOCIATIVE_CACHE_ADDS; i++)
    {
        Goldilocks::Element key[4];
        vector<Goldilocks::Element> value;
        for(int k=0; k<12; ++k){
            value.push_back(fr.fromU64(i));
        } 
        poseidon_test.hash(key,(Goldilocks::Element(&)[12])(value.data()[0]));
        value.clear();
        bResult = cache1.findKey(key, value);
        //check the value is found
        if (!bResult)
        {
            zklog.error("DatabaseCacheTest() failed calling cache1.findKey() of key=" + keyString);
            numberOfFailed++;
        }
        //check the value is correct
        for(int k=0; k<12; ++k){
            if(value[k].fe != fr.fromU64(i+1).fe){
                zklog.error("DatabaseCacheTest() failed calling cache1.findKey() of key=" + keyString);
                numberOfFailed++;
            }
        }

    }
    //test clear: clear the cache and check I do not find previous keys
    cache1.clear();
    for (uint64_t i=0; i<NUMBER_OF_DB_ASSOCIATIVE_CACHE_ADDS; i++)
    {
        Goldilocks::Element key[4];
        vector<Goldilocks::Element> value;
        for(int k=0; k<12; ++k){
            value.push_back(fr.fromU64(i));
        } 
        poseidon_test.hash(key,(Goldilocks::Element(&)[12])(value.data()[0]));
        bResult = cache1.findKey(key, value);
        //check the value is found
        if (bResult)
        {
            zklog.error("DatabaseCacheTest() failed calling cache1.findKey() of key=" + keyString);
            numberOfFailed++;
        }
    }
    //collisions and auxBufferKeysValues
    Goldilocks::Element key1[4];
    Goldilocks::Element key2[4];
    Goldilocks::Element key3[4];
    key1[0] = fr.fromU64(0);
    key1[1] = fr.fromU64(0);
    key1[2] = fr.fromU64(0);
    key1[3] = fr.fromU64(0);

    key2[0] = fr.fromU64(uint64_t(1)<<50);
    key2[1] = fr.fromU64(0);
    key2[2] = fr.fromU64(0);
    key2[3] = fr.fromU64(0);
    
    key3[0] = fr.fromU64(0);
    key3[1] = fr.fromU64(uint64_t(1)<<44);
    key3[2] = fr.fromU64(0);
    key3[3] = fr.fromU64(0);
    
    // Key 0 and Key 1 have only one position availe in the indexes table, if I add both the buffer auxBufferKeysValues should be used
    vector<Goldilocks::Element> value1(12);
    vector<Goldilocks::Element> value2(12);
    vector<Goldilocks::Element> value3(12);
    for(int k=0; k<12; ++k){
        value1[k] = fr.fromU64(1);
        value2[k] = fr.fromU64(2);
        value3[k] = fr.fromU64(3);
    }
    cache1.addKeyValue(key1, value1, false);
    if(cache1.getAuxBufferKeysValuesSize()!=0){
        zklog.error("DatabaseCacheTest() failed calling cache1.getAuxBufferKeysValuesSize()");
        numberOfFailed++;
    }
    cache1.addKeyValue(key2, value2, false);
    if(cache1.getAuxBufferKeysValuesSize()!=17){
        zklog.error("DatabaseCacheTest() failed calling cache1.getAuxBufferKeysValuesSize()");
        numberOfFailed++;
    }
    cache1.addKeyValue(key3, value3, false);
    if(cache1.getAuxBufferKeysValuesSize()!=34){
        zklog.error("DatabaseCacheTest() failed calling cache1.getAuxBufferKeysValuesSize()");
        numberOfFailed++;
    }
    
    //check the values are correct
    vector<Goldilocks::Element> value1_;
    bResult = cache1.findKey(key1, value1_);
    if (!bResult)
    {
        zklog.error("DatabaseCacheTest() failed calling cache1.findKey() of key=" + keyString);
        numberOfFailed++;
    }
    for(int k=0; k<12; ++k){
        if(value1_[k].fe != value1[k].fe){
            zklog.error("DatabaseCacheTest() failed calling cache1.findKey() of key=" + keyString);
            numberOfFailed++;
        }
    }
    
    vector<Goldilocks::Element> value2_;
    bResult = cache1.findKey(key2, value2_);
    if (!bResult)
    {
        zklog.error("DatabaseCacheTest() failed calling cache1.findKey() of key=" + keyString);
        numberOfFailed++;
    }
    for(int k=0; k<12; ++k){
        if(value2_[k].fe != value2[k].fe){
            zklog.error("DatabaseCacheTest() failed calling cache1.findKey() of key=" + keyString);
            numberOfFailed++;
        }
    }
    
    vector<Goldilocks::Element> value3_;
    bResult = cache1.findKey(key3, value3_);
    if (!bResult)
    {
        zklog.error("DatabaseCacheTest() failed calling cache1.findKey() of key=" + keyString);
        numberOfFailed++;
    }
    for(int k=0; k<12; ++k){
        if(value3_[k].fe != value3[k].fe){
            zklog.error("DatabaseCacheTest() failed calling cache1.findKey() of key=" + keyString);
            numberOfFailed++;
        }
    }

    //check that after doing a round to the cache the elements are not anymore into de auxBufferKeysValues
    for (uint64_t i=0; i<2*cache1.getCacheSize(); i++)
    {
        Goldilocks::Element key[4];
        vector<Goldilocks::Element> value;
        for(int k=0; k<12; ++k){
            value.push_back(fr.fromU64(i));
        } 
        poseidon_test.hash(key,(Goldilocks::Element(&)[12])(value.data()[0]));
        // modify the value
        for(int k=0; k<12; ++k){
            value[k] = fr.fromU64(i+1);
        }
        cache1.addKeyValue(key, value, true);
        if(cache1.getAuxBufferKeysValuesSize()==0 && i<cache1.getCacheSize()-2){
            zklog.error("DatabaseCacheTest() auxBufferKeysValues should not be cleaned" + to_string(cache1.getAuxBufferKeysValuesSize()));
            numberOfFailed++;
        }
    }
    
    if(cache1.getAuxBufferKeysValuesSize()!=0){
        zklog.error("DatabaseCacheTest() failed the cleaning of auxBufferKeysValues"+ to_string(cache1.getAuxBufferKeysValuesSize()));
        numberOfFailed++;
    }
    TimerStopAndLog(DATABASE_ASSOCIATIVE_CACHE_TEST);
    assert(numberOfFailed == 0);
    return numberOfFailed;
}

uint64_t DatabaseCacheBenchmark (void){

    // Generate 10M of hashes
    PoseidonGoldilocks poseidon_test;
    Goldilocks::Element* keys= new Goldilocks::Element[NUMBER_OF_DB_CACHE_BENCHS*4];
    vector<string> keyStrings(NUMBER_OF_DB_CACHE_BENCHS);
    for (uint64_t i=0; i<NUMBER_OF_DB_CACHE_BENCHS; i++)
    {
        Goldilocks::Element key[4];
        vector<Goldilocks::Element> InputValue;
        for(int k=0; k<12; ++k){
            InputValue.push_back(fr.fromU64(i));
        } 
        poseidon_test.hash(key,(Goldilocks::Element(&)[12])(InputValue.data()[0]));
        keys[i*4] = key[0];
        keys[i*4+1] = key[1];
        keys[i*4+2] = key[2];
        keys[i*4+3] = key[3];
        keyStrings[i] = fea2string(fr, (Goldilocks::Element(&)[4]) key);
    }
    vector<Goldilocks::Element> value(12);

    DatabaseMTCache cache_map;
    cache_map.setMaxSize(uint64_t(8)*uint64_t(1024)*uint64_t(1024)*uint64_t(1024));
    DatabaseMTAssociativeCache cache_ass(28, 25, "associativeCache");
    
    TimerStart(DATABASE_CACHE_BENCHMARK);
    int count_hits = 0;
    for(int k=0; k<4; k++){
        for (uint64_t i=0; i<NUMBER_OF_DB_CACHE_BENCHS; i++)
        {
            string keyString = fea2string(fr, (Goldilocks::Element(&)[4]) keys[4*i]); 
            cache_map.add(keyString,value, false);
            //cache_map.add(keyStrings[i],value, false);

        }
    }
    for(int k=0; k<4; k++){
        for (uint64_t i=0; i<NUMBER_OF_DB_CACHE_BENCHS; i++)
        {
            string keyString = fea2string(fr, (Goldilocks::Element(&)[4]) keys[4*i]); 
            /*if(cache_map.find(keyStrings[i], value)){
                ++count_hits;
            }*/
            if(cache_map.find(keyString, value)){
                ++count_hits;
            }

        }
    }
    //std::cout<<"Hits with cache map:"<<count_hits<<::endl;
    count_hits = 0;
    TimerStopAndLog(DATABASE_CACHE_BENCHMARK);
    cache_map.clear();
    TimerStart(DATABASE_CACHE_ASSOCIATIVE_BENCHMARK);
    for(int k=0; k<4; k++){
        for (uint64_t i=0; i<NUMBER_OF_DB_CACHE_BENCHS; i++)
        {
            cache_ass.addKeyValue((Goldilocks::Element(&)[4]) keys[4*i], value, false);

        }
    }
    for(int k=0; k<4; k++){
        for (uint64_t i=0; i<NUMBER_OF_DB_CACHE_BENCHS; i++)
        {
            if(cache_ass.findKey((Goldilocks::Element(&)[4]) keys[4*i], value)){
                ++count_hits;
            }

        }
    }
    //std::cout<<"Hits with cache associative:"<<count_hits<<::endl;
    TimerStopAndLog(DATABASE_CACHE_ASSOCIATIVE_BENCHMARK);
    delete[] keys;
    return 1;

}