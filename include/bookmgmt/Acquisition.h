
#pragma once

#include <iosfwd>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "bookmgmt/Budget.h"
#include "bookmgmt/Catalog.h"

namespace bookmgmt
{

    struct PurchaseRequest
    {
        std::string resourceId;
        int quantity;
    };

    struct PurchaseRecord
    {
        int orderNo;
        std::string resourceId;
        std::string title;
        ResourceCategory category;
        int quantity;
        Money preTaxCost;
        Money cost; // Post-tax cost
        bool approved;
        std::string reason;
        bool cancellation = false;
        int relatedOrderNo = 0;
    };

    class AcquisitionManager
    {
    public:
        AcquisitionManager(Catalog &catalog, Budget &budget,
                           int printTaxPercent = 0,
                           int electronicTaxPercent = 0);

        Money quote(const std::string &id, int quantity) const;

        bool canPurchase(const std::string &id, int quantity,
                         std::string *reason = nullptr) const;

        const PurchaseRecord &purchase(const std::string &id, int quantity);
        const PurchaseRecord &cancel(int orderNo);

        std::vector<PurchaseRecord> processBatch(
            const std::vector<PurchaseRequest> &requests);

        const std::vector<PurchaseRecord> &history() const { return history_; }
        Money totalSpent() const;

        void printReport(std::ostream &os) const;

        // Set the maximum number of distinct resource IDs purchasable per category.
        void setTitleLimit(ResourceCategory category, int maxTitles);

    private:
        Money applyTax(const Resource &resource, Money preTaxCost) const;

        std::string checkTitleLimit(const Resource &resource,
                                    const std::string &id) const;

        PurchaseRecord &record(const Resource *resource, const std::string &id,
                               int quantity, Money preTaxCost, Money postTaxCost,
                               bool approved, std::string reason);

        Catalog &catalog_;
        Budget &budget_;
        int printTaxPercent_;
        int electronicTaxPercent_;

        std::map<ResourceCategory, int> titleLimits_;
        std::map<ResourceCategory, std::set<std::string>> purchasedTitles_;

        std::vector<PurchaseRecord> history_;
        int nextOrderNo_ = 1;
        std::set<int> cancelledOrders_;
    };

} // namespace bookmgmt
