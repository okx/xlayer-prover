#ifndef HASHDB_REMOTE_HPP
#define HASHDB_REMOTE_HPP

#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>
#include <sys/time.h>
#include "hashdb.grpc.pb.h"
#include "goldilocks_base_field.hpp"
#include "smt.hpp"
#include "hashdb_interface.hpp"
#include "zkresult.hpp"
#include "utils/time_metric.hpp"
#include "timer.hpp"

class HashDBRemote : public HashDBInterface
{
private:
    Goldilocks &fr;
    const Config &config;
    ::hashdb::v1::HashDBService::Stub *stub;
#ifdef LOG_TIME_STATISTICS_HASHDB_REMOTE
    TimeMetricStorage tms;
    struct timeval t;
#endif
public:
    HashDBRemote(Goldilocks &fr, const Config &config);
    ~HashDBRemote();
    zkresult set(const Goldilocks::Element (&oldRoot)[4], const Goldilocks::Element (&key)[4], const mpz_class &value, const bool persistent, Goldilocks::Element (&newRoot)[4], SmtSetResult *result, DatabaseMap *dbReadLog);
    zkresult get(const Goldilocks::Element (&root)[4], const Goldilocks::Element (&key)[4], mpz_class &value, SmtGetResult *result, DatabaseMap *dbReadLog);
    zkresult setProgram(const Goldilocks::Element (&key)[4], const vector<uint8_t> &data, const bool persistent);
    zkresult getProgram(const Goldilocks::Element (&key)[4], vector<uint8_t> &data, DatabaseMap *dbReadLog);
    void loadDB(const DatabaseMap::MTMap &input, const bool persistent);
    void loadProgramDB(const DatabaseMap::ProgramMap &input, const bool persistent);
    zkresult flush(uint64_t &flushId, uint64_t &lastSentFlushId);
    zkresult getFlushStatus(uint64_t &lastSentFlushId, uint64_t &sendingFlushId, uint64_t &lastFlushId);
    zkresult getFlushData(uint64_t lastGotFlushId, uint64_t &lastSentFlushId, vector<FlushData> (&nodes), vector<FlushData> (&nodesUpdate), vector<FlushData> (&program), vector<FlushData> (&programUpdate), string &nodesStateRoot);
    void clearCache(void) {};
};

#endif