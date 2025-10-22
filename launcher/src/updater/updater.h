#pragma once

#include <string>
#include <optional>

// Update Status
enum class UpdateStatus {
    NOT_STARTED,
    CHECKING,
    UP_TO_DATE,
    DOWNLOADING,
    RESTART_NEEDED,
    UNKNOWN_ERROR
};

// Updater
struct Updater {
    // Instance
    static Updater *instance;

    // Constructor
    Updater();
    virtual ~Updater() = default;

    // Methods
    [[nodiscard]] std::string get_status() const;
    void log_status(bool is_ui) const;
    [[nodiscard]] bool can_start() const;
    void start();
    void run();

protected:
    // Implementation
    virtual std::optional<std::string> check() = 0;
    virtual bool download(const std::string &version) = 0;
    virtual void restart() = 0;

private:
    // Properties
    UpdateStatus status = UpdateStatus::NOT_STARTED;
};