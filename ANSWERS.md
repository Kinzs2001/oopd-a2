# OOPD Assignment 2 - Answers

## Questions Attempted

I have attempted all questions from Q1 to Q16. The following is a brief
summary of what I implemented for each question.

### Q1 - Journal
I added a Journal resource with details such as its ISSN, number of issues
published per year, and subscription duration.

### Q2 - EBook
I added an EBook class with details specific to electronic books, including
their format and DRM protection.

### Q3 - AudioBook and Thesis
I added AudioBook and Thesis resource types. Each type stores the information
relevant to it, such as the narrator and duration for an audiobook, and the
university, degree, and supervisor for a thesis.

### Q4 - Hardcover
I implemented hardcover pricing while keeping the existing paperback
pricing behaviour unchanged.

### Q5 - Bulk Discounts
I implemented the required bulk-discount rules and electronic-resource
pricing behaviour.

### Q6 - Taxes and Reports
I added tax handling and updated the relevant calculations and reports to
show the required costs.

### Q7 - Different-Title Limits
I implemented limits on the number of different titles that can be purchased
and added the necessary checks before approving a purchase.

### Q8 - Cancellation
I added purchase cancellation. When a purchase is cancelled, the related
budget usage and catalogue holdings are updated, and the cancellation is
recorded in the history.

### Q9 - Department Budgets
I added support for separate department budgets and connected them to the
acquisition process.

### Q10 - Budget Rollover
I implemented the required budget rollover calculation, including checks
for invalid rollover values.

### Q11 - All-or-Nothing Batch Purchases
I implemented batch purchasing so that a failed batch does not leave behind
partial purchases or unintended changes to budgets and holdings.

### Q12 - Vendors
I added vendor offers, selection of the cheapest eligible offer, and
vendor information in purchase records and reports.

### Q13 - Catalogue Searches
I implemented the required catalogue search and filtering operations,
including searches by title, author, identifier, year, and category.

### Q14 - Lending
I added operations for borrowing and returning physical copies, as well as
opening and closing sessions for electronic resources. I also added checks
for invalid operations.

### Q15 - Currency Support
I updated the Money class to support different currencies. INR remains the
default currency, and operations involving incompatible currencies are
rejected.

### Q16 - Test Assertions
I replaced the custom CHECK and CHECK_THROWS test macros with standard C++
assert statements. I also added a helper to check whether an operation
throws the expected type of exception.

## Design Decisions

While working on the assignment, I tried to keep the existing project
structure and avoid changes that were not required by the questions.

- I used inheritance to share common resource functionality while keeping
  the details specific to each resource type in its own class.
- I kept money calculations and currency-related checks inside the Money
  class so that they could be handled consistently.
- I added validation before making changes to budgets and catalogue
  holdings, especially for purchases, cancellations, and batch operations.
- I kept department budgets, vendor selection, and purchase history
  connected to the existing acquisition system rather than creating
  separate, unrelated implementations.
- I used exceptions to handle invalid operations and standard assertions
  to test expected behaviour.

## Testing

I built the project using CMake after completing the changes. The build
succeeded, and the test executable ran successfully after the Q16 changes.

I also checked that no CHECK or CHECK_THROWS calls remained in the test
file after converting the tests to assertions.

