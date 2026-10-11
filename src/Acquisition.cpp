
#include "bookmgmt/Acquisition.h"

#include <iomanip>
#include <ostream>
#include <stdexcept>
#include <utility>
#include <tuple>

#include "bookmgmt/Exceptions.h"

namespace bookmgmt
{

    AcquisitionManager::AcquisitionManager(Catalog &catalog, Budget &budget,
                                           int printTaxPercent,
                                           int electronicTaxPercent)
        : catalog_(catalog),
          budget_(budget),
          printTaxPercent_(printTaxPercent),
          electronicTaxPercent_(electronicTaxPercent)
    {
        if (printTaxPercent_ < 0 || electronicTaxPercent_ < 0)
        {
            throw std::invalid_argument("tax rates must not be negative");
        }
    }

    Money AcquisitionManager::quote(const std::string &id, int quantity) const
    {
        return catalog_.get(id).costFor(quantity);
    }

    Money AcquisitionManager::applyTax(const Resource &resource,
                                       Money preTaxCost) const
    {
        const int taxPercent =
            resource.isDigital() ? electronicTaxPercent_ : printTaxPercent_;

        return Money::fromMinor(
            preTaxCost.minorUnits() * (100 + taxPercent) / 100);
    }

    std::string AcquisitionManager::checkTitleLimit(
        const Resource &resource, const std::string &id) const
    {
        const auto limit = titleLimits_.find(resource.category());

        if (limit == titleLimits_.end())
        {
            return {};
        }

        const auto titles = purchasedTitles_.find(resource.category());

        const bool alreadyPurchased =
            titles != purchasedTitles_.end() &&
            titles->second.count(id) > 0;

        const std::size_t currentCount =
            titles == purchasedTitles_.end() ? 0 : titles->second.size();

        if (!alreadyPurchased &&
            currentCount >= static_cast<std::size_t>(limit->second))
        {
            return std::string(categoryName(resource.category())) +
                   " distinct-title limit exceeded";
        }

        return {};
    }

    void AcquisitionManager::setTitleLimit(ResourceCategory category,
                                           int maxTitles)
    {
        if (maxTitles < 0)
        {
            throw std::invalid_argument("title limit must not be negative");
        }

        titleLimits_[category] = maxTitles;
    }

    bool AcquisitionManager::canPurchase(
        const std::string &id, int quantity, std::string *reason) const
    {
        return canPurchase(id, quantity, "General", reason);
    }

    bool AcquisitionManager::canPurchase(
        const std::string &id, int quantity,
        const std::string &department, std::string *reason) const
    {
        std::string why;
        const Budget *budget = budgetFor(department);

        if (!budget)
        {
            why = "unknown department: " + department;
        }
        else if (const Resource *resource = catalog_.find(id))
        {
            if (quantity <= 0)
            {
                why = "quantity must be positive";
            }
            else
            {
                const Money preTaxCost = resource->costFor(quantity);
                const Money postTaxCost = applyTax(*resource, preTaxCost);

                why = checkTitleLimit(*resource, id);

                if (why.empty())
                    why = budget->check(resource->category(), quantity,
                                        postTaxCost);
            }
        }
        else
        {
            why = "resource not found: " + id;
        }

        if (reason)
            *reason = why;

        return why.empty();
    }

    PurchaseRecord &AcquisitionManager::record(
        const Resource *resource, const std::string &id, int quantity,
        Money preTaxCost, Money postTaxCost, bool approved,
        std::string reason, const std::string &department)
    {
        history_.push_back(PurchaseRecord{
            nextOrderNo_++,
            id,
            resource ? resource->title() : std::string("(unknown)"),
            resource ? resource->category() : ResourceCategory::Book,
            quantity,
            preTaxCost,
            postTaxCost,
            approved,
            std::move(reason),
            false,
            0,
            department});

        return history_.back();
    }

    const PurchaseRecord &AcquisitionManager::purchase(
        const std::string &id, int quantity, const std::string &department)
    {
        const Resource &resource = catalog_.get(id);
        Budget *budget = budgetFor(department);

        if (!budget)
            throw std::invalid_argument("unknown department: " + department);

        const Money preTaxCost = resource.costFor(quantity);
        const Money postTaxCost = applyTax(resource, preTaxCost);

        const std::string reason = checkTitleLimit(resource, id);

        if (!reason.empty())
            throw QuotaExceededError(reason);

        budget->commit(resource.category(), quantity, postTaxCost);
        catalog_.addHoldings(id, quantity);
        purchasedTitles_[resource.category()].insert(id);

        return record(&resource, id, quantity, preTaxCost,
                      postTaxCost, true, {}, department);
    }

