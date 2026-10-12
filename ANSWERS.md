# OOPD Assignment 2 - Answers

I attempted Q1 to Q16. For each question, I have explained the requirement, the changes made, the reason for the approach, and the testing performed in simple language.

## Q1. Journal

### What was required
The task was to add a `Journal` class derived from `Resource`. A journal needed to store its ISSN, issues published per year, and subscription length. The subscription should default to one year, values below one year should be rejected, and the cost should depend on the annual unit price and subscription length.

### Implementation steps
1. Added `Journal` as a subclass of `Resource`.
2. Added fields for ISSN, issues per year, and subscription length.
3. Added validation for the issue count and subscription length.
4. Overrode `category()` and `costFor()` for journal-specific behaviour.
5. Updated resource category handling so journals work with quotas and budget reports.
6. Updated `printDetails()` to display journal-specific information.

### Design decision
I treated a journal as a print resource because it is purchased as physical copies and does not use electronic licence-seat behaviour.

### Testing
Tests cover the default subscription length, invalid values, multi-year pricing, printed details, and journal quotas and holdings.

## Q2. EBook

### What was required
The task was to add an `EBook` class derived from `ElectronicResource`. It needed to store authors, ISBN, file format, and DRM status. Only PDF, EPUB, and HTML formats should be accepted, and existing electronic-resource pricing should remain unchanged.

### Implementation steps
1. Added `EBook` as a subclass of `ElectronicResource`.
2. Added fields for authors, ISBN, format, and DRM status.
3. Added validation for supported file formats.
4. Kept pricing behaviour inherited from `ElectronicResource`.
5. Connected the new type to the existing category and display logic.

### Design decision
I reused `ElectronicResource` for common electronic-resource behaviour instead of duplicating its pricing and licence-related logic.

### Testing
Tests check ebook-specific information and format validation, along with integration into the existing resource system.

## Q3. AudioBook and Thesis

### What was required
The task was to add `AudioBook` and `Thesis` resource types. An audiobook stores details such as narrator and duration, while a thesis stores details such as university, degree, and supervisor.

### Implementation steps
1. Added `AudioBook` and `Thesis` using the existing inheritance structure.
2. Added the fields specific to each type.
3. Added validation for type-specific values where required.
4. Connected both types to resource categories, display, and acquisition code.

### Design decision
I kept audiobook and thesis details in their own classes. This leaves common resource behaviour in the base classes and avoids adding unrelated fields to every resource.

### Testing
Tests exercise both resource types, their specific details, and their integration with the existing resource system.

## Q4. Hardcover

### What was required
The task was to support hardcover books with the required pricing without changing the existing paperback pricing behaviour.

### Implementation steps
1. Added the hardcover-specific pricing behaviour.
2. Kept the paperback calculation unchanged.
3. Connected the calculation to the existing purchase and cost-handling code.

### Design decision
I kept hardcover pricing separate from paperback pricing so the new requirement would not change existing paperback behaviour.

### Testing
The project was rebuilt and tested after the pricing changes to check that existing functionality still worked.

## Q5. Bulk Discounts

### What was required
The task was to implement the required bulk-discount rules and handle electronic-resource pricing correctly.

### Implementation steps
1. Added the required discount calculation for eligible purchases.
2. Kept electronic-resource pricing consistent with its existing rules.
3. Connected the calculation to the acquisition flow so the purchase cost reflects the applicable rule.

### Design decision
I kept discount handling within the existing pricing and acquisition flow rather than creating a separate purchase process.

### Testing
The project tests were run after the changes to check the updated pricing behaviour and its interaction with the existing code.

## Q6. Taxes and Reports

### What was required
The task was to add tax handling and update calculations and reports to show the required costs.

### Implementation steps
1. Added the required tax calculation.
2. Updated cost calculations that use tax information.
3. Updated the relevant reports to show the required cost details.

### Design decision
I connected tax handling to the existing cost and reporting flow so the displayed figures use the same purchase information.

### Testing
The project was rebuilt and tested after the tax and reporting changes.

## Q7. Different-Title Limits

### What was required
The task was to limit how many different titles can be purchased and reject a purchase that exceeds the allowed limit.

### Implementation steps
1. Added checks for the number of distinct titles in a purchase.
2. Checked the limit before approving the purchase.
3. Connected the check to the existing acquisition process.

### Design decision
I checked distinct titles rather than counting every copy as a different title, because the requirement concerns the number of different titles.

### Testing
Tests cover the title limit, including rejection when the allowed number of different titles has been reached.

## Q8. Cancellation

### What was required
The task was to support purchase cancellation and update the related budget usage, catalogue holdings, and history.

### Implementation steps
1. Added cancellation to the acquisition flow.
2. Updated budget usage when a purchase is cancelled.
3. Updated catalogue holdings to reflect the cancellation.
4. Recorded the cancellation in the history.

### Design decision
I used the existing acquisition and history structures so budget, holdings, and recorded activity remain connected.

