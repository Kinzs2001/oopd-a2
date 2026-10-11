// Minimal self-contained test runner (no external framework needed).

#include <cassert>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include "bookmgmt/bookmgmt.h"

using namespace bookmgmt;

template <typename Exception, typename Callable>
static bool throwsAs(Callable &&action)
{
    try
    {
        action();
    }
    catch (const Exception &)
    {
        return true;
    }
    catch (...)
    {
        return false;
    }

    return false;
}

static void testMoney()
{
    assert(Money::of(12, 5).toString() == "12.05");
    assert(Money::of(-3, 50).toString() == "-3.50");
    assert(Money::fromMinor(7).toString() == "0.07");
    assert(Money::of(10) + Money::of(0, 50) == Money::fromMinor(1050));
    assert(Money::of(3) * 4 == Money::of(12));
    assert(Money::of(1) < Money::of(2));
    assert(throwsAs<std::invalid_argument>([&]()
                                           { (void)(Money::of(1, 100)); }));

    // Q15: Currency support
    Money inr = Money::of(10, 50);
    Money usd = Money::of(10, 50, "USD");
    Money eur = Money::of(10, 50, "EUR");

    assert(inr.currency() == "INR");
    assert(usd.currency() == "USD");
    assert(usd.toString() == "10.50");

    assert((Money::of(1, 0, "USD") +
            Money::of(2, 0, "USD"))
               .toString() == "3.00");

    assert(throwsAs<std::invalid_argument>([&]()
                                           { (void)(Money::of(1, 0, "USD") +
                                                    Money::of(1, 0, "EUR")); }));

    assert(throwsAs<std::invalid_argument>([&]()
                                           { (void)(Money::of(1, 0, "USD") -
                                                    Money::of(1, 0, "EUR")); }));

    assert(throwsAs<std::invalid_argument>([&]()
                                           { (void)(Money::of(1, 0, "USD") ==
                                                    Money::of(1, 0, "EUR")); }));

    assert(throwsAs<std::invalid_argument>([&]()
                                           { (void)(Money::of(1, 0, "USD") <
                                                    Money::of(1, 0, "EUR")); }));

    assert(throwsAs<std::invalid_argument>([&]()
                                           { (void)(Money::of(1, 0, "USD") <=
                                                    Money::of(1, 0, "EUR")); }));

    assert(throwsAs<std::invalid_argument>([&]()
                                           { (void)(Money::of(1, 0, "USD") >
                                                    Money::of(1, 0, "EUR")); }));

    assert(throwsAs<std::invalid_argument>([&]()
                                           { (void)(Money::of(1, 0, "USD") >=
                                                    Money::of(1, 0, "EUR")); }));

    assert(throwsAs<std::invalid_argument>([&]()
                                           { (void)(Money::of(1, 0, "")); }));
}

