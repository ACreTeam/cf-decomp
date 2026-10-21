///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "gsPlatform.h"
#include "gsPlatformSocket.h"
#include "gsPlatformUtil.h"
#include "gsMemory.h"

#include <rvl/os/osThread.h>

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// time functions

time_t gsiTimeInSec(time_t* timer) {
  time_t aSeconds = (time_t)OSTicksToSeconds(OSGetTime());
  if (timer != NULL)
    *timer = aSeconds;
  return aSeconds;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// string helpers

char* goastrdup(const char* src) {
  char* res;
  if (src == NULL)
    return NULL;
  res = (char*)gsimalloc(strlen(src) + 1);
  if (res != NULL)
    strcpy(res, src);
  return res;
}

char* _strlwr(char* string) {
  char* hold = string;
  while (*string) {
    *string = (char)tolower(*string);
    string++;
  }
  return hold;
}

void SocketStartUp() {}

void SocketShutDown() {}

gsi_time current_time() {
  OSTime aTickNow = OSGetTime();
  gsi_time aMilliseconds = (gsi_time)OSTicksToMilliseconds(aTickNow);
  return aMilliseconds;
}

void msleep(gsi_time msec) { OSSleepMilliseconds(msec); }

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// random numbers

static long randomnum = 1;

static long nextlongrand(long seed) {
  unsigned long lo, hi;

  lo = 16807 * (long)(seed & 0xFFFF);
  hi = 16807 * (long)((unsigned long)seed >> 16);
  lo += (hi & 0x7FFF) << 16;
  if (lo > 2147483647) {
    lo &= 2147483647;
    ++lo;
  }
  lo += hi >> 15;
  if (lo > 2147483647) {
    lo &= 2147483647;
    ++lo;
  }
  return (long)lo;
}

static long longrand() {
  randomnum = nextlongrand(randomnum);
  return randomnum;
}

void Util_RandSeed(unsigned long seed) {
  randomnum = seed ? (long)(seed & 0x7FFFFFFF) : 1;
}

int Util_RandInt(int low, int high) {
  unsigned int range = (unsigned int)(high - low);
  int num;

  if (range == 0)
    return low;

  num = (int)(longrand() % range);
  return num + low;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// base 64

static const char defaultEncoding[3] = "+/=";
static const char alternateEncoding[3] = "[]_";
static const char urlSafeEncodeing[3] = "-_=";

static void QuartToTrip(char* quart, char* trip, int inlen) {
  if (inlen >= 2)
    trip[0] = (char)((quart[0] << 2) | (quart[1] >> 4));
  if (inlen >= 3)
    trip[1] = (char)(((quart[1] & 0x0F) << 4) | (quart[2] >> 2));
  if (inlen >= 4)
    trip[2] = (char)(((quart[2] & 0x03) << 6) | quart[3]);
}

static void TripToQuart(const char* trip, char* quart, int inlen) {
  unsigned char triptemp[3];
  int i;

  for (i = 0; i < min(inlen, 3); i++)
    triptemp[i] = (unsigned char)trip[i];
  for (; i < 3; i++)
    triptemp[i] = 0;

  quart[0] = (char)(triptemp[0] >> 2);
  quart[1] = (char)(((triptemp[0] & 3) << 4) | (triptemp[1] >> 4));
  quart[2] = (char)(((triptemp[1] & 0x0F) << 2) | (triptemp[2] >> 6));
  quart[3] = (char)(triptemp[2] & 0x3F);
}

void B64Decode(const char* input, char* output, int inlen, int* outlen,
               int encodingType) {
  const char* encoding;
  int readpos = 0;
  int writepos = 0;
  const char* holdin;
  char block[4];

  switch (encodingType) {
  case 1:
    encoding = alternateEncoding;
    break;
  case 2:
    encoding = urlSafeEncodeing;
    break;
  default:
    encoding = defaultEncoding;
    break;
  }

  if (inlen <= 0) {
    if (outlen)
      *outlen = 0;
    output[0] = '\0';
    return;
  }

  holdin = input;
  while (readpos < inlen && encoding[2] != *holdin) {
    if (*holdin >= '0' && *holdin <= '9')
      block[readpos % 4] = (char)(*holdin + 4);
    else if (*holdin >= 'a' && *holdin <= 'z')
      block[readpos % 4] = (char)(*holdin - 71);
    else if (*holdin >= 'A' && *holdin <= 'Z')
      block[readpos % 4] = (char)(*holdin - 'A');
    else if (encoding[0] == *holdin)
      block[readpos % 4] = 62;
    else if (encoding[1] == *holdin)
      block[readpos % 4] = 63;
    else if (input[readpos] == encoding[2] || input[readpos] == '\0')
      break;
    else {
      if (outlen)
        *outlen = 0;
      output[0] = '\0';
      return;
    }

    if (readpos % 4 == 3) {
      QuartToTrip(block, output + writepos, 4);
      writepos += 3;
    }
    readpos++;
    holdin++;
  }

  if ((readpos != 0) && ((readpos % 4) != 0)) {
    memset(block + (readpos % 4), encoding[2], (unsigned int)4 - (readpos % 4));
    QuartToTrip(block, output + writepos, readpos % 4);
    if (readpos % 4 == 3)
      writepos += 2;
    else
      writepos += 1;
  }

  if (outlen)
    *outlen = writepos;
}

void B64Encode(const char* input, char* output, int inlen, int encodingType) {
  const char* encoding;
  char* holdout = output;
  char* lastchar;
  int todo = inlen;

  switch (encodingType) {
  case 1:
    encoding = alternateEncoding;
    break;
  case 2:
    encoding = urlSafeEncodeing;
    break;
  default:
    encoding = defaultEncoding;
    break;
  }

  while (todo > 0) {
    TripToQuart(input, output, todo);
    input += 3;
    todo -= 3;
    output += 4;
  }

  lastchar = output;
  if (inlen % 3 == 1)
    lastchar = output - 2;
  else if (inlen % 3 == 2)
    lastchar = output - 1;
  *output = '\0';

  while (output > holdout) {
    output--;
    if (output >= lastchar)
      *output = encoding[2];
    else if (*output <= 25)
      *output += 'A';
    else if (*output <= 51)
      *output += 71;
    else if (*output <= 61)
      *output -= 4;
    else if (*output == 62)
      *output = encoding[0];
    else if (*output == 63)
      *output = encoding[1];
  }
}
