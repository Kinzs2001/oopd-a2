#include "bookmgmt/Resource.h"

#include <algorithm>
#include <ostream>
#include <sstream>
#include <stdexcept>

namespace bookmgmt
{

    const char *categoryName(ResourceCategory c)
    {
        switch (c)
        {
        case ResourceCategory::Book:
            return "Book";
        case ResourceCategory::ElectronicResource:
            return "ElectronicResource";
        case ResourceCategory::Journal:
            return "Journal";
        case ResourceCategory::EBook:
            return "EBook";
        case ResourceCategory::AudioBook:
            return "AudioBook";
        case ResourceCategory::Thesis:
            return "Thesis";
        }
        return "Unknown";
    }

    Resource::Resource(std::string id, std::string title, std::string publisher,
                       int year, Money unitPrice)
        : id_(std::move(id)),
          title_(std::move(title)),
          publisher_(std::move(publisher)),
          year_(year),
          unitPrice_(unitPrice)
    {
        if (id_.empty())
            throw std::invalid_argument("resource id must not be empty");
        if (title_.empty())
            throw std::invalid_argument("resource title must not be empty");
        if (unitPrice_.isNegative())
            throw std::invalid_argument("price must not be negative");
    }

    void Resource::setUnitPrice(Money price)
    {
        if (price.isNegative())
            throw std::invalid_argument("price must not be negative");
        unitPrice_ = price;
    }

    void Resource::addVendorOffer(std::string vendor, Money price)
    {
        if (vendor.empty())
        {
            throw std::invalid_argument("vendor name must not be empty");
        }

        if (price.isNegative())
        {
            throw std::invalid_argument("vendor price must not be negative");
        }

        for (const auto &offer : vendorOffers_)
        {
            if (offer.vendor == vendor)
            {
                throw std::invalid_argument("vendor offer already exists");
            }
        }

        vendorOffers_.push_back({std::move(vendor), price});
    }

    const VendorOffer &Resource::cheapestVendorOffer() const
    {
        if (vendorOffers_.empty())
        {
            throw std::logic_error("resource has no vendor offers");
        }

        const auto cheapest = std::min_element(
            vendorOffers_.begin(), vendorOffers_.end(),
            [](const VendorOffer &a, const VendorOffer &b)
            {
                return a.unitPrice < b.unitPrice;
            });

        return *cheapest;
    }

    void Resource::requirePositive(int quantity)
    {
        if (quantity <= 0)
            throw std::invalid_argument("quantity must be positive");
    }

    Money Resource::costFor(int quantity) const
    {
        return costForAtPrice(quantity, unitPrice());
    }

    Money Resource::costForAtPrice(int quantity, Money price) const
    {
        requirePositive(quantity);

        if (price.isNegative())
        {
            throw std::invalid_argument("price must not be negative");
        }

        return price * quantity;
    }

    void Resource::print(std::ostream &os) const
    {
        os << categoryName(category()) << " " << id_ << "\n"
           << "  title: " << title_ << "\n"
           << "  publisher: " << publisher_ << "\n"
           << "  year: " << year_ << "\n"
           << "  unit price: " << unitPrice_ << "\n";
        printDetails(os);
    }

    void Resource::printDetails(std::ostream &) const {}

    std::string Resource::summary() const
    {
        std::ostringstream os;
        os << "[" << categoryName(category()) << "] " << id_ << "  " << title_
           << " (" << year_ << ")  @ " << unitPrice_;
        return os.str();
    }

    std::ostream &operator<<(std::ostream &os, const Resource &r)
    {
        r.print(os);
        return os;
    }

} // namespace bookmgmt