static void testResourcesAndCost()
{
    Book b("B1", "T", {"A", "B", "C"}, "isbn", "P", 2020, Money::of(100));
    assert(b.category() == ResourceCategory::Book);
    assert(!b.isDigital());
    assert(b.costFor(3) == Money::of(300));
    assert(throwsAs<std::invalid_argument>([&]()
                                           { (void)(b.costFor(0)); }));
    assert(joinAuthors(b.authors()) == "A, B and C");

    ElectronicResource e("R1", "DB", "P", 2026, Money::of(10), "url",
                         LicenseModel::AnnualSubscription, Money::of(100));
    assert(e.isDigital());
    assert(e.costFor(5) == Money::of(150));

    assert(e.category() == ResourceCategory::ElectronicResource);

    // Polymorphism through a base-class reference
    const Resource &r = e;
    assert(r.costFor(1) == Money::of(110));
    std::ostringstream os;
    os << r;
    assert(os.str().find("platform fee: 100.00") != std::string::npos);

    assert(throwsAs<std::invalid_argument>([&]()
                                           { (void)(Book("", "T", {}, "", "", 2000, Money::of(1))); }));
    assert(throwsAs<std::invalid_argument>([&]()
                                           { (void)(Book("B", "T", {}, "", "", 2000, Money::fromMinor(-1))); }));

    Journal j("J1", "Nature", "Springer Nature", 2026,
              "0028-0836", 52, Money::of(500), 2);

    assert(j.category() == ResourceCategory::Journal);
    assert(!j.isDigital());
    assert(j.costFor(1) == Money::of(1000));
    assert(j.costFor(3) == Money::of(3000));
    assert(j.issn() == "0028-0836");
    assert(j.issuesPerYear() == 52);
    assert(j.subscriptionYears() == 2);
    assert(throwsAs<std::invalid_argument>([&]()
                                           { (void)(j.costFor(0)); }));
    assert(throwsAs<std::invalid_argument>([&]()
                                           { (void)(Journal("J2", "Nature", "Springer", 2026,
                                                            "0028-0836", 52, Money::of(500), 0)); }));

    EBook eb("EB1", "Effective Modern C++",
             {"Scott Meyers"}, "978-1491903995", "O'Reilly",
             2014, Money::of(20), "https://example.com/ebook",
             EBookFormat::EPUB, true,
             LicenseModel::AnnualSubscription, Money::of(50));

    assert(eb.category() == ResourceCategory::EBook);
    assert(eb.isDigital());
    assert(eb.authors().size() == 1);
    assert(eb.isbn() == "978-1491903995");
    assert(eb.format() == EBookFormat::EPUB);
    assert(eb.drmProtected());
    assert(eb.costFor(2) == Money::of(90));
    assert(std::string(formatName(EBookFormat::PDF)) == "PDF");

    AudioBook audio("A1", "Clean Code Audio", "Publisher", 2024,
                    Money::of(15), "https://example.com/audio",
                    "Narrator Name", 300,
                    LicenseModel::AnnualSubscription, Money::of(25));

    assert(audio.category() == ResourceCategory::AudioBook);
    assert(audio.isDigital());
    assert(audio.narrator() == "Narrator Name");
    assert(audio.durationMinutes() == 300);
    assert(audio.costFor(2) == Money::of(55));

    Thesis thesis("T1", "Graph Algorithms", "IIIT Delhi", 2026,
                  Money::of(0), "IIIT Delhi", "M.Tech", "Dr. Example");

    assert(thesis.category() == ResourceCategory::Thesis);
    assert(!thesis.isDigital());
    assert(thesis.university() == "IIIT Delhi");
    assert(thesis.degree() == "M.Tech");
    assert(thesis.supervisor() == "Dr. Example");
    assert(thesis.costFor(3) == Money::of(0));

    Book paperback("BP", "Paperback", {"A"}, "I1", "P", 2024,
                   Money::of(100), 1, Binding::Paperback);
    Book hardcover("BH", "Hardcover", {"A"}, "I2", "P", 2024,
                   Money::of(100), 1, Binding::Hardcover);

    assert(paperback.costFor(2) == Money::of(200));
    assert(hardcover.costFor(2) == Money::of(240));
    assert(throwsAs<std::invalid_argument>([&]()
                                           { (void)(hardcover.costFor(0)); }));

    assert(e.costFor(50) == Money::of(600));
    assert(e.costFor(60) == Money::of(650));
}

