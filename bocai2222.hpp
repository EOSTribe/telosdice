#pragma once
#include <algorithm>
#include <eosio/transaction.hpp>
#include "telos.token.hpp"
#include "types.hpp"
#include "pradata.hpp"
#include <eosio/eosio.hpp>

using namespace eosio;
using std::string;

class [[eosio::contract("bocai2222")]] bocai2222 : public contract
{
  public:
    using contract::contract;

    bocai2222(name receiver, name code, datastream<const char *> ds)
        : contract(receiver, code, ds),
          _bets(receiver, receiver.value),
          _users(receiver, receiver.value),
          _fund_pool(receiver, receiver.value),
          _tokens(receiver, receiver.value),
          _global(receiver, receiver.value) {};

    [[eosio::action]]
    void transfer(name from, name to, asset quantity, string memo);

    void onTransfer(name from, name to, extended_asset quantity, string memo);

    [[eosio::action]]
    void reveal(const uint64_t &id);

    [[eosio::action]]
    void reveal1(const uint64_t &id);

    [[eosio::action]]
    void addtoken(name contract, asset quantity);

    [[eosio::action]]
    void init();

    void apply(name code, name action);

  private:
    tb_bets _bets;
    tb_users _users;
    tb_fund_pool _fund_pool;
    tb_global _global;
    tb_tokens _tokens;

    void parse_memo(string memo, uint8_t *roll_under, name *referrer) {
        memo.erase(std::remove_if(memo.begin(), memo.end(), ::isspace), memo.end());
        size_t sep_count = std::count(memo.begin(), memo.end(), '-');
        check(sep_count == 3, "invalid memo");

        size_t pos;
        string container;
        pos = sub2sep(memo, &container, '-', 0, true);
        pos = sub2sep(memo, &container, '-', ++pos, true);
        pos = sub2sep(memo, &container, '-', ++pos, true);
        check(!container.empty(), "no roll under");
        *roll_under = stoi(container);
        container = memo.substr(++pos);
        *referrer = container.empty() ? PRIZEPOOL : name(container);
    }

    asset compute_referrer_reward(const st_bet &bet) { return asset(bet.amount.quantity.amount * 2 / 1000, bet.amount.quantity.symbol); }
    asset compute_dev_reward(const st_bet &bet) { return asset(bet.amount.quantity.amount * 4 / 1000, bet.amount.quantity.symbol); }
    asset compute_pool_reward(const st_bet &bet) { return asset(bet.amount.quantity.amount * 12 / 1000, bet.amount.quantity.symbol); }
    asset compute_fomopool_reward(const st_bet &bet) { return asset(bet.amount.quantity.amount * 2 / 1000, bet.amount.quantity.symbol); }

    uint64_t next_id() {
        st_global global = _global.get_or_default();
        global.current_id += 1;
        _global.set(global, get_self());
        return global.current_id;
    }

    string referrer_memo(const st_bet &bet) {
        return "bet id:" + std::to_string(bet.id) + " player: " + bet.player.to_string() + " referral reward! tlosdice.vip";
    }

    string winner_memo(const st_bet &bet) {
        return "bet id:" + std::to_string(bet.id) + " player: " + bet.player.to_string() + " winner! tlosdice.vip";
    }

    void assert_quantity(const extended_asset &quantity) {
        auto itr = _tokens.find(quantity.contract.value + quantity.quantity.symbol.code().raw());
        check(itr != _tokens.end(), "Non-existent token");
        check(quantity.quantity.amount > 0, "quantity invalid");
        check(quantity.quantity.amount >= itr->minAmout, "transfer quantity must be greater than minimum");
    }

    bool isTelosToken(const extended_asset &quantity) {
        return (quantity.contract == "eosio.token"_n && quantity.quantity.symbol == TLOS_SYMBOL);
    }

    void assert_roll_under(const uint8_t &roll_under, const extended_asset &quantity) {
        check(roll_under >= 2 && roll_under <= 96, "roll under overflow, must be greater than 2 and less than 96");
        check(max_payout(roll_under, quantity.quantity) <= max_bonus(quantity), "offered overflow, expected earning is greater than the maximum bonus");
    }

