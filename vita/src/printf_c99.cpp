// VitaSDK's newlib is built without C99 printf formats (_WANT_IO_C99_FORMATS
// is off in newlib.h): it does not know the z, t, j and hh length modifiers,
// skips them, and takes the following arguments out of step. The PC port
// prints sizes with %zu, so "%zu ... %s" crashed in strlen on a number.
//
// The printf family is wrapped (-Wl,--wrap=..., CMakeLists.txt) to rewrite
// those modifiers before newlib sees the format: size_t and ptrdiff_t are
// 32-bit here (z, t: dropped), intmax_t is 64-bit (j: ll), and hh becomes h
// (the argument arrives promoted to int either way).
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern "C" {
int __real_vfprintf(FILE* f, const char* fmt, va_list ap);
int __real_vsnprintf(char* buf, size_t size, const char* fmt, va_list ap);
int __real_vsprintf(char* buf, const char* fmt, va_list ap);
}

namespace {

// Holds the rewritten format: on the stack when it fits, else on the heap.
class Format {
public:
	explicit Format(const char* fmt) : text_(fmt)
	{
		if (!fmt || !needs_rewrite(fmt))
			return;
		const size_t n = strlen(fmt) * 2 + 1; // j becomes ll: at most twice as long
		char* out = n <= sizeof local_ ? local_ : (heap_ = (char*)malloc(n));
		if (!out)
			return;
		rewrite(fmt, out);
		text_ = out;
	}
	~Format() { free(heap_); }
	const char* get() const { return text_; }

private:
	static bool needs_rewrite(const char* f)
	{
		for (; (f = strchr(f, '%')); f++) {
			f++;
			while (*f && strchr("-+ #0123456789.*'", *f))
				f++;
			if (*f == 'z' || *f == 't' || *f == 'j' || (f[0] == 'h' && f[1] == 'h'))
				return true;
			if (!*f)
				break;
		}
		return false;
	}
	static void rewrite(const char* f, char* out)
	{
		while (*f) {
			if (*f != '%') {
				*out++ = *f++;
				continue;
			}
			*out++ = *f++;
			if (*f == '%') { // "%%"
				*out++ = *f++;
				continue;
			}
			while (*f && strchr("-+ #0123456789.*'", *f))
				*out++ = *f++;
			if (*f == 'z' || *f == 't') {
				f++;
			} else if (*f == 'j') {
				*out++ = 'l';
				*out++ = 'l';
				f++;
			} else if (f[0] == 'h' && f[1] == 'h') {
				*out++ = 'h';
				f += 2;
			}
		}
		*out = 0;
	}

	const char* text_;
	char local_[512];
	char* heap_ = nullptr;
};

} // namespace

extern "C" {

int __wrap_vfprintf(FILE* f, const char* fmt, va_list ap)
{
	Format format(fmt);
	return __real_vfprintf(f, format.get(), ap);
}

int __wrap_vprintf(const char* fmt, va_list ap)
{
	Format format(fmt);
	return __real_vfprintf(stdout, format.get(), ap);
}

int __wrap_vsnprintf(char* buf, size_t size, const char* fmt, va_list ap)
{
	Format format(fmt);
	return __real_vsnprintf(buf, size, format.get(), ap);
}

int __wrap_vsprintf(char* buf, const char* fmt, va_list ap)
{
	Format format(fmt);
	return __real_vsprintf(buf, format.get(), ap);
}

int __wrap_fprintf(FILE* f, const char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	Format format(fmt);
	const int r = __real_vfprintf(f, format.get(), ap);
	va_end(ap);
	return r;
}

int __wrap_printf(const char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	Format format(fmt);
	const int r = __real_vfprintf(stdout, format.get(), ap);
	va_end(ap);
	return r;
}

int __wrap_snprintf(char* buf, size_t size, const char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	Format format(fmt);
	const int r = __real_vsnprintf(buf, size, format.get(), ap);
	va_end(ap);
	return r;
}

int __wrap_sprintf(char* buf, const char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	Format format(fmt);
	const int r = __real_vsprintf(buf, format.get(), ap);
	va_end(ap);
	return r;
}

} // extern "C"
