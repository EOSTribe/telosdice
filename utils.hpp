#include <eosio/crypto.hpp>
#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include <eosio/singleton.hpp>
#include <eosio/time.hpp>
#include <eosio/name.hpp>

using namespace eosio;

string uint64_to_string(uint64_t input)
{
    string result;
    uint8_t base = 10;
    do
    {
        char c = input % base;
        input /= base;
        if (c < 10)
            c += '0';
        else
            c += 'A' - 10;
        result = c + result;
    } while (input);
    return result;
}

size_t sub2sep(const string &input,
               string *output,
               const char &separator,
               const size_t &first_pos = 0,
               const bool &required = false)
{
    check(first_pos != string::npos, "invalid first position");
    auto pos = input.find(separator, first_pos);
    if (pos == string::npos)
    {
        check(!required, "parse memo error");
        return string::npos;
    }
    *output = input.substr(first_pos, pos - first_pos);
    return pos;
}
