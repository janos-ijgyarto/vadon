# Using EXCLUDE_FROM_ALL so we don't install any headers
CPMAddPackage(
  NAME concurrentqueue
  URL https://github.com/cameron314/concurrentqueue/archive/refs/tags/v1.0.4.zip
  VERSION 1.0.4
  EXCLUDE_FROM_ALL YES
)