static void testCatalog()
{
    Catalog c;
    c.emplace<Book>("B1", "Clean Code", std::vector<std::string>{"M"}, "i", "P", 2008,
                    Money::of(1));
    c.emplace<Book>("B2", "Clean Architecture", std::vector<std::string>{"M"}, "i", "P",
                    2017, Money::of(1));
    c.emplace<ElectronicResource>("R1", "ACM Digital Library", "ACM", 2026, Money::of(1),
                                  "url");

    c.emplace<EBook>("E1", "Modern C++", std::vector<std::string>{"Scott Meyers"},
                     "ISBN-123", "Publisher", 2019, Money::of(10), "url",
                     EBookFormat::PDF, true);

    c.emplace<Journal>("J1", "Nature", "Springer", 2022, "ISSN-456", 52,
                       Money::of(100));

    assert(c.size() == 5);
    assert(c.contains("B1"));
    assert(c.find("nope") == nullptr);
    assert(throwsAs<NotFoundError>([&]()
                                   { (void)(c.get("nope")); }));
    assert(throwsAs<DuplicateIdError>([&]()
                                      { (void)(c.emplace<Book>("B1", "dup", std::vector<std::string>{}, "", "", 1,
                                                               Money::of(1))); }));

    assert(c.searchTitle("clean").size() == 2);

    // Q13: Search by author.
    assert(c.searchAuthor("scott meyers").size() == 1);
    assert(c.searchAuthor("MEYERS").size() == 1);
    assert(c.searchAuthor("unknown author").empty());

    // Q13: Search by ISBN/ISSN.
    assert(c.searchIdentifier("ISBN-123").size() == 1);
    assert(c.searchIdentifier("issn-456").size() == 1);
    assert(c.searchIdentifier("unknown").empty());

    // Q13: Inclusive publication-year range.
    assert(c.searchYearRange(2008, 2019).size() == 3);
    assert(c.searchYearRange(2022, 2022).size() == 1);
    assert(c.searchYearRange(2030, 2040).empty());
    assert(throwsAs<std::invalid_argument>([&]()
                                           { (void)(c.searchYearRange(2020, 2010)); }));

    assert(c.byCategory(ResourceCategory::ElectronicResource).size() == 1);
    assert(c.where([](const Resource &r)
                   { return r.isDigital(); })
               .size() == 2);

    assert(c.holdings("B1") == 0);
    c.addHoldings("B1", 3);
    assert(c.holdings("B1") == 3);
    assert(throwsAs<std::invalid_argument>([&]()
                                           { (void)(c.addHoldings("B1", -5)); }));

    c.remove("R1");
    assert(c.size() == 4);
    assert(throwsAs<NotFoundError>([&]()
                                   { (void)(c.remove("R1")); }));
}

static void testBudget()
{
    Budget b(Money::of(1000));
    b.setQuota(ResourceCategory::Book, {5, Money::of(400)});

    assert(b.check(ResourceCategory::Book, 2, Money::of(200)).empty());
    assert(!b.check(ResourceCategory::Book, 6, Money::of(10)).empty());                 // units
    assert(!b.check(ResourceCategory::Book, 1, Money::of(401)).empty());                // spend
    assert(!b.check(ResourceCategory::ElectronicResource, 1, Money::of(1001)).empty()); // overall
    assert(b.check(ResourceCategory::ElectronicResource, 1, Money::of(900)).empty());   // no quota

    b.commit(ResourceCategory::Book, 4, Money::of(300));
    assert(b.spent() == Money::of(300));
    assert(*b.unitsRemaining(ResourceCategory::Book) == 1);
    assert(*b.spendRemaining(ResourceCategory::Book) == Money::of(100));
    assert(!b.unitsRemaining(ResourceCategory::ElectronicResource).has_value());

    assert(throwsAs<QuotaExceededError>([&]()
                                        { (void)(b.commit(ResourceCategory::Book, 2, Money::of(10))); }));
    assert(throwsAs<BudgetExceededError>([&]()
                                         { (void)(b.commit(ResourceCategory::ElectronicResource, 1, Money::of(800))); }));
    assert(throwsAs<std::invalid_argument>([&]()
                                           { (void)(b.commit(ResourceCategory::ElectronicResource, 0, Money::of(1))); }));
    assert(b.spent() == Money::of(300)); // failed commits changed nothing
}

