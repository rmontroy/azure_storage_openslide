#pragma once

#include <azure/storage/blobs.hpp>
#include <tiffio.h>
#include "cache.h"

using namespace Azure::Storage::Blobs;

class Context {
    public:
        Context():
        _request_offset(0) {}

        static Context* Open(std::string url);
        tsize_t Read(tdata_t buffer, tsize_t size);
        toff_t Seek(toff_t offset, int whence);
        toff_t Size() { return _object_size; }

    private:
        std::unique_ptr<BlobClient> _blobClient;
        std::string _blobUrl;
        tsize_t _object_size;
        tsize_t _request_offset;
        toff_t _last_fetched_offset = std::numeric_limits<uint64_t>::max();
        tsize_t _num_ranges_to_fetch = 1;

        const tsize_t _max_range_size = 16384;
        const int _max_regions = 1000;
        using RangeCache = Cache<toff_t,std::shared_ptr<std::vector<uint8_t>>>;
        std::unique_ptr<RangeCache> _cache = 
            std::unique_ptr<RangeCache>(new RangeCache(_max_range_size * 1000));
        std::shared_ptr<std::vector<uint8_t>> GetCachedRange(toff_t);
        void PutCachedRange(toff_t, std::vector<uint8_t>);
        std::vector<uint8_t> FetchRange(toff_t range_offset, int num_ranges);
};
