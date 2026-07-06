#include "pch.h"
#include "tecs/debug.h"

namespace tecs
{

JobLogExporter::JobLogExporter(JobTracker& job_tracker)
    : job_tracker_(job_tracker)
{
}

bool JobLogExporter::Analyze()
{
    // Get the thread information from the JobTracker
    std::vector<JobExecutionInfo> thread_infos = job_tracker_.GetThreadInfos();

    for (JobExecutionInfo& thread_info : thread_infos)
    {
        uint32_t thread_id = thread_info.GetWorkerThreadID();

        // Store the job execution information in the job_history_ map
        job_history_[thread_id].emplace_back(std::move(thread_info));
    }
    
    analyzed_frame_count_++; // Increment the analyzed frame count

    return true;
}

void JobLogExporter::Clear()
{
    job_history_.clear(); // Clear the job history map
    analyzed_frame_count_ = 0; // Reset the analyzed frame count
}

std::unique_ptr<uint8_t[]> JobLogExporter::Export(uint32_t& size)
{
    // Calculate the total amount of job history
    uint32_t total_history_count = job_tracker_.GetWorkerThreadCount() * analyzed_frame_count_;

    std::string log_csv;

    // Create the column headers for the CSV
    for (uint32_t frame = 0; frame < analyzed_frame_count_; ++frame)
        log_csv += "Frame " + std::to_string(frame) + ",";
    log_csv += "\n";

    for (uint32_t thread_id = 0; thread_id < job_tracker_.GetWorkerThreadCount(); ++thread_id)
    {
        log_csv += "Thread " + std::to_string(thread_id) + ",";
        for (uint32_t frame = 0; frame < analyzed_frame_count_; ++frame)
        {
            const std::vector<JobExecutionInfo>& thread_infos = job_history_[thread_id];
            const JobExecutionInfo& thread_info = thread_infos[frame];
            log_csv += thread_info.GetJobType().GetName().data();
            log_csv += " (ID: " + std::to_string(thread_info.GetJobType().GetID()) + ")";
            log_csv += ",";
        }
        log_csv += "\n";
    }

    // Convert the CSV string to a byte array
    size = static_cast<uint32_t>(log_csv.size());
    std::unique_ptr<uint8_t[]> log_data = std::make_unique<uint8_t[]>(size);
    std::memcpy(log_data.get(), log_csv.data(), size);

    return log_data;
}

SystemLogExporter::SystemLogExporter(std::vector<std::unique_ptr<SystemView>>&& system_views)
    : system_views_(std::move(system_views))
{
}

bool SystemLogExporter::Analyze()
{
    return true;
}

void SystemLogExporter::Clear()
{
}

std::unique_ptr<uint8_t[]> SystemLogExporter::Export(uint32_t& size)
{
    size = 0;
    return std::make_unique<uint8_t[]>(size);
}

EntityLogExporter::EntityLogExporter(Registry& registry)
    : registry_(registry)
{
}

bool EntityLogExporter::Analyze()
{
    return true;
}

void EntityLogExporter::Clear()
{
}

std::unique_ptr<uint8_t[]> EntityLogExporter::Export(uint32_t& size)
{
    size = 0;
    return std::make_unique<uint8_t[]>(size);
}

Debug::Debug(JobScheduler& job_scheduler)
    : job_scheduler_(job_scheduler)
{
}

void Debug::AddLogExporter(uint32_t exporter_id, std::unique_ptr<LogExporter> exporter)
{
    assert(exporter != nullptr && "LogExporter pointer cannot be null");
    assert(log_exporters_.find(exporter_id) == log_exporters_.end() && "LogExporter with the same ID already exists");
    log_exporters_.emplace(exporter_id, std::move(exporter));
}

bool Debug::ExportLog(uint32_t exporter_id, std::string_view output_file) const
{
    return true;
}

DebugLog::DebugLog(Debug& debug)
    : debug_(debug)
{
}

bool DebugLog::ExportLog(uint32_t exporter_id, std::string_view output_file) const
{
    return debug_.ExportLog(exporter_id, output_file);
}

} // namespace tecs