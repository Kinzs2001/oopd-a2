// Demo: builds a small catalog, sets a budget with per-category quotas,
// and runs a batch of purchase requests through the acquisition manager.

#include <iostream>

#include "bookmgmt/bookmgmt.h"

using namespace bookmgmt;

int main()
{
    Catalog catalog;

    catalog.emplace<Book>("B001", "Clean Code", std::vector<std::string>{"Robert C. Martin"},
                          "978-0132350884", "Prentice Hall", 2008, Money::of(450));
    catalog.emplace<Book>("B002", "The C++ Programming Language",
                          std::vector<std::string>{"Bjarne Stroustrup"}, "978-0321563842",
                          "Addison-Wesley", 2013, Money::of(1200), 4, Binding::Hardcover);
    catalog.emplace<ElectronicResource>("R001", "IEEE Xplore Digital Library", "IEEE", 2026,
                                        Money::of(150), "https://ieeexplore.example",
                                        LicenseModel::AnnualSubscription, Money::of(2000));
    catalog.emplace<ElectronicResource>("R002", "MATLAB Campus Licence", "MathWorks", 2026,
                                        Money::of(400), "https://licensing.example/matlab",
                                        LicenseModel::Perpetual);

    std::cout << "=== Catalog ===\n";
    for (const Resource *r : catalog.all())
        std::cout << r->summary() << "\n";

    std::cout << "\n=== Details of R001 ===\n"
              << catalog.get("R001");

    Budget budget(Money::of(20000));
    budget.setQuota(ResourceCategory::Book, {10, Money::of(8000)});
    budget.setQuota(ResourceCategory::ElectronicResource, {40, Money::of(12000)});

    AcquisitionManager acq(catalog, budget, 10, 18);
    acq.setTitleLimit(ResourceCategory::Book, 2);
    std::cout << "\n=== Quotes ===\n";
    std::cout << "5 copies of B002  = " << acq.quote("B002", 5) << "\n";
    std::cout << "20 seats of R001  = " << acq.quote("R001", 20) << "  (incl. platform fee)\n";

    acq.processBatch({
        {"B001", 4},  // 1800  ok
        {"B002", 5},  // 6000  ok  -> book spend 7800
        {"B001", 1},  // 450   rejected: book spend quota (200 left)
        {"R001", 20}, // 5000  ok
        {"R002", 25}, // 10000 rejected: e-resource unit quota (20 seats left)
        {"R002", 15}, // 6000  ok  -> e-resource spend 11000
        {"R002", 5},  // 2000  rejected: e-resource spend quota (1000 left)
        {"X999", 1},  // rejected: unknown id
    });
    std::cout << "\n=== Cancellation ===\n";
    try
    {
        const auto &cancelled = acq.cancel(1);
        std::cout << "Cancelled order #" << cancelled.relatedOrderNo
                  << ": " << cancelled.title << "\n";
    }
    catch (const std::exception &e)
    {
        std::cout << "Cancellation failed: " << e.what() << "\n";
    }

    std::cout << "\n=== Acquisition report ===\n";
    acq.printReport(std::cout);

    std::cout << "\n=== Budget ===\n";
    budget.print(std::cout);

    std::cout << "\n=== Holdings ===\n";
    for (const Resource *r : catalog.all())
        std::cout << "  " << r->id() << ": " << catalog.holdings(r->id())
                  << (r->isDigital() ? " seats" : " copies") << "\n";

    // Direct purchase: errors are reported with exceptions
    std::cout << "\n=== Direct purchase that breaks a quota ===\n";
    try
    {
        acq.purchase("B002", 1);
    }
    catch (const QuotaExceededError &e)
    {
        std::cout << "QuotaExceededError: " << e.what() << "\n";
    }

    // Q10: Year-end budget rollover
    std::cout << "\n=== Q10: Year-end budget rollover ===\n";

    const int carryForwardPercent = 50;
    const Budget nextYearBudget = budget.rollover(carryForwardPercent);

    std::cout << "This year's unspent budget: " << budget.remaining() << "\n";
    std::cout << "Carry-forward percentage: " << carryForwardPercent << "%\n";
    std::cout << "Next year's budget: " << nextYearBudget.total() << "\n";

    // Q11: All-or-nothing batch processing
    std::cout << "\n=== Q11: All-or-nothing batch ===\n";

    Catalog atomicCatalog;
    atomicCatalog.emplace<Book>(
        "B101", "Atomic Batch Book", std::vector<std::string>{"Author A"},
        "ISBN-101", "Publisher", 2024, Money::of(100));
    atomicCatalog.emplace<Book>(
        "B102", "Second Batch Book", std::vector<std::string>{"Author B"},
        "ISBN-102", "Publisher", 2025, Money::of(100));

    Budget atomicBudget(Money::of(500));
    AcquisitionManager atomicAcq(atomicCatalog, atomicBudget);

    std::cout << "Attempting a batch containing an unknown resource:\n";
    const auto aborted = atomicAcq.processBatch(
        {{"B101", 2}, {"UNKNOWN", 1}}, true);

    for (const auto &result : aborted)
        std::cout << "  " << result.resourceId << ": "
                  << (result.approved ? "approved" : "rejected")
                  << " (" << result.reason << ")\n";

    std::cout << "Spent after aborted batch: "
              << atomicBudget.spent() << "\n";
    std::cout << "B101 holdings after aborted batch: "
              << atomicCatalog.holdings("B101") << "\n";

    std::cout << "Attempting a valid batch:\n";
    const auto accepted = atomicAcq.processBatch(
        {{"B101", 2}, {"B102", 1}}, true);

    for (const auto &result : accepted)
        std::cout << "  " << result.resourceId << ": "
                  << (result.approved ? "approved" : "rejected") << "\n";

    std::cout << "Spent after successful batch: "
              << atomicBudget.spent() << "\n";

    // Q12: Vendor selection
    std::cout << "\n=== Q12: Cheapest vendor selection ===\n";

    Catalog vendorCatalog;
    auto &vendorBook = vendorCatalog.emplace<Book>(
        "B201", "Vendor Selection Book",
        std::vector<std::string>{"Author C"},
        "ISBN-201", "Publisher", 2026, Money::of(100));

    vendorBook.addVendorOffer("Vendor A", Money::of(120));
    vendorBook.addVendorOffer("Vendor B", Money::of(80));
    vendorBook.addVendorOffer("Vendor C", Money::of(100));

    Budget vendorBudget(Money::of(1000));
    AcquisitionManager vendorAcq(vendorCatalog, vendorBudget);

    std::cout << "Quote for 2 copies: "
              << vendorAcq.quote("B201", 2) << "\n";

    const auto &vendorOrder = vendorAcq.purchase("B201", 2);

    std::cout << "Selected vendor: " << vendorOrder.vendor << "\n";
    std::cout << "Pre-tax cost: " << vendorOrder.preTaxCost << "\n";
    std::cout << "Post-tax cost: " << vendorOrder.cost << "\n";

    std::cout << "Vendor purchase report:\n";
    vendorAcq.printReport(std::cout);

    // Q13: Catalogue searches
    std::cout << "\n=== Q13: Catalogue searches ===\n";

    Catalog searchCatalog;

    searchCatalog.emplace<Book>(
        "B301", "Clean Code",
        std::vector<std::string>{"Robert C. Martin"},
        "978-0132350884", "Prentice Hall", 2008, Money::of(450));

    searchCatalog.emplace<EBook>(
        "E301", "Effective Modern C++",
        std::vector<std::string>{"Scott Meyers"},
        "978-1491903995", "O'Reilly", 2014, Money::of(300),
        "https://example.com", EBookFormat::EPUB, false);

    searchCatalog.emplace<Journal>(
        "J301", "Nature", "Springer", 2022,
        "0028-0836", 52, Money::of(500));

    std::cout << "Author search for 'Scott Meyers':\n";
    for (const Resource *resource : searchCatalog.searchAuthor("Scott Meyers"))
        std::cout << resource->summary() << "\n";

    std::cout << "Identifier search for '0028-0836':\n";
    for (const Resource *resource : searchCatalog.searchIdentifier("0028-0836"))
        std::cout << resource->summary() << "\n";

    std::cout << "Resources published from 2010 to 2022:\n";
    for (const Resource *resource : searchCatalog.searchYearRange(2010, 2022))
        std::cout << resource->summary() << "\n";

    // Q14: Lending and electronic sessions
    std::cout << "\n=== Q14: Lending and electronic sessions ===\n";

    Catalog lendingCatalog;

    lendingCatalog.emplace<Book>(
        "B401", "Test Book", std::vector<std::string>{"Author"},
        "ISBN-401", "Publisher", 2020, Money::of(100));

    lendingCatalog.emplace<EBook>(
        "E401", "Test EBook", std::vector<std::string>{"Author"},
        "ISBN-E401", "Publisher", 2021, Money::of(50),
        "https://example.com/ebook", EBookFormat::PDF, false,
        LicenseModel::AnnualSubscription, Money::of(100));

    lendingCatalog.addHoldings("B401", 2);
    lendingCatalog.addHoldings("E401", 2);

    lendingCatalog.borrowCopy("B401", "P1");
    std::cout << "P1 borrowed print book B401\n";

    lendingCatalog.returnCopy("B401", "P1");
    std::cout << "P1 returned print book B401\n";

    lendingCatalog.openSession("E401", "P2");
    std::cout << "P2 opened electronic session E401\n";

    lendingCatalog.closeSession("E401", "P2");
    std::cout << "P2 closed electronic session E401\n";

    std::cout << "\n=== Q15: Currency-aware Money ===\n";

    Money inr = Money::of(100, 50);
    Money usd = Money::of(100, 50, "USD");

    std::cout << "Default currency: " << inr.currency() << '\n';
    std::cout << "USD amount: " << usd.toString()
              << ' ' << usd.currency() << '\n';

    std::cout << "USD sum: "
              << (Money::of(10, 0, "USD") +
                  Money::of(20, 0, "USD"))
                     .toString()
              << " USD\n";

    try
    {
        auto invalid = inr + usd;
        std::cout << invalid.toString() << '\n';
    }
    catch (const std::invalid_argument &e)
    {
        std::cout << "Mixed-currency addition rejected: "
                  << e.what() << '\n';
    }

    return 0;
}
