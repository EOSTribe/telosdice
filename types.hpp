#include "utils.hpp"
#include <eosio/singleton.hpp>
#include <eosio/asset.hpp>
#include <eosio/name.hpp>

#define TLOS_SYMBOL symbol("TLOS", 4)
#define DICE_SYMBOL symbol("BOCAI", 4)
#define LOG "bocailogs"_n
#define DICETOKEN "bocai1111"_n
#define DEV "bocaidevv"_n
#define PRIZEPOOL "bocai1111"_n
#define DICESUPPLY 88000000000000
#define FOMOTIME (24 * 60 * 60)

typedef uint32_t tlostime;
using eosio::extended_asset;
using eosio::singleton;
using eosio::multi_index;
using eosio::name;

// Table: bets
struct [[eosio::table, eosio::contract("your_contract")]] st_bet {
    uint64_t id;
    name player;
    name referrer;
    extended_asset amount;
    uint8_t roll_under;
    uint64_t created_at;

    uint64_t primary_key() const { return id; }
};

// Table: tokens
struct [[eosio::table, eosio::contract("your_contract")]] st_tokens {
    name contract;        // Contract account
    symbol symbol;        // Token symbol
    uint64_t minAmout;    // Minimum allowable bet

    uint64_t primary_key() const { return contract.value + symbol.code().raw(); }
};

typedef multi_index<"tokens"_n, st_tokens> tb_tokens;

// Table: users1 (singleton)
struct [[eosio::table, eosio::contract("your_contract")]] st_user1 {
    asset amount = asset(0, TLOS_SYMBOL);
    uint32_t count = 0;
};

typedef singleton<"users1"_n, st_user1> tb_users1;

// Table: users
struct [[eosio::table, eosio::contract("your_contract")]] st_user {
    name owner;
    asset amount;
    uint32_t count;

    uint64_t primary_key() const { return owner.value; }
};

typedef multi_index<"users"_n, st_user> tb_users;

// Structure: result
struct st_result {
    uint64_t bet_id;
    name player;
    name referrer;
    asset amount;
    uint8_t roll_under;
    uint8_t random_roll;
    asset payout;
};

// Table: fundpool (singleton)
struct [[eosio::table, eosio::contract("your_contract")]] st_fund_pool {
    asset locked;
};

typedef singleton<"fundpool"_n, st_fund_pool> tb_fund_pool;

// Table: global (singleton)
struct [[eosio::table, eosio::contract("your_contract")]] st_global {
    uint64_t current_id;
    double tlosperdice;
    uint64_t nexthalve;
    uint64_t initStatu;
    name lastPlayer;
    tlostime endtime;
    asset fomopool;
};

typedef singleton<"global"_n, st_global> tb_global;

// Table: bets
typedef multi_index<"bets"_n, st_bet> tb_bets;
