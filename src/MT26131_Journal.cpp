// MT26131 Piyush Saxena
#include "bookmgmt/MT26131_Journal.h"

#include <stdexcept>
#include <ostream>

namespace bookmgmt
{

    Journal::Journal(std::string id, std::string title, std::string publisher, int year,
                     std::string issn, int issuesPerYear, Money unitPrice,
                     int subscriptionYears)
        : Resource(std::move(id), std::move(title), std::move(publisher), year, unitPrice),
          issn_(std::move(issn)),
          issuesPerYear_(issuesPerYear),
          subscriptionYears_(subscriptionYears)
    {
        if (subscriptionYears_ < 1)
            throw std::invalid_argument("subscription years must be >= 1");
    }

    Money Journal::costFor(int quantity) const
    {
        return costForAtPrice(quantity, unitPrice());
    }

    Money Journal::costForAtPrice(int quantity, Money price) const
    {
        requirePositive(quantity);

        if (price.isNegative())
        {
            throw std::invalid_argument("price must not be negative");
        }

        return price * quantity * subscriptionYears_;
    }

    void Journal::printDetails(std::ostream &os) const
    {
        os << "  issn: " << issn_ << "\n"
           << "  issues per year: " << issuesPerYear_ << "\n"
           << "  subscription years: " << subscriptionYears_ << "\n";
    }

} // namespace bookmgmt