#include "pch.h"
#include "tecs/debug.h"

namespace tecs
{

JobLogExporter::JobLogExporter(JobTracker& job_tracker)
    : job_tracker_(job_tracker)
{
}

bool JobLogExporter::Analyze() const
{
    return true;
}

std::unique_ptr<uint8_t[]> JobLogExporter::Export(uint32_t& size) const
{
    size = 0;
    return std::make_unique<uint8_t[]>(size);
}

SystemLogExporter::SystemLogExporter(std::vector<std::unique_ptr<SystemView>>&& system_views)
    : system_views_(std::move(system_views))
{
}

bool SystemLogExporter::Analyze() const
{
    return true;
}

std::unique_ptr<uint8_t[]> SystemLogExporter::Export(uint32_t& size) const
{
    size = 0;
    return std::make_unique<uint8_t[]>(size);
}

EntityLogExporter::EntityLogExporter(Registry& registry)
    : registry_(registry)
{
}

bool EntityLogExporter::Analyze() const
{
    return true;
}

std::unique_ptr<uint8_t[]> EntityLogExporter::Export(uint32_t& size) const
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