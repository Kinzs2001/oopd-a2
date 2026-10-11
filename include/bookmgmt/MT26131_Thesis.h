// MT26131 Piyush Saxena
#pragma once

#include <string>

#include "bookmgmt/Resource.h"

namespace bookmgmt {

// Thesis derives directly from Resource because it is neither
// a Book nor an ElectronicResource. A thesis can be free of cost
// by using a unit price of Money::of(0).

class Thesis : public Resource {
public:
    Thesis(std::string id, std::string title, std::string publisher,
           int year, Money unitPrice, std::string university,
           std::string degree, std::string supervisor);

    const std::string& university() const { return university_; }
    const std::string& degree() const { return degree_; }
    const std::string& supervisor() const { return supervisor_; }

    ResourceCategory category() const override {
        return ResourceCategory::Thesis;
    }

protected:
    void printDetails(std::ostream& os) const override;

private:
    std::string university_;
    std::string degree_;
    std::string supervisor_;
};

}  // namespace bookmgmt