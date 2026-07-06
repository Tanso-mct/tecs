#pragma once

// STL
#include <string>
#include <string_view>
#include <memory>

// TECS
#include "tecs/job.h"
#include "tecs/system.h"
#include "tecs/registry.h"

namespace tecs
{

class LogExporter
{
public:
    virtual ~LogExporter() = default;

    /**
     * @brief : Analyzes the log data and returns true if the analysis is successful, false otherwise
     * @return bool : Returns true if the log analysis is successful, false otherwise
     */
    virtual bool Analyze() = 0;

    /**
     * @brief : Clears the log data, resetting any internal state
     */
    virtual void Clear() = 0;

    /**
     * @brief : Exports the log data to a byte array
     * @param size : Reference to a variable that will hold the size of the exported log data
     * @return std::unique_ptr<uint8_t[]> : Returns a unique pointer to a byte array containing the exported log data
     */
    virtual std::unique_ptr<uint8_t[]> Export(uint32_t& size) = 0;

};

class JobLogExporter
    : public LogExporter
{
public:
    /**
     * @brief : Constructor for JobLogExporter
     * @param job_tracker : Reference to a JobTracker object
     */
    JobLogExporter(JobTracker& job_tracker);

    /**
     * @brief : Analyzes the job log data and returns true if the analysis is successful, false otherwise
     * @return bool : Returns true if the job log analysis is successful, false otherwise
     */
    bool Analyze() override;

    /**
     * @brief : Clears the job log data, resetting any internal state
     */
    void Clear() override;

    /**
     * @brief : Exports the job log data to a byte array
     * @param size : Reference to a variable that will hold the size of the exported job log data
     * @return std::unique_ptr<uint8_t[]> : Returns a unique pointer to a byte array containing the exported job log data
     */
    std::unique_ptr<uint8_t[]> Export(uint32_t& size) override;

private:
    // Reference to the JobTracker object used for exporting job logs
    JobTracker& job_tracker_;

    // A map that associates frame numbers with their corresponding job execution information for historical tracking
    std::unordered_map<uint32_t, std::vector<JobExecutionInfo>> job_history_;

    // Counter for the number of analyzed frames
    uint32_t analyzed_frame_count_ = 0;

};

class SystemLogExporter
    : public LogExporter
{
public:
    /**
     * @brief : Constructor for SystemLogExporter
     * @param system_views : A vector of unique pointers to SystemView objects
     */
    SystemLogExporter(std::vector<std::unique_ptr<SystemView>>&& system_views);

    /**
     * @brief : Analyzes the system log data and returns true if the analysis is successful, false otherwise
     * @return bool : Returns true if the system log analysis is successful, false otherwise
     */
    bool Analyze() override;

    /**
     * @brief : Clears the job log data, resetting any internal state
     */
    void Clear() override;

    /**
     * @brief : Exports the system log data to a byte array
     * @param size : Reference to a variable that will hold the size of the exported system log data
     * @return std::unique_ptr<uint8_t[]> : Returns a unique pointer to a byte array containing the exported system log data
     */
    std::unique_ptr<uint8_t[]> Export(uint32_t& size) override;

private:
    // A vector of unique pointers to SystemView objects used for exporting system logs
    std::vector<std::unique_ptr<SystemView>> system_views_;

    // A map that associates frame numbers with their corresponding system task information for historical tracking
    std::unordered_map<uint32_t, std::vector<TaskInfo>> system_task_history_;

    // Counter for the number of analyzed frames
    uint32_t analyzed_frame_count_ = 0;
};

class EntityLogExporter
    : public LogExporter
{
public:
    /**
     * @brief : Constructor for EntityLogExporter
     * @param registry : Reference to a Registry object
     */
    EntityLogExporter(Registry& registry);

    /**
     * @brief : Analyzes the entity log data and returns true if the analysis is successful, false otherwise
     * @return bool : Returns true if the entity log analysis is successful, false otherwise
     */
    bool Analyze() override;

    /**
     * @brief : Clears the job log data, resetting any internal state
     */
    void Clear() override;

    /**
     * @brief : Exports the entity log data to a byte array
     * @param size : Reference to a variable that will hold the size of the exported entity log data
     * @return std::unique_ptr<uint8_t[]> : Returns a unique pointer to a byte array containing the exported entity log data
     */
    std::unique_ptr<uint8_t[]> Export(uint32_t& size) override;

private:
    // Reference to the Registry object used for exporting entity logs
    Registry& registry_;

};

class Debug
{
public:
    /**
     * @brief : Constructor for Debug class
     * @param job_scheduler : Reference to a JobScheduler object
     */
    Debug(JobScheduler& job_scheduler);

    ~Debug() = default;

    /**
     * @brief : Adds a log exporter to the Debug class
     * @param exporter_id : The ID of the log exporter
     * @param exporter : A unique pointer to the LogExporter instance to be added
     */
    void AddLogExporter(uint32_t exporter_id, std::unique_ptr<LogExporter> exporter);

    /**
     * @brief : Exports the log data using the specified log exporter
     * @param exporter_id : The ID of the log exporter to be used for exporting
     * @param output_file : The path to the output file where the exported log data will be saved
     * @return bool : Returns true if the export was successful, false otherwise
     */
    bool ExportLog(uint32_t exporter_id, std::string_view output_file) const;

private:
    // Reference to the JobScheduler object used for managing jobs
    JobScheduler& job_scheduler_;

    // A map that associates log exporter IDs with their corresponding LogExporter instances
    std::unordered_map<uint32_t, std::unique_ptr<LogExporter>> log_exporters_;

};

class DebugLog
{
public:
    /**
     * @brief : Constructor for DebugLog class
     * @param debug : Reference to a Debug object
     */
    DebugLog(Debug& debug);

    /**
     * @brief : Exports the log data using the specified log exporter
     * @param exporter_id : The ID of the log exporter to be used for exporting
     * @param output_file : The path to the output file where the exported log data will be saved
     * @return bool : Returns true if the export was successful, false otherwise
     */
    bool ExportLog(uint32_t exporter_id, std::string_view output_file) const;

private:
    // Reference to the Debug object used for exporting logs
    Debug& debug_;

};

} // namespace tecs