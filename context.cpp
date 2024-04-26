#include <iostream>
#include "context.h"
#include <azure/identity/default_azure_credential.hpp>

using namespace Azure::Identity;

std::shared_ptr<std::vector<uint8_t>> Context::GetCachedRange(toff_t offset)
{
    std::shared_ptr<std::vector<uint8_t>> out;
    return _cache->tryGet(offset, out) ?
        out :
        nullptr;
}

void Context::PutCachedRange(toff_t offset, std::vector<uint8_t> pData)
{
    std::shared_ptr<std::vector<uint8_t>> value(new std::vector<uint8_t>(pData));
    _cache->put(offset, value);
}

Context* Context::Open(std::string url) {
    auto ctx = new Context;
    ctx->_blobUrl = url;

    auto credential = std::make_shared<DefaultAzureCredential>();
    ctx->_blobClient = std::unique_ptr<BlockBlobClient>(new BlockBlobClient(url, credential));

    auto response = ctx->_blobClient->GetProperties();
    auto properties = response.Value;
    ctx->_object_size = properties.BlobSize;
    return ctx;
}

tsize_t Context::Read(tdata_t request_buffer, tsize_t buffer_size)
{
    if (buffer_size == 0) return 0;

    void* working_buffer = request_buffer;
    tsize_t remaining_bytes = buffer_size;
    tsize_t iter_offset = _request_offset;
    while (remaining_bytes > 0)
    {
        if(iter_offset >= _object_size)
        {
            break;
        }

        // align fetch offset
        const toff_t fetch_offset = (iter_offset / _max_range_size) * _max_range_size;
        std::vector<uint8_t> range_data;
        std::shared_ptr<std::vector<uint8_t>> cached_range = GetCachedRange(fetch_offset);
        if (cached_range != nullptr)
        {
            range_data = *cached_range;
        }
        else
        {
            if( fetch_offset == _last_fetched_offset )
            {
                // In case of consecutive reads (of small size), we use a
                // heuristic that we will read the file sequentially, so
                // we double the requested size to decrease the number of
                // client/server roundtrips.
                if( _num_ranges_to_fetch < 100 )
                    _num_ranges_to_fetch *= 2;
            }
            else
            {
                // Random reads. Cancel the above heuristics.
                _num_ranges_to_fetch = 1;
            }

            // Ensure that we will request at least the number of blocks
            // to satisfy the remaining buffer size to read.
            const toff_t end_offset =
                ((iter_offset + remaining_bytes + _max_range_size - 1) / _max_range_size) *
                _max_range_size;
            const int min_ranges_to_fetch =
                static_cast<int>((end_offset - fetch_offset) / _max_range_size);
            if( _num_ranges_to_fetch < min_ranges_to_fetch )
                _num_ranges_to_fetch = min_ranges_to_fetch;
            
            // Avoid reading already cached data.
            for( int i = 1; i < _num_ranges_to_fetch; i++ )
            {
                if( GetCachedRange(
                        fetch_offset + i * _max_range_size) != nullptr )
                {
                    _num_ranges_to_fetch = i;
                    break;
                }
            }

            if( _num_ranges_to_fetch > _max_regions )
                _num_ranges_to_fetch = _max_regions;

            range_data = FetchRange(fetch_offset, _num_ranges_to_fetch);
            if(range_data.empty()) return 0;
        }

        toff_t region_offset = iter_offset - fetch_offset;
        if (range_data.size() < region_offset)
        {
            break;
        }

        const int bytes_to_copy = static_cast<int>(
            std::min(static_cast<toff_t>(remaining_bytes),
                     range_data.size() - region_offset));
        memcpy(working_buffer,
               range_data.data() + region_offset,
               bytes_to_copy);
        working_buffer = static_cast<char *>(working_buffer) + bytes_to_copy;
        iter_offset += bytes_to_copy;
        remaining_bytes -= bytes_to_copy;
        if( range_data.size() < static_cast<size_t>(_max_range_size) &&
            remaining_bytes != 0 )
        {
            break;
        }
    }

    tsize_t bytes_read = iter_offset - _request_offset;
    _request_offset = iter_offset;
    return bytes_read;
}

std::vector<uint8_t> Context::FetchRange(toff_t start_offset, int num_ranges) {
    DownloadBlobToOptions downloadOptions;
    downloadOptions.Range = Azure::Core::Http::HttpRange(); // Have to specify a range, and the range
                                                            // cannot be larger than 4MiB
    downloadOptions.Range.Value().Offset = start_offset;
    downloadOptions.Range.Value().Length = num_ranges * _max_range_size;
    std::vector<uint8_t> buffer(num_ranges * _max_range_size);
    auto response = _blobClient->DownloadTo(buffer.data(), buffer.size(), downloadOptions);
    auto statusCode = response.RawResponse->GetStatusCode();
    if (statusCode != Azure::Core::Http::HttpStatusCode::Ok &&
        statusCode != Azure::Core::Http::HttpStatusCode::PartialContent)
    {
        std::cout << response.RawResponse->GetReasonPhrase() << std::endl;
        return std::vector<uint8_t>();
    }
    tsize_t fetch_size = response.Value.ContentRange.Length.Value();

    // Populate the range cache with chunks from this fetch
    _last_fetched_offset = start_offset + num_ranges * _max_range_size;
    toff_t range_offset = start_offset;
    auto iter = buffer.begin();
    while( fetch_size > 0 )
    {
        const size_t range_size = std::min(_max_range_size, fetch_size);
        auto range_data = std::vector<uint8_t>(iter, iter + range_size);
        PutCachedRange(range_offset, range_data);
        range_offset += range_size;
        iter += range_size;
        fetch_size -= range_size;
    }

    return buffer;
}

toff_t Context::Seek(toff_t offset, int whence) {
    switch (whence) {
        case SEEK_SET: {
            _request_offset = offset;
            break;
        }
        case SEEK_CUR: {
            _request_offset += offset;
            break;
        }
        case SEEK_END: {
            _request_offset = _object_size + offset;
            break;
        }
        default:
            return -1;
    }
    return _request_offset;
}
