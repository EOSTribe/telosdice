#include "bocai2222.hpp"

void bocai2222::reveal(const uint64_t &id)
{
    require_auth(get_self());
    st_bet bet = find_or_error(id);
    uint8_t random_roll = random(bet.player, bet.id);
    asset payout = asset(0, bet.amount.quantity.symbol);

    if (random_roll < bet.roll_under)
    {
        payout = compute_payout(bet.roll_under, bet.amount.quantity);
        action(
            permission_level{get_self(), "active"_n},
            bet.amount.contract,
            "transfer"_n,
            std::make_tuple(get_self(), bet.player, payout, winner_memo(bet))
        ).send();
    }

    if (isTelosToken(bet.amount))
    {
        issue_token(bet.player, bet.amount.quantity, "mining! telosdice.vip");
        unlock(bet.amount.quantity);
    }

    st_result result{
        .bet_id = bet.id,
        .player = bet.player,
        .referrer = bet.referrer,
        .amount = bet.amount.quantity,
        .roll_under = bet.roll_under,
        .random_roll = random_roll,
        .payout = payout
    };

    action(
        permission_level{get_self(), "active"_n},
        LOG,
        "result"_n,
        result
    ).send();

    action(
        permission_level{get_self(), "active"_n},
        bet.amount.contract,
        "transfer"_n,
        std::make_tuple(get_self(), DEV, compute_dev_reward(bet), std::string("for dev"))
    ).send();

    action(
        permission_level{get_self(), "active"_n},
        bet.amount.contract,
        "transfer"_n,
        std::make_tuple(get_self(), bet.referrer, compute_referrer_reward(bet), referrer_memo(bet))
    ).send();

    remove(bet.id);
}

void bocai2222::reveal1(const uint64_t &id)
{
    require_auth(get_self());
    send_defer_action(permission_level{get_self(), "active"_n}, get_self(), "reveal"_n, id);
}

void bocai2222::onTransfer(name from, name to, extended_asset quantity, string memo)
{
    if (from == get_self() || to != get_self()) return;

    if (memo.substr(0, 4) != "dice") return;

    checkAccount1(from);

    uint8_t roll_under;
    name referrer;
    parse_memo(memo, &roll_under, &referrer);
    check(is_account(referrer), "referrer account does not exist");
    check(referrer != from, "referrer cannot be self");

    assert_roll_under(roll_under, quantity);
    assert_quantity(quantity);

    const st_bet _bet{
        .id = next_id(),
        .player = from,
        .referrer = referrer,
        .amount = quantity,
        .roll_under = roll_under,
        .created_at = current_time_point().sec_since_epoch()
    };
    save(_bet);

    if (isTelosToken(quantity))
    {
        iplay(from, quantity.quantity);
        vipcheck(from, quantity.quantity);
        lock(quantity.quantity);
        fomo(_bet);
    }

    action(
        permission_level{get_self(), "active"_n},
        _bet.amount.contract,
        "transfer"_n,
        std::make_tuple(get_self(), "bocai1111"_n, compute_pool_reward(_bet), std::string("Dividing pool"))
    ).send();

    send_defer_action(permission_level{get_self(), "active"_n}, get_self(), "reveal1"_n, _bet.id);
}

void bocai2222::addtoken(name contract, asset quantity)
{
    require_auth(get_self());
    _tokens.emplace(get_self(), [&](auto &r) {
        r.contract = contract;
        r.symbol = quantity.symbol;
        r.minAmout = quantity.amount;
    });
}

void bocai2222::init()
{
    require_auth(get_self());
    st_global global = _global.get_or_default();

    global.current_id = 515789;
    global.nexthalve = static_cast<uint64_t>(7524000000 * 1e4);
    global.tlosperdice = 100;
    global.initStatu = 1;
    global.lastPlayer = "eosbocai1111"_n;
    global.endtime = current_time_point().sec_since_epoch() + 60 * 5;
    global.fomopool = asset(0, TLOS_SYMBOL);

    _global.set(global, get_self());
}

void bocai2222::transfer(name from, name to, asset quantity, string memo) {
    require_auth(from);

    // Ensure transfer is to this contract
    if (to != get_self()) return;

    // Process the transfer
    onTransfer(from, to, extended_asset(quantity, "eosio.token"_n), memo);
}

void bocai2222::apply(name code, name action) {
    if (code != get_self()) return;

    if (action == "transfer"_n) {
        auto transfer_data = unpack_action_data<st_transfer>();
        onTransfer(transfer_data.from, transfer_data.to, extended_asset(transfer_data.quantity, code), transfer_data.memo);
        return;
    }

    if (action == "reveal"_n) {
        reveal(unpack_action_data<uint64_t>());
    } else if (action == "reveal1"_n) {
        reveal1(unpack_action_data<uint64_t>());
    } else if (action == "addtoken"_n) {
        auto args = unpack_action_data<std::tuple<name, asset>>();
        addtoken(std::get<0>(args), std::get<1>(args));
    } else if (action == "init"_n) {
        init();
    }
}
