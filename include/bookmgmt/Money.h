
#pragma once
// Money: fixed-point currency amount stored in minor units.

#include <cstdint>
#include <iosfwd>
#include <stdexcept>
#include <string>

namespace bookmgmt
{
    class Money
    {
    public:
        Money() = default;

        static Money fromMinor(std::int64_t minor)
        {
            return Money(minor);
        }

        static Money of(std::int64_t major, int minor = 0);

        static Money fromMinor(std::int64_t minor, std::string currency)
        {
            return Money(minor, std::move(currency));
        }

        static Money of(std::int64_t major, int minor,
                        std::string currency)
        {
            Money amount = of(major, minor);
            amount.currency_ = std::move(currency);
            amount.validateCurrency();
            return amount;
        }

        std::int64_t minorUnits() const { return minor_; }
        const std::string &currency() const { return currency_; }

        double toDouble() const
        {
            return static_cast<double>(minor_) / 100.0;
        }

        std::string toString() const;

        bool isZero() const { return minor_ == 0; }
        bool isNegative() const { return minor_ < 0; }

        Money &operator+=(Money other)
        {
            ensureSameCurrency(other);
            minor_ += other.minor_;
            return *this;
        }

        Money &operator-=(Money other)
        {
            ensureSameCurrency(other);
            minor_ -= other.minor_;
            return *this;
        }

        Money &operator*=(std::int64_t factor)
        {
            minor_ *= factor;
            return *this;
        }

        friend Money operator+(Money a, Money b) { return a += b; }
        friend Money operator-(Money a, Money b) { return a -= b; }
        friend Money operator*(Money a, std::int64_t k) { return a *= k; }
        friend Money operator*(std::int64_t k, Money a) { return a *= k; }

        friend bool operator==(Money a, Money b)
        {
            a.ensureSameCurrency(b);
            return a.minor_ == b.minor_;
        }

        friend bool operator!=(Money a, Money b) { return !(a == b); }

        friend bool operator<(Money a, Money b)
        {
            a.ensureSameCurrency(b);
            return a.minor_ < b.minor_;
        }

        friend bool operator<=(Money a, Money b) { return !(b < a); }
        friend bool operator>(Money a, Money b) { return b < a; }
        friend bool operator>=(Money a, Money b) { return !(a < b); }

    private:
        explicit Money(std::int64_t minor) : minor_(minor) {}

        Money(std::int64_t minor, std::string currency)
            : minor_(minor), currency_(std::move(currency))
        {
            validateCurrency();
        }

        void validateCurrency() const
        {
            if (currency_.empty())
                throw std::invalid_argument("currency code cannot be empty");
        }

        void ensureSameCurrency(const Money &other) const
        {
            if (currency_ != other.currency_)
                throw std::invalid_argument("currency mismatch: " +
                                            currency_ + " vs " + other.currency_);
        }

        std::int64_t minor_ = 0;
        std::string currency_ = "INR";
    };

    std::ostream &operator<<(std::ostream &os, Money m);
} // namespace bookmgmt
