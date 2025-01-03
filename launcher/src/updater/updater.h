#pragma once

#include <string>

// Update Status
enum UpdateStatus {
    NOT_STARTED,
    CHECKING,
    UP_TO_DATE,
    DOWNLOADING,
    RESTART_NEEDED
};

// Updater
struct Updater {
    // Instance
    static Updater *instance;
    // Constructor
    Updater();
    virtual ~Updater() = default;
    // Implementation
    virtual void update() = 0;
    virtual void restart() = 0;
    // Methods
    [[nodiscard]] std::string get_status() const;
    [[nodiscard]] bool can_start() const;
    void start();
    // Properties
    UpdateStatus status = NOT_STARTED;
};