#pragma once
// Catalog: owns every Resource, keyed by its id, and tracks copies/seats held.

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "bookmgmt/Resource.h"

namespace bookmgmt
{

    class Catalog
    {
    public:
        // Takes ownership. Throws DuplicateIdError if the id is already present.
        Resource &add(std::unique_ptr<Resource> r);

        // Convenience: catalog.emplace<Book>(ctor args...)
        template <typename T, typename... Args>
        T &emplace(Args &&...args)
        {
            auto p = std::make_unique<T>(std::forward<Args>(args)...);
            T &ref = *p;
            add(std::move(p));
            return ref;
        }

        bool contains(const std::string &id) const;
        Resource *find(const std::string &id);             // nullptr if absent
        const Resource *find(const std::string &id) const; // nullptr if absent
        Resource &get(const std::string &id);              // throws NotFoundError
        const Resource &get(const std::string &id) const;  // throws NotFoundError
        void remove(const std::string &id);                // throws NotFoundError

        std::size_t size() const { return items_.size(); }
        bool empty() const { return items_.empty(); }

        // Holdings: number of copies (print) or seats (electronic) owned.
        int holdings(const std::string &id) const;
        void addHoldings(const std::string &id, int units);

        // Queries. Results are in id order and point into the catalog.
        std::vector<const Resource *> all() const;
        std::vector<const Resource *> byCategory(ResourceCategory c) const;
        std::vector<const Resource *> searchTitle(const std::string &text) const; // case-insensitive
        std::vector<const Resource *> searchAuthor(const std::string &text) const;
        std::vector<const Resource *> searchIdentifier(const std::string &text) const;
        std::vector<const Resource *> searchYearRange(int minYear, int maxYear) const;
        std::vector<const Resource *> where(
            const std::function<bool(const Resource &)> &pred) const;

        void borrowCopy(const std::string &resourceId, const std::string &patronId);
        void returnCopy(const std::string &resourceId, const std::string &patronId);

        void openSession(const std::string &resourceId, const std::string &patronId);
        void closeSession(const std::string &resourceId, const std::string &patronId);

    private:
        struct Entry
        {
            std::unique_ptr<Resource> resource;
            int holdings = 0;
        };
        std::map<std::string, Entry> items_;
        std::map<std::string, std::vector<std::string>> borrowedByResource_;
        std::map<std::string, std::vector<std::string>> sessionsByResource_;
    };

} // namespace bookmgmt
