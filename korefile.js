var project = new Project("KoreVideo");

project.addDefine("KOREVIDEO");

project.addIncludeDir("Sources");
project.addIncludeDir("ogg/include");
project.addIncludeDir("theora/include");
project.addIncludeDir("vorbis/include");
project.addIncludeDir("vorbis/lib");

project.addFile("Sources/**");
project.addFile("ogg/src/*.c");
project.addFile("theora/lib/*.c");
project.addFile("vorbis/lib/*.c");

//project:addExclude("theora/lib/encode.c");
//project:addExclude("theora/lib/encapiwrapper.c");
project.addExclude("theora/lib/encoder_disabled.c");
project.addExclude("vorbis/lib/barkmel.c");
project.addExclude("vorbis/lib/misc.c");
project.addExclude("vorbis/lib/psytune.c");
project.addExclude("vorbis/lib/tone.c");

project.addFile('libtheoraplayer/**');
project.addExclude('libtheoraplayer/src/YUV/libyuv/src/row_x86.asm');
project.addExclude('libtheoraplayer/src/YUV/libyuv/src/x86inc.asm');

project.addIncludeDir('libtheoraplayer/src/YUV');
project.addIncludeDir('libtheoraplayer/src/YUV/libyuv/include');
project.addIncludeDir('libtheoraplayer/include/theoraplayer');
project.addIncludeDir('libtheoraplayer/src/Theora');

project.addDefine('LIBYUV_DISABLE_X86');
project.addDefine('LIBYUV_DISABLE_NEON');
project.addDefine('_LIB');
project.addDefine('_YUV_C');
project.addDefine('__THEORA');

return project;
