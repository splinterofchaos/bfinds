bfinds
======


bfinds is stupid. It doesn't know how to fallow links, match by regex, know what a `stat_t` or `ino_t` is, or do any error reporting. It thinks maintaining a list of visited directories is a waste of time. Because of its stupidity, bfinds much faster than Gnu's find.

On my computer, today, with my current directory tree, running 'find / -name foo' will take 3.9 to 4 seconds to complete. 'bfinds foo /' will take between 1.7 and 1.9 seconds.

Faster does not mean better! bfinds is an experiment and lacks most of find's features, ones that *some* people probably actually use. However, 99% of the time when I need to find a file, there aren't any links, I don't want "permission denied" errors filling the screen, and none of find's numerous options seem necessary. bfinds is not a find replacement, but it does the job.


TODO
-----
 * Profiling reveals that bfinds spends half its time in `path_append()`, which is called on every file as it's appended to the "search later" list. The `dirent` structure gives us each entry's `ino`, so if we could get a file descriptor from that and call `fdopendir()`, we wouldn't need to call `path_append()` at all.
 * I wonder if performance could be increased by having one thread read the directories while another checked for matches (and called `path_append()`).
 * More options, as long as they don't slow it down.
