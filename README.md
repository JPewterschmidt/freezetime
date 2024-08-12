# freezetime

FreezeTime or `libfreezetime` is a single file thread-safe hooks of GNU libc time related functions.
As the name suggests, this project is used to assist in testing your software by freezing the return value of the current time-related function.
It's support
- `time()`
- `gettimeofday()`
- `ftime()`
- `times()`
- `clock_gettime()`

The value of time will be initialzed by the first call to any function above.
The loggic is quite simple. You can look at the code yourself.
PR is welcomed.

## Usage

Compile by yourself
```
g++ -fPIC -shared -o libfreezetime.so ./freezetime.cc -ldl
```
or just download a piece from [release](https://github.com/JPewterschmidt/freezetime/releases) page.