### Testing
Tests check cancellation and its effect on related budget and holdings information.

## Q9. Department Budgets

### What was required
The task was to support separate budgets for departments and use the correct budget during acquisition.

### Implementation steps
1. Added support for department-specific budgets.
2. Connected the department budget to the acquisition process.
3. Kept budget checks within the existing purchase flow.

### Design decision
I reused the existing budget and acquisition structure instead of creating a separate purchasing system for every department.

### Testing
The project was rebuilt and tested after department-budget support was added.

## Q10. Budget Rollover

### What was required
The task was to calculate the required budget rollover and reject invalid rollover values.

### Implementation steps
1. Added the rollover calculation to the budget logic.
2. Added validation for invalid rollover values.
3. Connected the result to existing budget handling.

### Design decision
I kept rollover calculations in the budget-related code so budget rules stay together and are easier to maintain.

### Testing
The project was rebuilt and tested after the rollover changes.

## Q11. All-or-Nothing Batch Purchases

### What was required
The task was to support batch purchases without leaving partial purchases or unintended budget and holdings changes if any part of the batch fails.

### Implementation steps
1. Added batch-purchase handling.
2. Ensured a failed batch does not leave only part of the batch applied.
3. Kept budget and catalogue updates consistent with the batch result.

### Design decision
I treated the batch as one operation. This avoids leaving the system in a partially updated state when a purchase cannot be completed.

### Testing
Tests check batch-purchase behaviour, including the all-or-nothing requirement.

## Q12. Vendors

### What was required
The task was to support vendor offers, select the cheapest eligible offer, and include vendor information in purchase records and reports.

### Implementation steps
1. Added vendor offers to the purchasing flow.
2. Added selection of the cheapest eligible offer.
3. Included vendor information in purchase records and reports.

### Design decision
I connected vendor selection to the existing acquisition process so vendor choice and purchase records use the same flow.

### Testing
The project was rebuilt and tested after the vendor changes.

## Q13. Catalogue Searches

### What was required
The task was to support catalogue searches and filters by title, author, identifier, year, and category.

### Implementation steps
1. Added the required catalogue search and filtering operations.
2. Supported the specified search fields.
3. Connected searches to the existing catalogue data.

### Design decision
I used the existing catalogue as the source for searches instead of maintaining a separate copy of resource information.

### Testing
The project was rebuilt and tested after the search changes.

## Q14. Lending

### What was required
The task was to support borrowing and returning physical copies, as well as opening and closing sessions for electronic resources. Invalid operations also needed to be rejected.

### Implementation steps
1. Added operations for borrowing and returning physical copies.
2. Added operations for opening and closing electronic-resource sessions.
3. Added checks for invalid operations.

### Design decision
I kept physical borrowing and electronic sessions as separate operations because they represent different ways of using a resource.

### Testing
Tests cover lending operations and checks for invalid actions.

## Q15. Currency Support

### What was required
The task was to update the `Money` class to support different currencies while keeping INR as the default. Operations involving incompatible currencies needed to be rejected.

### Implementation steps
1. Updated `Money` to store and handle currency information.
2. Kept INR as the default currency.
3. Added checks to reject operations involving incompatible currencies.
4. Updated relevant tests and demo code.

### Design decision
I kept currency handling inside `Money` so currency checks and money operations are handled consistently instead of being repeated throughout the project.

### Testing
The project built successfully after the currency changes, and the tests passed before the later filename updates.

## Q16. Test Assertions

### What was required
The task was to replace custom `CHECK` and `CHECK_THROWS` test macros with standard C++ `assert` statements and retain a way to check expected exception types.

### Implementation steps
1. Replaced the custom test macros with standard `assert` statements.
2. Added a `throwsAs<Exception>` helper to check whether an operation throws the expected exception type.
3. Checked that the old `CHECK` and `CHECK_THROWS` usages were removed from the test file.

### Design decision
I used standard assertions as required and kept the exception-checking helper small so the tests remain readable.

### Testing
The build succeeded and the test executable printed `All assertion-based tests passed`. A search for the old header paths also returned no matches after the resource-file renaming.

## Overall Design Decisions

While working on the assignment, I tried to keep the existing project structure and avoid changes that were not required by the questions.

- I used inheritance to share common resource functionality while keeping the details of each resource type in its own class.
- I kept money calculations and currency-related checks inside the `Money` class so they could be handled consistently.
- I added validation before changing budgets and catalogue holdings, especially for purchases, cancellations, and batch operations.
- I kept department budgets, vendor selection, and purchase history connected to the existing acquisition system rather than creating separate implementations.
- I used exceptions to handle invalid operations and standard assertions to test expected behaviour.

## Overall Testing

After the filename changes, I ran:

```bash
cmake -S . -B build
cmake --build build
./build/bookmgmt_tests
```

The build completed successfully, and the test executable printed:

```text
All assertion-based tests passed
```

I also searched the project for the old `AudioBook`, `EBook`, `Journal`, and `Thesis` header paths. The search returned no matches in the checked directories.
