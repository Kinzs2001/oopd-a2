
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
        std::string department = "General";
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
        std::string department = "General";
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

        const PurchaseRecord &cancel(int orderNo);

        std::vector<PurchaseRecord> processBatch(
            const std::vector<PurchaseRequest> &requests);

        const std::vector<PurchaseRecord> &history() const { return history_; }
        Money totalSpent() const;

        void printReport(std::ostream &os) const;

        // Set the maximum number of distinct resource IDs purchasable per category.
        void setTitleLimit(ResourceCategory category, int maxTitles);

        void addDepartment(const std::string &name, Money total);

        Budget &departmentBudget(const std::string &name);
        const Budget &departmentBudget(const std::string &name) const;

        bool canPurchase(const std::string &id, int quantity,
                         const std::string &department,
                         std::string *reason = nullptr) const;

        const PurchaseRecord &purchase(
            const std::string &id, int quantity,
            const std::string &department = "General");

    private:
        Money applyTax(const Resource &resource, Money preTaxCost) const;

        std::string checkTitleLimit(const Resource &resource,
                                    const std::string &id) const;

        PurchaseRecord &record(const Resource *resource, const std::string &id,
                               int quantity, Money preTaxCost, Money postTaxCost,
                               bool approved, std::string reason,
                               const std::string &department = "General");
        Catalog &catalog_;
        Budget &budget_;
        int printTaxPercent_;
        int electronicTaxPercent_;

        std::map<ResourceCategory, int> titleLimits_;
        std::map<ResourceCategory, std::set<std::string>> purchasedTitles_;

        std::vector<PurchaseRecord> history_;
        int nextOrderNo_ = 1;

        std::set<int> cancelledOrders_;
        std::map<std::string, Budget> departmentBudgets_;

        Budget *budgetFor(const std::string &department);
        const Budget *budgetFor(const std::string &department) const;
    };

} // namespace bookmgmt
