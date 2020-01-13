#!lua

if os.execute("clang -v") then
	premake.gcc.cc = "clang"
	premake.gcc.cxx = "clang++"
end

solution "TeX_Server"
	location "."
	targetdir "bin"
	configurations { "debug", "release" }
	platforms { "native" }

	project "Server"
		kind "ConsoleApp"
		language "C"
		files { "src/**.h", "src/**.c" }
		includedirs { "./include" , "/usr/include" , "ini_parser"}
		libdirs { "./lib" , "/usr/lib"}
		links { "microhttpd" , "ini_parser" }
		buildoptions {"--std=gnu11", "-Wall"}

		configuration "debug"
			defines { "DEBUG" }
			flags { "Symbols" }

		configuration "release"
			defines { "RELEASE" }
			flags { "Optimize" }
	
	project "ini_parser"
		kind "StaticLib"
		language "C"
		files { "ini_parser/*.c", "ini_parser/*.h"}
		buildoptions {"--std=gnu11", "-Wno-int-conversion"}
