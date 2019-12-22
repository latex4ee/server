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
		language "C++"
		files { "src/**.h", "src/**.cpp" }
		includedirs { "./include" , "./include/spdlog"}
		libdirs { "./lib" }
		--links { "libspdlog" }
		buildoptions {"--std=c++17"}

		configuration "debug"
			defines { "DEBUG" }
			flags { "Symbols" }

		configuration "release"
			defines { "RELEASE" }
			flags { "Optimize" }
