/**
 * @file
 * @copyright defined in gxc/LICENSE
 */
#include <gxc.token/gxc.token.hpp>

namespace gxc {

   void token_contract::account::setopts(const std::vector<key_value>& opts) {
      check(opts.size(), "no changes on options");
      require_vauth(issuer());

      _tbl.modify(_this, same_payer, [&](auto& a) {
         for (auto o : opts) {
            if (o.key == "frozen") {
               check((*_st)->get_opt(token::opt::can_freeze), "not configured to freeze account");
               auto value = unpack<bool>(o.value);
               check(a.get_opt(opt::frozen) != value, "option already has give value");
               a.set_opt(opt::frozen, value);
            } else if (o.key == "whitelist") {
               check((*_st)->get_opt(token::opt::can_whitelist), "not configured to whitelist account");
               auto value = unpack<bool>(o.value);
               check(a.get_opt(opt::whitelist) != value, "option already has give value");
               a.set_opt(opt::whitelist, value);
            } else {
               check(false, "unknown option `" + o.key + "`");
            }
         }
      });
   }

   void token_contract::account::sub_balance(extended_asset value, bool keep_balance) {
      check_account_is_valid();
      check(_this->balance.amount >= value.quantity.amount, "overdrawn balance");

      if (!_this->get_opt(opt::whitelist) && !keep_balance &&
          _this->balance.amount == value.quantity.amount &&
          _this->get_deposit().amount == 0)
      {
         _tbl.erase(_this);
      } else {
         _tbl.modify(_this, same_payer, [&](auto& a) {
            a.balance -= value.quantity;
         });
      }
   }

   void token_contract::account::add_balance(extended_asset value) {
      if (!exists()) {
         check(!(*_st)->get_opt(token::opt::enforce_whitelist) || has_vauth(value.contract), "required to open balance manually");
         _tbl.emplace(code(), [&](auto& a) {
            a.balance = value.quantity;
            a.set_deposit(asset(0, value.quantity.symbol));
            a.set_issuer(value.contract);
         });
      } else {
         check_account_is_valid();
         _tbl.modify(_this, same_payer, [&](auto& a) {
            a.balance += value.quantity;
         });
      }
   }

   void token_contract::account::sub_deposit(extended_asset value, bool keep_balance) {
      check_account_is_valid();
      check(_this->get_deposit().amount >= value.quantity.amount, "overdrawn deposit");

      if (!_this->get_opt(opt::whitelist) && !keep_balance &&
          _this->get_deposit().amount == value.quantity.amount &&
          _this->balance.amount == 0)
      {
         _tbl.erase(_this);
      } else {
         _tbl.modify(_this, same_payer, [&](auto& a) {
            a.set_deposit(a.get_deposit() - value.quantity);
         });
      }
   }

   void token_contract::account::add_deposit(extended_asset value) {
      if (!exists()) {
         check(!(*_st)->get_opt(token::opt::enforce_whitelist) || has_vauth(value.contract), "required to open deposit manually");
         _tbl.emplace(code(), [&](auto& a) {
            a.balance = asset(0, value.quantity.symbol);
            a.set_deposit(value.quantity);
            a.set_issuer(value.contract);
         });
      } else {
         check_account_is_valid();
         _tbl.modify(_this, same_payer, [&](auto& a) {
            a.set_deposit(a.get_deposit() + value.quantity);
         });
      }
   }

   void token_contract::account::open() {
      require_vauth(issuer());
      if (!exists()) {
         _tbl.emplace(code(), [&](auto& a) {
            a.balance.symbol = (*_st)->supply.symbol;
            a.set_issuer((*_st)->issuer);
         });
      }
   }

   void token_contract::account::close() {
      require_auth(owner());
      check(exists(), "account balance doesn't exist");
      check(!_this->balance.amount && !_this->get_deposit().amount, "cannot close non-zero balance");
      _tbl.erase(_this);
   }
}
