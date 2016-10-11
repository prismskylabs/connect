#include "api/account.h"

#include <cstdint>
#include <vector>

#include <cpr/cpr.h>
#include <json.hpp>

#include "api/instrument.h"

namespace prism {
namespace connect {
namespace api {

Account Account::GetAccount(std::uint32_t id) {
}

std::vector<Account> Account::GetAccounts() {
}

std::vector<Instrument> Account::GetInstruments() {
}

} // namespace api
} // namespace connect
} // namespace prism