    std::vector<PurchaseRecord> AcquisitionManager::processBatch(
        const std::vector<PurchaseRequest> &requests, bool allOrNothing)
    {

        if (allOrNothing)
        {
            struct Validation
            {
                const Resource *resource;
                Money preTaxCost;
                Money postTaxCost;
                std::string reason;
            };

            std::vector<Validation> validations;
            validations.reserve(requests.size());

            // Simulate purchases without changing the real budgets or titles.
            std::map<std::string, Budget> simulatedBudgets;
            auto simulatedTitles = purchasedTitles_;
            bool hasFailure = false;

            for (const auto &request : requests)
            {
                const Resource *resource = catalog_.find(request.resourceId);
                const Budget *actualBudget = budgetFor(request.department);

                Validation validation{resource, Money{}, Money{}, {}};

                if (!actualBudget)
                {
                    validation.reason =
                        "unknown department: " + request.department;
                }
                else if (!resource)
                {
                    validation.reason =
                        "resource not found: " + request.resourceId;
                }
                else if (request.quantity <= 0)
                {
                    validation.reason = "quantity must be positive";
                }
                else
                {
                    validation.preTaxCost =
                        resource->costFor(request.quantity);
                    validation.postTaxCost =
                        applyTax(*resource, validation.preTaxCost);

                    const auto category = resource->category();
                    const auto limit = titleLimits_.find(category);
                    const auto titles = simulatedTitles.find(category);

                    const bool alreadyPurchased =
                        titles != simulatedTitles.end() &&
                        titles->second.count(request.resourceId) > 0;

                    const std::size_t titleCount =
                        titles == simulatedTitles.end()
                            ? 0
                            : titles->second.size();

                    if (limit != titleLimits_.end() &&
                        !alreadyPurchased &&
                        titleCount >=
                            static_cast<std::size_t>(limit->second))
                    {
                        validation.reason =
                            std::string(categoryName(category)) +
                            " distinct-title limit exceeded";
                    }

                    if (validation.reason.empty())
                    {
                        auto it = simulatedBudgets.find(request.department);

                        if (it == simulatedBudgets.end())
                        {
                            it = simulatedBudgets
                                     .emplace(request.department,
                                              *actualBudget)
                                     .first;
                        }

                        validation.reason =
                            it->second.check(category, request.quantity,
                                             validation.postTaxCost);

                        if (validation.reason.empty())
                        {
                            it->second.commit(category, request.quantity,
                                              validation.postTaxCost);
                            simulatedTitles[category].insert(
                                request.resourceId);
                        }
                    }
                }

                if (!validation.reason.empty())
                    hasFailure = true;

                validations.push_back(std::move(validation));
            }

            if (hasFailure)
            {
                std::vector<PurchaseRecord> rejected;
                rejected.reserve(requests.size());

                for (std::size_t i = 0; i < requests.size(); ++i)
                {
                    const auto &request = requests[i];
                    const auto &validation = validations[i];

                    const std::string reason =
                        validation.reason.empty()
                            ? "batch aborted: another request was rejected"
                            : validation.reason;

                    rejected.push_back(
                        record(validation.resource, request.resourceId,
                               request.quantity, validation.preTaxCost,
                               validation.postTaxCost, false, reason,
                               request.department));
                }

                return rejected;
            }
        }

        std::vector<PurchaseRecord> results;
        results.reserve(requests.size());

        for (const auto &request : requests)
        {
            const Resource *resource = catalog_.find(request.resourceId);
            const Budget *budget = budgetFor(request.department);

            Money preTaxCost;
            Money postTaxCost;
            std::string reason;

            if (!budget)
            {
                reason = "unknown department: " + request.department;
            }
            else if (!resource)
            {
                reason = "resource not found: " + request.resourceId;
            }
            else if (request.quantity <= 0)
            {
                reason = "quantity must be positive";
            }
            else
            {
                preTaxCost = resource->costFor(request.quantity);
                postTaxCost = applyTax(*resource, preTaxCost);

                reason = checkTitleLimit(*resource, request.resourceId);

                if (reason.empty())
                    reason = budget->check(resource->category(),
                                           request.quantity, postTaxCost);
            }

            if (reason.empty())
            {
                results.push_back(
                    purchase(request.resourceId, request.quantity,
                             request.department));
            }
            else
            {
                results.push_back(
                    record(resource, request.resourceId, request.quantity,
                           preTaxCost, postTaxCost, false, reason,
                           request.department));
            }
        }

        return results;
    }

    Money AcquisitionManager::totalSpent() const
    {
        Money sum;

        for (const auto &entry : history_)
        {
            if (!entry.approved)
                continue;

            if (entry.cancellation)
                sum -= entry.cost;
            else
                sum += entry.cost;
        }

        return sum;
    }

