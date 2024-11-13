#pragma once
#include <eosio/eosio.hpp>
#include <eosio/name.hpp>
#include <string>

namespace prochain
{

using eosio::name;

// Rating table structure
struct [[eosio::table, eosio::contract("your_contract")]] Rating {
    name account;                 // account name
    uint8_t account_type;         // enum_account_type: 0, normal account; 1, code account;
    uint8_t normal_account_level; // rating level for normal account, from 0 to 10
    uint8_t code_account_level;   // rating level for code account, from 0 to 10

    uint64_t primary_key() const { return account.value; }
};

typedef eosio::multi_index<"trating"_n, Rating> rating_index;

// Account type enumeration
enum enum_account_type
{
    normal_account = 0, // 0: normal account
    code_account,       // 1: code account
    account_type_count
};

// Levels defined for both normal and code accounts
constexpr uint8_t BP_BLACKLIST = 0;

// Levels defined for normal account
constexpr uint8_t PRABOX_BLACKLIST = 2;
constexpr uint8_t PRABOX_GREYLIST = 3;
constexpr uint8_t PRABOX_AUTH_VERIFIED = 6;
constexpr uint8_t PRABOX_KYC_VERIFIED = 6;

// Levels defined for code account
constexpr uint8_t MALICIOUS_CODE_ACCOUNT = 0;

// Callback results
const std::string RESULT_FOUND = "FOUND";
const std::string RESULT_NOTFOUND = "NOTFOUND";

} // namespace prochain
