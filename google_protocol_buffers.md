# Introduction #

Google's protocol buffers is an data interchange protocol between different programming languages.


# Details #

A _proto_-file describes the data which can be interchanged. Google proviedes a compiler, which is able to generates code in your programming language to read/write/modify the data in an object oriented way. ReconstructMeQt uses some messages to interact with the ReconstructMeSDK. These messages are available in the _proto_ directory in the source code repository. Since CMake 2.8.5, Google protocol buffers are supported. Thus, the relevant code can be generated in the CMake process, wich makes the usage of Google protocol buffers even more comfortable.