static void testAcquisition()
{
    Catalog c;
    c.emplace<Book>("B1", "Book", std::vector<std::string>{"A"}, "i", "P", 2020,
                    Money::of(100));
    c.emplace<ElectronicResource>("R1", "DB", "P", 2026, Money::of(10), "url",
                                  LicenseModel::AnnualSubscription, Money::of(50));
    Budget b(Money::of(500));
    b.setQuota(ResourceCategory::Book, {3, Money::of(1000)});
    AcquisitionManager acq(c, b);

    assert(acq.quote("R1", 5) == Money::of(100));
    std::string why;
    assert(acq.canPurchase("B1", 3, &why) && why.empty());
    assert(!acq.canPurchase("B1", 4, &why) && !why.empty());
    assert(!acq.canPurchase("nope", 1, &why));

    const auto &rec = acq.purchase("B1", 2);
    assert(rec.approved && rec.cost == Money::of(200) && rec.orderNo == 1);
    assert(c.holdings("B1") == 2);

    assert(throwsAs<QuotaExceededError>([&]()
                                        { (void)(acq.purchase("B1", 2)); }));
    assert(throwsAs<NotFoundError>([&]()
                                   { (void)(acq.purchase("nope", 1)); }));
    assert(acq.history().size() == 1); // exceptions don't record

    auto res = acq.processBatch({{"R1", 10}, {"R1", 100}, {"B1", 1}, {"zzz", 1}, {"B1", 0}});
    assert(res.size() == 5);
    assert(res[0].approved && res[0].cost == Money::of(150));
    assert(!res[1].approved); // 1050 > remaining 150
    assert(res[2].approved);
    assert(!res[3].approved && res[3].reason.find("not found") != std::string::npos);
    assert(!res[4].approved);
    assert(acq.totalSpent() == Money::of(450));
    assert(b.spent() == acq.totalSpent());
    assert(c.holdings("R1") == 10 && c.holdings("B1") == 3);
    assert(acq.history().size() == 6);
}

static void testTitleLimits()
{
    Catalog c;
    c.emplace<Book>("B1", "Book One", std::vector<std::string>{"A"},
                    "i1", "P", 2020, Money::of(100));
    c.emplace<Book>("B2", "Book Two", std::vector<std::string>{"B"},
                    "i2", "P", 2021, Money::of(100));

    Budget b(Money::of(1000));
    AcquisitionManager acq(c, b);
    acq.setTitleLimit(ResourceCategory::Book, 1);

    assert(acq.canPurchase("B1", 1));
    assert(acq.purchase("B1", 1).approved);

    // Buying the same title again is allowed.
    assert(acq.canPurchase("B1", 1));
    assert(acq.purchase("B1", 1).approved);

    // A different title in the same category is rejected.
    std::string reason;
    assert(!acq.canPurchase("B2", 1, &reason));
    assert(reason.find("title limit") != std::string::npos);

    assert(throwsAs<QuotaExceededError>([&]()
                                        { (void)(acq.purchase("B2", 1)); }));

    // A rejected batch request must not consume a title slot.
    acq.setTitleLimit(ResourceCategory::Book, 2);
    auto results = acq.processBatch({{"B2", 1}});
    assert(results.size() == 1);
    assert(results[0].approved);

    assert(throwsAs<std::invalid_argument>([&]()
                                           { (void)(acq.setTitleLimit(ResourceCategory::Book, -1)); }));
}

static void testCancellation()
{
    Catalog c;
    c.emplace<Book>("B1", "Book One", std::vector<std::string>{"A"},
                    "i1", "P", 2020, Money::of(100));

    Budget b(Money::of(1000));
    b.setQuota(ResourceCategory::Book, {5, Money::of(500)});

    AcquisitionManager acq(c, b, 10, 0);

    // Purchase 2 copies: pre-tax 200, post-tax 220.
    const auto &purchase = acq.purchase("B1", 2);
    const int orderNo = purchase.orderNo;

    assert(purchase.approved);
    assert(b.spent() == Money::of(220));
    assert(b.usageFor(ResourceCategory::Book).units == 2);
    assert(b.usageFor(ResourceCategory::Book).spent == Money::of(220));
    assert(c.holdings("B1") == 2);

    // Cancel the approved order.
    const auto &cancelRecord = acq.cancel(orderNo);

    assert(cancelRecord.cancellation);
    assert(cancelRecord.relatedOrderNo == orderNo);
    assert(cancelRecord.reason.find("Cancellation") != std::string::npos);

    // Budget, category quota, and holdings are restored.
    assert(b.spent() == Money::of(0));
    assert(b.remaining() == Money::of(1000));
    assert(b.usageFor(ResourceCategory::Book).units == 0);
    assert(b.usageFor(ResourceCategory::Book).spent == Money::of(0));
    assert(c.holdings("B1") == 0);
    assert(acq.totalSpent() == Money::of(0));

    // Original purchase and separate cancellation record remain in history.
    assert(acq.history().size() == 2);
    assert(acq.history()[0].orderNo == orderNo);
    assert(acq.history()[0].approved);
    assert(!acq.history()[0].cancellation);
    assert(acq.history()[1].cancellation);

    // Duplicate cancellation must fail.
    assert(throwsAs<std::invalid_argument>([&]()
                                           { (void)(acq.cancel(orderNo)); }));

    // Rejected and nonexistent orders cannot be cancelled.
    const auto rejected = acq.processBatch({{"B1", 0}});
    assert(rejected.size() == 1);
    assert(!rejected[0].approved);
    assert(throwsAs<std::invalid_argument>([&]()
                                           { (void)(acq.cancel(rejected[0].orderNo)); }));
    assert(throwsAs<std::invalid_argument>([&]()
                                           { (void)(acq.cancel(9999)); }));

    // Cancellation releases the distinct-title slot.
    acq.setTitleLimit(ResourceCategory::Book, 1);
    assert(acq.canPurchase("B1", 1));
}

