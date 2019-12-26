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
		includedirs { "./include" , "/usr/include" }
		libdirs { "./lib" , "/usr/lib"}
		links { "microhttpd" }
		buildoptions {"--std=c11", "-Wall"}

		configuration "debug"
			defines { "DEBUG" }
			flags { "Symbols" }

		configuration "release"
			defines { "RELEASE" }
			flags { "Optimize" }
