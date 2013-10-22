project = Project.new("KoreVideo")

project:addDefine("KOREVIDEO")

project:addIncludeDir("Sources");
project:addIncludeDir("ogg/include")
project:addIncludeDir("theora/include")
project:addIncludeDir("vorbis/include")

project:addFile("Sources/**");
project:addFile("ogg/src/*.c");
project:addFile("theora/lib/*.c");
project:addFile("vorbis/lib/*.c");

--project:addExclude("theora/lib/encode.c")
--project:addExclude("theora/lib/encapiwrapper.c")
project:addExclude("theora/lib/encoder_disabled.c")
project:addExclude("vorbis/lib/misc.c")
project:addExclude("vorbis/lib/psytune.c")
project:addExclude("vorbis/lib/tone.c")
