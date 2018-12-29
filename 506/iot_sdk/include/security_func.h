
#ifndef __COMMON_DEFINE_HEADER__
#define __COMMON_DEFINE_HEADER__

#define iots_strlen(str)       strlen(str)
#define iots_fopen(path, mode) fopen(path, mode)
#define iots_fread(buf, size, count, stream) fread(buf, size, count, stream)
#define iots_read(fd, buf, size) read(fd, buf, size)
#define iots_fwrite(buf, size, count, stream) fwrite(buf, size, count, stream)
#ifdef ANDROID_OS
#define iots_open(path, flags, mode) open(path, flags, mode)
#else
#define iots_open(path, flags) open(path, flags)
#endif  //!ANDROID_OS

#ifdef WIN32
#define iots_memcpys(dst, ndst, src, cplen) memcpy_s(dst, ndst, src, cplen)
#define iots_memcpy(dst, src, cplen)        memcpy_s(dst, cplen, src, cplen)
#else
#define iots_memcpys(dst, ndst, src, cplen) memcpy(dst, src, (cplen < ndst)?cplen:ndst)
#define iots_memcpy(dst, src, cplen)        memcpy(dst, src, cplen)
#endif

#ifdef WIN32
#define iots_sprintfs(buf, bufsize, fmt, ...) \
                    sprintf_s(buf, bufsize, fmt, ##__VA_ARGS__)
#define iots_snprintf(buf, cnt, fmt, ...)     _snprintf_s(buf, cnt, cnt, fmt, ##__VA_ARGS__)
#else
#define iots_sprintfs(buf, bufsize, fmt, arg...)   sprintf(buf, fmt, ##arg)
#define iots_sprintf(buf, fmt, arg...)             sprintf(buf, fmt, ##arg)
#define iots_snprintf(buf, cnt, fmt, arg...)  snprintf(buf, cnt, fmt, ##arg)
#endif

#ifdef WIN32
#define iots_sscanf(buf, fmt, ...)  sscanf_s(buf, fmt, ##__VA_ARGS__)
#define iots_strcats(des, dsz, src) strcat_s(des, dsz, src)
#define iots_strtok(str, deli)      strtok(str, deli)
#else
#define iots_sscanf(buf, fmt, arg...)  sscanf(buf, fmt, ##arg)
#define iots_strcats(des, dsz, src)    strcat(des, src)
#define iots_strtok(str, deli)         strtok(str, deli)
#endif

#ifdef WIN32
#define iots_strcpys(des, dsz, src) strcpy_s(des, dsz, src)
#define iots_strcpy(des, src)       strcpy_s(des, (iots_strlen(src)+1), src)//not safe

#define iots_strncpys(des, dsz, src, cnt) strncpy_s(des, dsz, src, cnt)
#define iots_strncpy(des, src, cnt)       strncpy_s(des, (cnt+1), src, cnt)//not safe
#else
#define iots_strcpys(des, dsz, src) do {\
        UINT32 _slen = iots_strlen(src); \
        char   *_des = (des); \
        if((dsz) > _slen) { strcpy(_des, (src)); } \
        else { strncpy(_des, (src), ((dsz)-1));_des[((dsz)-1)]='\0';} \
    }while(0)
#define iots_strcpy(des, src)       strcpy(des, src)

#define iots_strncpys(des, dsz, src, cnt) strncpy(des, src, (dsz>cnt)?cnt:dsz)
#define iots_strncpy(des, src, cnt)       strncpy(des, src, cnt)
#endif

#define iots_strcat(des, src)          strcat(des, src)
#define iots_strncat(des, src, size)   strncat(des, src, size)

#endif	//!__COMMON_DEFINE_HEADER__
