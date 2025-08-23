#pragma once
#include <string>
#include <vector>
#include <functional>
#include <filesystem>
#include <optional>
#include <chrono>

namespace Erlik {

    class ResourceManager {
    public:
        // Bir dosyayý izle ve deðiþtiðinde 'reload' callback'ini çalýþtýr
        void track(const std::string& path, std::function<void()> reload);

        // Ýzlemeyi býrak (opsiyonel)
        void untrack(const std::string& path);

        // Deðiþiklikleri kontrol et. force=true ise callback'i hemen çalýþtýrýr
        void check(bool force = false);

    private:
        struct Entry {
            std::string path;
            std::filesystem::file_time_type lastWrite{};
            std::function<void()> reload;
            bool firstInit = true; // ilk track'te force ile tetiklemeyi kolaylaþtýrýr
        };
        std::vector<Entry> m_entries;

        static std::optional<std::filesystem::file_time_type> safe_mtime(const std::string& path);
    };

} // namespace Erlik
