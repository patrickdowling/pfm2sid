// Copyright 2023 Patrick Dowling
//
// Author: Patrick Dowling (pld@gurkenkiste.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// See http://creativecommons.org/licenses/MIT/ for more information.
//
// -----------------------------------------------------------------------------
// syscall stubs to avoid linker complaining:
// warning: _close is not implemented and will always fail et al.
#include <sys/stat.h>

int _close(int file)
{
  (void)file;
  return -1;
}
int _lseek(int file, int amount, int dir)
{
  (void)file;
  (void)amount;
  (void)dir;
  return -1;
}
int _read(int file, char *data, int len)
{
  (void)file;
  (void)data;
  (void)len;
  return -1;
}
int _write(int file, char *data, int len)
{
  (void)file;
  (void)data;
  (void)len;
  return -1;
}

int _getpid(void)
{
  return 1;
}

int _fstat(int file, struct stat *st)
{
  (void)file;
  st->st_mode = S_IFCHR;
  return 0;
}

int _isatty(int file)
{
  (void)file;
  return 1;
}

int _kill(int pid, int sig)
{
  (void)pid;
  (void)sig;
  // errno = EINVAL;
  return -1;
}

int __errno;
