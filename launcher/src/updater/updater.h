#pragma once

#include <string>
#include <optional>
#include <functional>

// Update Status
enum class UpdateStatus {
    NOT_STARTED,
    CHECKING,
    UP_TO_DATE,
    UPDATE_AVAILABLE,
    DOWNLOADING,
    RESTART_NEEDED,
    UNKNOWN_ERROR
};

// Updater
struct Updater final {
    // Constructor
    Updater();
    ~Updater();

    // Get Status
    [[nodiscard]] std::string get_status() const;
    [[nodiscard]] bool can_launch_safely() const;

    // Asynchronous Mode
    [[nodiscard]] bool can_start() const;
    void tick();
    void start();

    // Synchronous Mode
    void run();

private:
    // Implementation
    void check();
    typedef std::function<bool(const std::string &version)> Downloader;
    [[nodiscard]] std::optional<Downloader> get_downloader(bool is_ui) const;
    void download(bool is_ui);
    static void restart();

    // Logging
    void log_error(bool is_sync) const;

    // Thread
    pthread_t thread;
    volatile bool is_running;
    volatile bool is_done;
    void wait();
    static void *thread_func(void *user_data);
    void start_thread(const std::function<void(Updater *)> &func);

    // Properties
    UpdateStatus status;
    std::string latest_version;
    bool has_started_download;
};

// Helper Methods
std::optional<std::string> run_wget(const std::string &dest, const std::string &url);
void add_version_to_url(std::string &url, const std::string &version);