static void testDepartmentBudgets()
{
    Catalog c;
    c.emplace<Book>("B1", "Book One", std::vector<std::string>{"A"},
                    "i1", "P", 2020, Money::of(100));

    Budget generalBudget(Money::of(1000));
    AcquisitionManager acq(c, generalBudget, 10, 0);

    acq.addDepartment("Computer Science", Money::of(500));
    acq.addDepartment("Physics", Money::of(150));

    // Each department has independent quotas.
    acq.departmentBudget("Computer Science")
        .setQuota(ResourceCategory::Book, {2, Money::of(500)});

    acq.departmentBudget("Physics")
        .setQuota(ResourceCategory::Book, {1, Money::of(150)});

    // Computer Science buys 2 copies; Physics buys 1.
    const auto results = acq.processBatch({{"B1", 2, "Computer Science"},
                                           {"B1", 1, "Physics"},
                                           {"B1", 1, "Unknown Department"}});

    assert(results.size() == 3);
    assert(results[0].approved);
    assert(results[1].approved);
    assert(!results[2].approved);

    // Each purchase is charged to the requested department.
    assert(results[0].department == "Computer Science");
    assert(results[1].department == "Physics");
    assert(results[2].department == "Unknown Department");

    Budget &cs = acq.departmentBudget("Computer Science");
    Budget &physics = acq.departmentBudget("Physics");

    // Post-tax price is 110 per copy.
    assert(cs.spent() == Money::of(220));
    assert(physics.spent() == Money::of(110));

    assert(cs.usageFor(ResourceCategory::Book).units == 2);
    assert(physics.usageFor(ResourceCategory::Book).units == 1);

    // Department spending must not affect the general budget.
    assert(generalBudget.spent() == Money::of(0));
    assert(generalBudget.remaining() == Money::of(1000));

    // Physics cannot exceed its own unit quota.
    assert(throwsAs<bookmgmt::QuotaExceededError>([&]()
                                                  { (void)(acq.purchase("B1", 1, "Physics")); }));

    assert(physics.usageFor(ResourceCategory::Book).units == 1);
    assert(physics.spent() == Money::of(110));

    // Cancelling Computer Science's order refunds only its budget.
    const int csOrderNo = results[0].orderNo;
    const auto &cancelRecord = acq.cancel(csOrderNo);

    assert(cancelRecord.cancellation);
    assert(cancelRecord.department == "Computer Science");
    assert(cs.spent() == Money::of(0));
    assert(cs.usageFor(ResourceCategory::Book).units == 0);

    assert(physics.spent() == Money::of(110));
    assert(physics.usageFor(ResourceCategory::Book).units == 1);

    // Holdings reflect the active purchases after cancellation.
    assert(c.holdings("B1") == 1);

    // Invalid department operations must fail.
    assert(throwsAs<std::invalid_argument>([&]()
                                           { (void)(acq.departmentBudget("Unknown Department")); }));
    assert(throwsAs<std::invalid_argument>([&]()
                                           { (void)(acq.addDepartment("Physics", Money::of(100))); }));
}

