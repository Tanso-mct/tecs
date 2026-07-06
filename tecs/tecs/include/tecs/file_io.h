#pragma once

#include <memory>
#include <string_view>

namespace tecs
{

/**
 * @brief Reads the contents of a file into a buffer
 * @param file_path : The path to the file to be read
 * @param size : An output parameter that will hold the size of the buffer
 * @return std::unique_ptr<uint8_t[]> : Buffer containing the file contents, or nullptr if the file could not be read
 */
std::unique_ptr<uint8_t[]> ReadFileToBuffer(std::string_view file_path, fpos_t& size);

/**
 * @brief Writes a buffer to a file
 * @param file_path : The path to the file to be written
 * @param buffer : The buffer containing the data to be written
 * @param size : The size of the buffer
 * @return bool : True if the file was successfully written, false otherwise
 */
bool WriteBufferToFile(std::string_view file_path, const uint8_t* buffer, uint32_t size);

} // namespace tecs