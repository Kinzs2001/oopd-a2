#include "bookmgmt/Catalog.h"

#include "bookmgmt/Book.h"
#include "bookmgmt/MT26131_EBook.h"
#include "bookmgmt/MT26131_Journal.h"

#include <algorithm>
#include <cctype>
#include <stdexcept>

#include <algorithm>
#include <stdexcept>

#include "bookmgmt/Exceptions.h"

namespace bookmgmt
{

    namespace
    {
        std::string lower(std::string s)
        {
            std::transform(s.begin(), s.end(), s.begin(),
                           [](unsigned char ch)
                           { return static_cast<char>(std::tolower(ch)); });
            return s;
        }
    } // namespace

    Resource &Catalog::add(std::unique_ptr<Resource> r)
    {
        if (!r)
            throw std::invalid_argument("cannot add a null resource");
        const std::string id = r->id();
        if (items_.count(id))
            throw DuplicateIdError(id);
        Entry &e = items_[id];
        e.resource = std::move(r);
        return *e.resource;
    }

    bool Catalog::contains(const std::string &id) const { return items_.count(id) > 0; }

    Resource *Catalog::find(const std::string &id)
    {
        auto it = items_.find(id);
        return it == items_.end() ? nullptr : it->second.resource.get();
    }

    const Resource *Catalog::find(const std::string &id) const
    {
        auto it = items_.find(id);
        return it == items_.end() ? nullptr : it->second.resource.get();
    }

    Resource &Catalog::get(const std::string &id)
    {
        if (Resource *r = find(id))
            return *r;
        throw NotFoundError(id);
    }

    const Resource &Catalog::get(const std::string &id) const
    {
        if (const Resource *r = find(id))
            return *r;
        throw NotFoundError(id);
    }

    void Catalog::remove(const std::string &id)
    {
        if (items_.erase(id) == 0)
            throw NotFoundError(id);
    }

    int Catalog::holdings(const std::string &id) const
    {
        auto it = items_.find(id);
        if (it == items_.end())
            throw NotFoundError(id);
        return it->second.holdings;
    }

    void Catalog::addHoldings(const std::string &id, int units)
    {
        auto it = items_.find(id);
        if (it == items_.end())
            throw NotFoundError(id);
        if (it->second.holdings + units < 0)
            throw std::invalid_argument("holdings cannot become negative");
        it->second.holdings += units;
    }

    std::vector<const Resource *> Catalog::searchAuthor(
        const std::string &text) const
    {
        const std::string needle = lower(text);

        return where([&needle](const Resource &resource)
                     {
        if (const auto* book = dynamic_cast<const Book*>(&resource)) {
            for (const auto& author : book->authors()) {
                if (lower(author).find(needle) != std::string::npos) {
                    return true;
                }
            }
        }

        if (const auto* ebook = dynamic_cast<const EBook*>(&resource)) {
            for (const auto& author : ebook->authors()) {
                if (lower(author).find(needle) != std::string::npos) {
                    return true;
                }
            }
        }

        return false; });
    }

    std::vector<const Resource *> Catalog::searchIdentifier(
        const std::string &text) const
    {
        const std::string needle = lower(text);

        return where([&needle](const Resource &resource)
                     {
        if (const auto* book = dynamic_cast<const Book*>(&resource)) {
            if (lower(book->isbn()).find(needle) != std::string::npos) {
                return true;
            }
        }

        if (const auto* ebook = dynamic_cast<const EBook*>(&resource)) {
            if (lower(ebook->isbn()).find(needle) != std::string::npos) {
                return true;
            }
        }

        if (const auto* journal = dynamic_cast<const Journal*>(&resource)) {
            if (lower(journal->issn()).find(needle) != std::string::npos) {
                return true;
            }
        }

        return false; });
    }