static void testBudgetRollover()
{
    Budget current(Money::of(10000));
    current.setQuota(ResourceCategory::Book,
                     {10, Money::of(5000)});

    current.commit(ResourceCategory::Book, 2, Money::of(3000));

    // Unspent = 10000 - 3000 = 7000.
    const Budget half = current.rollover(50);
    assert(half.total() == Money::of(3500));
    assert(half.spent() == Money::of(0));
    assert(half.remaining() == Money::of(3500));

    // The original budget remains unchanged.
    assert(current.total() == Money::of(10000));
    assert(current.spent() == Money::of(3000));
    assert(current.remaining() == Money::of(7000));

    // Zero and 100% carry-forward.
    assert(current.rollover(0).total() == Money::of(0));
    assert(current.rollover(100).total() == Money::of(7000));

    // Invalid percentages must be rejected.
    assert(throwsAs<std::invalid_argument>([&]()
                                           { (void)(current.rollover(-1)); }));
    assert(throwsAs<std::invalid_argument>([&]()
                                           { (void)(current.rollover(101)); }));
}

static void testAllOrNothingBatch()
{
    Catalog c;
    c.emplace<Book>("B1", "Book One", std::vector<std::string>{"A"},
                    "i1", "P", 2020, Money::of(100));
    c.emplace<Book>("B2", "Book Two", std::vector<std::string>{"B"},
                    "i2", "P", 2021, Money::of(100));

    Budget b(Money::of(500));
    AcquisitionManager acq(c, b);

    // A later invalid request must abort the entire batch.
    const auto rejected = acq.processBatch(
        {{"B1", 2}, {"UNKNOWN", 1}}, true);

    assert(rejected.size() == 2);
    assert(!rejected[0].approved);
    assert(!rejected[1].approved);
    assert(rejected[1].reason.find("not found") != std::string::npos);
    assert(b.spent() == Money::of(0));
    assert(b.remaining() == Money::of(500));
    assert(c.holdings("B1") == 0);
    assert(acq.totalSpent() == Money::of(0));

    // A batch whose combined cost exceeds the budget must also abort.
    const auto overBudget = acq.processBatch(
        {{"B1", 3}, {"B2", 3}}, true);

    assert(overBudget.size() == 2);
    assert(!overBudget[0].approved);
    assert(!overBudget[1].approved);
    assert(b.spent() == Money::of(0));
    assert(c.holdings("B1") == 0);
    assert(c.holdings("B2") == 0);

    // A fully valid batch should approve every request.
    const auto approved = acq.processBatch(
        {{"B1", 2}, {"B2", 1}}, true);

    assert(approved.size() == 2);
    assert(approved[0].approved);
    assert(approved[1].approved);
    assert(b.spent() == Money::of(300));
    assert(c.holdings("B1") == 2);
    assert(c.holdings("B2") == 1);
}

static void testVendors()
{
    Catalog c;
    auto &book = c.emplace<Book>(
        "B1", "Vendor Test Book", std::vector<std::string>{"A"},
        "i1", "P", 2020, Money::of(100), 1, Binding::Paperback);

    book.addVendorOffer("Vendor A", Money::of(120));
    book.addVendorOffer("Vendor B", Money::of(80));
    book.addVendorOffer("Vendor C", Money::of(100));

    assert(book.vendorOffers().size() == 3);
    assert(book.cheapestVendorOffer().vendor == "Vendor B");
    assert(book.cheapestVendorOffer().unitPrice == Money::of(80));
    assert(book.costForAtPrice(2, Money::of(80)) == Money::of(160));

    assert(throwsAs<std::invalid_argument>([&]()
                                           { (void)(book.addVendorOffer("Vendor B", Money::of(70))); }));
    assert(throwsAs<std::invalid_argument>([&]()
                                           { (void)(book.addVendorOffer("", Money::of(70))); }));
    assert(throwsAs<std::invalid_argument>([&]()
                                           { (void)(book.addVendorOffer("Vendor D", Money::fromMinor(-1))); }));

    Budget budget(Money::of(1000));
    AcquisitionManager acq(c, budget);

    assert(acq.quote("B1", 2) == Money::of(160));
    assert(acq.canPurchase("B1", 2));

    const auto &order = acq.purchase("B1", 2);
    assert(order.approved);
    assert(order.vendor == "Vendor B");
    assert(order.preTaxCost == Money::of(160));
    assert(order.cost == Money::of(160));

    std::ostringstream report;
    acq.printReport(report);
    assert(report.str().find("Vendor: Vendor B") != std::string::npos);

    const auto &cancellation = acq.cancel(order.orderNo);
    assert(cancellation.cancellation);
    assert(cancellation.vendor == "Vendor B");
    assert(acq.totalSpent() == Money::of(0));

    const auto batch = acq.processBatch({{"B1", 1}}, true);
    assert(batch.size() == 1);
    assert(batch[0].approved);
    assert(batch[0].vendor == "Vendor B");
    assert(batch[0].preTaxCost == Money::of(80));

    Catalog emptyCatalog;
    emptyCatalog.emplace<Book>(
        "B2", "No Vendor Book", std::vector<std::string>{"B"},
        "i2", "P", 2021, Money::of(50));

    Budget emptyBudget(Money::of(200));
    AcquisitionManager emptyAcq(emptyCatalog, emptyBudget);
    const auto &ordinary = emptyAcq.purchase("B2", 1);
    assert(ordinary.approved);
    assert(ordinary.vendor.empty());
    assert(ordinary.cost == Money::of(50));

    Book noOffers("B3", "No Offers", {"C"}, "i3", "P", 2022,
                  Money::of(30));
    assert(throwsAs<std::logic_error>([&]()
                                      { (void)(noOffers.cheapestVendorOffer()); }));
}

