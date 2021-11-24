#include "error_trace.h"

std::vector<TraceNode*> __error_trace_glob;

TraceNode::TraceNode(errcode_t code, const char function[], const char file[], int line) {
    this->code = code;
    this->function = function;
    this->file = file;
    this->line = line;
    this->context = "";
}

void TraceNode::SetCtx(const char* ctx) {
    this->context = ctx;
}

TraceNode::TraceNode(TraceNode& other) {
    this->code = other.code;
    this->function = other.function;
    this->file = other.file;
    this->line = other.line;
}

std::string TraceNode::Dump() {
    std::ostringstream stream;
    stream << this->file << ":" << this->function << ":" << this->line << " Code: " << this->code << " (" << get_err_desc(this->code) << ")";

    if (this->context != "")
        stream << " [" << this->context << "]";

    return stream.str(); 
}

void panic_with_trace(bool throw_exc_on_return) {
    std::cout << "\n=========[ ERROR TRACE ]=========\n";

    for (std::vector<TraceNode*>::iterator tr = __error_trace_glob.begin(); tr < __error_trace_glob.end(); tr++) {
        std::cout << "\n>>> " << (*tr)->Dump();
    }

    std::cout << "\n\n";

    if (throw_exc_on_return)
        throw std::runtime_error("Trace");
}

void _trace_append(errcode_t code, const char* func, const char* file, int line) {
    TraceNode* tr = new TraceNode(code, func, file, line);
    __error_trace_glob.push_back(tr);
}

void _trace_append_with_ctx(errcode_t code, const char* context, const char* func, const char* file, int line) {
    TraceNode* tr = new TraceNode(code, func, file, line);
    tr->SetCtx(context);
    __error_trace_glob.push_back(tr);
}

void panic_with_context(const char* function, const char* file, int line) {
    panic_with_trace(false);

    std::cout << "==> Panicked from: " << file << ":" << function << ":" << line << "\n\n";

    throw std::runtime_error("Context");
}