    void unlock(const asset &amount) {
        st_fund_pool pool = get_fund_pool();
        pool.locked -= amount;
        check(pool.locked.amount >= 0, "fund unlock error");
        _fund_pool.set(pool, get_self());
    }

    void lock(const asset &amount) {
        st_fund_pool pool = get_fund_pool();
        pool.locked += amount;
        _fund_pool.set(pool, get_self());
    }

    asset compute_payout(const uint8_t &roll_under, const asset &offer) {
        return std::min(max_payout(roll_under, offer), max_bonus(extended_asset{offer, get_self()}));
    }


    asset max_payout(const uint8_t &roll_under, const asset &offer) {
        const double ODDS = 98.0 / (roll_under - 1.0);
        return asset(static_cast<int64_t>(ODDS * offer.amount), offer.symbol);
    }

    asset max_bonus(const extended_asset &quantity) {
        return available_balance(quantity) / 10;
    }

    asset available_balance(const extended_asset &quantity) {
        token token_contract(get_self(), quantity.contract, datastream<const char*>(nullptr, 0));
        const asset balance = token_contract.get_balance(get_self(), quantity.quantity.symbol.code());
        return isTelosToken(quantity) ? balance - get_fund_pool().locked : balance;
    }

    st_fund_pool get_fund_pool() {
        return _fund_pool.get_or_create(get_self(), st_fund_pool{asset(0, TLOS_SYMBOL)});
    }

    template <typename... Args>
    void send_defer_action(Args &&... args) {
        transaction trx;
        trx.actions.emplace_back(std::forward<Args>(args)...);
        trx.delay_sec = 1;
        trx.send(next_id(), get_self(), false);
    }

    uint64_t getDiceSupply() {
        token eos_token(get_self(), DICETOKEN, datastream<const char*>(nullptr, 0));
        return eos_token.get_supply(DICE_SYMBOL.code()).amount;
    }

    uint8_t random(name player, uint64_t game_id) {
        auto mixed = tapos_block_prefix() * tapos_block_num() + player.value + game_id - current_time_point().sec_since_epoch();
        checksum256 result = sha256(reinterpret_cast<const char *>(&mixed), sizeof(mixed));
        return static_cast<uint8_t>(*(uint64_t *)&result.data()[0] % 100 + 1);
    }

    void issue_token(name to, asset quantity, string memo) {
        st_global global = _global.get_or_default();
        auto supply = getDiceSupply();
        auto nexthalve = global.nexthalve;
        if ((DICESUPPLY - supply) <= nexthalve) {
            global.nexthalve = global.nexthalve * 95 / 100;
            global.tlosperdice = global.tlosperdice * 3 / 4;
            _global.set(global, get_self());
        }

        asset send_amount = asset(quantity.amount * global.tlosperdice, DICE_SYMBOL);
        action(
            permission_level{get_self(), "active"_n},
            DICETOKEN,
            "issue"_n,
            std::make_tuple(to, send_amount, memo)
        ).send();
    }

    void iplay(name from, asset quantity) {
        tb_users1 _users1(get_self(), from.value);
        auto v = _users1.get_or_create(get_self(), st_user1{});
        v.amount += quantity;
        v.count += 1;
        _users1.set(v, get_self());
    }

    st_bet find_or_error(const uint64_t &id) {
        auto itr = _bets.find(id);
        check(itr != _bets.end(), "Bet not found");
        return *itr;
    }

    void remove(const uint64_t bet_id) {
        auto itr = _bets.find(bet_id);
        check(itr != _bets.end(), "Bet not found for deletion");
        _bets.erase(itr);
    }

    void checkAccount1(name from) {
        // Implement according to your contract logic
    }

    void save(const st_bet &bet) {
        _bets.emplace(get_self(), [&](st_bet &new_bet) {
            new_bet = bet;
        });
    }

    void vipcheck(name from, asset quantity) {
        // Implement VIP check logic here
    }

    void fomo(const st_bet &bet) {
        // Implement FOMO logic here
    }

    struct st_transfer {
        name from;
        name to;
        asset quantity;
        string memo;
    };
};

extern "C" {
    [[noreturn]] void apply(uint64_t receiver, uint64_t code, uint64_t action) {
        bocai2222 contract(name(receiver), name(code), datastream<const char *>(nullptr, 0));
        contract.apply(name(code), name(action));
        eosio_exit(0);
    }
}
