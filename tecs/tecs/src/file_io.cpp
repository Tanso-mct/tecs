#include "pch.h"
#include "tecs/file_io.h"

namespace tecs
{

std::unique_ptr<uint8_t[]> ReadFileToBuffer(std::string_view file_path, fpos_t &size)
{
    FILE* fp = nullptr;
	errno_t error;

    // Open the file in binary mode
	error = fopen_s(&fp, file_path.data(), "rb");
	if (error != 0) return nullptr;

	// Seek to the end of the file to determine its size
	fseek(fp, 0L, SEEK_END);
	fgetpos(fp, &size);

	// Seek back to the beginning of the file
	fseek(fp, 0L, SEEK_SET);

    // Read the file contents into the buffer
	std::unique_ptr<uint8_t[]> buffer = std::make_unique<uint8_t[]>(size);
	fread(buffer.get(), 1, size, fp);
	fclose(fp);

	return buffer;
}

bool WriteBufferToFile(std::string_view file_path, const uint8_t *buffer, uint32_t size)
{
    FILE* fp = nullptr;
	errno_t error;

    // Open the file in binary mode for writing
	error = fopen_s(&fp, file_path.data(), "wb");
	if (error != 0) return false;

    // Write the buffer contents to the file
	fwrite(buffer, sizeof(uint8_t), size, fp);
	fclose(fp);

	return true;
}

} // namespace tecs