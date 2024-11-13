/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#pragma once

#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include <eosio/name.hpp>
#include <string>

namespace eosiosystem
{
class system_contract;
}

namespace eosio
{

using std::string;

class [[eosio::contract("token")]] token : public contract
{
    public:
      using contract::contract;

      [[eosio::action]]
      void create(name issuer,
                  asset maximum_supply);

      [[eosio::action]]
      void issue(name to, asset quantity, string memo);

      [[eosio::action]]
      void retire(asset quantity, string memo);

      [[eosio::action]]
      void transfer(name from,
                    name to,
                    asset quantity,
                    string memo);

      [[eosio::action]]
      void close(name owner, symbol_code sym_code);

      [[eosio::table]]
      struct account
      {
            asset balance;

            uint64_t primary_key() const { return balance.symbol.code().raw(); }
      };

      [[eosio::table]]
      struct currency_stats
      {
            asset supply;
            asset max_supply;
            name issuer;

            uint64_t primary_key() const { return supply.symbol.code().raw(); }
      };

      using accounts = eosio::multi_index<"accounts"_n, account>;
      using stats = eosio::multi_index<"stat"_n, currency_stats>;

      asset get_supply(symbol_code sym_code) const;

      asset get_balance(name owner, symbol_code sym_code) const;

    private:
      void sub_balance(name owner, asset value);
      void add_balance(name owner, asset value, name ram_payer);

    public:
      struct transfer_args
      {
            name from;
            name to;
            asset quantity;
            string memo;
      };
};

asset token::get_supply(symbol_code sym_code) const
{
      stats statstable(get_self(), sym_code.raw());
      const auto &st = statstable.get(sym_code.raw(), "symbol does not exist");
      return st.supply;
}

asset token::get_balance(name owner, symbol_code sym_code) const
{
      accounts accountstable(get_self(), owner.value);
      const auto &ac = accountstable.get(sym_code.raw(), "no balance object found");
      return ac.balance;
}

} // namespace eosio