    std::vector<const Resource *> Catalog::searchYearRange(
        int minYear, int maxYear) const
    {
        if (minYear > maxYear)
        {
            throw std::invalid_argument("minimum year cannot exceed maximum year");
        }

        return where([minYear, maxYear](const Resource &resource)
                     { return resource.year() >= minYear && resource.year() <= maxYear; });
    }

    std::vector<const Resource *> Catalog::where(
        const std::function<bool(const Resource &)> &pred) const
    {
        std::vector<const Resource *> out;

        for (const auto &[id, entry] : items_)
        {
            if (pred(*entry.resource))
            {
                out.push_back(entry.resource.get());
            }
        }

        return out;
    }

    std::vector<const Resource *> Catalog::all() const
    {
        return where([](const Resource &)
                     { return true; });
    }

    std::vector<const Resource *> Catalog::byCategory(ResourceCategory c) const
    {
        return where([c](const Resource &r)
                     { return r.category() == c; });
    }

    std::vector<const Resource *> Catalog::searchTitle(const std::string &text) const
    {
        const std::string needle = lower(text);
        return where([&needle](const Resource &r)
                     { return lower(r.title()).find(needle) != std::string::npos; });
    }

    void Catalog::borrowCopy(const std::string &resourceId,
                             const std::string &patronId)
    {
        const Resource &resource = get(resourceId);

        if (resource.isDigital())
        {
            throw std::invalid_argument("Cannot borrow a digital resource as a print copy");
        }

        if (patronId.empty())
        {
            throw std::invalid_argument("Patron ID cannot be empty");
        }

        auto &borrowers = borrowedByResource_[resourceId];

        if (std::find(borrowers.begin(), borrowers.end(), patronId) != borrowers.end())
        {
            throw std::logic_error("Patron has already borrowed this resource");
        }

        if (static_cast<int>(borrowers.size()) >= holdings(resourceId))
        {
            throw std::runtime_error("No print copies available");
        }

        borrowers.push_back(patronId);
    }

    void Catalog::returnCopy(const std::string &resourceId,
                             const std::string &patronId)
    {
        const Resource &resource = get(resourceId);

        if (resource.isDigital())
        {
            throw std::invalid_argument("Digital resources use sessions, not print returns");
        }

        auto it = borrowedByResource_.find(resourceId);

        if (it == borrowedByResource_.end())
        {
            throw std::logic_error("No active borrowing found");
        }

        auto &borrowers = it->second;
        auto patron = std::find(borrowers.begin(), borrowers.end(), patronId);

        if (patron == borrowers.end())
        {
            throw std::logic_error("Patron has not borrowed this resource");
        }

        borrowers.erase(patron);
    }

    void Catalog::openSession(const std::string &resourceId,
                              const std::string &patronId)
    {
        const Resource &resource = get(resourceId);

        if (!resource.isDigital())
        {
            throw std::invalid_argument("Sessions are only for digital resources");
        }

        if (patronId.empty())
        {
            throw std::invalid_argument("Patron ID cannot be empty");
        }

        auto &sessions = sessionsByResource_[resourceId];

        if (std::find(sessions.begin(), sessions.end(), patronId) != sessions.end())
        {
            throw std::logic_error("Patron already has an active session");
        }

        if (static_cast<int>(sessions.size()) >= holdings(resourceId))
        {
            throw std::runtime_error("No electronic sessions available");
        }

        sessions.push_back(patronId);
    }

    void Catalog::closeSession(const std::string &resourceId,
                               const std::string &patronId)
    {
        const Resource &resource = get(resourceId);

        if (!resource.isDigital())
        {
            throw std::invalid_argument("Only digital resources have sessions");
        }

        auto it = sessionsByResource_.find(resourceId);

        if (it == sessionsByResource_.end())
        {
            throw std::logic_error("No active session found");
        }

        auto &sessions = it->second;
        auto patron = std::find(sessions.begin(), sessions.end(), patronId);

        if (patron == sessions.end())
        {
            throw std::logic_error("Patron has no active session");
        }

        sessions.erase(patron);
    }

} // namespace bookmgmt
