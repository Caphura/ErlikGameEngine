#include "ResourceManager.h"
#include <system_error>
#include <iostream>

namespace Erlik {

    std::optional<std::filesystem::file_time_type>
        ResourceManager::safe_mtime(const std::string& path) {
        std::error_code ec;
        auto t = std::filesystem::last_write_time(path, ec);
        if (ec) return std::nullopt;
        return t;
    }

    void ResourceManager::track(const std::string& path, std::function<void()> reload) {
        // Eðer zaten izleniyorsa güncelle
        for (auto& e : m_entries) {
            if (e.path == path) {
                e.reload = std::move(reload);
                e.firstInit = true;
                if (auto mt = safe_mtime(path)) e.lastWrite = *mt;
                return;
            }
        }
        Entry e;
        e.path = path;
        e.reload = std::move(reload);
        e.firstInit = true;
        if (auto mt = safe_mtime(path)) e.lastWrite = *mt;
        m_entries.push_back(std::move(e));
    }

    void ResourceManager::untrack(const std::string& path) {
        m_entries.erase(std::remove_if(m_entries.begin(), m_entries.end(),
            [&](const Entry& e) { return e.path == path; }),
            m_entries.end());
    }

    void ResourceManager::check(bool force) {
        for (auto& e : m_entries) {
            bool changed = false;
            if (!force) {
                if (auto mt = safe_mtime(e.path)) {
                    if (e.firstInit) {
                        // ilk kontrol: lastWrite'ý güncelle ve tetikleME
                        e.lastWrite = *mt;
                    }
                    else if (*mt != e.lastWrite) {
                        e.lastWrite = *mt;
                        changed = true;
                    }
                }
            }
            if (force || changed) {
                try {
                    e.reload();
                }
                catch (...) {
                    // Reload sýrasýnda bir hata olursa sessiz geçmeyelim
                    std::cerr << "[hotreload] reload callback threw for: " << e.path << "\n";
                }
            }
            e.firstInit = false;
        }
    }

} // namespace Erlik
