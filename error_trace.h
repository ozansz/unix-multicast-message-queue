#ifndef __ERROR_TRACE_H
#define __ERROR_TRACE_H

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include "errors.h"

extern uint64_t ___times__get_or_create_shared_memory_called;

#define ENABLE_ERROR_TRACING    1
#define TRACE_APPEND(code) { if (ENABLE_ERROR_TRACING) _trace_append((errcode_t)(code), __func__, __FILE__, __LINE__); }
#define TRACE_APPEND_WITH_CTX(code, ctx) { if (ENABLE_ERROR_TRACING) _trace_append_with_ctx((errcode_t)(code), (ctx), __func__, __FILE__, __LINE__); }
#define TRACE_AND_RETURN(code) { TRACE_APPEND((code)) return (code); }
#define TRACE_WITH_CTX_AND_RETURN(code, ctx) { TRACE_APPEND_WITH_CTX((code), (ctx)) return (code); }
#define TRACE_WITH_ERRNO_AND_RETURN(code) { TRACE_APPEND_WITH_CTX((code), ERRNO_STR) return (code); }
#define PANIC_WITH_CTX { panic_with_context(__func__, __FILE__, __LINE__); }

class TraceNode {
public:
    TraceNode(errcode_t code, const char function[], const char file[], int line);
    TraceNode(TraceNode& other);
    void SetCtx(const char* ctx);
    // ~TraceNode();
    std::string Dump();

private:
    errcode_t code;
    std::string context;
    std::string function;
    std::string file;
    int line;
};

void panic_with_trace(bool);
void panic_with_context(const char*, const char*, int);
void _trace_append(errcode_t, const char*, const char*, int);
void _trace_append_with_ctx(errcode_t, const char*, const char*, const char*, int);

#endif