static void testLending()
{
    Catalog c;

    c.emplace<Book>("B14", "Test Book",
                    std::vector<std::string>{"Author"},
                    "ISBN-14", "Publisher", 2020, Money::of(100));

    c.emplace<EBook>("E14", "Test EBook",
                     std::vector<std::string>{"Author"},
                     "ISBN-E14", "Publisher", 2021, Money::of(50),
                     "https://example.com/ebook", EBookFormat::PDF, false,
                     LicenseModel::AnnualSubscription, Money::of(100));

    c.addHoldings("B14", 2);
    c.addHoldings("E14", 2);

    // Print borrowing and return.
    c.borrowCopy("B14", "P1");
    c.borrowCopy("B14", "P2");

    assert(throwsAs<std::runtime_error>([&]()
                                        { (void)(c.borrowCopy("B14", "P3")); }));
    assert(throwsAs<std::logic_error>([&]()
                                      { (void)(c.borrowCopy("B14", "P1")); }));
    assert(throwsAs<std::logic_error>([&]()
                                      { (void)(c.returnCopy("B14", "P3")); }));

    c.returnCopy("B14", "P1");
    c.borrowCopy("B14", "P3");

    // Electronic sessions.
    c.openSession("E14", "P1");
    c.openSession("E14", "P2");

    assert(throwsAs<std::runtime_error>([&]()
                                        { (void)(c.openSession("E14", "P3")); }));
    assert(throwsAs<std::logic_error>([&]()
                                      { (void)(c.openSession("E14", "P1")); }));
    assert(throwsAs<std::logic_error>([&]()
                                      { (void)(c.closeSession("E14", "P3")); }));

    c.closeSession("E14", "P1");
    c.openSession("E14", "P3");

    // Wrong resource types and invalid patron IDs.
    assert(throwsAs<std::invalid_argument>([&]()
                                           { (void)(c.borrowCopy("E14", "P4")); }));
    assert(throwsAs<std::invalid_argument>([&]()
                                           { (void)(c.returnCopy("E14", "P4")); }));
    assert(throwsAs<std::invalid_argument>([&]()
                                           { (void)(c.openSession("B14", "P4")); }));
    assert(throwsAs<std::invalid_argument>([&]()
                                           { (void)(c.closeSession("B14", "P4")); }));
    assert(throwsAs<std::invalid_argument>([&]()
                                           { (void)(c.borrowCopy("B14", "")); }));
    assert(throwsAs<std::invalid_argument>([&]()
                                           { (void)(c.openSession("E14", "")); }));
}

int main()
{
    testMoney();
    testResourcesAndCost();
    testCatalog();
    testBudget();
    testAcquisition();
    testTitleLimits();
    testCancellation();
    testDepartmentBudgets();
    testBudgetRollover();
    testAllOrNothingBatch();
    testVendors();
    testLending();

    std::cout << "All assertion-based tests passed\n";
    return 0;
}
