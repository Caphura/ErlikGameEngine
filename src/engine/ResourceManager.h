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
        // Bir dosyay� izle ve de�i�ti�inde 'reload' callback'ini �al��t�r
        void track(const std::string& path, std::function<void()> reload);

        // �zlemeyi b�rak (opsiyonel)
        void untrack(const std::string& path);

        // De�i�iklikleri kontrol et. force=true ise callback'i hemen �al��t�r�r
        void check(bool force = false);

    private:
        struct Entry {
            std::string path;
            std::filesystem::file_time_type lastWrite{};
            std::function<void()> reload;
            bool firstInit = true; // ilk track'te force ile tetiklemeyi kolayla�t�r�r
        };
        std::vector<Entry> m_entries;

        static std::optional<std::filesystem::file_time_type> safe_mtime(const std::string& path);
    };

} // namespace Erlik
