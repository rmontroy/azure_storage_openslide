#include "remotetiff.h"
#include "context.h"

#define SUCCESS 0;
#define FAILURE -1;

extern "C" {

    thandle_t remotetiff_open(const char * filename)
    {
        Context *ctx = Context::Open(filename);
        return static_cast<void*>(ctx);
    }
    tsize_t remotetiff_read(thandle_t userdata, tdata_t buffer, tsize_t size)
    {
        if (userdata == NULL) return FAILURE;
        Context *ctx = static_cast<Context*>(userdata);
        return ctx->Read(buffer, size);
    }
    toff_t remotetiff_seek(thandle_t userdata, toff_t offset, int whence)
    {
        if (userdata == NULL) return FAILURE;
        Context *ctx = static_cast<Context*>(userdata);
        return ctx->Seek(offset, whence);
    }
    toff_t remotetiff_size(thandle_t userdata)
    {
        if (userdata == NULL) return FAILURE;
        Context *ctx = static_cast<Context*>(userdata);
        return ctx->Size();
    }
    int remotetiff_close(thandle_t userdata)
    {
        if (userdata == NULL) return FAILURE;
        Context *ctx = static_cast<Context*>(userdata);
        delete ctx;
        return SUCCESS;
    }
}