    const PurchaseRecord &AcquisitionManager::cancel(int orderNo)
    {
        auto it = history_.end();

        for (auto current = history_.begin(); current != history_.end(); ++current)
        {
            if (current->orderNo == orderNo)
            {
                it = current;
                break;
            }
        }

        if (it == history_.end())
            throw std::invalid_argument("order not found");

        if (!it->approved || it->cancellation)
            throw std::invalid_argument("order cannot be cancelled");

        if (cancelledOrders_.count(orderNo))
            throw std::invalid_argument("order already cancelled");

        const PurchaseRecord original = *it;

        // Reverse the original purchase's financial and holdings effects.
        Budget *budget = budgetFor(original.department);

        if (!budget)
            throw std::invalid_argument("department budget not found");

        budget->refund(original.category, original.quantity, original.cost);

        catalog_.addHoldings(original.resourceId, -original.quantity);
        cancelledOrders_.insert(orderNo);

        // Release the title slot only if no other active purchase uses it.
        bool stillPurchased = false;

        for (const auto &entry : history_)
        {
            if (entry.approved &&
                !entry.cancellation &&
                entry.resourceId == original.resourceId &&
                entry.category == original.category &&
                !cancelledOrders_.count(entry.orderNo))
            {
                stillPurchased = true;
                break;
            }
        }

        if (!stillPurchased)
        {
            auto titles = purchasedTitles_.find(original.category);

            if (titles != purchasedTitles_.end())
                titles->second.erase(original.resourceId);
        }

        // Preserve the original record and append a separate refund record.
        history_.push_back(PurchaseRecord{
            nextOrderNo_++,
            original.resourceId,
            original.title,
            original.category,
            original.quantity,
            original.preTaxCost,
            original.cost,
            true,
            "Cancellation of order #" + std::to_string(orderNo),
            true,
            orderNo,
            original.department});

        return history_.back();
    }

    void AcquisitionManager::printReport(std::ostream &os) const
    {
        os << "Order history (" << history_.size() << " orders)\n";

        Money preTaxTotal;
        Money postTaxTotal;

        for (const auto &entry : history_)
        {
            os << "  #" << std::setw(3) << std::left << entry.orderNo << " "
               << ((entry.cancellation
                        ? "CANCELLED"
                        : (entry.approved ? "APPROVED" : "REJECTED")))
               << "  "
               << std::setw(6) << entry.resourceId << " x"
               << std::setw(3) << entry.quantity << " "
               << entry.title
               << "\n        Pre-tax: " << entry.preTaxCost
               << "  Post-tax: " << entry.cost;

            if (entry.cancellation)
            {
                preTaxTotal -= entry.preTaxCost;
                postTaxTotal -= entry.cost;
                os << "\n        reason: " << entry.reason;
            }
            else if (!entry.approved)
            {
                os << "\n        reason: " << entry.reason;
            }
            else
            {
                preTaxTotal += entry.preTaxCost;
                postTaxTotal += entry.cost;
            }
            os << "\n";
        }

        os << "Pre-tax total: " << preTaxTotal << "\n";
        os << "Post-tax total: " << postTaxTotal << "\n";
        os << "Total spent: " << totalSpent() << "\n";
    }

    Budget *AcquisitionManager::budgetFor(const std::string &department)
    {
        if (department == "General")
            return &budget_;

        auto it = departmentBudgets_.find(department);
        return it == departmentBudgets_.end() ? nullptr : &it->second;
    }

    const Budget *AcquisitionManager::budgetFor(
        const std::string &department) const
    {
        if (department == "General")
            return &budget_;

        auto it = departmentBudgets_.find(department);
        return it == departmentBudgets_.end() ? nullptr : &it->second;
    }

    void AcquisitionManager::addDepartment(const std::string &name, Money total)
    {
        if (name.empty() || name == "General")
            throw std::invalid_argument("invalid or reserved department name");

        auto result = departmentBudgets_.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(name),
            std::forward_as_tuple(total));

        if (!result.second)
            throw std::invalid_argument("department already exists: " + name);
    }

    Budget &AcquisitionManager::departmentBudget(const std::string &name)
    {
        Budget *budget = budgetFor(name);

        if (!budget)
            throw std::invalid_argument("unknown department: " + name);

        return *budget;
    }

    const Budget &AcquisitionManager::departmentBudget(
        const std::string &name) const
    {
        const Budget *budget = budgetFor(name);

        if (!budget)
            throw std::invalid_argument("unknown department: " + name);

        return *budget;
    }
} // namespace bookmgmt
